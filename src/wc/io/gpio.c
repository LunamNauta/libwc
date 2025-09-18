#include "wc/io/gpio.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <fcntl.h>

#include <linux/gpio.h>
#include <sys/ioctl.h>

//------------------------------------------------------------------------

size_t _wcgpio_integer_strlen(size_t num){
    return num < 10 ? 1 : (size_t)((ceil(log10(num))));
}

//------------------------------------------------------------------------

int wcgpio_chip_init(wcgpio_chip_t* chip, size_t id){
    static const char* PWMCHIP_BASE_PATH = "/dev/gpiochip";
    size_t num_len = _wcgpio_integer_strlen(id);
    chip->path = malloc(strlen(PWMCHIP_BASE_PATH) + num_len + 1);
    if (!chip->path) return -1;
    strcpy(chip->path, PWMCHIP_BASE_PATH);
    sprintf(chip->path + strlen(PWMCHIP_BASE_PATH), "%zu", id);
    int fd = open(chip->path, O_NONBLOCK | O_WRONLY);
    if (fd < 0){
        free(chip->path);
        return -1;
    }
    chip->id = id;
    chip->fd = fd;
    return 0;
}
void wcgpio_chip_free(const wcgpio_chip_t* chip){
    free(chip->path);
}

//------------------------------------------------------------------------

void wcgpio_line_init(wcgpio_line_t* line){
    memset(&line->req, 0, sizeof(struct gpio_v2_line_request));
    memset(&line->conf, 0, sizeof(struct gpio_v2_line_config));
}

int wcgpio_line_set_offsets(wcgpio_line_t* line, uint32_t* offsets, size_t count){
    if (count > GPIO_V2_LINES_MAX) return -1;
    for (size_t a = 0; a < count; a++) line->req.offsets[a] = offsets[a];
    line->req.num_lines = count;
    return 0;
}
int wcgpio_line_set_consumer(wcgpio_line_t* line, const char* consumer){
    if (strlen(consumer)+1 > GPIO_MAX_NAME_SIZE) return -1;
    strcpy(line->req.consumer, consumer);
    return 0;
}
void wcgpio_line_set_ev_buf_size(wcgpio_line_t* line, uint32_t size){
    line->req.event_buffer_size = size;
}

void wcgpio_line_set_flags(wcgpio_line_t* line, uint64_t flags){
    line->conf.flags = flags;
}
int wcgpio_line_set_attrs(wcgpio_line_t* line, uint32_t* ids, uint64_t* attrs, uint64_t* masks, size_t count){
    if (count > GPIO_V2_LINE_NUM_ATTRS_MAX) return -1;
    for (size_t a = 0; a < count; a++){
        line->conf.attrs[a].attr.id = ids[a];
        if (ids[a] == GPIO_V2_LINE_ATTR_ID_DEBOUNCE) line->conf.attrs[a].attr.debounce_period_us = attrs[a];
        else if (ids[a] == GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES) line->conf.attrs[a].attr.values = attrs[a];
        else if (ids[a] == GPIO_V2_LINE_ATTR_ID_FLAGS) line->conf.attrs[a].attr.flags = attrs[a];
        line->conf.attrs[a].mask = masks[a];
    }
    line->conf.num_attrs = count;
    return 0;
}

int wcgpio_line_request(const wcgpio_chip_t* chip, const wcgpio_line_t* line){
    return ioctl(chip->fd, GPIO_V2_GET_LINE_IOCTL, &line->req);
}

int wcgpio_line_set_vals(const wcgpio_line_t* line, const wcgpio_line_vals_t* vals){
    return ioctl(line->req.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals);
}

int wcgpio_line_get_vals(const wcgpio_line_t* line, wcgpio_line_vals_t* out_vals){
    return ioctl(line->req.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &out_vals);
}

//------------------------------------------------------------------------

void wcgpio_line_vals_set(wcgpio_line_vals_t* vals, bool* states, size_t* offsets, size_t count){
    memset(&vals->bits, 0, sizeof(__aligned_u64));
    memset(&vals->mask, 0, sizeof(__aligned_u64));
    for (size_t a = 0; a < count; a++){
        vals->bits |= (((__aligned_u64)states[a]) << offsets[a]);
        vals->mask |= (((__aligned_u64)1) << offsets[a]);
    }
}

int wcgpio_line_vals_test(const wcgpio_line_vals_t* vals, size_t offset){
    if (!(vals->mask & offset)) return -1;
    return (vals->bits & offset) != 0;
}

//------------------------------------------------------------------------

