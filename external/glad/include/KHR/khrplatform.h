/*
 * Khronos Platform Header
 *
 * Provides cross-platform type definitions used by OpenGL, OpenGL ES, Vulkan, etc.
 * These types ensure consistent sizes across different compilers and platforms.
 */

#ifndef __khrplatform_h_
#define __khrplatform_h_

#include <stdint.h>

/* =============================================================================
 * Basic Integer Types
 * These ensure we get exact sizes regardless of platform (32-bit vs 64-bit)
 * ============================================================================= */

typedef int32_t  khronos_int32_t;    /* Exactly 32 bits, signed */
typedef uint32_t khronos_uint32_t;   /* Exactly 32 bits, unsigned */
typedef int64_t  khronos_int64_t;    /* Exactly 64 bits, signed */
typedef uint64_t khronos_uint64_t;   /* Exactly 64 bits, unsigned */

typedef int8_t   khronos_int8_t;     /* Exactly 8 bits, signed */
typedef uint8_t  khronos_uint8_t;    /* Exactly 8 bits, unsigned */
typedef int16_t  khronos_int16_t;    /* Exactly 16 bits, signed */
typedef uint16_t khronos_uint16_t;   /* Exactly 16 bits, unsigned */

/* =============================================================================
 * Pointer-Sized Types
 * These change size based on whether you're on 32-bit or 64-bit system.
 * Used for buffer sizes and memory offsets.
 * ============================================================================= */

#if defined(_WIN64) || defined(__LP64__) || defined(__x86_64__) || defined(__aarch64__)
    /* 64-bit platforms */
    typedef int64_t  khronos_intptr_t;   /* Can hold a pointer value */
    typedef uint64_t khronos_uintptr_t;
    typedef int64_t  khronos_ssize_t;    /* Signed size (can be negative for errors) */
    typedef uint64_t khronos_usize_t;    /* Unsigned size */
#else
    /* 32-bit platforms */
    typedef int32_t  khronos_intptr_t;
    typedef uint32_t khronos_uintptr_t;
    typedef int32_t  khronos_ssize_t;
    typedef uint32_t khronos_usize_t;
#endif

/* =============================================================================
 * Floating Point Types
 * ============================================================================= */

typedef float khronos_float_t;    /* 32-bit IEEE 754 float */

#endif /* __khrplatform_h_ */
