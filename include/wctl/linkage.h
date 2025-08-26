#undef WCTL_API
#undef WCTL_DEF

#if !defined WCTL_STATIC && (defined WCTL_HEADER  || defined WCTL_IMPLEMENT)
    #define WCTL_API extern
    #define WCTL_DEF
#else
    #define WCTL_IMPLEMENT
    #if defined __GNUC__ || defined __clang__ || defined __INTEL_LLVM_COMPILER
        #define WCTL_API static __attribute__((unused))
    #else
        #define WCTL_API static inline
    #endif
    #define WCTL_DEF static
#endif
