#include "wc/io/gpio.h"

#include <stdio.h>

int main(){
    wcgpio_chip_t chip;
    wcgpio_line_req_t req;
    wcgpio_line_cfg_t cfg;
    wcgpio_line_vals_t vals;

    if (wcgpio_chip_init(&chip, 0) < 0){
        printf("Error: Failed to initialize GPIO-chip 0");
        return -1;
    }

    wcgpio_line_cfg_init(&cfg);
    wcgpio_line_cfg_add_attr(
        &cfg,
        WCGPIO_LINE_ATTR_FLAGS,
        WCGPIO_LINE_FLAG_OUTPUT | WCGPIO_LINE_FLAG_BIAS_PULL_DOWN, 
        WCGPIO_PIN_MASK(0, 1)
    );
    wcgpio_line_cfg_add_attr(
        &cfg,
        WCGPIO_LINE_ATTR_OUTPUT_VALUES,
        WCGPIO_PIN_LOW_MASK(0) | WCGPIO_PIN_HIGH_MASK(1),
        WCGPIO_PIN_MASK(0, 1)
    );

    wcgpio_line_req_init(&req);
    wcgpio_line_req_set_offsets(&req, WCGPIO_PIN_MASK(0, 1));
    wcgpio_line_req_set_consumer(&req, "solenoid_flipper_gpio");
    wcgpio_line_req_set_config(&req, &cfg);

    if (wcgpio_line_req_get_line(&req, &chip) < 0){
        printf("Error: Failed to setup GPIO pins\n");
        return -1;
    }

    wcgpio_line_vals_init(&vals);
    wcgpio_line_vals_set_mask(&vals, WCGPIO_PIN_MASK(0, 1));
    if (wcgpio_line_vals_get_request(&vals, &req) < 0){
        printf("Error: Failed to get GPIO pin states\n");
        return -1;
    }

    printf("Pin 0 is %s\n", wcgpio_line_vals_get_val(&vals, WCGPIO_PIN_MASK(0)) == WCGPIO_LINE_FLAG_VAL_ACTIVE ? "active" : "inactive");
    printf("Pin 1 is %s\n", wcgpio_line_vals_get_val(&vals, WCGPIO_PIN_MASK(1)) == WCGPIO_LINE_FLAG_VAL_ACTIVE ? "active" : "inactive");

    wcgpio_line_vals_set_bits(&vals, WCGPIO_PIN_LOW_MASK(1) | WCGPIO_PIN_HIGH_MASK(0));
    if (wcgpio_line_vals_set_request(&vals, &req) < 0){
        printf("Error: Failed to set GPIO pin states\n");
        return -1;
    }

    if (wcgpio_line_vals_get_request(&vals, &req) < 0){
        printf("Error: Failed to get GPIO pin states\n");
        return -1;
    }

    printf("Pin 0 is %s\n", wcgpio_line_vals_get_val(&vals, WCGPIO_PIN_MASK(0)) == WCGPIO_LINE_FLAG_VAL_ACTIVE ? "active" : "inactive");
    printf("Pin 1 is %s\n", wcgpio_line_vals_get_val(&vals, WCGPIO_PIN_MASK(1)) == WCGPIO_LINE_FLAG_VAL_ACTIVE ? "active" : "inactive");

    wcgpio_chip_free(&chip);
}
