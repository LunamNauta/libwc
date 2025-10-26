#ifndef WC_VECTOR_HEADER
#define WC_VECTOR_HEADER

#include <stdbool.h>
#include <stddef.h>

typedef struct wcvector{
    char* data;
    size_t cap;
    size_t siz;
    size_t dsiz;
} wcvector_t;

void wcvector_init(wcvector_t* vec, size_t dsiz);
void wcvector_free(const wcvector_t* vec);

size_t wcvector_capacity(const wcvector_t* vec);
bool wcvector_empty(const wcvector_t* vec);
size_t wcvector_size(const wcvector_t* vec);

void* wcvector_get(const wcvector_t* vec, size_t index);
void* wcvector_front(const wcvector_t* vec);
void* wcvector_back(const wcvector_t* vec);

int wcvector_reserve(wcvector_t* vec, size_t ncap);
int wcvector_resize(wcvector_t* vec, size_t nsiz);
int wcvector_shrink(wcvector_t* vec);

int wcvector_push_back(wcvector_t* vec, const void* val);
int wcvector_insert(wcvector_t* vec, const void* val, size_t index);

void wcvector_pop_back(wcvector_t* vec);
void wcvector_erase(wcvector_t* vec, size_t index);

#endif
