/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ot_inc/toolchain.h#1 $*/
/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @addtogroup plat-toolchain
 *
 * @brief
 *   This module defines a toolchain abstraction layer through macros.
 *
 * Usage:
 *
 *    @code
 *
 *    typedef
 *    OT_TOOL_PACKED_BEGIN
 *    struct
 *    {
 *        char mField1;
 *        union
 *        {
 *            char mField2;
 *            long mField3;
 *        } OT_TOOL_PACKED_FIELD;
 *    } OT_TOOL_PACKED_END packed_struct_t;
 *
 *    @endcode
 *
 * @{
 *
 */

#ifndef OPENTHREAD_PLATFORM_TOOLCHAIN_H_
#define OPENTHREAD_PLATFORM_TOOLCHAIN_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def OT_MUST_USE_RESULT
 *
 * Compiler-specific indication that a class or enum must be used when it is
 * the return value of a function.
 *
 * @note This is currently only available with clang (C++17 implements it
 *       as attribute [[nodiscard]]).
 * @note To suppress the 'unused-result' warning/error, please use the
 *       '-Wno-unused-result' compiler option.
 *
 */
#if defined(__clang__) && (__clang_major__ >= 4 || (__clang_major__ >= 3 && __clang_minor__ >= 9))
#define OT_MUST_USE_RESULT __attribute__((warn_unused_result))
#else
#define OT_MUST_USE_RESULT
#endif

/**
 * @def OT_TOOL_PACKED_BEGIN
 *
 * Compiler-specific indication that a class or struct must be byte packed.
 *
 */

/**
 * @def OT_TOOL_PACKED_FIELD
 *
 * Indicate to the compiler a nested struct or union to be packed
 * within byte packed class or struct.
 *
 */

/**
 * @def OT_TOOL_PACKED_END
 *
 * Compiler-specific indication at the end of a byte packed class or struct.
 *
 */

/**
 * @def OT_TOOL_WEAK
 *
 * Compiler-specific weak symbol modifier.
 *
 */

/**
 * @def OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK
 *
 * Specifies that a function or method takes `printf` style arguments and should be type-checked against
 * a format string.
 *
 * Must be added after the function/method declaration. For example:
 *
 *    `void MyPrintf(void *aObject, const char *aFormat, ...) OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK(2, 3);`
 *
 * The two argument index values indicate format string and first argument to check against it. They start at index 1
 * for the first parameter in a function and at index 2 for the first parameter in a method.
 *
 * @param[in] aFmtIndex    The argument index of the format string.
 * @param[in] aStartIndex  The argument index of the first argument to check against the format string.
 *
 */

// =========== TOOLCHAIN SELECTION : START ===========

#if defined(__GNUC__) || defined(__clang__) || defined(__CC_ARM) || defined(__TI_ARM__)

// https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html
// http://www.keil.com/support/man/docs/armcc/armcc_chr1359124973480.htm

#define OT_TOOL_PACKED_BEGIN
#define OT_TOOL_PACKED_FIELD __attribute__((packed))
#define OT_TOOL_PACKED_END __attribute__((packed))
#define OT_TOOL_WEAK __attribute__((weak))

#define OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK(aFmtIndex, aStartIndex) \
    __attribute__((format(printf, aFmtIndex, aStartIndex)))

#elif defined(__ICCARM__) || defined(__ICC8051__)

// http://supp.iar.com/FilesPublic/UPDINFO/004916/arm/doc/EWARM_DevelopmentGuide.ENU.pdf

#include "intrinsics.h"

#define OT_TOOL_PACKED_BEGIN __packed
#define OT_TOOL_PACKED_FIELD
#define OT_TOOL_PACKED_END
#define OT_TOOL_WEAK __weak

#define OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK(aFmtIndex, aStartIndex)

#elif defined(__SDCC)

// Structures are packed by default in sdcc, as it primarily targets 8-bit MCUs.

#define OT_TOOL_PACKED_BEGIN
#define OT_TOOL_PACKED_FIELD
#define OT_TOOL_PACKED_END
#define OT_TOOL_WEAK

#define OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK(aFmtIndex, aStartIndex)

#else

#error "Error: No valid Toolchain specified"

// Symbols for Doxygen

#define OT_TOOL_PACKED_BEGIN
#define OT_TOOL_PACKED_FIELD
#define OT_TOOL_PACKED_END
#define OT_TOOL_WEAK

#define OT_TOOL_PRINTF_STYLE_FORMAT_ARG_CHECK(aFmtIndex, aStartIndex)

#endif

// =========== TOOLCHAIN SELECTION : END ===========

/**
 * @def OT_UNUSED_VARIABLE
 *
 * Suppress unused variable warning in specific toolchains.
 *
 */

/**
 * @def OT_UNREACHABLE_CODE
 *
 * Suppress Unreachable code warning in specific toolchains.
 *
 */

#if defined(__ICCARM__)

#include <stddef.h>

#define OT_UNUSED_VARIABLE(VARIABLE) \
    do                               \
    {                                \
        if (&VARIABLE == NULL)       \
        {                            \
        }                            \
    } while (false)

#define OT_UNREACHABLE_CODE(CODE)                                                                    \
    _Pragma("diag_suppress=Pe111") _Pragma("diag_suppress=Pe128") CODE _Pragma("diag_default=Pe111") \
        _Pragma("diag_default=Pe128")

#elif defined(__CC_ARM)

#include <stddef.h>

#define OT_UNUSED_VARIABLE(VARIABLE) \
    do                               \
    {                                \
        if (&VARIABLE == NULL)       \
        {                            \
        }                            \
    } while (false)

#define OT_UNREACHABLE_CODE(CODE) CODE

#elif defined(__TI_ARM__)

#include <stddef.h>

#define OT_UNUSED_VARIABLE(VARIABLE) \
    do                               \
    {                                \
        if (&VARIABLE == NULL)       \
        {                            \
        }                            \
    } while (false)

/*
 * #112-D statement is unreachable
 * #129-D loop is not reachable
 */
#define OT_UNREACHABLE_CODE(CODE) \
    _Pragma("diag_push") _Pragma("diag_suppress 112") _Pragma("diag_suppress 129") CODE _Pragma("diag_pop")

#else

#define OT_UNUSED_VARIABLE(VARIABLE) \
    do                               \
    {                                \
        (void)(VARIABLE);            \
    } while (false)

#define OT_UNREACHABLE_CODE(CODE) CODE

#endif

/*
 * Keil and IAR compiler doesn't provide type limits for C++.
 */
#ifdef __cplusplus
#if defined(__CC_ARM) || defined(__ICCARM__)

#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#endif

#endif
#endif

#ifdef __APPLE__
#define OT_APPLE_IGNORE_GNU_FOLDING_CONSTANT(...)                                               \
    _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wgnu-folding-constant\"") \
        __VA_ARGS__ _Pragma("GCC diagnostic pop")
#else
#define OT_APPLE_IGNORE_GNU_FOLDING_CONSTANT(...) __VA_ARGS__
#endif

/**
 * @def OT_FALL_THROUGH
 *
 * Suppress fall through warning in specific compiler.
 *
 */
#if defined(__cplusplus) && (__cplusplus >= 201703L)
#define OT_FALL_THROUGH [[fallthrough]]
#elif defined(__clang__)
#define OT_FALL_THROUGH [[clang::fallthrough]]
#elif defined(__GNUC__) && (__GNUC__ >= 7)
#define OT_FALL_THROUGH __attribute__((fallthrough))
#else
#define OT_FALL_THROUGH \
    do                  \
    {                   \
    } while (false) /* fallthrough */
#endif

/**
 * @}
 *
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OPENTHREAD_PLATFORM_TOOLCHAIN_H_
