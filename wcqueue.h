/*
#define T wcqueue_int_t int
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

/*
#ifndef T
#error "Template paramater not provided for wcqueue"
#endif
*/
#define T wcqueue_int, int

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

static inline const VAL_TYPE* MEMB(front)(Self* que);
static inline const VAL_TYPE* MEMB(back)(Self* que);

static inline int MEMB(reserve)(Self* que, size_t cap);

static inline size_t MEMB(size)(Self* que);
static inline bool MEMB(empty)(Self* que);

static inline int MEMB(push)(Self* que, VAL_TYPE value);
static inline void MEMB(pop)(Self* que);

/* Implementation ------------------------------------------------------------------------*/

static int MEMB(init)(Self* que, size_t cap){
    que->data = NULL;
    que->front = -1;
    que->back = -1;
    que->cap = 0;
    return MEMB(reserve)(que, cap);
}
static void MEMB(free)(Self* que){
    free(que->data);
}

static const VAL_TYPE* MEMB(front)(Self* que){

}
