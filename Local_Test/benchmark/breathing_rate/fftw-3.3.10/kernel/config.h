/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if the machine architecture "naturally" prefers fused multiply-add
   instructions */
#define ARCH_PREFERS_FMA

   /* Define to compile in long-double precision. */
#define BENCHFFT_LDOUBLE
   
   /* Define to compile in quad precision. */
#define BENCHFFT_QUAD
   
   /* Define to compile in single precision. */
#define BENCHFFT_SINGLE
   
   /* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
      systems. This function is required for `alloca.c' support on those systems.
      */
#define CRAY_STACKSEG_END
   
   /* Define to 1 if using `alloca.c'. */
#define C_ALLOCA
   
   /* Define to disable Fortran wrappers. */
#define DISABLE_FORTRAN
   
   /* Define to dummy `main' function (if any) required to link to the Fortran
      libraries. */
#define F77_DUMMY_MAIN
   
   /* Define to a macro mangling the given C identifier (in lower and upper
      case), which must not contain underscores, for linking with Fortran. */
#define F77_FUNC
   
   /* As F77_FUNC, but for C identifiers containing underscores. */
#define F77_FUNC_
   
   /* Define if F77_FUNC and F77_FUNC_ are equivalent. */
#define F77_FUNC_EQUIV
   
   /* Define if F77 and FC dummy `main' functions are identical. */
#define FC_DUMMY_MAIN_EQ_F77
   
   /* C compiler name and flags */
#define FFTW_CC
   
   /* Define to enable extra FFTW debugging code. */
#define FFTW_DEBUG
   
   /* Define to enable the use of alloca(). */
#define FFTW_ENABLE_ALLOCA
   
   /* Define to compile in long-double precision. */
#define FFTW_LDOUBLE
   
   /* Define to compile in quad precision. */
#define FFTW_QUAD
   
   /* Define to enable pseudorandom estimate planning for debugging. */
#define FFTW_RANDOM_ESTIMATOR
   
   /* Define to compile in single precision. */
#define FFTW_SINGLE
   
   /* Define to 1 if you have the `abort' function. */
#define HAVE_ABORT
   
   /* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA
   
   /* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
      */
#define HAVE_ALLOCA_H
   
   /* Define to enable Altivec optimizations. */
#define HAVE_ALTIVEC
   
   /* Define to 1 if you have the <altivec.h> header file. */
#define HAVE_ALTIVEC_H
   
   /* Define if you have enabled the CNTVCT cycle counter on ARMv7a */
#define HAVE_ARMV7A_CNTVCT
   
   /* Define if you have enabled the PMCCNTR cycle counter on ARMv7a */
#define HAVE_ARMV7A_PMCCNTR
   
   /* Define if you have enabled the CNTVCT_EL0 cycle counter on ARMv8 */
#define HAVE_ARMV8_CNTVCT_EL0
   
   /* Define if you have enabled the PMCCNTR_EL0 cycle counter on ARMv8 */
#define HAVE_ARMV8_PMCCNTR_EL0
   
   /* Define to enable AVX optimizations. */
#define HAVE_AVX
   
   /* Define to enable AVX2 optimizations. */
#define HAVE_AVX2
   
   /* Define to enable AVX512 optimizations. */
#define HAVE_AVX512
   
   /* Define to enable 128-bit FMA AVX optimization */
#define HAVE_AVX_128_FMA
   
   /* Define to 1 if you have the `BSDgettimeofday' function. */
#define HAVE_BSDGETTIMEOFDAY
   
   /* Define to 1 if you have the `clock_gettime' function. */
#define HAVE_CLOCK_GETTIME
   
   /* Define to 1 if you have the `cosl' function. */
#define HAVE_COSL
   
   /* Define to 1 if you have the declaration of `cosl', and to 0 if you don't.
      */
#define HAVE_DECL_COSL
   
   /* Define to 1 if you have the declaration of `cosq', and to 0 if you don't.
      */
#define HAVE_DECL_COSQ
   
   /* Define to 1 if you have the declaration of `drand48', and to 0 if you
      don't. */
#define HAVE_DECL_DRAND48
   
   /* Define to 1 if you have the declaration of `memalign', and to 0 if you
      don't. */
#define HAVE_DECL_MEMALIGN
   
   /* Define to 1 if you have the declaration of `posix_memalign', and to 0 if
      you don't. */
#define HAVE_DECL_POSIX_MEMALIGN
   
   /* Define to 1 if you have the declaration of `sinl', and to 0 if you don't.
      */
#define HAVE_DECL_SINL
   
   /* Define to 1 if you have the declaration of `sinq', and to 0 if you don't.
      */
#define HAVE_DECL_SINQ
   
   /* Define to 1 if you have the declaration of `srand48', and to 0 if you
      don't. */
#define HAVE_DECL_SRAND48
   
   /* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H
   
   /* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#define HAVE_DOPRNT
   
   /* Define to 1 if you have the `drand48' function. */
#define HAVE_DRAND48
   
   /* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H
   
   /* Define to 1 if you have the <fenv.h> header file. */
#define HAVE_FENV_H
   
   /* Define to enable generic (gcc) 128-bit SIMD optimizations. */
#define HAVE_GENERIC_SIMD128
   
   /* Define to enable generic (gcc) 256-bit SIMD optimizations. */
#define HAVE_GENERIC_SIMD256
   
   /* Define to 1 if you have the `gethrtime' function. */
#define HAVE_GETHRTIME
   
   /* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE
   
   /* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY
   
   /* Define to 1 if hrtime_t is defined in <sys/time.h> */
#define HAVE_HRTIME_T
   
   /* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H
   
   /* Define if the isnan() function/macro is available. */
