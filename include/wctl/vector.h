//#define T wc_self, wc_type
//#define T vec_int, int

/* Includes ---------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "template.h"
#include "linkage.h"
#include "common.h"

/* Sanity checks ---------------------------------------------------------------------------*/

#ifndef T
    #error "Error: Template not provided for vec"
#endif

#if wc_NUMARGS(T) != 2
    #error "Error: Template for vec must have exactly two parameters"
#endif

/* Typedefs ---------------------------------------------------------------------------*/

#undef wc_self
#undef wc_type
#define wc_self wc_GETARG(1, T)
#define wc_type wc_GETARG(2, T)
#define wc_iter wc_MEMB(_iter)
typedef wc_type wc_MEMB(_type);

/* Type Declarations ---------------------------------------------------------------------------*/

typedef struct wc_JOIN(wc_self, _s){
    wc_type* data;
    size_t cap;
    size_t siz;
} wc_self;

typedef struct wc_JOIN(wc_iter, _s){
    wc_type* ref;
    wc_type* end;
} wc_iter;

/* Inlined Function Declarations/Implementations ---------------------------------------------------------------------------*/

WCTL_INLINE const wc_type* wc_MEMB(_get)(wc_self* vec, size_t index){
    return &vec->data[index];
}
WCTL_INLINE const wc_type* wc_MEMB(_front)(wc_self* vec){
    return &vec->data[0];
}
WCTL_INLINE const wc_type* wc_MEMB(_back)(wc_self* vec){
    return &vec->data[vec->siz - 1];
}
WCTL_INLINE wc_type* wc_MEMB(_get_mut)(wc_self* vec, size_t index){
    return &vec->data[index];
}
WCTL_INLINE wc_type* wc_MEMB(_front_mut)(wc_self* vec){
    return &vec->data[0];
}
WCTL_INLINE wc_type* wc_MEMB(_back_mut)(wc_self* vec){
    return &vec->data[vec->siz - 1];
}

WCTL_INLINE size_t wc_MEMB(_capacity)(wc_self* vec){
    return vec->cap;
}
WCTL_INLINE size_t wc_MEMB(_size)(wc_self* vec){
    return vec->siz;
}
WCTL_INLINE bool wc_MEMB(_empty)(wc_self* vec){
    return !vec->siz;
}

WCTL_INLINE wc_iter wc_MEMB(_begin)(const wc_self* vec) {
    wc_iter it = {vec->data, vec->data};
    if (vec->siz) it.end += vec->siz;
    else it.ref = NULL;
    return it;
}
WCTL_INLINE wc_iter wc_MEMB(_rbegin)(const wc_self* vec) {
    wc_iter it = {vec->data, vec->data};
    if (vec->siz){
        it.ref += vec->siz - 1;
        it.end -= 1; 
    }
    else it.ref = NULL;
    return it;
}
WCTL_INLINE wc_iter wc_MEMB(_end)(const wc_self* vec){
    wc_iter it = {0};
    return it; 
}
WCTL_INLINE wc_iter wc_MEMB(_rend)(const wc_self* vec){
    wc_MEMB(_iter) it = {0};
    return it;
}
WCTL_INLINE void wc_MEMB(_iter_next)(wc_iter* it){
    if (++it->ref == it->end) it->ref = NULL;
}
WCTL_INLINE void wc_MEMB(_iter_rnext)(wc_iter* it){
    if (--it->ref == it->end) it->ref = NULL;
}
WCTL_INLINE wc_iter wc_MEMB(_iter_advance)(wc_iter it, size_t n){
    if ((it.ref += n) >= it.end) it.ref = NULL;
    return it;
}
WCTL_INLINE size_t wc_MEMB(_iter_index)(const wc_self* vec, wc_iter it){
    return (it.ref - vec->data);
}
WCTL_INLINE const wc_type* wc_MEMB(_iter_deref)(wc_iter it){
    return it.ref;
}
WCTL_INLINE wc_type* wc_MEMB(_iter_deref_mut)(wc_iter it){
    return it.ref;
}
WCTL_INLINE int wc_MEMB(_iter_cmp)(wc_iter it1, wc_iter it2){
    if (it1.ref < it2.ref) return -1;
    else if (it1.ref > it2.ref) return 1;
    return 0;
}

/* Non-Inlined Function Declarations ---------------------------------------------------------------------------*/

WCTL_API int wc_MEMB(_init)(wc_self* vec, size_t cap);
WCTL_API void wc_MEMB(_free)(wc_self* vec);

WCTL_API int wc_MEMB(_reserve)(wc_self* vec, size_t cap);
WCTL_API int wc_MEMB(_resize)(wc_self* vec, size_t siz);
WCTL_API int wc_MEMB(_shrink_fit)(wc_self* vec);

WCTL_API void wc_MEMB(_erase)(wc_self* vec, size_t index);
WCTL_API void wc_MEMB(_clear)(wc_self* vec);

WCTL_API int wc_MEMB(_insert)(wc_self* vec, size_t index, wc_type in);
WCTL_API int wc_MEMB(_push_back)(wc_self* vec, wc_type in);
WCTL_API void wc_MEMB(_pop_back)(wc_self* vec);

/* Non-Inlined Function Implementations ---------------------------------------------------------------------------*/

WCTL_DEF int wc_MEMB(_init)(wc_self* vec, size_t cap){
    cap = cap ? cap : 1;
    vec->data = NULL;
    vec->cap = 0;
    vec->siz = 0;
    return wc_MEMB(_reserve)(vec, cap);
}
WCTL_DEF void wc_MEMB(_free)(wc_self* vec){
    free(vec->data);
}

WCTL_DEF int wc_MEMB(_reserve)(wc_self* vec, size_t cap){
    if (cap <= vec->cap) return 0;
    wc_type* tmp = realloc(vec->data, sizeof(wc_type)*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    vec->data = tmp;
    vec->cap = cap;
    return 0;
}
WCTL_DEF int wc_MEMB(_resize)(wc_self* vec, size_t siz){
    if (siz > vec->cap && wc_MEMB(_reserve)(vec, siz)) return -1;
    vec->siz = siz;
    return 0;
}
WCTL_DEF int wc_MEMB(_shrink_fit)(wc_self* vec){
    if (wc_MEMB(_reserve)(vec, vec->siz ? vec->siz : 1)) return -1;
    return 0;
}

WCTL_DEF void wc_MEMB(_erase)(wc_self* vec, size_t index){
    for (size_t a = index; a < vec->siz - 1; a++) vec->data[a] = vec->data[a + 1];
    vec->siz--;
}
WCTL_DEF void wc_MEMB(_clear)(wc_self* vec){
    vec->siz = 0;
}

WCTL_DEF int wc_MEMB(_insert)(wc_self* vec, size_t index, wc_type in){
    if (vec->siz + 1 > vec->cap && wc_MEMB(_reserve)(vec, vec->cap*3/2)) return -1;
    for (size_t a = vec->siz; a > index; a--) vec->data[a] = vec->data[a - 1];
    vec->data[index] = in;
    vec->siz++;
    return 0;
}
WCTL_DEF int wc_MEMB(_push_back)(wc_self* vec, wc_type in){
    if (vec->siz + 1 > vec->cap && wc_MEMB(_reserve)(vec, vec->cap*3/2)) return -1;
    vec->data[vec->siz++] = in;
    return 0;
}
WCTL_DEF void wc_MEMB(_pop_back)(wc_self* vec){
    vec->siz--;
}

#undef T
