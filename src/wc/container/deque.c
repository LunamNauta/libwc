#include "wc/container/deque.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

// TODO: Completely replace this deque with a c++ style deque (with fixed data blocks)

void wcdeque_init(wcdeque_t* que, size_t dsiz){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    que->dsiz = dsiz;
}
void wcdeque_free(const wcdeque_t* que){
    free(que->data);
}

void* wcdeque_get(const wcdeque_t* que, size_t ind){
    return (char*)que->data + (que->dsiz*(que->front + ind) % que->cap);
}
void* wcdeque_front(const wcdeque_t* que){
    return (char*)que->data + que->dsiz*que->front;
}
void* wcdeque_back(const wcdeque_t* que){
    if (!que->back) return (char*)que->data + que->dsiz*(que->cap - 1);
    return (char*)que->data + que->dsiz*(que->back - 1);
}

size_t wcdeque_wasted(const wcdeque_t* que){
    if (que->front >= que->back) return 0;
    return que->front;
}
size_t wcdeque_capacity(const wcdeque_t* que){
    if (!que->cap) return 0;
    return que->cap - 1;
}
size_t wcdeque_size(const wcdeque_t* que){
    if (que->back >= que->front) return que->back - que->front;
    return que->cap - que->front + que->back;
}
bool wcdeque_empty(const wcdeque_t* que){
    return que->back == que->front;
}

void wcdeque_unwaste(wcdeque_t* que){
    if (que->front >= que->back) return;
    memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->back - que->front));
    que->back -= que->front;
    que->front = 0;
}
int wcdeque_reserve(wcdeque_t* que, size_t cap){
    if (cap <= que->cap) return 0;
    size_t len = que->front > que->back ? que->cap - que->front : 0;
    void* tmp = realloc(que->data, que->dsiz*(cap + 1));
    if (!tmp){
        errno = -ENOMEM;
        return -1;
    }
    que->data = tmp;
    que->cap = cap + 1;
    if (len){
        memmove((char*)que->data + que->dsiz*(cap - len), (char*)que->data + que->dsiz*que->front, que->dsiz*len);
        que->front = cap - len;
    }
    else if (que->front < que->back && que->dsiz*que->front >= WC_DEQUE_SHIFT_CAP){
        wcdeque_unwaste(que);
    }
    return 0;
}
int wcdeque_resize(wcdeque_t* que, size_t siz){
    if (!siz){
        que->front = 0;
        que->back = 0;
        return 0;
    }
    if (que->front < que->back) memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->back - que->front));
    else if (que->front > que->back){
        void* tmp = malloc(que->dsiz*(que->cap - que->front));
        if (!tmp){
            errno = -ENOMEM;
            return -1;
        }
        memmove(tmp, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->cap - que->front));
        memmove((char*)que->data + que->dsiz*(que->cap - que->front), que->data, que->dsiz*que->back);
        memmove(que->data, tmp, que->dsiz*(que->cap - que->front));
    }
    if (siz > (que->cap - 1) && wcdeque_reserve(que, siz)) return -1;
    que->back = siz;
    que->front = 0;
    return 0;
}
int wcdeque_shrink(wcdeque_t* que){
    if (que->back == que->front){
        free(que->data);
        que->data = NULL;
        que->front = 0;
        que->back = 0;
        que->cap = 0;
        return 0;
    }
    if (que->front < que->back){
        memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->back - que->front));
        size_t tmp = que->cap;
        que->cap = 0;
        if (wcdeque_reserve(que, que->back - que->front)){
            que->cap = tmp;
            return -1;
        }
        return 0;
    }
    memmove((char*)que->data + que->dsiz*(que->back + 1), (char*)que->data + que->dsiz*que->front, que->dsiz*(que->cap - que->front - 1));
    size_t tmp = que->cap;
    que->cap = 0;
    if (wcdeque_reserve(que, que->cap - que->front)){
        que->cap = tmp;
        return -1;
    }
    que->front = que->back + 1;
    return 0;
}

void wcdeque_pop_front(wcdeque_t* que){
    que->front = (que->front + 1) % que->cap;
    if (que->back == que->front){
        que->front = 0;
        que->back = 0;
        return;
    }
    else if (que->front < que->back && que->dsiz*que->front >= WC_DEQUE_SHIFT_CAP){
        wcdeque_unwaste(que);
    }
}
void wcdeque_pop_back(wcdeque_t* que){
    if (!que->back) que->back = que->cap;
    else que->back--;
    if (que->back == que->front){
        que->front = 0;
        que->back = 0;
        return;
    }
}
void wcdeque_clear(wcdeque_t* que){
    que->front = 0;
    que->back = 0;
}

void wcdeque_push_back_rot(wcdeque_t* restrict que, const void* restrict in){
    if ((que->back + 1) % que->cap == que->front) que->front = (que->front + 1) % que->cap;
    memcpy((char*)que->data + que->dsiz*que->back, in, que->dsiz);
    que->back = (que->back + 1) % que->cap;
}
int wcdeque_push_back(wcdeque_t* restrict que, const void* restrict in){
    if (!que->data) wcdeque_reserve(que, 2);
    else if ((que->back + 1) % que->cap == que->front && wcdeque_reserve(que, que->cap*3/2)) return -1;
    memcpy((char*)que->data + que->dsiz*que->back, in, que->dsiz);
    que->back = (que->back + 1) % que->cap;
    return 0;
}
void wcdeque_push_front_rot(wcdeque_t* restrict que, const void* restrict in){
    if ((que->front ? que->front - 1 : que->cap) == que->back) que->back = que->back ? que->back - 1 : que->cap; 
    que->front = que->front ? que->front - 1 : que->cap;
    memcpy((char*)que->data + que->dsiz*que->front, in, que->dsiz);
}
int wcdeque_push_front(wcdeque_t* restrict que, const void* restrict in){
    if (!que->data) wcdeque_reserve(que, 2);
    else if ((que->front ? que->front - 1 : que->cap) == que->back && wcdeque_reserve(que, que->cap*3/2)) return -1;
    que->front = que->front ? que->front - 1 : que->cap;
    memcpy((char*)que->data + que->dsiz*que->front, in, que->dsiz);
    return 0;
}
