#ifndef WC_VEC_HEADER
#define WC_VEC_HEADER

#include <stdlib.h>
#include <string.h>

typedef struct wcvec_t{
    void* data;
    size_t cap;
    size_t siz;
    size_t dsiz;
} wcvec_t;

int wcvec_init_buf(wcvec_t* vec, void* buf, size_t dsiz, size_t siz);
void wcvec_init(wcvec_t* vec, size_t dsiz);
void wcvec_free(const wcvec_t* vec);

void* wcvec_get_mut(const wcvec_t* vec, size_t index);
void* wcvec_front_mut(const wcvec_t* vec);
void* wcvec_back_mut(const wcvec_t* vec);
void* wcvec_data_mut(const wcvec_t* vec);
const void* wcvec_get(const wcvec_t* vec, size_t index);
const void* wcvec_front(const wcvec_t* vec);
const void* wcvec_back(const wcvec_t* vec);
const void* wcvec_data(const wcvec_t* vec);

size_t wcvec_capacity(const wcvec_t* vec);
size_t wcvec_size(const wcvec_t* vec);
bool wcvec_empty(const wcvec_t* vec);

int wcvec_reserve(wcvec_t* vec, size_t cap);
int wcvec_resize(wcvec_t* vec, size_t siz);
int wcvec_shrink_fit(wcvec_t* vec);

void wcvec_erase(wcvec_t* vec, size_t index);
void wcvec_clear(wcvec_t* vec);

int wcvec_insert(wcvec_t* vec, size_t index, const void* in);
int wcvec_push_back(wcvec_t* vec, const void* in);
void wcvec_pop_back(wcvec_t* vec);

#endif
