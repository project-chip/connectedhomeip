/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file contains free functions for mapping OS and LwIP
 *      stack-specific errors into CHIP System Layer-specific errors
 *      and for converting those mapped errors into descriptive
 *      error strings.
 */

// Include module header
#include <system/SystemError.h>

// Include common private header
#include "SystemLayerPrivate.h"

#include <support/DLLUtil.h>
#include <support/ErrorStr.h>

#include <core/CHIPConfig.h>

// Include local headers
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS
#include <string.h>
#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS

#include <stddef.h>

#include <limits>

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS

/**
 *  @def CHIP_SYSTEM_POSIX_ERROR_MIN
 *
 *  @brief
 *      This defines the base or minimum CHIP System Layer error number range, when passing through errors from an underlying
 *      POSIX layer.
 */
#define CHIP_SYSTEM_POSIX_ERROR_MIN 2000

/**
 *  @def CHIP_SYSTEM_POSIX_ERROR_MAX
 *
 *  @brief
 *      This defines the base or maximum CHIP System Layer error number range, when passing through errors from an underlying
 *      POSIX layer.
 */
#define CHIP_SYSTEM_POSIX_ERROR_MAX 2999

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS

/**
 *  @def CHIP_SYSTEM_LWIP_ERROR_MIN
 *
 *  @brief
 *      This defines the base or minimum CHIP System Layer error number range, when passing through errors from an underlying LWIP
 *      stack.
 */
#ifndef CHIP_SYSTEM_LWIP_ERROR_MIN
#define CHIP_SYSTEM_LWIP_ERROR_MIN 3000
#endif // CHIP_SYSTEM_LWIP_ERROR_MIN

/**
 *  @def CHIP_SYSTEM_LWIP_ERROR_MAX
 *
 *  @brief
 *      This defines the base or maximum CHIP System Layer error number range, when passing through errors from an underlying LWIP
 *      layer.
 */
#ifndef CHIP_SYSTEM_LWIP_ERROR_MAX
#define CHIP_SYSTEM_LWIP_ERROR_MAX 3128
#endif // CHIP_SYSTEM_LWIP_ERROR_MAX

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace System {

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS
/**
 * This implements a mapping function for CHIP System Layer errors that allows mapping integers in the number space of the
 * underlying POSIX network and OS stack errors into a platform- or system-specific range. Error codes beyond those currently
 * defined by POSIX or the ISO C/C++ standards are mapped similar to the standard ones.
 *
 *  @param[in] aError  The POSIX network or OS error to map.
 *
 *  @return The mapped POSIX network or OS error.
 */
DLL_EXPORT CHIP_ERROR MapErrorPOSIX(int aError)
{
    return (aError == 0 ? CHIP_NO_ERROR : static_cast<CHIP_ERROR>(CHIP_SYSTEM_POSIX_ERROR_MIN + aError));
}

/**
 * This implements a function to return an NULL-terminated OS-specific descriptive C string, associated with the specified, mapped
 * OS error.
 *
 *  @param[in] aError  The mapped OS-specific error to describe.
 *
 *  @return A NULL-terminated, OS-specific descriptive C string describing the error.
 */
DLL_EXPORT const char * DescribeErrorPOSIX(CHIP_ERROR aError)
{
    const int lError = (static_cast<int>(aError) - CHIP_SYSTEM_POSIX_ERROR_MIN);
    return strerror(lError);
}

/**
 * This implements an introspection function for CHIP System Layer errors that allows the caller to determine whether the
 * specified error is an internal, underlying OS error.
 *
 *  @param[in] aError  The mapped error to determine whether it is an OS error.
 *
 *  @return True if the specified error is an OS error; otherwise, false.
 */
DLL_EXPORT bool IsErrorPOSIX(CHIP_ERROR aError)
{
    return (aError >= CHIP_SYSTEM_POSIX_ERROR_MIN && aError <= CHIP_SYSTEM_POSIX_ERROR_MAX);
}

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS

/**
 * Register a text error formatter for POSIX errors.
 */
void RegisterPOSIXErrorFormatter()
{
    static ErrorFormatter sPOSIXErrorFormatter = { FormatPOSIXError, nullptr };

    RegisterErrorFormatter(&sPOSIXErrorFormatter);
}

/**
 * Given a POSIX error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not a POSIX error.
 *
 */
bool FormatPOSIXError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const CHIP_ERROR sysErr = static_cast<CHIP_ERROR>(err);

    if (IsErrorPOSIX(sysErr))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            NULL;
#else
            DescribeErrorPOSIX(sysErr);
#endif
        FormatError(buf, bufSize, "OS", err, desc);
        return true;
    }

    return false;
}

