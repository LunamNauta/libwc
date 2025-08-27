#include "wc/containers/vec.h"

#include <string.h>
#include <errno.h>

void wcvec_init(wcvec_t* vec, size_t dsiz){
    vec->data = NULL;
    vec->cap = 0;
    vec->siz = 0;
    vec->dsiz = dsiz;
}
void wcvec_free(const wcvec_t* vec){
    free(vec->data);
}

const void* wcvec_get(const wcvec_t* vec, size_t index){
    return (char*)vec->data + vec->dsiz*index;
}
const void* wcvec_front(const wcvec_t* vec){
    return vec->data;
}
const void* wcvec_back(const wcvec_t* vec){
    return (char*)vec->data + vec->dsiz*(vec->siz - 1);
}
void* wcvec_get_mut(const wcvec_t* vec, size_t index){
    return (char*)vec->data + vec->dsiz*index;
}
void* wcvec_front_mut(const wcvec_t* vec){
    return vec->data;
}
void* wcvec_back_mut(const wcvec_t* vec){
    return (char*)vec->data + vec->dsiz*(vec->siz - 1);
}

size_t wcvec_capacity(const wcvec_t* vec){
    return vec->cap;
}
size_t wcvec_size(const wcvec_t* vec){
    return vec->siz;
}
bool wcvec_empty(const wcvec_t* vec){
    return !vec->siz;
}

int wcvec_reserve(wcvec_t* vec, size_t cap){
    if (cap <= vec->cap) return 0;
    void* tmp = realloc(vec->data, vec->dsiz*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    vec->data = tmp;
    vec->cap = cap;
    return 0;
}
int wcvec_resize(wcvec_t* vec, size_t siz){
    if (siz > vec->cap && wcvec_reserve(vec, siz)) return -1;
    vec->siz = siz;
    return 0;
}
int wcvec_shrink_fit(wcvec_t* vec){
    size_t tmp = vec->cap;
    vec->cap = 0;
    if (wcvec_reserve(vec, vec->siz ? vec->siz : 1)){
        vec->cap = tmp;
        return -1;
    }
    return 0;
}

void wcvec_erase(wcvec_t* vec, size_t index){
    for (size_t a = index; a < vec->siz - 1; a++){
        memcpy((char*)vec->data + vec->dsiz*a, (char*)vec->data + vec->dsiz*(a + 1), vec->dsiz);
    }
    vec->siz--;
}
void wcvec_clear(wcvec_t* vec){
    vec->siz = 0;
}

int wcvec_insert(wcvec_t* vec, size_t index, const void* in){
    if (vec->siz + 1 > vec->cap && wcvec_reserve(vec, vec->data ? vec->cap*3/2 : 2)) return -1;
    for (size_t a = vec->siz; a > index; a--){
        memcpy((char*)vec->data + vec->dsiz*a, (char*)vec->data + vec->dsiz*(a - 1), vec->dsiz);
    }
    memcpy((char*)vec->data + vec->dsiz*index, in, vec->dsiz);
    vec->siz++;
    return 0;
}
int wcvec_push_back(wcvec_t* vec, const void* in){
    if (vec->siz + 1 > vec->cap && wcvec_reserve(vec, vec->data ? vec->cap*3/2 : 2)) return -1;
    memcpy((char*)vec->data + vec->dsiz*(vec->siz++), in, vec->dsiz);
    return 0;
}
void wcvec_pop_back(wcvec_t* vec){
    vec->siz--;
}
