#include "wc/containers/que.h"

#include <string.h>
#include <errno.h>

int wcque_init(wcque_t* que, size_t dsiz){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    que->dsiz = dsiz;
    return wcque_reserve(que, WC_QUE_DEFAULT_CAP);
}
void wcque_free(const wcque_t* que){
    free(que->data);
}

const void* wcque_get(const wcque_t* que, size_t index){
    return (char*)que->data + (que->dsiz*(que->front + index) % que->cap);
}
const void* wcque_front(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->front;
}
const void* wcque_back(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->back;
}
void* wcque_get_mut(const wcque_t* que, size_t index){
    return (char*)que->data + (que->dsiz*(que->front + index) % que->cap);
}
void* wcque_front_mut(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->front;
}
void* wcque_back_mut(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->back;
}

size_t wcque_capacity(const wcque_t* que){
    return que->cap;
}
size_t wcque_size(const wcque_t* que){
    return que->back - que->front;
}
bool wcque_empty(const wcque_t* que){
    return que->front == que->back;
}

int wcque_reserve(wcque_t* que, size_t cap){
    if (cap <= que->cap) return 0;
    void* tmp = realloc(que->data, que->dsiz*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    que->data = tmp;
    que->cap = cap;
    return 0;
}

int wcque_push(wcque_t* que, const void* in){
    if ((que->back + 1) % que->cap == que->front && wcque_reserve(que, que->cap*3/2)) return -1;
    que->back++;
    memcpy((char*)que->data + que->dsiz*que->back, in, que->dsiz);
    return 0;
}
void wcque_pop(wcque_t* que){
    que->front = (que->front + 1) % que->cap;
    if (wcque_empty(que)){
        que->front = 0;
        que->back = 0;
    }
    else if (que->dsiz*que->front >= WC_QUE_SHIFT_CAP){
        size_t siz = wcque_size(que);
        memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*siz);
        que->back -= que->front;
        que->front = 0;
    }
}
