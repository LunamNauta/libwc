#include "wc/io/pwm.h"

#include <stdio.h>

int main(){
    wcpwm_chip_t chip;
    wcpwm_pin_t pin;

    if (wcpwm_chip_init(&chip, 0) < 0){
        printf("Error: Failed to initialize PWM chip 0\n");
        return -1;
    }

    if (wcpwm_pin_init(&pin, &chip, 0) < 0){
        printf("Error: Failed to initialize PWM pin 0 on PWM chip 0\n");
        wcpwm_chip_free(&chip);
        return -1;
    }
    if (wcpwm_pin_enable(&pin) < 0){
        printf("Error: Failed to enable PWM pin 0 on PWM chip 0\n");
        wcpwm_chip_free(&chip);
        wcpwm_pin_free(&pin);
        return -1;
    }

    printf("%s\n", chip.path);
    printf("%s\n", pin.path);

    wcpwm_chip_free(&chip);
    wcpwm_pin_free(&pin);
}
