#ifndef WC_PWM_HEADER
#define WC_PWM_HEADER

#include <stdlib.h>

//------------------------------------------------------------------------

typedef struct wcpwm_chip{
    char* path;
    size_t id;
} wcpwm_chip_t;

typedef struct wcpwm_pin{
    const wcpwm_chip_t* chip;
    char* path;
    size_t id;
    int fd;
} wcpwm_pin_t;

//------------------------------------------------------------------------

int wcpwm_chip_init(wcpwm_chip_t* chip, size_t id);
void wcpwm_chip_free(const wcpwm_chip_t* chip);

int wcpwm_pin_init(wcpwm_pin_t* pin, const wcpwm_chip_t* chip, size_t id);
int wcpwm_pin_free(const wcpwm_pin_t* pin);

int wcpwm_pin_enable(wcpwm_pin_t* pin);
int wcpwm_pin_disable(wcpwm_pin_t* pin);

// Period in nanoseconds
int wcpwm_pin_set_period(wcpwm_pin_t* pin, size_t period);
// Duty in nanoseconds
int wcpwm_pin_set_duty(wcpwm_pin_t* pin, size_t duty);
// True is normal, false is inversed
int wcpwm_pin_set_polarity(wcpwm_pin_t* pin, bool polarity);

//------------------------------------------------------------------------

#endif
