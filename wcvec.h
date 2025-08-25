/*
#define T wcvec_int_t int
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#ifndef T
#error "Template paramater not provided for wcvec"
#endif

#include "wctemplate.h"
#define Self GET_SPLIT(T, 1)
#define VAL_TYPE GET_SPLIT(T, 2)

/* Declaration ---------------------------------------------------------------------------*/

typedef VAL_TYPE MEMB(VAL_TYPE);
typedef struct Self{
    VAL_TYPE* data;
    size_t cap;
    size_t siz;
} Self;

static inline int MEMB(init)(Self* vec, size_t cap);
static inline void MEMB(free)(Self* vec);

static inline const VAL_TYPE* MEMB(get)(Self* vec, size_t index);
static inline const VAL_TYPE* MEMB(front)(Self* vec);
static inline const VAL_TYPE* MEMB(back)(Self* vec);
static inline const VAL_TYPE* MEMB(data)(Self* vec);

static inline size_t MEMB(capacity)(Self* vec);
static inline size_t MEMB(size)(Self* vec);
static inline bool MEMB(empty)(Self* vec);

static inline int MEMB(reserve)(Self* vec, size_t cap);
static inline int MEMB(resize)(Self* vec, size_t siz);
static inline int MEMB(shrink_fit)(Self* vec);

static inline void MEMB(erase)(Self* vec, size_t index);
static inline void MEMB(clear)(Self* vec);

static inline int MEMB(insert)(Self* vec, size_t index, VAL_TYPE in);
static inline int MEMB(push_back)(Self* vec, VAL_TYPE in);
static inline void MEMB(pop_back)(Self* vec);

/* Implementation ------------------------------------------------------------------------*/

static int MEMB(init)(Self* vec, size_t cap){
    vec->data = NULL;
    vec->cap = 0;
    vec->siz = 0;
    return MEMB(reserve)(vec, cap);
}
static void MEMB(free)(Self* vec){
    free(vec->data);
}

static const VAL_TYPE* MEMB(get)(Self* vec, size_t index){
    return &vec->data[index];
}
static const VAL_TYPE* MEMB(front)(Self* vec){
    return &vec->data[0];
}
static const VAL_TYPE* MEMB(back)(Self* vec){
    return &vec->data[vec->siz - 1];
}
static const VAL_TYPE* MEMB(data)(Self* vec){
    return vec->data;
}

static size_t MEMB(capacity)(Self* vec){
    return vec->cap;
}
static size_t MEMB(size)(Self* vec){
    return vec->siz;
}
static bool MEMB(empty)(Self* vec){
    return !vec->siz;
}

static int MEMB(reserve)(Self* vec, size_t cap){
    if (cap <= vec->cap) return 0;
    VAL_TYPE* tmp = (VAL_TYPE*)malloc(sizeof(VAL_TYPE)*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    vec->data = tmp;
    vec->cap = cap;
    return 0;
}
static int MEMB(resize)(Self* vec, size_t siz){
    if (siz > vec->cap && MEMB(reserve)(vec, siz)) return -1;
    vec->siz = siz;
    return 0;
}
static int MEMB(shrink_fit)(Self* vec){
    if (vec->siz == 0){
        free(vec->data);
        vec->data = NULL;
        vec->cap = 0;
        return 0;
    }
    if (MEMB(reserve)(vec, vec->siz)) return -1;
    return 0;
}

static void MEMB(erase)(Self* vec, size_t index){
    for (size_t a = index; a < vec->siz - 1; a++) vec->data[a] = vec->data[a + 1];
    vec->siz--;
}
static void MEMB(clear)(Self* vec){
    vec->siz = 0;
}

static int MEMB(insert)(Self* vec, size_t index, VAL_TYPE in){
    if (vec->siz + 1 > vec->cap && MEMB(reserve)(vec, vec->cap*3/2)) return -1;
    for (size_t a = vec->siz; a > index; a--) vec->data[a] = vec->data[a - 1];
    vec->data[index] = in;
    vec->siz++;
    return 0;
}
static int MEMB(push_back)(Self* vec, VAL_TYPE in){
    if (vec->siz + 1 > vec->cap && MEMB(reserve)(vec, vec->cap*3/2)) return -1;
    vec->data[vec->siz++] = in;
    return 0;
}
static void MEMB(pop_back)(Self* vec){
    vec->siz--;
}

#undef T
