#ifndef WC_GPIO_HEADER
#define WC_GPIO_HEADER

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

#include <linux/gpio.h>
#include <sys/ioctl.h>

//------------------------------------------------------------------------

typedef enum wcgpio_line_val{
    WCPGIO_LINE_VAL_INACTIVE,
    WCGPIO_LINE_VAL_ACTIVE,
    WCGPIO_LINE_VAL_ERR
} wcgpio_line_val_t;

typedef enum wcpgio_line_dir{
    WCGPIO_LINE_DIR_AS_IS = 0,
    WCGPIO_LINE_DIR_OUT = GPIO_V2_LINE_FLAG_OUTPUT,
    WCGPIO_LINE_DIR_IN = GPIO_V2_LINE_FLAG_INPUT
} wcgpio_line_dir_t;

typedef enum wcgpio_line_edge{
    WCGPIO_LINE_EDGE_NONE = 0,
    WCGPIO_LINE_EDGE_RISING = GPIO_V2_LINE_FLAG_EDGE_RISING,
    WCGPIO_LINE_EDGE_FALLING = GPIO_V2_LINE_FLAG_EDGE_FALLING,
    WCGPIO_LINE_EDGE_BOTH = GPIO_V2_LINE_FLAG_EDGE_FALLING | GPIO_V2_LINE_FLAG_EDGE_RISING
} wcgpio_line_edge_t;

typedef enum wcgpio_line_bias{
    WCGPIO_LINE_BIAS_AS_IS = 0,
    WCGPIO_LINE_BIAS_PULL_UP = GPIO_V2_LINE_FLAG_BIAS_PULL_UP,
    WCGPIO_LINE_BIAS_PULL_DOWN = GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN,
    WCGPIO_LINE_BIAS_DISABLED = GPIO_V2_LINE_FLAG_BIAS_DISABLED
} wcgpio_line_bias_t;

typedef enum wcgpio_line_drv{
    WCGPIO_LINE_DRV_PUSH_PULL = 0,
    WCGPIO_LINE_DRV_OPEN_SOURCE = GPIO_V2_LINE_FLAG_OPEN_SOURCE,
    WCGPIO_LINE_DRV_OPEN_DRAIN = GPIO_V2_LINE_FLAG_OPEN_DRAIN
} wcgpio_line_drv_t;

typedef enum wcgpio_line_clk{
    WCGPIO_LINE_CLK_MONOTONIC = 0,
    WCGPIO_LINE_CLK_REALTIME = GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME,
    WCGPIO_LINE_CLK_HTE = GPIO_V2_LINE_FLAG_EVENT_CLOCK_HTE
} wcgpio_line_clk_t;

//------------------------------------------------------------------------

size_t _wcgpio_integer_strlen(size_t num){
    return num < 10 ? 1 : (size_t)((ceil(log10(num))));
}

//------------------------------------------------------------------------

typedef struct wcgpio_chip{
    char* path;
    size_t id;
    int fd;
} wcgpio_chip_t;

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
    close(chip->fd);
}

const char* wcpgio_chip_get_path(const wcgpio_chip_t* chip){
    return chip->path;
}
size_t wcpgio_chip_get_id(const wcgpio_chip_t* chip){
    return chip->id;
}
int wcpgio_chip_get_fd(const wcgpio_chip_t* chip){
    return chip->fd;
}

//------------------------------------------------------------------------

typedef struct wcgpio_chip_info{
    struct gpiochip_info info;
} wcgpio_chip_info_t;

int wcgpio_chip_info_init(wcgpio_chip_info_t* info, const wcgpio_chip_t* chip){
    if (ioctl(chip->fd, GPIO_GET_CHIPINFO_IOCTL, &info->info) < 0) return -1;
    return 0;
}

const char* wcpgio_chip_info_get_name(const wcgpio_chip_info_t* info){
    return info->info.name;
}
const char* wcpgio_chip_info_get_label(const wcgpio_chip_info_t* info){
    return info->info.label;
}
size_t wcpgio_chip_info_get_lines(const wcgpio_chip_info_t* info){
    return info->info.lines;
}

//------------------------------------------------------------------------

