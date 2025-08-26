#ifndef WCTL_COMMON_HEADER
#define WCTL_COMMON_HEADER

#if defined __GNUC__
    #define WCTL_INLINE static inline __attribute__((unused))
#else
    #define WCTL_INLINE static inline
#endif

#define wc_JOIN0(a, b) a ## b
#define wc_JOIN(a, b) wc_JOIN0(a, b)
#define wc_NUMARGS(...) _wc_APPLY_ARG_N((__VA_ARGS__, _wc_RSEQ_N))
#define _wc_APPLY_ARG_N(args) _wc_ARG_N args
#define _wc_RSEQ_N 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
#define _wc_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,N,...) N

#define wc_GETARG(N, ...) _wc_ARG_##N(__VA_ARGS__,)
#define _wc_ARG_1(a, ...) a
#define _wc_ARG_2(a, b, ...) b
#define _wc_ARG_3(a, b, c, ...) c
#define _wc_ARG_4(a, b, c, d, ...) d

#endif
