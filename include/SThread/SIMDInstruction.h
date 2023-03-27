/******************************************************************/
/*!
    @file    SIMDInstruction.h
    @brief    SIMD instrucsion
    @note
    @todo
    @bug

    @author    Naoto Nakamura
    @date     Jan. 10, 2022
*/
/******************************************************************/

#ifndef STHREAD_SIMDINSTRUCTION_H
#define STHREAD_SIMDINSTRUCTION_H


#if defined (ARCHTECTURE_IA)
#define SIMDARCH_SSE
#elif defined (ARCHTECTURE_ARM) && !defined (ANDROID_ARMEABI)
#define SIMDARCH_NEON
#endif


#if defined SIMDARCH_SSE
#include <xmmintrin.h>
typedef __m128 SIMD128;
#elif defined SIMDARCH_NEON
#include <arm_neon.h>
typedef float32x4_t SIMD128;
#else
typedef float SIMD128 __attribute__((__vector_size__(16)));
#endif

#if defined OS_ANDROID
#   if ANDROID_API >= 16
int posix_memalign(void** memptr, size_t alignment, size_t size) __INTRODUCED_IN(16);
#   else
#include <stdlib.h>
#   endif /* ANDROID_API >= 16 */
#endif


#if defined COMPILER_MSVC
#    if defined(_DEBUG)
#        undef new
#    endif
#endif

#include <cstddef>

namespace SThread{

//attribute
#if defined COMPILER_MSVC

#define ATTRIBUTE_ALIGN(n) __declspec(align(n))

#elif defined COMPILER_GCC

#define ATTRIBUTE_ALIGN(n) __attribute__((aligned(n)))

#endif

    template <typename Ty, std::size_t N = 16>

    class AlignedBlockAllocator
    {
       public:
          typedef Ty value_type;
          typedef std::size_t size_type;
          typedef std::ptrdiff_t difference_type;

          typedef Ty * pointer;
          typedef const Ty * const_pointer;

          typedef Ty & reference;
          typedef const Ty & const_reference;

       public:
          inline AlignedBlockAllocator () throw ()
          {
          }

          template <typename ValTy>
          inline AlignedBlockAllocator (const AlignedBlockAllocator<ValTy, N> &) throw ()
          {
          }

          inline ~AlignedBlockAllocator () throw ()
          {
          }

          inline pointer address (reference r)
          {
             return &r;
          }

          inline const_pointer address (const_reference r) const
          {
             return &r;
          }

          inline pointer allocate (size_type n)
          {
#if defined (SIMD_SSE)
              return (pointer)_mm_malloc(n * sizeof(value_type), N);
#elif (SIMD_NEON)
              void* p;
              posix_memalign (&p, N, n * sizeof(value_type));
              return p;
#else
              return malloc(n * sizeof(value_type));
#endif
          }

          inline void deallocate (pointer p, size_type)
          {
             _mm_free(p);
          }

          inline void construct (pointer p, const value_type & wert)
          {
              new (p) value_type (wert);
          }


          inline void destroy (pointer p)
          {
             p->~value_type ();
          }

          inline size_type max_size () const throw ()
          {
             return size_type (-1) / sizeof (value_type);
          }

          template <typename ValTy>
          struct rebind
          {
             typedef AlignedBlockAllocator<ValTy, N> other;
          };
    };


#define BBIT_ORDER(a,b,c,d) (((a) << 6) | ((b) << 4) | ((c) << 2) | ((d)))

#if defined ASM_MS
#define ASM_REF_POINTER(a) a
#elif defined ASM_GCC
#define ASM_REF_POINTER(a) &a
#endif

};    //namespace SThread


#if defined COMPILER_MSVC
#    if defined(_DEBUG)
#        define new DBG_NEW
#    endif
#endif


#endif //STHREAD_SIMDINSTRUCTION_H
