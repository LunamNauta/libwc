#include "wc/io/gpio.h"

#include <linux/gpio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

//------------------------------------------------------------------------

size_t _wcgpio_pin_mask(size_t first, ...){
    size_t mask = (((size_t)(1)) << first);
    va_list args;
    size_t index;
    va_start(args, first);
    while ((index = va_arg(args, size_t)) != ((size_t)(-1))) mask |= (((size_t)(1)) << index);
    va_end(args);
    return mask;
}
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
    int fd = open(chip->path, O_RDWR);
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
    close(chip->fd);
}

const char* wcgpio_chip_get_path(const wcgpio_chip_t* chip){
    return chip->path;
}
size_t wcgpio_chip_get_id(const wcgpio_chip_t* chip){
    return chip->id;
}
int wcgpio_chip_get_fd(const wcgpio_chip_t* chip){
    return chip->fd;
}

//------------------------------------------------------------------------

int wcgpio_chip_info_init(wcgpio_chip_info_t* info, const wcgpio_chip_t* chip){
    if (ioctl(chip->fd, GPIO_GET_CHIPINFO_IOCTL, &info->info) < 0) return -1;
    return 0;
}

const char* wcgpio_chip_info_get_name(const wcgpio_chip_info_t* info){
    return info->info.name;
}
const char* wcgpio_chip_info_get_label(const wcgpio_chip_info_t* info){
    return info->info.label;
}
size_t wcgpio_chip_info_get_lines(const wcgpio_chip_info_t* info){
    return info->info.lines;
}

//------------------------------------------------------------------------

int wcgpio_line_info_init(wcgpio_line_info_t* info, size_t offset, const wcgpio_chip_t* chip){
    info->info.offset = offset;
    if (ioctl(chip->fd, GPIO_V2_GET_LINEINFO_IOCTL, &info->info) < 0) return -1;
    return 0;
}

const char* wcgpio_line_info_get_name(const wcgpio_line_info_t* info){
    return info->info.name;
}
const char* wcgpio_line_info_get_consumer(const wcgpio_line_info_t* info){
    return info->info.consumer;
}
size_t wcgpio_line_info_get_offset(const wcgpio_line_info_t* info){
    return info->info.offset;
}
wcgpio_line_flag_dir_t wcgpio_line_info_get_dir(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_FLAG_OUTPUT | WCGPIO_LINE_FLAG_INPUT);
}
wcgpio_line_flag_edge_t wcgpio_line_info_get_edge(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_FLAG_EDGE_RISING | WCGPIO_LINE_FLAG_EDGE_FALLING);
}
wcgpio_line_flag_bias_t wcgpio_line_info_get_bias(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_FLAG_BIAS_PULL_DOWN | WCGPIO_LINE_FLAG_BIAS_PULL_UP | WCGPIO_LINE_FLAG_BIAS_DISABLED);
}
wcgpio_line_flag_drv_t wcgpio_line_info_get_drive(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_FLAG_OPEN_DRAIN | WCGPIO_LINE_FLAG_OPEN_SOURCE);
}
wcgpio_line_flag_clk_t wcgpio_line_info_get_clock(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_FLAG_CLOCK_REALTIME | WCGPIO_LINE_FLAG_CLOCK_HTE);
}
bool wcgpio_line_info_get_used(const wcgpio_line_info_t* info){
    return info->info.flags & WCGPIO_LINE_FLAG_USED;
}
bool wcgpio_line_info_get_active_low(const wcgpio_line_info_t* info){
    return info->info.flags & WCGPIO_LINE_FLAG_ACTIVE_LOW;
}
bool wcgpio_line_info_get_debounced(const wcgpio_line_info_t* info){
    for (size_t a = 0; a < info->info.num_attrs; a++){
        if (info->info.attrs[a].id & GPIO_V2_LINE_ATTR_ID_DEBOUNCE){
            return true;
        }
    }
    return false;
}
size_t wcgpio_line_info_get_debounce_us(const wcgpio_line_info_t* info){
    size_t out = 0;
    for (size_t a = 0; a < info->info.num_attrs; a++){
        if (info->info.attrs[a].id & GPIO_V2_LINE_ATTR_ID_DEBOUNCE){
            out = info->info.attrs[a].debounce_period_us;
        }
    }
    return out;
}

//------------------------------------------------------------------------

void wcgpio_line_cfg_zero(wcgpio_line_cfg_t* cfg){
    memset(cfg, 0, sizeof(wcgpio_line_cfg_t));
}

