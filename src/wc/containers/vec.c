#include "wc/containers/vec.h"

#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <errno.h>

int wcvec_init_copy(wcvec_t* restrict vec, void* restrict in, size_t siz, size_t dsiz){
    vec->data = NULL;
    vec->cap = 0;
    vec->siz = 0;
    vec->dsiz = dsiz;
    if (wcvec_reserve(vec, siz)) return -1;
    memcpy(vec->data, in, dsiz*siz);
    vec->siz = dsiz;
    return 0;
}
void wcvec_init_take(wcvec_t* restrict vec, void* restrict in, size_t siz, size_t dsiz){
    vec->data = in;
    vec->cap = siz;
    vec->siz = siz;
    vec->dsiz = dsiz;
}
int wcvec_init_reserved(wcvec_t* vec, size_t dsiz, size_t cap){
    wcvec_init(vec, dsiz);
    return wcvec_reserve(vec, cap);
}
void wcvec_init(wcvec_t* vec, size_t dsiz){
    vec->data = NULL;
    vec->cap = 0;
    vec->siz = 0;
    vec->dsiz = dsiz;
}
void* wcvec_free_steal(const wcvec_t* vec){
    return vec->data;
}
void wcvec_free(const wcvec_t* vec){
    free(vec->data);
}

void* wcvec_get(const wcvec_t* vec, size_t ind){
    return (char*)vec->data + vec->dsiz*ind;
}
void* wcvec_front(const wcvec_t* vec){
    return vec->data;
}
void* wcvec_back(const wcvec_t* vec){
    return (char*)vec->data + vec->dsiz*(vec->siz - 1);
}
void* wcvec_data(const wcvec_t* vec){
    return vec->data;
}

size_t wcvec_capacity(const wcvec_t* vec){
    return vec->cap;
}
size_t wcvec_size(const wcvec_t* vec){
    return vec->siz;
}
size_t wcvec_dsiz(const wcvec_t* vec){
    return vec->dsiz;
}
bool wcvec_empty(const wcvec_t* vec){
    return !vec->siz;
}

