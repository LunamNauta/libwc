#ifndef WC_GPIO_HEADER
#define WC_GPIO_HEADER

#include <stdlib.h>

#include <linux/gpio.h>

//------------------------------------------------------------------------

#define WCGPIO_PIN_HIGH_MASK(a, ...) (_wcgpio_pin_mask(a, ##__VA_ARGS__, ((size_t)(-1))))
#define WCGPIO_PIN_MASK(a, ...) (_wcgpio_pin_mask(a, ##__VA_ARGS__, ((size_t)(-1))))
#define WCGPIO_PIN_LOW_MASK(...) ((size_t)(0))

size_t _wcgpio_pin_mask(size_t first, ...);
size_t _wcgpio_integer_strlen(size_t num);

//------------------------------------------------------------------------

typedef enum wcgpio_line_attr{
    WCGPIO_LINE_ATTR_OUTPUT_VALUES = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES,
    WCGPIO_LINE_ATTR_DEBOUNCE = GPIO_V2_LINE_ATTR_ID_DEBOUNCE,
    WCGPIO_LINE_ATTR_FLAGS = GPIO_V2_LINE_ATTR_ID_FLAGS
} wcgpio_line_attr_t;

typedef enum wcgpio_line_flag_val{
    WCPGIO_LINE_FLAG_VAL_INACTIVE = 0,
    WCGPIO_LINE_FLAG_VAL_ACTIVE = 1,
    WCGPIO_LINE_FLAG_VAL_ERR = 2
} wcgpio_line_flag_val_t;

typedef enum wcgpio_line_flag_dir{
    WCGPIO_LINE_FLAG_DIR_AS_IS = 0,
    WCGPIO_LINE_FLAG_OUTPUT = GPIO_V2_LINE_FLAG_OUTPUT,
    WCGPIO_LINE_FLAG_INPUT = GPIO_V2_LINE_FLAG_INPUT
} wcgpio_line_flag_dir_t;

typedef enum wcgpio_line_flag_edge{
    WCGPIO_LINE_FLAG_EDGE_NONE = 0,
    WCGPIO_LINE_FLAG_EDGE_RISING = GPIO_V2_LINE_FLAG_EDGE_RISING,
    WCGPIO_LINE_FLAG_EDGE_FALLING = GPIO_V2_LINE_FLAG_EDGE_FALLING,
    WCGPIO_LINE_FLAG_EDGE_BOTH = GPIO_V2_LINE_FLAG_EDGE_FALLING | GPIO_V2_LINE_FLAG_EDGE_RISING
} wcgpio_line_flag_edge_t;

typedef enum wcgpio_line_flag_bias{
    WCGPIO_LINE_FLAG_BIAS_AS_IS = 0,
    WCGPIO_LINE_FLAG_BIAS_PULL_UP = GPIO_V2_LINE_FLAG_BIAS_PULL_UP,
    WCGPIO_LINE_FLAG_BIAS_PULL_DOWN = GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN,
    WCGPIO_LINE_FLAG_BIAS_DISABLED = GPIO_V2_LINE_FLAG_BIAS_DISABLED
} wcgpio_line_flag_bias_t;

typedef enum wcgpio_line_flag_drv{
    WCGPIO_LINE_FLAG_PUSH_PULL = 0,
    WCGPIO_LINE_FLAG_OPEN_SOURCE = GPIO_V2_LINE_FLAG_OPEN_SOURCE,
    WCGPIO_LINE_FLAG_OPEN_DRAIN = GPIO_V2_LINE_FLAG_OPEN_DRAIN
} wcgpio_line_flag_drv_t;

typedef enum wcgpio_line_flag_clk{
    WCGPIO_LINE_FLAG_CLOCK_MONOTONIC = 0,
    WCGPIO_LINE_FLAG_CLOCK_REALTIME = GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME,
    WCGPIO_LINE_FLAG_CLOCK_HTE = GPIO_V2_LINE_FLAG_EVENT_CLOCK_HTE
} wcgpio_line_flag_clk_t;

#define WCGPIO_LINE_FLAG_ACTIVE_LOW GPIO_V2_LINE_FLAG_ACTIVE_LOW
#define WCGPIO_LINE_FLAG_USED GPIO_V2_LINE_FLAG_USED

//------------------------------------------------------------------------

typedef struct wcgpio_chip{
    char* path;
    size_t id;
    int fd;
} wcgpio_chip_t;

int wcgpio_chip_init(wcgpio_chip_t* chip, size_t id);
void wcgpio_chip_free(const wcgpio_chip_t* chip);

const char* wcgpio_chip_get_path(const wcgpio_chip_t* chip);
size_t wcgpio_chip_get_id(const wcgpio_chip_t* chip);
int wcgpio_chip_get_fd(const wcgpio_chip_t* chip);

