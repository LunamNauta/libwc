#include "wc/containers/que.h"

#include <string.h>
#include <errno.h>

int wcque_init_copy(wcque_t* restrict que, void* restrict in, size_t siz, size_t dsiz){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    que->dsiz = dsiz;
    if (wcque_reserve(que, siz)) return -1;
    memcpy(que->data, in, dsiz*siz);
    que->back = siz;
    return 0;
}
void wcque_init_take(wcque_t* que, void* in, size_t siz, size_t dsiz){
    que->data = in;
    que->front = 0;
    que->back = siz;
    que->cap = siz + 1;
    que->dsiz = dsiz;
}
int wcque_init_reserved(wcque_t* que, size_t dsiz, size_t cap){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    que->dsiz = dsiz;
    return wcque_reserve(que, cap);
}
void wcque_init(wcque_t* que, size_t dsiz){
    que->data = NULL;
    que->front = 0;
    que->back = 0;
    que->cap = 0;
    que->dsiz = dsiz;
}
int wcque_free_steal(const wcque_t* restrict que, void** restrict out){
    if (que->data && que->front == que->back){
        memcpy(out, &que->data, sizeof(void*));
        return 0;
    }
    if (que->front < que->back){
        memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->back - que->front));
        memcpy(out, &que->data, sizeof(void*));
        return 0;
    }
    void* tmp = malloc(que->dsiz*(que->cap - que->front));
    if (!tmp){
        errno = -ENOMEM;
        return -1;
    }
    memmove(tmp, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->cap - que->front));
    memmove((char*)que->data + que->dsiz*(que->cap - que->front), que->data, que->dsiz*que->back);
    memmove(que->data, tmp, que->dsiz*(que->cap - que->front));
    memcpy(out, &que->data, sizeof(void*));
    return 0;
}
void wcque_free(const wcque_t* que){
    free(que->data);
}

void* wcque_get(const wcque_t* que, size_t ind){
    return (char*)que->data + (que->dsiz*(que->front + ind) % que->cap);
}
void* wcque_front(const wcque_t* que){
    return (char*)que->data + que->dsiz*que->front;
}
void* wcque_back(const wcque_t* que){
    if (!que->back) return (char*)que->data + que->dsiz*(que->cap - 1);
    return (char*)que->data + que->dsiz*(que->back - 1);
}

size_t wcque_wasted(const wcque_t* que){
    if (que->front >= que->back) return 0;
    return que->front;
}
size_t wcque_capacity(const wcque_t* que){
    if (!que->cap) return 0;
    return que->cap - 1;
}
size_t wcque_size(const wcque_t* que){
    if (que->back >= que->front) return que->back - que->front;
    return que->cap - que->front + que->back;
}
size_t wcque_dsiz(const wcque_t* que){
    return que->dsiz;
}
bool wcque_empty(const wcque_t* que){
    return que->back == que->front;
}

void wcque_unwaste(wcque_t* que){
    if (que->front >= que->back) return;
    memmove(que->data, (char*)que->data + que->dsiz*que->front, que->dsiz*(que->back - que->front));
    que->back -= que->front;
    que->front = 0;
}
int wcque_reserve(wcque_t* que, size_t cap){
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
    else if (que->front < que->back && que->dsiz*que->front >= WC_QUE_SHIFT_CAP){
        wcque_unwaste(que);
    }
    return 0;
}
int wcque_resize(wcque_t* que, size_t siz){
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
    if (siz > (que->cap - 1) && wcque_reserve(que, siz)) return -1;
    que->back = siz;
    que->front = 0;
    return 0;
}
int wcque_shrink_fit(wcque_t* que){
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
        if (wcque_reserve(que, que->back - que->front)){
            que->cap = tmp;
            return -1;
        }
        return 0;
    }
    memmove((char*)que->data + que->dsiz*(que->back + 1), (char*)que->data + que->dsiz*que->front, que->dsiz*(que->cap - que->front - 1));
    size_t tmp = que->cap;
    que->cap = 0;
    if (wcque_reserve(que, que->cap - que->front)){
        que->cap = tmp;
        return -1;
    }
    que->front = que->back + 1;
    return 0;
}

void wcque_remove_range(wcque_t* que, size_t beg, size_t end);
void wcque_erase_fast(wcque_t* que, size_t ind);
void wcque_erase(wcque_t* que, size_t ind);
void wcque_pop_front(wcque_t* que){
    que->front = (que->front + 1) % que->cap;
    if (que->back == que->front){
        que->front = 0;
        que->back = 0;
        return;
    }
    else if (que->front < que->back && que->dsiz*que->front >= WC_QUE_SHIFT_CAP){
        wcque_unwaste(que);
    }
}
void wcque_pop_back(wcque_t* que){
    if (!que->back) que->back = que->cap;
    else que->back--;
    if (que->back == que->front){
        que->front = 0;
        que->back = 0;
        return;
    }
}
void wcque_clear(wcque_t* que){
    que->front = 0;
    que->back = 0;
}

int wcque_insert_vals(wcque_t* restrict que, size_t ind, const void* restrict in, size_t len);
int wcque_insert(wcque_t* restrict que, size_t ind, const void* restrict in);
int wcque_push_back_vals(wcque_t* restrict que, const void* restrict in, size_t len);
void wcque_push_back_rot(wcque_t* restrict que, const void* restrict in){
    if ((que->back + 1) % que->cap == que->front) que->front = (que->front + 1) % que->cap;
    memcpy((char*)que->data + que->dsiz*que->back, in, que->dsiz);
    que->back = (que->back + 1) % que->cap;
}
int wcque_push_back(wcque_t* restrict que, const void* restrict in){
    if (!que->data) wcque_reserve(que, 2);
    else if ((que->back + 1) % que->cap == que->front && wcque_reserve(que, que->cap*3/2)) return -1;
    memcpy((char*)que->data + que->dsiz*que->back, in, que->dsiz);
    que->back = (que->back + 1) % que->cap;
    return 0;
}
int wcque_push_front_vals(wcque_t* restrict que, const void* restrict in, size_t len);
void wcque_push_front_rot(wcque_t* restrict que, const void* restrict in){
    if ((que->front ? que->front - 1 : que->cap) == que->back) que->back = que->back ? que->back - 1 : que->cap; 
    que->front = que->front ? que->front - 1 : que->cap;
    memcpy((char*)que->data + que->dsiz*que->front, in, que->dsiz);
}
int wcque_push_front(wcque_t* restrict que, const void* restrict in){
    if (!que->data) wcque_reserve(que, 2);
    else if ((que->front ? que->front - 1 : que->cap) == que->back && wcque_reserve(que, que->cap*3/2)) return -1;
    que->front = que->front ? que->front - 1 : que->cap;
    memcpy((char*)que->data + que->dsiz*que->front, in, que->dsiz);
    return 0;
}

size_t wcque_bsearch(const wcque_t* que, const void* val, size_t beg, size_t end, int (*cmp)(const void*, const void*));
int wcque_copy(const wcque_t* restrict que, wcque_t* restrict out, size_t beg, size_t end, void (*cpy)(void*, const void*));
void wcque_sort(wcque_t* que, size_t beg, size_t end, int (*cmp)(const void*, const void*));