int wcvec_reserve(wcvec_t* vec, size_t cap){
    if (cap <= vec->cap) return 0;
    void* tmp = realloc(vec->data, vec->dsiz*cap);
    if (!tmp){
        errno = -ENOMEM;
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
    if (!vec->siz){
        free(vec->data);
        vec->data = NULL;
        vec->cap = 0;
        return 0;
    }
    size_t tmp = vec->cap;
    vec->cap = 0;
    if (wcvec_reserve(vec, vec->siz)){
        vec->cap = tmp;
        return -1;
    }
    return 0;
}

void wcvec_remove_range(wcvec_t* vec, size_t beg, size_t len){
    if (len) memmove((char*)vec->data + vec->dsiz*beg, (char*)vec->data + vec->dsiz*(beg + len), vec->dsiz*len);
    vec->siz -= len;
}
void wcvec_erase_fast(wcvec_t* vec, size_t ind){
    if (ind != vec->siz - 1) memcpy((char*)vec->data + vec->dsiz*ind, (char*)vec->data + vec->dsiz*(vec->siz - 1), vec->dsiz);
    vec->siz--;
}
void wcvec_erase(wcvec_t* vec, size_t ind){
    if (ind != vec->siz - 1) memmove((char*)vec->data + vec->dsiz*ind, (char*)vec->data + vec->dsiz*(ind + 1), vec->dsiz*(vec->siz - ind - 1));
    vec->siz--;
}
void wcvec_pop_back(wcvec_t* vec){
    vec->siz--;
}
void wcvec_clear(wcvec_t* vec){
    vec->siz = 0;
}

int wcvec_insert_vals(wcvec_t* restrict vec, size_t ind, const void* restrict in, size_t len){
    if (vec->siz + len > vec->cap){
        size_t cap = vec->cap ? vec->cap*3/2 : 2;
        while (vec->siz + len > cap) cap = cap*3/2;
        if (wcvec_reserve(vec, cap)) return -1;
    }
    if (ind != vec->siz - 1) memmove((char*)vec->data + vec->dsiz*(ind + len), (char*)vec->data + vec->dsiz*ind, vec->dsiz*(vec->siz - ind - 1));
    memcpy((char*)vec->data + vec->dsiz*ind, in, vec->dsiz*len);
    vec->siz += len;
    return 0;
}
int wcvec_insert(wcvec_t* restrict vec, size_t ind, const void* restrict in){
    if (vec->siz + 1 > vec->cap && wcvec_reserve(vec, vec->data ? vec->cap*3/2 : 2)) return -1;
    if (ind != vec->siz - 1) memmove((char*)vec->data + vec->dsiz*(ind + 1), (char*)vec->data + vec->dsiz*ind, vec->dsiz*(vec->siz - ind - 1));
    memcpy((char*)vec->data + vec->dsiz*ind, in, vec->dsiz);
    vec->siz++;
    return 0;
}
int wcvec_push_back_vals(wcvec_t* restrict vec, const void* restrict in, size_t len){
    if (vec->siz + len > vec->cap){
        size_t cap = vec->cap ? vec->cap*3/2 : 2;
        while (vec->siz + len > cap) cap = cap*3/2;
        if (wcvec_reserve(vec, cap)) return -1;
    }
    memcpy((char*)vec->data + vec->dsiz*(vec->siz++), in, vec->dsiz*len);
    return 0;
}
int wcvec_push_back(wcvec_t* restrict vec, const void* restrict in){
    if (vec->siz + 1 > vec->cap && wcvec_reserve(vec, vec->data ? vec->cap*3/2 : 2)) return -1;
    memcpy((char*)vec->data + vec->dsiz*(vec->siz++), in, vec->dsiz);
    return 0;
}
int wcvec_prepend_vals(wcvec_t* restrict vec, const void* restrict in, size_t len){
    if (vec->siz + len > vec->cap){
        size_t cap = vec->cap ? vec->cap*3/2 : 2;
        while (vec->siz + len > cap) cap = cap*3/2;
        if (wcvec_reserve(vec, cap)) return -1;
    }
    memmove((char*)vec->data + vec->dsiz*len, vec->data, vec->dsiz*vec->siz);
    memcpy(vec->data, in, vec->dsiz*len);
    return 0;
}
int wcvec_prepend(wcvec_t* restrict vec, const void* restrict in){
    if (vec->siz + 1 > vec->cap && wcvec_reserve(vec, vec->data ? vec->cap*3/2 : 2)) return -1;
    memmove((char*)vec->data + vec->dsiz, vec->data, vec->dsiz*vec->siz);
    memcpy(vec->data, in, vec->dsiz);
    return 0;
}

size_t wcvec_bsearch(const wcvec_t* restrict vec, const void* restrict val, size_t beg, size_t end, int (*cmp)(const void*, const void*)){
    size_t mid;
    int comp;
    while (beg <= end){
        mid = (beg + end) / 2;
        comp = cmp((char*)vec->data + vec->dsiz*mid, val);
        if (comp == 0) return mid;
        else if (comp < 0) beg = mid + 1;
        else end = mid - 1;
    }
    return vec->siz;
}
int wcvec_copy(const wcvec_t* restrict vec, wcvec_t* restrict out, size_t beg, size_t end, void (*cpy)(void*, const void*)){
    if (wcvec_init_reserved(out, vec->dsiz, vec->cap)) return -1;
    if (!cpy){
        memcpy(out->data, vec->data, vec->dsiz*vec->cap);
        return 0;
    }
    const void* vbeg = (char*)vec->data + vec->dsiz*beg;
    void* obeg = out->data;
    for (size_t a = beg; a < end; a++, vbeg++, obeg++) cpy(obeg, vbeg);
    return 0;
}

size_t _wcvec_sort_partition(wcvec_t* vec, size_t beg, size_t end, void* tmp_buf, int (*cmp)(const void*, const void*)){
    void* pivot_ref = (char*)vec->data + vec->dsiz * (end - 1);
    size_t tmp_pivot = beg;
    void* tmp_pivot_ref;
    void* index_ref;

    for (size_t a = beg; a < end - 1; a++) {
        index_ref = (char*)vec->data + vec->dsiz*a;
        if (cmp(index_ref, pivot_ref) < 0) {
            tmp_pivot_ref = (char*)vec->data + vec->dsiz*tmp_pivot;
            memcpy(tmp_buf, tmp_pivot_ref, vec->dsiz);
            memcpy(tmp_pivot_ref, index_ref, vec->dsiz);
            memcpy(index_ref, tmp_buf, vec->dsiz);
            tmp_pivot++;
        }
    }
    tmp_pivot_ref = (char*)vec->data + vec->dsiz*tmp_pivot;
    memcpy(tmp_buf, tmp_pivot_ref, vec->dsiz);
    memcpy(tmp_pivot_ref, pivot_ref, vec->dsiz);
    memcpy(pivot_ref, tmp_buf, vec->dsiz);
    return tmp_pivot;
}

void _wcvec_sort(wcvec_t* vec, size_t beg, size_t end, void* tmp_buf, int (*cmp)(const void*, const void*)){
    if (beg + 1 >= end) return;
    size_t pivot = _wcvec_sort_partition(vec, beg, end, tmp_buf, cmp);
    wcvec_sort(vec, beg, pivot, cmp);
    wcvec_sort(vec, pivot + 1, end, cmp);
}
void wcvec_sort(wcvec_t* vec, size_t beg, size_t end, int (*cmp)(const void*, const void*)){
    void* tmp_buf = alloca(vec->dsiz);
    return _wcvec_sort(vec, beg, end, tmp_buf, cmp);
}
