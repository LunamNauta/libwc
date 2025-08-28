#ifndef WC_VEC_HEADER
#define WC_VEC_HEADER

#include <stdbool.h>
#include <stdlib.h>

typedef struct wcvec_t{
    void* data;
    size_t cap;
    size_t siz;
    size_t dsiz;
} wcvec_t;

void wcvec_init_take(wcvec_t* vec, void* in, size_t siz, size_t dsiz);
int wcvec_init_reserved(wcvec_t* vec, size_t dsiz, size_t cap);
void wcvec_init(wcvec_t* vec, size_t dsiz);
void wcvec_free(const wcvec_t* vec);
void* wcvec_steal(const wcvec_t* vec);

void* wcvec_get(const wcvec_t* vec, size_t ind);
void* wcvec_front(const wcvec_t* vec);
void* wcvec_back(const wcvec_t* vec);
void* wcvec_data(const wcvec_t* vec);

size_t wcvec_capacity(const wcvec_t* vec);
size_t wcvec_size(const wcvec_t* vec);
size_t wcvec_dsiz(const wcvec_t* vec);
bool wcvec_empty(const wcvec_t* vec);

int wcvec_reserve(wcvec_t* vec, size_t cap);
int wcvec_resize(wcvec_t* vec, size_t siz);
int wcvec_shrink_fit(wcvec_t* vec);

void wcvec_remove_range(wcvec_t* vec, size_t beg, size_t end);
void wcvec_erase_fast(wcvec_t* vec, size_t ind);
void wcvec_erase(wcvec_t* vec, size_t ind);
void wcvec_pop_back(wcvec_t* vec);
void wcvec_clear(wcvec_t* vec);

int wcvec_insert_vals(wcvec_t* restrict vec, size_t ind, const void* restrict in, size_t len);
int wcvec_insert(wcvec_t* restrict vec, size_t ind, const void* restrict in);
int wcvec_push_back_vals(wcvec_t* restrict vec, const void* restrict in, size_t len);
int wcvec_push_back(wcvec_t* restrict vec, const void* restrict in);
int wcvec_prepend_vals(wcvec_t* restrict vec, const void* restrict in, size_t len);
int wcvec_prepend(wcvec_t* restrict vec, const void* restrict in);

size_t wcvec_bsearch(const wcvec_t* vec, const void* val, size_t beg, size_t end, int (*cmp)(const void*, const void*));
int wcvec_copy(const wcvec_t* vec, wcvec_t* out, size_t beg, size_t end, void (*cpy)(void*, const void*));
void wcvec_sort(wcvec_t* vec, size_t beg, size_t end, int (*cmp)(const void*, const void*));

#endif
