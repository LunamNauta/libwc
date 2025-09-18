#include "wc/io/pwm.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

//------------------------------------------------------------------------

size_t _wcpwm_integer_strlen(size_t num){
    return num < 10 ? 1 : (size_t)((ceil(log10(num))));
}

int wcpwm_chip_init(wcpwm_chip_t* chip, size_t id){
    static const char* PWMCHIP_BASE_PATH = "/sys/class/pwm/pwmchip";
    size_t num_len = _wcpwm_integer_strlen(id);
    chip->path = malloc(strlen(PWMCHIP_BASE_PATH) + num_len + 1);
    if (!chip->path) return -1;
    strcpy(chip->path, PWMCHIP_BASE_PATH);
    sprintf(chip->path + strlen(PWMCHIP_BASE_PATH), "%zu", id);
    chip->id = id;
    return 0;
}
void wcpwm_chip_free(const wcpwm_chip_t* chip){
    free(chip->path);
}

int wcpwm_pin_init(wcpwm_pin_t* pin, const wcpwm_chip_t* chip, size_t id){
    size_t extra_len = strlen("/pwm") + _wcpwm_integer_strlen(id) + strlen("duty_cycle");
    char* path = malloc(strlen(chip->path) + extra_len + 1);
    if (!path) return -1;
    strcpy(path, chip->path);
    strcpy(path + strlen(chip->path), "/export");
    int fd = open(path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        free(path);
        return -1;
    }
    sprintf(path, "%zu", id);
    write(fd, path, strlen(path));
    close(fd);
    strcpy(path, chip->path);
    strcpy(path + strlen(chip->path), "/pwm");
    sprintf(path + strlen(chip->path) + strlen("/pwm"), "%zu", id);
    fd = open(path, O_NONBLOCK);
    if (fd < 0){
        free(path);
        return -1;
    }
    pin->chip = chip;
    pin->path = path;
    pin->id = id;
    pin->fd = fd;
    return 0;
}
int wcpwm_pin_free(const wcpwm_pin_t* pin){
    char* path = malloc(strlen(pin->chip->path) + strlen("/unexport") + 1);
    if (!path) return -1;
    strcpy(path, pin->chip->path);
    strcpy(path + strlen(pin->chip->path), "/unexport");
    int fd = open(path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        free(path);
        return -1;
    }
    sprintf(path, "%zu", pin->id);
    write(fd, path, strlen(path));
    close(fd);
    free(pin->path);
    close(pin->fd);
    free(path);
    return 0;
}

int wcpwm_pin_enable(wcpwm_pin_t* pin){
    size_t path_len = strlen(pin->path);
    strcpy(pin->path + path_len, "/enable");
    int fd = open(pin->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        pin->path[path_len] = 0;
        return -1;
    }
    write(fd, "1", strlen("1"));
    close(fd);
    pin->path[path_len] = 0;
    return 0;
}
int wcpwm_pin_disable(wcpwm_pin_t* pin){
    size_t path_len = strlen(pin->path);
    strcpy(pin->path + path_len, "/enable");
    int fd = open(pin->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        pin->path[path_len] = 0;
        return -1;
    }
    write(fd, "0", strlen("1"));
    close(fd);
    pin->path[path_len] = 0;
    return 0;
}

int wcpwm_pin_set_period(wcpwm_pin_t* pin, size_t period){
    size_t text_len = _wcpwm_integer_strlen(period);
    char* text = malloc(text_len + 1);
    if (!text) return -1;
    sprintf(text, "%zu", period);
    size_t path_len = strlen(pin->path);
    strcpy(pin->path + path_len, "/period");
    int fd = open(pin->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        pin->path[path_len] = 0;
        return -1;
    }
    write(fd, text, text_len);
    close(fd);
    pin->path[path_len] = 0;
    return 0;
}
int wcpwm_pin_set_duty(wcpwm_pin_t* pin, size_t duty){
    size_t text_len = _wcpwm_integer_strlen(duty);
    char* text = malloc(text_len + 1);
    if (!text) return -1;
    sprintf(text, "%zu", duty);
    size_t path_len = strlen(pin->path);
    strcpy(pin->path + path_len, "/duty");
    int fd = open(pin->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        pin->path[path_len] = 0;
        return -1;
    }
    write(fd, text, text_len);
    close(fd);
    pin->path[path_len] = 0;
    return 0;
}
int wcpwm_pin_set_polarity(wcpwm_pin_t* pin, bool polarity){
    size_t text_len = strlen(polarity ? "normal" : "inversed");
    char* text = malloc(text_len + 1);
    if (!text) return -1;
    strcpy(text, polarity ? "normal" : "inversed");
    size_t path_len = strlen(pin->path);
    strcpy(pin->path + path_len, "/polarity");
    int fd = open(pin->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        pin->path[path_len] = 0;
        return -1;
    }
    write(fd, text, text_len);
    close(fd);
    pin->path[path_len] = 0;
    return 0;
}

//------------------------------------------------------------------------

