#include <stdio.h>

#define WCTL_DECLARE_BUILTINS
#include "vec.h"

#define WCTL_DECLARE_BUILTINS
#include "queue.h"

int main(){
    wcvec_float vec;
    wcvec_float_init(&vec, 1);
    wcvec_float_push_back(&vec, 3.14f);
    wcvec_float_push_back(&vec, 2.178f);
    wcvec_float_push_back(&vec, 10.5f);
    wcvec_float_erase(&vec, 1);
    wcvec_float_push_back(&vec, 1.56f);
    for (wcvec_float_iter it = wcvec_float_begin(&vec); wcvec_float_iter_cmp(it, wcvec_float_end(&vec)) != 0; wcvec_float_iter_next(&it)){
        printf("%f\n", *wcvec_float_iter_deref(it));
    }

    printf("\n");

    wcqueue_float que;
    wcqueue_float_init(&que, 1);
    wcqueue_float_push(&que, 3.14f);
    wcqueue_float_push(&que, 2.178f);
    wcqueue_float_push(&que, 10.5f);
    for (size_t a = 0; a < wcqueue_float_size(&que); a++){
        printf("%f\n", *wcqueue_float_get(&que, a));
    }
    wcqueue_float_pop(&que);
    wcqueue_float_push(&que, 10.0f);
    wcqueue_float_push(&que, 30.0f);
    printf("\n");
    for (size_t a = 0; a < wcqueue_float_size(&que); a++){
        printf("%f\n", *wcqueue_float_get(&que, a));
    }
}
