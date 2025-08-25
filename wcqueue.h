/*
#define T wcqueue_int_t int
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#ifndef T
#error "Template paramater not provided for wcqueue"
#endif

#include "wctemplate.h"
#define Self GET_SPLIT(T, 1)
#define VAL_TYPE GET_SPLIT(T, 2)

/* Declaration ---------------------------------------------------------------------------*/

typedef VAL_TYPE MEMB(VAL_TYPE);
typedef struct Self{
    VAL_TYPE* data;
    size_t front;
    size_t back;
    size_t cap;
} Self;

static inline int MEMB(init)(Self* que, size_t cap);
static inline void MEMB(free)(Self* que);

static inline const VAL_TYPE* MEMB(get)(Self* que, size_t index);
static inline const VAL_TYPE* MEMB(front)(Self* que);
static inline const VAL_TYPE* MEMB(back)(Self* que);

static inline size_t MEMB(size)(Self* que);
static inline bool MEMB(empty)(Self* que);

static inline int MEMB(reserve)(Self* que, size_t cap);

static inline int MEMB(push)(Self* que, VAL_TYPE value);
static inline void MEMB(pop)(Self* que);

/* Implementation ------------------------------------------------------------------------*/

static int MEMB(init)(Self* que, size_t cap){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    return MEMB(reserve)(que, cap >= 2 ? cap : 2);
}
static void MEMB(free)(Self* que){
    free(que->data);
}

static const VAL_TYPE* MEMB(get)(Self* que, size_t index){
    return &que->data[(que->front + index) % que->cap];
}
static const VAL_TYPE* MEMB(front)(Self* que){
    return &que->data[que->front];
}
static const VAL_TYPE* MEMB(back)(Self* que){
    return &que->data[que->back - 1];
}

static size_t MEMB(size)(Self* que){
    size_t diff = que->back - que->front;
    if (diff > que->cap) return que->cap - que->front + que->back;
    return diff;
}
static bool MEMB(empty)(Self* que){
    return que->front == que->back;
}

static int MEMB(reserve)(Self* que, size_t cap){
    if (cap <= que->cap) return 0;
    VAL_TYPE* tmp = (VAL_TYPE*)realloc(que->data, sizeof(VAL_TYPE)*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    que->data = tmp;
    que->cap = cap;
    return 0;
}

static int MEMB(push)(Self* que, VAL_TYPE value){
    que->data[que->back] = value;
    que->back = (que->back + 1) % que->cap;
    if (que->back == que->front){
        size_t len = que->cap - que->front;
        if (MEMB(reserve)(que, que->cap*3/2)) return -1;
        memmove(&que->data[que->cap - len], &que->data[que->front], sizeof(VAL_TYPE)*len);
        que->front = que->cap - len;
    }
    return 0;
}
static void MEMB(pop)(Self* que){
    que->front = (que->front + 1) % que->cap;
    if (MEMB(empty)(que)){
        que->front = 0;
        que->back = 0;
    }
}
