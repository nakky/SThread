/******************************************************************/
/*!
    @file    Common.h
    @brief    Common header
    @note
    @todo
    @bug

    @author    Naoto Nakamura
    @date    Jan. 10, 2022
*/
/******************************************************************/

#ifndef STHREAD_COMMON_H
#define STHREAD_COMMON_H

//stundard header
#include <math.h>

//OS
#if defined (_MSC_VER) || defined (__CYGWIN32__) || defined (__MINGW32__)
    #ifndef OS_WINDOWS
        #define OS_WINDOWS
    #endif
#elif defined (__IPHONE_OS_VERSION_MIN_REQUIRED)
    #ifndef OS_IPHONE
        #define OS_IPHONE
    #endif
#elif defined (__APPLE__) && defined (__MACH__)
    #ifndef OS_MACOSX
        #define OS_MACOSX
    #endif
#elif defined (__ANDROID__) || defined (ANDROID)
    #ifndef OS_ANDROID
        #define OS_ANDROID
    #endif
#elif defined (__linux__)
    #ifndef OS_LINUX
        #define OS_LINUX
    #endif
#endif

#if defined OS_IPHONE || defined OS_ANDROID
#define PLATFORM_MOBILE
#else
#define PLATFORM_DESKTOP
#endif

//compiler
#if defined(__GNUC__)
    #define COMPILER_GCC
//#elif defined(__INTEL_COMPILER)
//#    define COMPILER_INTEL
#elif defined(_MSC_VER)
    #define COMPILER_MSVC
#else
    #error Not supported (compiler)
#endif

//archtecture
#if defined(__i386__) || defined(_M_IX86)
    #define ARCHTECTURE_IA
    #define ARCHTECTURE_IA32
    #define ENDIAN_LITTLE 1
    #define ENDIAN_BIG    0
#elif defined(__x86_64) || defined(_M_X64) || defined(__amd64) || defined(_M_AMD64)
    #define ARCHTECTURE_IA
    #define ARCHTECTURE_IA64
    #define ENDIAN_LITTLE 1
    #define ENDIAN_BIG    0
#elif defined(arm) || defined(__arm__) || defined(__arm64) || defined(__aarch64__)
    #define ARCHTECTURE_ARM
    #if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7__)
        #define  ARCHTECTURE_ARM7
    #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
        #define  ARCHTECTURE_ARM6
    #elif defined(__arm64) || defined(__aarch64__)
        #define  ARCHTECTURE_ARM64
    #endif
    #if defined(__ARMEB__)
        #define ENDIAN_LITTLE 0
        #define ENDIAN_BIG    1
    #else
        #define ENDIAN_LITTLE 1
        #define ENDIAN_BIG    0
    #endif
#else
    #error Not supported (archtecture)
#endif

//Build architecture (Defined by compile option)
//////#define BUILD_DEBUG
//////#define BUILD_RELEASE

//Platform option

//MSVC//////////////////////////////////////////////////
#if defined COMPILER_MSVC

#define NOMINMAX

#include <winsock2.h>
#include <MSTcpIp.h>
#include <windows.h>

#if defined(_DEBUG)
#    define _CRTDBG_MAP_ALLOC
#    include <stdlib.h>
#    include <crtdbg.h>
#    ifndef DBG_NEW
#        define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#        define new DBG_NEW
#    endif
#else
#    undef _SECURE_SCL
#    define _SECURE_SCL 0
#endif

//Archtecture
#if defined _WIN64
    #define ARCHTECTURE_64BIT
#else
    #define ARCHTECTURE_32BIT
#endif

//API Interface
#define USE_WINDOWSTHREAD_INTERFACE
#define USE_WINSOCK_INTERFACE

//Strings
#if defined _UNICODE
    #define BASICSTRING_16BIT
#else
    #define BASICSTRING_8BIT
#endif

//force inline
#define FORCE_INLINE __forceinline

#define ENABLED_THREADLOCALSTORAGE 1
#ifndef TLS
#define TLS __declspec( thread )
#endif

//GCC//////////////////////////////////////////////////
#elif defined COMPILER_GCC

//Archtecture
#if defined __LP64__
    #define ARCHTECTURE_64BIT
#else
    #define ARCHTECTURE_32BIT
#endif

//API Interface
#define USE_PTHREAD_INTERFACE
#define USE_BSDSOCKET_INTERFACE

//Strings
//#define BASICSTRING_16BIT
#define BASICSTRING_8BIT