/**
 * This implements a mapping function for CHIP System Layer errors that allows mapping integers in the number space of the
 * Zephyr OS user API stack errors into the POSIX range.
 *
 *  @param[in] aError  The native Zephyr API error to map.
 *
 *  @return The mapped POSIX error.
 */
DLL_EXPORT CHIP_ERROR MapErrorZephyr(int aError)
{
    return MapErrorPOSIX(-aError);
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS

/**
 * This implements a mapping function for CHIP System Layer errors that allows mapping underlying LwIP network stack errors into a
 * platform- or system-specific range.
 *
 *  @param[in] aError  The LwIP error to map.
 *
 *  @return The mapped LwIP network or OS error.
 *
 */
DLL_EXPORT CHIP_ERROR MapErrorLwIP(err_t aError)
{
    static_assert(std::numeric_limits<err_t>::min() == CHIP_SYSTEM_LWIP_ERROR_MIN - CHIP_SYSTEM_LWIP_ERROR_MAX,
                  "Can't represent all LWIP errors");
    return (aError == ERR_OK ? CHIP_NO_ERROR : CHIP_SYSTEM_LWIP_ERROR_MIN - aError);
}

/**
 * This implements a function to return an NULL-terminated LwIP-specific descriptive C string, associated with the specified,
 * mapped LwIP error.
 *
 *  @param[in] aError  The mapped LwIP-specific error to describe.
 *
 *  @return A NULL-terminated, LwIP-specific descriptive C string describing the error.
 *
 */
DLL_EXPORT const char * DescribeErrorLwIP(CHIP_ERROR aError)
{
    if (!IsErrorLwIP(aError))
    {
        return nullptr;
    }

    // Error might be a signed or unsigned type.  But we know the value is no
    // larger than CHIP_SYSTEM_LWIP_ERROR_MAX and that this means it's safe to
    // store in int.
    static_assert(INT_MAX > CHIP_SYSTEM_LWIP_ERROR_MAX, "Our subtraction will fail");
    const int lErrorWithoutOffset = aError - CHIP_SYSTEM_LWIP_ERROR_MIN;
    // Cast is safe because the range from CHIP_SYSTEM_LWIP_ERROR_MIN to
    // CHIP_SYSTEM_LWIP_ERROR_MAX all fits inside err_t.  See static_assert in
    // MapErrorLwIP.
    const err_t lError = static_cast<err_t>(-lErrorWithoutOffset);

    // If we are not compiling with LWIP_DEBUG asserted, the unmapped
    // local value may go unused.

    (void) lError;

    return lwip_strerr(lError);
}

/**
 * This implements an introspection function for CHIP System Layer errors that
 * allows the caller to determine whether the specified error is an
 * internal, underlying LwIP error.
 *
 *  @param[in] aError  The mapped error to determine whether it is a LwIP error.
 *
 *  @return True if the specified error is a LwIP error; otherwise, false.
 *
 */
DLL_EXPORT bool IsErrorLwIP(CHIP_ERROR aError)
{
    return (aError >= CHIP_SYSTEM_LWIP_ERROR_MIN && aError <= CHIP_SYSTEM_LWIP_ERROR_MAX);
}

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS

/**
 * Register a text error formatter for LwIP errors.
 */
void RegisterLwIPErrorFormatter(void)
{
    static ErrorFormatter sLwIPErrorFormatter = { FormatLwIPError, NULL };

    RegisterErrorFormatter(&sLwIPErrorFormatter);
}

/**
 * Given an LwIP error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not an LwIP error.
 *
 */
bool FormatLwIPError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const CHIP_ERROR sysErr = static_cast<CHIP_ERROR>(err);

    if (IsErrorLwIP(sysErr))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            NULL;
#else
            DescribeErrorLwIP(sysErr);
#endif
        chip::FormatError(buf, bufSize, "LwIP", err, desc);
        return true;
    }
    else
    {
        return false;
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace chip