typedef struct wcgpio_line_info{
    struct gpio_v2_line_info info;
} wcgpio_line_info_t;

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
wcgpio_line_dir_t wcgpio_line_info_get_dir(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_DIR_OUT | WCGPIO_LINE_DIR_IN);
}
wcgpio_line_edge_t wcgpio_line_info_get_edge(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_EDGE_RISING | WCGPIO_LINE_EDGE_FALLING);
}
wcgpio_line_bias_t wcgpio_line_info_get_bias(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_BIAS_PULL_DOWN | WCGPIO_LINE_BIAS_PULL_UP | WCGPIO_LINE_BIAS_DISABLED);
}
wcgpio_line_drv_t wcgpio_line_info_get_drive(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_DRV_OPEN_DRAIN | WCGPIO_LINE_DRV_OPEN_SOURCE);
}
wcgpio_line_clk_t wcgpio_line_info_get_clock(const wcgpio_line_info_t* info){
    return info->info.flags & (WCGPIO_LINE_CLK_REALTIME | WCGPIO_LINE_CLK_HTE);
}
bool wcgpio_line_info_get_used(const wcgpio_line_info_t* info){
    return info->info.flags & GPIO_V2_LINE_FLAG_USED;
}
bool wcgpio_line_info_get_active_low(const wcgpio_line_info_t* info){
    return info->info.flags & GPIO_V2_LINE_FLAG_ACTIVE_LOW;
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

typedef struct wcgpio_line_cfg{
    struct gpio_v2_line_config cfg;
} wcgpio_line_cfg_t;

void wcgpio_line_cfg_init(wcgpio_line_cfg_t* cfg){
    memset(cfg, 0, sizeof(wcgpio_line_cfg_t));
}

void wcgpio_line_cfg_set_dir(wcgpio_line_cfg_t* cfg, wcgpio_line_dir_t dir){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_DIR_OUT | WCGPIO_LINE_DIR_IN));
    cfg->cfg.flags |= dir;
}
void wcgpio_line_cfg_set_edge(wcgpio_line_cfg_t* cfg, wcgpio_line_edge_t edge){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_EDGE_RISING | WCGPIO_LINE_EDGE_FALLING));
    cfg->cfg.flags |= edge;
}
void wcgpio_line_cfg_set_bias(wcgpio_line_cfg_t* cfg, wcgpio_line_bias_t bias){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_BIAS_PULL_DOWN | WCGPIO_LINE_BIAS_PULL_UP | WCGPIO_LINE_BIAS_DISABLED));
    cfg->cfg.flags |= bias;
}
void wcgpio_line_cfg_set_drive(wcgpio_line_cfg_t* cfg, wcgpio_line_drv_t drv){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_DRV_OPEN_DRAIN | WCGPIO_LINE_DRV_OPEN_SOURCE));
    cfg->cfg.flags |= drv;
}
void wcgpio_line_cfg_set_clock(wcgpio_line_cfg_t* cfg, wcgpio_line_clk_t clk){
    cfg->cfg.flags ^= (cfg->cfg.flags & (WCGPIO_LINE_CLK_REALTIME | WCGPIO_LINE_CLK_HTE));
    cfg->cfg.flags |= clk;
}
void wcgpio_line_cfg_set_used(wcgpio_line_cfg_t* cfg, bool used){
    cfg->cfg.flags ^= (cfg->cfg.flags & GPIO_V2_LINE_FLAG_USED);
    cfg->cfg.flags |= (used ? GPIO_V2_LINE_FLAG_USED : 0);
}
void wcgpio_line_cfg_set_active_low(wcgpio_line_cfg_t* cfg, bool active_low){
    cfg->cfg.flags ^= (cfg->cfg.flags & GPIO_V2_LINE_FLAG_ACTIVE_LOW);
    cfg->cfg.flags |= (active_low ? GPIO_V2_LINE_FLAG_ACTIVE_LOW : 0);
}
void wcgpio_line_cfg_add_attr(wcgpio_line_cfg_t* cfg, size_t id, size_t val, size_t mask){
    cfg->cfg.attrs[cfg->cfg.num_attrs].mask = mask;
    cfg->cfg.attrs[cfg->cfg.num_attrs].attr.id = id;
    if (id == GPIO_V2_LINE_ATTR_ID_DEBOUNCE) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.debounce_period_us = val;
    else if (id == GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.values = val;
    else if (id == GPIO_V2_LINE_ATTR_ID_FLAGS) cfg->cfg.attrs[cfg->cfg.num_attrs].attr.flags = val;
    cfg->cfg.num_attrs++;
}

//------------------------------------------------------------------------

typedef struct wcgpio_line_req{
    struct gpio_v2_line_request req;
} wcgpio_line_req_t;

void wcgpio_line_req_init(wcgpio_line_req_t* req){
    memset(req, 0, sizeof(wcgpio_line_req_t));
}

void wcgpio_line_req_set_offsets(wcgpio_line_req_t* req, size_t* offsets, size_t count){
    for (size_t a = 0; a < count; a++) req->req.offsets[a] = offsets[a];
    req->req.num_lines = count;
}
void wcgpio_line_req_set_consumer(wcgpio_line_req_t* req, const char* consumer){
    strcpy(req->req.consumer, consumer);
}
void wcgpio_line_req_set_config(wcgpio_line_req_t* req, const wcgpio_line_cfg_t* cfg){
    req->req.config = cfg->cfg;
}
void wcgpio_set_event_buf_size(wcgpio_line_req_t* req, size_t size){
    req->req.event_buffer_size = size;
}

//------------------------------------------------------------------------

typedef struct wcgpio_line_vals{
    struct gpio_v2_line_values vals;
} wcgpio_line_vals_t;

void wcgpio_line_vals_init(wcgpio_line_vals_t* vals){
    memset(vals, 0, sizeof(wcgpio_line_vals_t));
}

wcgpio_line_val_t wcgpio_line_vals_get(const wcgpio_line_vals_t* vals, size_t offset){
    if (!(vals->vals.mask & ((typeof(vals->vals.mask))(1) << offset))) return WCGPIO_LINE_VAL_ERR;
    return (vals->vals.bits & ((typeof(vals->vals.bits))(1) << offset)) ? WCGPIO_LINE_VAL_ACTIVE : WCPGIO_LINE_VAL_INACTIVE;
}
void wcgpio_line_vals_set(wcgpio_line_vals_t* vals, size_t offset, wcgpio_line_val_t val){
    vals->vals.mask |= ((typeof(vals->vals.mask))(1) << offset);
    vals->vals.bits ^= vals->vals.bits;
    vals->vals.bits |= ((typeof(vals->vals.bits))(val) << offset);
}

//------------------------------------------------------------------------

#endif
