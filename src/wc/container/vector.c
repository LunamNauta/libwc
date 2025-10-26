#include "wc/container/vector.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void wcvector_init(wcvector_t* vec, size_t dsiz){
    memset(vec, 0, sizeof(wcvector_t));
    vec->dsiz = dsiz;
}
void wcvector_free(const wcvector_t* vec){
    free(vec->data);
}

size_t wcvector_capacity(const wcvector_t* vec){
    return vec->cap;
}
bool wcvector_empty(const wcvector_t* vec){
    return !vec->siz;
}
size_t wcvector_size(const wcvector_t* vec){
    return vec->siz;
}

void* wcvector_get(const wcvector_t* vec, size_t index){
    assert(index < vec->siz);
    return vec->data + index*vec->dsiz;
}
void* wcvector_front(const wcvector_t* vec){
    assert(vec->siz != 0);
    return vec->data;
}
void* wcvector_back(const wcvector_t* vec){
    assert(vec->siz != 0);
    return vec->data + (vec->siz - 1)*vec->dsiz;
}

int wcvector_reserve(wcvector_t* vec, size_t ncap){
    if (ncap <= vec->cap && vec->cap) return 0;
    if (ncap < 2) ncap = 2;
    void* tdata = realloc(vec->data, ncap*vec->dsiz);
    if (!tdata) return -1;
    vec->data = tdata;
    vec->cap = ncap;
    return 0;
}
int wcvector_resize(wcvector_t* vec, size_t nsiz){
    if (!nsiz){
        if (!vec->cap) return 0;
        free(vec->data);
        memset(vec, 0, sizeof(wcvector_t));
        return 0;
    }
    if (nsiz > vec->cap && wcvector_reserve(vec, nsiz) < 0) return -1;
    vec->siz = nsiz;
    return 0;
}
int wcvector_shrink(wcvector_t* vec){
    if (vec->siz == vec->cap) return 0;
    if (vec->siz == 0){
        free(vec->data);
        memset(vec, 0, sizeof(wcvector_t));
    }
    size_t tcap = vec->cap;
    vec->cap = 0;
    if (wcvector_reserve(vec, vec->siz) < 0){
        vec->cap = tcap;
        return -1;
    }
    return 0;
}

int wcvector_push_back(wcvector_t* vec, const void* val){
    if (vec->siz + 1 > vec->cap && wcvector_reserve(vec, vec->cap*3/2) < 0) return -1;
    memcpy(vec->data + vec->siz*vec->dsiz, val, vec->dsiz);
    vec->siz++;
    return 0;
}
int wcvector_insert(wcvector_t* vec, const void* val, size_t index){
    assert(index < vec->siz);
    if (vec->siz + 1 > vec->cap && wcvector_reserve(vec, vec->cap*3/2) < 0) return -1;
    memmove(vec->data + (index + 1)*vec->dsiz, vec->data + index*vec->dsiz, (vec->siz - index)*vec->dsiz);
    memcpy(vec->data + index*vec->dsiz, val, vec->dsiz);
    vec->siz++;
    return 0;
}

void wcvector_pop_back(wcvector_t* vec){
    assert(vec->siz != 0);
    vec->siz--;
}
void wcvector_erase(wcvector_t* vec, size_t index){
    assert(index < vec->siz);
    if (index != vec->siz - 1) memmove(vec->data + index*vec->dsiz, vec->data + (index + 1)*vec->dsiz, (vec->siz - index - 1)*vec->dsiz);
    vec->siz--;
}
