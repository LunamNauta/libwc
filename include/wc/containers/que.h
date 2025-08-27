#ifndef WC_QUE_HEADER
#define WC_QUE_HEADER

#include <stdlib.h>

#define WC_QUE_DEFAULT_CAP 16 // The base number of objects that can be stored
#define WC_QUE_SHIFT_CAP 1024 // The maximum number of bytes before the first element

typedef struct wcque_t{
    void* data;
    size_t front;
    size_t back;
    size_t cap;
    size_t dsiz;
} wcque_t;

int wcque_init(wcque_t* que, size_t dsiz);
void wcque_free(const wcque_t* que);

void* wcque_get_mut(const wcque_t* que, size_t index);
void* wcque_front_mut(const wcque_t* que);
void* wcque_back_mut(const wcque_t* que);
const void* wcque_get(const wcque_t* que, size_t index);
const void* wcque_front(const wcque_t* que);
const void* wcque_back(const wcque_t* que);

size_t wcque_capacity(const wcque_t* que);
size_t wcque_size(const wcque_t* que);
bool wcque_empty(const wcque_t* que);

size_t wcque_wasted(wcque_t* que);
void wcque_unwaste(wcque_t* que);

int wcque_reserve(wcque_t* que, size_t cap);

int wcque_push(wcque_t* que, const void* in);
void wcque_pop(wcque_t* que);

#endif
