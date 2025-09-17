#ifndef WC_QUE_HEADER
#define WC_QUE_HEADER

#include <stdlib.h>

#define WC_QUE_SHIFT_CAP 1024

typedef struct wcque{
    void* data;
    size_t front;
    size_t back;
    size_t cap;
    size_t dsiz;
} wcque_t;

int wcque_init_copy(wcque_t* restrict que, void* restrict in, size_t siz, size_t dsiz);
void wcque_init_take(wcque_t* restrict que, void* restrict in, size_t siz, size_t dsiz);
int wcque_init_reserved(wcque_t* que, size_t dsiz, size_t cap);
void wcque_init(wcque_t* que, size_t dsiz);
void* wcque_free_steal(const wcque_t* restrict que);
void wcque_free(const wcque_t* que);

void* wcque_get(const wcque_t* que, size_t ind);
void* wcque_front(const wcque_t* que);
void* wcque_back(const wcque_t* que);

size_t wcque_wasted(const wcque_t* que);
size_t wcque_capacity(const wcque_t* que);
size_t wcque_size(const wcque_t* que);
size_t wcque_dsiz(const wcque_t* que);
bool wcque_empty(const wcque_t* que);

void wcque_unwaste(wcque_t* que);
int wcque_reserve(wcque_t* que, size_t cap);
int wcque_resize(wcque_t* que, size_t siz);
int wcque_shrink_fit(wcque_t* que);

void wcque_remove_range(wcque_t* que, size_t beg, size_t end);
void wcque_erase_fast(wcque_t* que, size_t ind);
void wcque_erase(wcque_t* que, size_t ind);
void wcque_pop_front(wcque_t* que);
void wcque_pop_back(wcque_t* que);
void wcque_clear(wcque_t* que);

int wcque_insert_vals(wcque_t* restrict que, size_t ind, const void* restrict in, size_t len);
int wcque_insert(wcque_t* restrict que, size_t ind, const void* restrict in);
int wcque_push_back_vals(wcque_t* restrict que, const void* restrict in, size_t len);
void wcque_push_back_rot(wcque_t* restrict que, const void* restrict in);
int wcque_push_back(wcque_t* restrict que, const void* restrict in);
int wcque_push_front_vals(wcque_t* restrict que, const void* restrict in, size_t len);
void wcque_push_front_rot(wcque_t* restrict que, const void* restrict in);
int wcque_push_front(wcque_t* restrict que, const void* restrict in);

size_t wcque_bsearch(const wcque_t* restrict que, const void* restrict val, size_t beg, size_t end, int (*cmp)(const void*, const void*));
int wcque_copy(const wcque_t* restrict que, wcque_t* restrict out, size_t beg, size_t end, void (*cpy)(void*, const void*));

#endif
