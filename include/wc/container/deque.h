#ifndef WC_DEQUE_HEADER
#define WC_DEQUE_HEADER

#include <stdbool.h>
#include <stddef.h>

// TODO: Completely replace this deque with a c++ style deque (with fixed data blocks)

#define WC_DEQUE_SHIFT_CAP 1024

typedef struct wcdeque{
    void* data;
    size_t front;
    size_t back;
    size_t cap;
    size_t dsiz;
} wcdeque_t;

void wcdeque_init(wcdeque_t* que, size_t dsiz);
void wcdeque_free(const wcdeque_t* que);

void* wcdeque_get(const wcdeque_t* que, size_t ind);
void* wcdeque_front(const wcdeque_t* que);
void* wcdeque_back(const wcdeque_t* que);

size_t wcdeque_wasted(const wcdeque_t* que);
size_t wcdeque_capacity(const wcdeque_t* que);
size_t wcdeque_size(const wcdeque_t* que);
bool wcdeque_empty(const wcdeque_t* que);

void wcdeque_unwaste(wcdeque_t* que);
int wcdeque_reserve(wcdeque_t* que, size_t cap);
int wcdeque_resize(wcdeque_t* que, size_t siz);
int wcdeque_shrink_fit(wcdeque_t* que);

void wcdeque_pop_front(wcdeque_t* que);
void wcdeque_pop_back(wcdeque_t* que);
void wcdeque_clear(wcdeque_t* que);

void wcdeque_push_back_rot(wcdeque_t* restrict que, const void* restrict in);
int wcdeque_push_back(wcdeque_t* restrict que, const void* restrict in);
void wcdeque_push_front_rot(wcdeque_t* restrict que, const void* restrict in);
int wcdeque_push_front(wcdeque_t* restrict que, const void* restrict in);

#endif