//------------------------------------------------------------------------

typedef struct wcgpio_chip_info{
    struct gpiochip_info info;
} wcgpio_chip_info_t;

int wcgpio_chip_info_init(wcgpio_chip_info_t* info, const wcgpio_chip_t* chip);

const char* wcgpio_chip_info_get_name(const wcgpio_chip_info_t* info);
const char* wcgpio_chip_info_get_label(const wcgpio_chip_info_t* info);
size_t wcgpio_chip_info_get_lines(const wcgpio_chip_info_t* info);

//------------------------------------------------------------------------

typedef struct wcgpio_line_info{
    struct gpio_v2_line_info info;
} wcgpio_line_info_t;

int wcgpio_line_info_init(wcgpio_line_info_t* info, size_t offset, const wcgpio_chip_t* chip);

const char* wcgpio_line_info_get_name(const wcgpio_line_info_t* info);
const char* wcgpio_line_info_get_consumer(const wcgpio_line_info_t* info);
size_t wcgpio_line_info_get_offset(const wcgpio_line_info_t* info);
wcgpio_line_flag_dir_t wcgpio_line_info_get_dir(const wcgpio_line_info_t* info);
wcgpio_line_flag_edge_t wcgpio_line_info_get_edge(const wcgpio_line_info_t* info);
wcgpio_line_flag_bias_t wcgpio_line_info_get_bias(const wcgpio_line_info_t* info);
wcgpio_line_flag_drv_t wcgpio_line_info_get_drive(const wcgpio_line_info_t* info);
wcgpio_line_flag_clk_t wcgpio_line_info_get_clock(const wcgpio_line_info_t* info);
bool wcgpio_line_info_get_active_low(const wcgpio_line_info_t* info);
bool wcgpio_line_info_get_debounced(const wcgpio_line_info_t* info);
size_t wcgpio_line_info_get_debounce_us(const wcgpio_line_info_t* info);

//------------------------------------------------------------------------

typedef struct wcgpio_line_cfg{
    struct gpio_v2_line_config cfg;
} wcgpio_line_cfg_t;

void wcgpio_line_cfg_zero(wcgpio_line_cfg_t* cfg);

void wcgpio_line_cfg_set_dir(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_dir_t dir);
void wcgpio_line_cfg_set_edge(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_edge_t edge);
void wcgpio_line_cfg_set_bias(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_bias_t bias);
void wcgpio_line_cfg_set_drive(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_drv_t drv);
void wcgpio_line_cfg_set_clock(wcgpio_line_cfg_t* cfg, wcgpio_line_flag_clk_t clk);
void wcgpio_line_cfg_set_active_low(wcgpio_line_cfg_t* cfg, bool active_low);
void wcgpio_line_cfg_add_attr(wcgpio_line_cfg_t* cfg, size_t id, size_t val, size_t mask);

//------------------------------------------------------------------------

typedef struct wcgpio_line_req{
    struct gpio_v2_line_request req;
} wcgpio_line_req_t;

void wcgpio_line_req_zero(wcgpio_line_req_t* req);
int wcgpio_line_req_init(wcgpio_line_req_t* req, const wcgpio_chip_t* chip);
void wcgpio_line_req_free(const wcgpio_line_req_t* req);

void wcgpio_line_req_set_offsets(wcgpio_line_req_t* req, size_t bits);
void wcgpio_line_req_set_consumer(wcgpio_line_req_t* req, const char* consumer);
void wcgpio_line_req_set_config(wcgpio_line_req_t* req, const wcgpio_line_cfg_t* cfg);
void wcgpio_line_req_set_event_buf_size(wcgpio_line_req_t* req, size_t size);

//------------------------------------------------------------------------

typedef struct wcgpio_line_vals{
    struct gpio_v2_line_values vals;
} wcgpio_line_vals_t;

void wcgpio_line_vals_zero(wcgpio_line_vals_t* vals);

wcgpio_line_flag_val_t wcgpio_line_vals_get_val(const wcgpio_line_vals_t* vals, size_t bits);
void wcgpio_line_vals_set_vals(wcgpio_line_vals_t* vals, size_t bits, size_t mask);
void wcgpio_line_vals_set_mask(wcgpio_line_vals_t* vals, size_t mask);
void wcgpio_line_vals_set_bits(wcgpio_line_vals_t* vals, size_t bits);
size_t wcgpio_line_vals_get_mask(wcgpio_line_vals_t* vals);
size_t wcgpio_line_vals_get_bits(wcgpio_line_vals_t* vals);
int wcgpio_line_vals_get_request(wcgpio_line_vals_t* vals, const wcgpio_line_req_t* req);
int wcgpio_line_vals_set_request(wcgpio_line_vals_t* vals, const wcgpio_line_req_t* req);

//------------------------------------------------------------------------

#endif