void wcgpio_line_cfg_set_dir(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_dir_t dir){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_FLAG_OUTPUT | WCGPIO_LINE_FLAG_INPUT));
    cfg->cfg.flags |= dir;
}
void wcgpio_line_cfg_set_edge(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_edge_t edge){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_FLAG_EDGE_RISING | WCGPIO_LINE_FLAG_EDGE_FALLING));
    cfg->cfg.flags |= edge;
}
void wcgpio_line_cfg_set_bias(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_bias_t bias){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_FLAG_BIAS_PULL_DOWN | WCGPIO_LINE_FLAG_BIAS_PULL_UP | WCGPIO_LINE_FLAG_BIAS_DISABLED));
    cfg->cfg.flags |= bias;
}
void wcgpio_line_cfg_set_drive(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_drv_t drv){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_FLAG_OPEN_DRAIN | WCGPIO_LINE_FLAG_OPEN_SOURCE));
    cfg->cfg.flags |= drv;
}
void wcgpio_line_cfg_set_clock(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_clk_t clk){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_FLAG_CLOCK_REALTIME | WCGPIO_LINE_FLAG_CLOCK_HTE));
    cfg->cfg.flags |= clk;
}
void wcgpio_line_cfg_set_used(wcgpio_line_cfg_t* cfg, bool used){
    cfg->cfg.flags ^= (cfg->cfg.flags & WCGPIO_LINE_FLAG_USED);
    cfg->cfg.flags |= (used ? WCGPIO_LINE_FLAG_USED : 0);
}
void wcgpio_line_cfg_set_active_low(wcgpio_line_cfg_t* cfg, bool active_low){
    cfg->cfg.flags ^= (cfg->cfg.flags & WCGPIO_LINE_FLAG_ACTIVE_LOW);
    cfg->cfg.flags |= (active_low ? WCGPIO_LINE_FLAG_ACTIVE_LOW : 0);
}
void wcgpio_line_cfg_add_attr(wcgpio_line_cfg_t* cfg, size_t id, size_t val, size_t mask){
    if (cfg->cfg.num_attrs == GPIO_V2_LINE_NUM_ATTRS_MAX) return; 
    cfg->cfg.attrs[cfg->cfg.num_attrs].mask = mask;
    cfg->cfg.attrs[cfg->cfg.num_attrs].attr.id = id;
    if (id == WCGPIO_LINE_ATTR_DEBOUNCE) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.debounce_period_us = val;
    else if (id == WCGPIO_LINE_ATTR_OUTPUT_VALUES) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.values = val;
    else if (id == WCGPIO_LINE_ATTR_FLAGS) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.flags = val;
    cfg->cfg.num_attrs++;
}

//------------------------------------------------------------------------

void wcgpio_line_req_zero(wcgpio_line_req_t* req){
    memset(req, 0, sizeof(wcgpio_line_req_t));
}
int wcgpio_line_req_init(wcgpio_line_req_t* req, const wcgpio_chip_t* chip){
    if (ioctl(chip->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &req->req) < 0) return -1;
    return 0;
}
void wcgpio_line_req_free(const wcgpio_line_req_t* req){
    close(req->req.fd);
}

void wcgpio_line_req_set_offsets(wcgpio_line_req_t* req, size_t bits){
    for (size_t a = 1, b = 0; a != 0 && b < GPIO_V2_LINES_MAX; a <<= 1, b++){
        if (bits & a) req->req.offsets[req->req.num_lines++] = b;
    }
}
void wcgpio_line_req_set_consumer(wcgpio_line_req_t* req, const char* consumer){
    strncpy(req->req.consumer, consumer, GPIO_MAX_NAME_SIZE);
}
void wcgpio_line_req_set_config(wcgpio_line_req_t* req, const wcgpio_line_cfg_t* cfg){
    req->req.config = cfg->cfg;
}
void wcgpio_line_req_set_event_buf_size(wcgpio_line_req_t* req, size_t size){
    req->req.event_buffer_size = size;
}

//------------------------------------------------------------------------

void wcgpio_line_vals_zero(wcgpio_line_vals_t* vals){
    memset(vals, 0, sizeof(wcgpio_line_vals_t));
}

wcgpio_line_flag_val_t wcgpio_line_vals_get_val(const wcgpio_line_vals_t* vals, size_t bits){
    if ((vals->vals.mask & bits) != bits) return WCGPIO_LINE_FLAG_VAL_ERR;
    if ((vals->vals.bits & bits) == bits) return WCGPIO_LINE_FLAG_VAL_ACTIVE;
    if ((vals->vals.bits & bits) == 0) return WCPGIO_LINE_FLAG_VAL_INACTIVE;
    return WCGPIO_LINE_FLAG_VAL_ERR;
}
void wcgpio_line_vals_set_vals(wcgpio_line_vals_t* vals, size_t bits, size_t mask){
    vals->vals.bits = bits;
    vals->vals.mask = mask;
}
void wcgpio_line_vals_set_mask(wcgpio_line_vals_t* vals, size_t mask){
    vals->vals.mask = mask;
}
void wcgpio_line_vals_set_bits(wcgpio_line_vals_t* vals, size_t bits){
    vals->vals.bits = bits;
}
size_t wcgpio_line_vals_get_mask(wcgpio_line_vals_t* vals){
    return vals->vals.mask;
}
size_t wcgpio_line_vals_get_bits(wcgpio_line_vals_t* vals){
    return vals->vals.bits;
}
int wcgpio_line_vals_get_request(wcgpio_line_vals_t* vals, const wcgpio_line_req_t* req){
    if (ioctl(req->req.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &vals->vals) < 0) return -1;
    return 0;
}
int wcgpio_line_vals_set_request(wcgpio_line_vals_t* vals, const wcgpio_line_req_t* req){
    if (ioctl(req->req.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals->vals) < 0) return -1;
    return 0;
}

//------------------------------------------------------------------------
