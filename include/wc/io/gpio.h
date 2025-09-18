#ifndef WC_GPIO_HEADER
#define WC_GPIO_HEADER

#include <stdlib.h>
#include <stdint.h>

#include <linux/gpio.h>

//------------------------------------------------------------------------

typedef struct wcgpio_chip{
    char* path;
    size_t id;
    int fd;
} wcgpio_chip_t;

typedef struct wcgpio_line{
    struct gpio_v2_line_request req;
    struct gpio_v2_line_config conf;
} wcgpio_line_t;

typedef struct gpio_v2_line_values wcgpio_line_vals_t;

//------------------------------------------------------------------------

int wcgpio_chip_init(wcgpio_chip_t* chip, size_t id);
void wcgpio_chip_free(const wcgpio_chip_t* chip);

//------------------------------------------------------------------------

void wcgpio_line_init(wcgpio_line_t* line);

int wcgpio_line_set_offsets(wcgpio_line_t* line, uint32_t* offsets, size_t count);
int wcgpio_line_set_consumer(wcgpio_line_t* line, const char* consumer);
void wcgpio_line_set_ev_buf_size(wcgpio_line_t* line, uint32_t size);

void wcgpio_line_set_flags(wcgpio_line_t* line, uint64_t flags);
int wcgpio_line_set_attrs(wcgpio_line_t* line, uint32_t* ids, uint64_t* attrs, uint64_t* masks, size_t count);

int wcgpio_line_request(const wcgpio_chip_t* chip, const wcgpio_line_t* line);

int wcgpio_line_set_vals(const wcgpio_line_t* line, const wcgpio_line_vals_t* vals);

int wcgpio_line_get_vals(const wcgpio_line_t* line, wcgpio_line_vals_t* out_vals);

//------------------------------------------------------------------------

void wcgpio_line_vals_set(wcgpio_line_vals_t* vals, bool* states, size_t* offsets, size_t count);

int wcgpio_line_vals_test(const wcgpio_line_vals_t* vals, size_t offset);

//------------------------------------------------------------------------

#endif