#define HAVE_ISNAN
   
   /* Define to enable KCVI optimizations. */
#define HAVE_KCVI
   
   /* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM
   
   /* Define to 1 if you have the `quadmath' library (-lquadmath). */
#define HAVE_LIBQUADMATH
   
   /* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H
   
   /* Define to 1 if the compiler supports `long double' */
#define HAVE_LONG_DOUBLE
   
   /* Define to 1 if you have the `mach_absolute_time' function. */
#define HAVE_MACH_ABSOLUTE_TIME
   
   /* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H
   
   /* Define to 1 if you have the `memalign' function. */
#define HAVE_MEMALIGN
   
   /* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE
   
   /* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H
   
   /* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET
   
   /* Define to enable use of MIPS ZBus cycle-counter. */
#define HAVE_MIPS_ZBUS_TIMER
   
   /* Define if you have the MPI library. */
#define HAVE_MPI
   
   /* Define to enable ARM NEON optimizations. */
#define HAVE_NEON
   
   /* Define if OpenMP is enabled */
#define HAVE_OPENMP
   
   /* Define to 1 if you have the `posix_memalign' function. */
#define HAVE_POSIX_MEMALIGN
   
   /* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD
   
   /* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T
   
   /* Define to 1 if you have the `read_real_time' function. */
#define HAVE_READ_REAL_TIME
   
   /* Define to 1 if you have the `sinl' function. */
#define HAVE_SINL
   
   /* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF
   
   /* Define to 1 if you have the `sqrt' function. */
#define HAVE_SQRT
   
   /* Define to enable SSE/SSE2 optimizations. */
#define HAVE_SSE2
   
   /* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H
   
   /* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H
   
   /* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H
   
   /* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR
   
   /* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H
   
   /* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H
   
   /* Define to 1 if you have the `sysctl' function. */
#define HAVE_SYSCTL
   
   /* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H
   
   /* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H
   
   /* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H
   
   /* Define to 1 if you have the `tanl' function. */
#define HAVE_TANL
   
   /* Define if we have a threads library. */
#define HAVE_THREADS
   
   /* Define to 1 if you have the `time_base_to_time' function. */
#define HAVE_TIME_BASE_TO_TIME
   
   /* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T
   
   /* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H
   
   /* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF
   
   /* Define to enable IBM VSX optimizations. */
#define HAVE_VSX
   
   /* Define to 1 if you have the `_mm_free' function. */
#define HAVE__MM_FREE
   
   /* Define to 1 if you have the `_mm_malloc' function. */
#define HAVE__MM_MALLOC
   
   /* Define if you have the UNICOS _rtc() intrinsic. */
#define HAVE__RTC
   
   /* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR
   
   /* Name of package */
#define PACKAGE
   
   /* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT
   
   /* Define to the full name of this package. */
#define PACKAGE_NAME
   
   /* Define to the full name and version of this package. */
#define PACKAGE_STRING
   
   /* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME
   
   /* Define to the home page for this package. */
#define PACKAGE_URL
   
   /* Define to the version of this package. */
#define PACKAGE_VERSION
   
   /* Define to necessary symbol if this constant uses a non-standard name on
      your system. */
#define PTHREAD_CREATE_JOINABLE
   
   /* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE
   
   /* The size of `fftw_r2r_kind', as computed by sizeof. */
#define SIZEOF_FFTW_R2R_KIND
   
   /* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT
   
   /* The size of `int', as computed by sizeof. */
#define SIZEOF_INT
   
   /* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG
   
   /* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG
   
   /* The size of `MPI_Fint', as computed by sizeof. */
#define SIZEOF_MPI_FINT
   
   /* The size of `ptrdiff_t', as computed by sizeof. */
#define SIZEOF_PTRDIFF_T
   
   /* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T
   
   /* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT
   
   /* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG
   
   /* The size of `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG
   
   /* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P
   
   /* If using the C implementation of alloca, define if you know the
      direction of stack growth for your system; otherwise it will be
      automatically deduced at runtime.
       STACK_DIRECTION > 0 => grows toward higher addresses
       STACK_DIRECTION < 0 => grows toward lower addresses
       STACK_DIRECTION = 0 => direction of growth unknown */
#define STACK_DIRECTION
   
   /* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS
   
   /* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME
   
   /* Define if we have and are using POSIX threads. */
#define USING_POSIX_THREADS
   
   /* Version number of package */
#define VERSION
   
   /* Use common Windows Fortran mangling styles for the Fortran interfaces. */
#define WINDOWS_F77_MANGLING
   
   /* Include g77-compatible wrappers in addition to any other Fortran wrappers.
      */
#define WITH_G77_WRAPPERS
   
   /* Use our own aligned malloc routine; mainly helpful for Windows systems
      lacking aligned allocation system-library routines. */
#define WITH_OUR_MALLOC
   
   /* Use low-precision timers, making planner very slow */
#define WITH_SLOW_TIMER
   
   /* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
      <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #defineine below would cause a syntax error. */
#define _UINT32_T
   
   /* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
      <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #defineine below would cause a syntax error. */
#define _UINT64_T
   
   /* Define to empty if `const' does not conform to ANSI C. */
#define const
   
   /* Define to `__inline__' or `__inline' if that's what the C compiler
      calls it, or to nothing if 'inline' is not supported under any name.  */
   #ifndef __cplusplus
#define inline
   #endif
   
   /* Define to `unsigned int' if <sys/types.h> does not define. */
#define size_t
   
   /* Define to the type of an unsigned integer type of width exactly 32 bits if
      such a type exists and the standard includes do not define it. */
#define uint32_t
   
   /* Define to the type of an unsigned integer type of width exactly 64 bits if
      such a type exists and the standard includes do not define it. */
#define uint64_t
   