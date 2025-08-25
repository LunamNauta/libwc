#include <stdio.h>

#define T wcvec_float, float
#include "wcvec.h"

#define T wcvec_int, int
#include "wcvec.h"

#include "test.h"

int main(){
    wcvec_float test;
    wcvec_float_init(&test, 10);
    wcvec_float_insert(&test, 0, 3.14f);
    wcvec_float_insert(&test, 1, 2.718f);
    wcvec_float_insert(&test, 1, 10.5f);
    wcvec_float_erase(&test, 2);
    for (size_t a = 0; a < wcvec_float_size(&test); a++){
        printf("%f\n", *wcvec_float_get(&test, a));
    }

    wcvec_int test2;
}
