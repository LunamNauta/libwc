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

void* wcque_get_mut(const wcque_t* que, size_t index){
    return (char*)que->data + (que->dsiz*(que->front + index) % que->cap);
}
void* wcque_front_mut(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->front;
}
void* wcque_back_mut(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->back;
}
const void* wcque_get(const wcque_t* que, size_t index){
    return wcque_get_mut(que, index);
}
const void* wcque_front(const wcque_t* que){
    return wcque_front_mut(que);
}
const void* wcque_back(const wcque_t* que){
    return wcque_back_mut(que);
}

size_t wcque_capacity(const wcque_t* que){
    return que->cap;
}
size_t wcque_size(const wcque_t* que){
    if (que->back >= que->front) return que->back - que->front;
    return que->cap - que->front + que->back + 1;
}
bool wcque_empty(const wcque_t* que){
    return que->front == que->back;
}

size_t wcque_wasted(wcque_t* que){
    if (que->front >= que->back) return 0;
    return que->front;
}
void _wcque_unwaste(wcque_t* que){
    memmove((char*)que->data + que->dsiz*que->front, que->data, que->dsiz*(que->back - que->front + 1));
    que->back -= que->front;
    que->front = 0;
}
void wcque_unwaste(wcque_t* que){
    _wcque_unwaste(que);
}

int _wcque_reserve(wcque_t* que, size_t cap, bool reshift){
    if (cap <= que->cap) return 0;
    size_t len = que->front > que->back ? que->cap - que->front : 0;
    void* tmp = realloc(que->data, que->dsiz*cap);
    if (!tmp){
        errno = ENOMEM;
        return -1;
    }
    que->data = tmp;
    que->cap = cap;
    if (len){
        memmove((char*)que->data + que->dsiz*(que->cap - len), (char*)que->data + que->dsiz*que->front, que->dsiz*len);
        que->front = que->cap - len;
    }
    else if (reshift && que->front < que->back && que->dsiz*que->front >= WC_QUE_SHIFT_CAP){
        _wcque_unwaste(que);
    }
    return 0;
}
int wcque_reserve(wcque_t* que, size_t cap){
    return _wcque_reserve(que, cap, true);
}

int wcque_push(wcque_t* que, const void* in){
    if ((que->back + 1) % que->cap == que->front && _wcque_reserve(que, que->cap*3/2, false)) return -1;
    memcpy((char*)que->data + que->dsiz*((que->back + 1) % que->cap), in, que->dsiz);
    que->back = (que->back + 1) % que->cap;
    return 0;
}
void wcque_pop(wcque_t* que){
    que->front = (que->front + 1) % que->cap;
    if (wcque_empty(que)){
        que->front = 0;
        que->back = 0;
    }
    else if (que->front < que->back && que->dsiz*que->front >= WC_QUE_SHIFT_CAP){
        _wcque_unwaste(que);
    }
}
