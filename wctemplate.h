#ifndef WC_WCTEMPLATE_HEADER
#define WC_WCTEMPLATE_HEADER

#define SPLIT_1(x, ...) x
#define SPLIT_2(x0, x, ...) x
#define SPLIT_3(x0, x1, x, ...) x
#define GET_SPLIT(x, n) SPLIT_##n(x)

#define CONCAT_(A,B) A##B
#define CONCAT(A,B) CONCAT_(A,B)

#define MEMB(func) CONCAT(Self, _##func)

#endif
