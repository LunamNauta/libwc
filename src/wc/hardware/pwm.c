#include "wc/hardware/pwm.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

size_t _wcpwm_integer_strlen(size_t num){
    return num < 10 ? 1 : (size_t)((ceil(log10(num))));
}

int _wcpwm_read_str(wcpwm_chip_t* chip, int fd, char* str, size_t max){
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) < 0) return -1;
    size_t slen = fd_stat.st_size;
    read(fd, str, slen < max ? slen : max);
    return 0;
}
int _wcpwm_read_num(wcpwm_chip_t* chip, int fd, size_t* num){
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) < 0) return -1;
    size_t slen = fd_stat.st_size;
    if (slen > chip->pcap - chip->plen){
        char* tpath = realloc(chip->path, chip->pcap + slen + 1);
        if (!tpath) return -1;
        chip->path = tpath;
        chip->pcap += slen + 1;
    }
    read(fd, chip->path + chip->plen, slen);
    *num = strtoull(chip->path + chip->plen, NULL, 10);
    chip->path[chip->plen] = 0;
    return 0;
}

int _wcpwm_write_str(wcpwm_chip_t* chip, int fd, const char* str){
    size_t slen = strlen(str);
    if (slen > chip->pcap - chip->plen){
        char* tpath = realloc(chip->path, chip->pcap + slen + 1);
        if (!tpath) return -1;
        chip->path = tpath;
        chip->pcap += slen + 1;
    }
    sprintf(chip->path + chip->plen, "%s\n", str);
    ftruncate(fd, 0);
    write(fd, chip->path + chip->plen, slen + 1);
    chip->path[chip->plen] = 0;
    return 0;
}
int _wcpwm_write_num(wcpwm_chip_t* chip, int fd, size_t val){
    size_t slen = _wcpwm_integer_strlen(val);
    if (slen > chip->pcap - chip->plen){
        char* tpath = realloc(chip->path, chip->pcap + slen + 1);
        if (!tpath) return -1;
        chip->path = tpath;
        chip->pcap += slen + 1;
    }
    sprintf(chip->path + chip->plen, "%zu\n", val);
    ftruncate(fd, 0);
    write(fd, chip->path + chip->plen, slen + 1);
    chip->path[chip->plen] = 0;
    return 0;
}

int wcpwm_chip_init(wcpwm_chip_t* chip, size_t id){
    static const size_t rplen = strlen(WCPWM_CHIP_ROOT_PATH);
    chip->path = malloc(rplen + _wcpwm_integer_strlen(id) + strlen("/unexport") + 1);
    if (!chip->path) return -1;
    chip->pcap = rplen + _wcpwm_integer_strlen(id) + strlen("/unexport");
    strcpy(chip->path, WCPWM_CHIP_ROOT_PATH);
    sprintf(chip->path + rplen, "%zu/", id);
    chip->plen = rplen + _wcpwm_integer_strlen(id);

    sprintf(chip->path + rplen + _wcpwm_integer_strlen(id), "/npwm");
    int fd = open(chip->path, O_RDONLY);
    if (fd < 0){
        free(chip->path);
        return -1;
    }
    struct stat fd_stat;
    if (stat(chip->path, &fd_stat) < 0){
        free(chip->path);
        close(fd);
        return -1;
    }
    size_t npwm_siz = fd_stat.st_size;
    char* npwm_str = malloc(npwm_siz);
    read(fd, npwm_str, npwm_siz);
    size_t npwm = strtoull(npwm_str, NULL, 10);
    free(npwm_str);
    close(fd);
    if (!npwm){
        chip->pins = NULL;
        chip->path[chip->plen] = 0;
        chip->pcap = _wcpwm_integer_strlen(id) + strlen("/npwm");
        chip->npwm = 0;
        chip->export_fd = -1;
        chip->unexport_fd = -1;
        return 0;
    }
    chip->npwm = npwm;
    sprintf(chip->path + chip->plen, "/export");
    chip->export_fd = open(chip->path, O_RDWR | O_TRUNC | O_NONBLOCK);
    sprintf(chip->path + chip->plen, "/unexport");
    chip->unexport_fd = open(chip->path, O_RDWR | O_TRUNC | O_NONBLOCK);

    chip->pins = malloc(npwm*sizeof(wcpwm_pin_t));
    if (!chip->pins) return -1;
    char* tpath = realloc(chip->path, rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1) + strlen("/duty_cycle") + 1);
    if (!tpath){
        free(chip->path);
        return -1;
    }
    chip->pcap = rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1) + strlen("/duty_cycle") + 1;
    chip->path = tpath;
    sprintf(chip->path + rplen + _wcpwm_integer_strlen(id), "/pwm");
    for (size_t a = 0; a < npwm; a++){
        chip->pins[a].id = a;
        sprintf(chip->path + rplen + _wcpwm_integer_strlen(id) + strlen("/pwm"), "%zu", a);

        struct stat pwm_stat;
        if (stat(chip->path, &pwm_stat) < 0){
            chip->pins[a].period_fd = -1;
            chip->pins[a].duty_fd = -1;
            chip->pins[a].enabled_fd = -1;
            chip->pins[a].polarity_fd = -1;
            continue;
        }
        else if (!S_ISDIR(pwm_stat.st_mode)) return -1;

        sprintf(chip->path + rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1), "/period");
        chip->pins[a].period_fd = open(chip->path, O_RDWR | O_NONBLOCK);
        if (!chip->pins[a].period_fd) return -1;
        sprintf(chip->path + rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1), "/duty_cycle");
        chip->pins[a].duty_fd = open(chip->path, O_RDWR | O_NONBLOCK);
        if (!chip->pins[a].duty_fd) return -1;
        sprintf(chip->path + rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1), "/enabled");
        chip->pins[a].enabled_fd = open(chip->path, O_RDWR | O_NONBLOCK);
        if (!chip->pins[a].enabled_fd) return -1;
        sprintf(chip->path + rplen + _wcpwm_integer_strlen(id) + strlen("/pwm") + _wcpwm_integer_strlen(npwm - 1), "/polarity");
        chip->pins[a].polarity_fd = open(chip->path, O_RDWR | O_NONBLOCK);
        if (!chip->pins[a].polarity_fd) return -1;

        wcpwm_update_period(chip, a);
        wcpwm_update_duty(chip, a);
        wcpwm_update_enabled(chip, a);
        wcpwm_update_polarity(chip, a);
    }

    chip->path[chip->plen] = 0;
    return 0;
}
void wcpwm_chip_free(const wcpwm_chip_t* chip){
    free(chip->path);
    close(chip->export_fd);
    close(chip->unexport_fd);
    for (size_t a = 0; a < chip->npwm; a++){
        close(chip->pins[a].period_fd);
        close(chip->pins[a].duty_fd);
        close(chip->pins[a].enabled_fd);
        close(chip->pins[a].polarity_fd);
    }
    free(chip->pins);
}

