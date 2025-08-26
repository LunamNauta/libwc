//#define T wc_self, wc_type
//#define T queue_int, int

/* Includes ---------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "template.h"
#include "linkage.h"
#include "common.h"

/* Sanity Checks ---------------------------------------------------------------------------*/

#ifndef T
    #error "Error: Template not provided for queue"
#endif

#if wc_NUMARGS(T) != 2
    #error "Error: Template for queue must have exactly two parameters"
#endif

/* Typedefs ---------------------------------------------------------------------------*/

#undef wc_self
#undef wc_type
#define wc_self wc_GETARG(1, T)
#define wc_type wc_GETARG(2, T)
typedef wc_type wc_MEMB(_type);

/* Type Declarations ---------------------------------------------------------------------------*/

typedef struct wc_JOIN(wc_self, _s){
    wc_type* data;
    size_t front;
    size_t back;
    size_t cap;
} wc_self;

/* Inlined Function Declarations/Implementations ---------------------------------------------------------------------------*/

WCTL_INLINE const wc_type* wc_MEMB(_get)(wc_self* que, size_t index){
    return &que->data[(que->front + index) % que->cap];
}
WCTL_INLINE const wc_type* wc_MEMB(_front)(wc_self* que){
    return &que->data[que->front];
}
WCTL_INLINE const wc_type* wc_MEMB(_back)(wc_self* que){
    return &que->data[que->back - 1];
}
WCTL_INLINE wc_type* wc_MEMB(_get_mut)(wc_self* que, size_t index){
    return &que->data[(que->front + index) % que->cap];
}
WCTL_INLINE wc_type* wc_MEMB(_front_mut)(wc_self* que){
    return &que->data[que->front];
}
WCTL_INLINE wc_type* wc_MEMB(_back_mut)(wc_self* que){
    return &que->data[que->back - 1];
}

WCTL_INLINE size_t wc_MEMB(_size)(wc_self* que){
    size_t diff = que->back - que->front;
    if (diff > que->cap) return que->cap - que->front + que->back;
    return diff;
}
WCTL_INLINE bool wc_MEMB(_empty)(wc_self* que){
    return que->front == que->back;
}

/* Non-Inlined Function Declarations ---------------------------------------------------------------------------*/

WCTL_API int wc_MEMB(_init)(wc_self* que, size_t cap);
WCTL_API void wc_MEMB(_free)(wc_self* que);

WCTL_API int wc_MEMB(_reserve)(wc_self* que, size_t cap);

WCTL_API int wc_MEMB(_push)(wc_self* que, wc_type in);
WCTL_API void wc_MEMB(_pop)(wc_self* que);

/* Implementation ------------------------------------------------------------------------*/

WCTL_DEF int wc_MEMB(_init)(wc_self* que, size_t cap){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    return wc_MEMB(_reserve)(que, cap >= 2 ? cap : 2);
}
WCTL_DEF void wc_MEMB(_free)(wc_self* que){
    free(que->data);
}

WCTL_DEF int wc_MEMB(_reserve)(wc_self* que, size_t cap){
    if (cap <= que->cap) return 0;
    wc_type* tmp = realloc(que->data, sizeof(wc_type)*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    que->data = tmp;
    que->cap = cap;
    return 0;
}

WCTL_DEF int wc_MEMB(_push)(wc_self* que, wc_type in){
    que->data[que->back] = in;
    que->back = (que->back + 1) % que->cap;
    if (que->back == que->front){
        size_t len = que->cap - que->front;
        if (wc_MEMB(_reserve)(que, que->cap*3/2)) return -1;
        memmove(&que->data[que->cap - len], &que->data[que->front], sizeof(wc_type)*len);
        que->front = que->cap - len;
    }
    return 0;
}
WCTL_DEF void wc_MEMB(_pop)(wc_self* que){
    que->front = (que->front + 1) % que->cap;
    if (wc_MEMB(_empty)(que)){
        que->front = 0;
        que->back = 0;
    }
}

#undef T