//force inline
#define FORCE_INLINE inline __attribute__((always_inline))

#if defined OS_IPHONE
#define ENABLED_THREADLOCALSTORAGE 0
#else

#define ENABLED_THREADLOCALSTORAGE 1
#ifndef TLS
    #define TLS __thread
#endif

#endif

////////////////////////////////////////////////////////

//Android
#if defined OS_ANDROID
#   if defined ARCHTECTURE_ARM7
    #define ANDROID_ARMEABI_V7A
#   elif defined ARCHTECTURE_ARM64
    #define ANDROID_ARM64
#   elif defined ARCHTECTURE_IA32
    #define ANDROID_X86
#   elif defined ARCHTECTURE_IA64
    #define ANDROID_X86_64
#   elif defined ARCHTECTURE_ARM
    #define ANDROID_ARMEABI
#   else
    #error Unsupported Android Archtecture
#   endif
#endif


//ASM
#if defined OS_WINDOWS || defined OS_MACOSX || defined OS_IPHONE
    #define ASM_MS
#elif defined OS_LINUX || defined OS_ANDROID
    #define ASM_GCC
#endif

#endif
///////////////////////////////////////////////////////////

//macro definition
#ifndef NULL
    #define NULL 0
#endif

#ifndef TRUE
    #define TRUE true
#endif

#ifndef FALSE
    #define FALSE false
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
    #define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef NUM_ARRAY
    #define NUM_ARRAY( a ) sizeof(a)/sizeof(a[0])
#endif//NUM_ARRAY

#ifndef UP_CAST

#if defined BUILD_DEBUG
    #define UP_CAST(c) static_cast< c >
#else
    #define UP_CAST(c) static_cast< c >
#endif

#endif //UP_CAST

#ifndef DOWN_CAST

#ifdef BUILD_DEBUG
    #define DOWN_CAST(c) dynamic_cast<c>
#else//RELEASE
    #define DOWN_CAST(c) static_cast<c>
#endif//DEBUG/RELEASE

#endif //DOWN_CAST

#define CONST_BINARY_0000 ( 0x0 )
#define CONST_BINARY_0001 ( 0x1 )
#define CONST_BINARY_0010 ( 0x2 )
#define CONST_BINARY_0011 ( 0x3 )
#define CONST_BINARY_0100 ( 0x4 )
#define CONST_BINARY_0101 ( 0x5 )
#define CONST_BINARY_0110 ( 0x6 )
#define CONST_BINARY_0111 ( 0x7 )
#define CONST_BINARY_1000 ( 0x8 )
#define CONST_BINARY_1001 ( 0x9 )
#define CONST_BINARY_1010 ( 0xA )
#define CONST_BINARY_1011 ( 0xB )
#define CONST_BINARY_1100 ( 0xC )
#define CONST_BINARY_1101 ( 0xD )
#define CONST_BINARY_1110 ( 0xE )
#define CONST_BINARY_1111 ( 0xF )

#define BIN4( a )  \
    CONST_BINARY_ ## a
#define BIN8( a, b )  \
    ((unsigned char)( ((0x0F & (BIN4( a ))) << 4) | (0x0F & (BIN4( b ))) ))
#define BIN16( a, b, c, d )  \
    ((unsigned short)( ((0xFF & (BIN8( a, b ))) << 8) | (0xFF & (BIN8( c, d ))) ))
#define BIN32( a, b, c, d, e, f, g, h )  \
    ((unsigned long)( ((0xFFFF & (BIN16( a, b, c, d ))) << 16) | (0xFFFF & (BIN16( e, f, g, h ))) ))

//Pointer size/////////////////////////////
#if defined ARCHTECTURE_64BIT

#define POINTER_BYTE_SIZE 8

#elif defined ARCHTECTURE_32BIT

#define POINTER_BYTE_SIZE 4

#endif

#if defined BASICSTRING_16BIT

    #ifndef BASE_TEXT
    #define BASE_TEXT(lit) L ## lit
    #endif

#elif defined BASICSTRING_8BIT

    #ifndef BASE_TEXT
    #define BASE_TEXT(lit) lit
    #endif

#endif

#include "SThread/SIMDInstruction.h"

//Foundation Part

#if defined COMPILER_MSVC
#pragma warning(disable :4311)
#pragma warning(disable :4312)
#endif //COMPILER_MSVC

//Communication Part

#endif //STHREAD_COMMON_H