int wcpwm_pin_init(wcpwm_chip_t* chip, size_t pin){
    return _wcpwm_write_num(chip, chip->export_fd, pin);
}
int wcpwm_pin_free(wcpwm_chip_t* chip, size_t pin){
    return _wcpwm_write_num(chip, chip->unexport_fd, pin);
}

int wcpwm_update_period(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    if (_wcpwm_read_num(chip, chip->pins[pin].period_fd, &chip->pins[pin].period_val) < 0) return -1;
    return 0;
}
int wcpwm_update_duty(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    if (_wcpwm_read_num(chip, chip->pins[pin].duty_fd, &chip->pins[pin].duty_val) < 0) return -1;
    return 0;
}
int wcpwm_update_enabled(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    size_t tval;
    if (_wcpwm_read_num(chip, chip->pins[pin].enabled_fd, &tval) < 0) return -1;
    chip->pins[pin].enabled_val = tval;
    return 0;
}
int wcpwm_update_polarity(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    char tstr[10];
    if (_wcpwm_read_str(chip, chip->pins[pin].polarity_fd, tstr, 10) < 0) return -1;
    chip->pins[pin].polarity_val = strncmp(tstr, "inversed", 8) == 0;
    return 0;
}

size_t wcpwm_get_period(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    return chip->pins[pin].period_val;
}
size_t wcpwm_get_duty(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    return chip->pins[pin].duty_val;
}
bool wcpwm_get_enabled(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    return chip->pins[pin].enabled_val;
}
bool wcpwm_get_polarity(wcpwm_chip_t* chip, size_t pin){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    return chip->pins[pin].polarity_val;
}

int wcpwm_set_period(wcpwm_chip_t* chip, size_t pin, size_t period){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    if (_wcpwm_write_num(chip, chip->pins[pin].period_fd, period) < 0) return -1;
    chip->pins[pin].period_val = period;
    return 0;
}
int wcpwm_set_duty(wcpwm_chip_t* chip, size_t pin, size_t duty){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    if (_wcpwm_write_num(chip, chip->pins[pin].duty_fd, duty) < 0) return -1;
    chip->pins[pin].duty_val = duty;
    return 0;
}
int wcpwm_set_enabled(wcpwm_chip_t* chip, size_t pin, bool enabled){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    if (_wcpwm_write_num(chip, chip->pins[pin].enabled_fd, enabled) < 0) return -1;
    chip->pins[pin].enabled_val = enabled;
    return 0;
}
int wcpwm_set_polarity(wcpwm_chip_t* chip, size_t pin, bool inverse){
    assert(pin < chip->npwm && chip->pins[pin].enabled_fd >= 0);
    const char* tstr = inverse ? "inversed" : "normal";
    if (_wcpwm_write_str(chip, chip->pins[pin].polarity_fd, tstr) < 0) return -1;
    chip->pins[pin].polarity_val = inverse;
    return 0;
}
