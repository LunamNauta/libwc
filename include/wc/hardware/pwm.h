#ifndef WC_PWM_HEADER
#define WC_PWM_HEADER

#include <stdbool.h>
#include <stddef.h>

#define WCPWM_CHIP_ROOT_PATH "../pwm/pwmchip" //"/sys/class/pwm/pwmchip"

typedef struct wcpwm_pin{
    size_t id;
    size_t period_val;
    size_t duty_val;
    bool enabled_val;
    bool polarity_val;
    int period_fd;
    int duty_fd;
    int enabled_fd;
    int polarity_fd;
} wcpwm_pin_t;

typedef struct wcpwm_chip{
    char* path;
    wcpwm_pin_t* pins;
    size_t plen;
    size_t pcap;
    size_t npwm;
    int export_fd;
    int unexport_fd;
} wcpwm_chip_t;

int wcpwm_chip_init(wcpwm_chip_t* chip, size_t id);
void wcpwm_chip_free(const wcpwm_chip_t* chip);

int wcpwm_pin_init(wcpwm_chip_t* chip, size_t pin);
int wcpwm_pin_free(wcpwm_chip_t* chip, size_t pin);

int wcpwm_update_period(wcpwm_chip_t* chip, size_t pin);
int wcpwm_update_duty(wcpwm_chip_t* chip, size_t pin);
int wcpwm_update_enabled(wcpwm_chip_t* chip, size_t pin);
int wcpwm_update_polarity(wcpwm_chip_t* chip, size_t pin);

size_t wcpwm_get_period(wcpwm_chip_t* chip, size_t pin);
size_t wcpwm_get_duty(wcpwm_chip_t* chip, size_t pin);
bool wcpwm_get_enabled(wcpwm_chip_t* chip, size_t pin);
bool wcpwm_get_polarity(wcpwm_chip_t* chip, size_t pin);

int wcpwm_set_period(wcpwm_chip_t* chip, size_t pin, size_t period);
int wcpwm_set_duty(wcpwm_chip_t* chip, size_t pin, size_t duty);
int wcpwm_set_enabled(wcpwm_chip_t* chip, size_t pin, bool enabled);
int wcpwm_set_polarity(wcpwm_chip_t* chip, size_t pin, bool inverse);

#endif
