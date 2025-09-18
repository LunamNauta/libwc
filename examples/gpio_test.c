#include "wc/io/gpio.h"

#include <stdio.h>

int main(){
    wcgpio_chip_t chip;
    wcgpio_line_t line;

    if (wcgpio_chip_init(&chip, 0) < 0){
        printf("Error: Failed to initialize GPIO-chip 0");
        return -1;
    }
    wcgpio_line_init(&line);
    wcgpio_line_set_offsets(&line, (uint32_t[]){0, 1}, 2);
    wcgpio_line_set_consumer(&line, "solenoid_flipper_gpio");
    wcgpio_line_set_flags(&line, GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_DISABLED);
    wcgpio_line_set_attrs(&line, 
        (uint32_t[]){GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES, GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES},
        (uint64_t[]){0, 1},
        (uint64_t[]){0, 1},
        2
    );
    if (wcgpio_line_request(&chip, &line) < 0){
        printf("Error: Failed to setup GPIO pins\n");
        return -1;
    }

    // ... Do things here

    wcgpio_chip_free(&chip);
}
