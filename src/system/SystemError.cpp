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

#include <lib/core/ErrorStr.h>
#include <lib/support/DLLUtil.h>

#include <lib/core/CHIPConfig.h>

// Include local headers
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <limits>
#include <stddef.h>
#include <string.h>

namespace chip {
namespace System {

namespace Internal {
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
    return (aError == 0 ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPOSIX, static_cast<ChipError::ValueType>(aError)));
}

DLL_EXPORT CHIP_ERROR MapErrorPOSIX(int aError, const char * file, unsigned int line)
{
    return (aError == 0 ? CHIP_NO_ERROR
                        : CHIP_ERROR(ChipError::Range::kPOSIX, static_cast<ChipError::ValueType>(aError), file, line));
}
} // namespace Internal

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
    const int lError = static_cast<int>(aError.GetValue());
    return strerror(lError);
}

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
    if (err.IsRange(ChipError::Range::kPOSIX))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            nullptr;
#else
            DescribeErrorPOSIX(err);
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
    return Internal::MapErrorPOSIX(-aError);
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP

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
    static_assert(ChipError::CanEncapsulate(-std::numeric_limits<err_t>::min()), "Can't represent all LWIP errors");
    return (aError == ERR_OK ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kLwIP, static_cast<unsigned int>(-aError)));
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
    if (!aError.IsRange(ChipError::Range::kLwIP))
    {
        return nullptr;
    }

    const err_t lError = static_cast<err_t>(-static_cast<err_t>(aError.GetValue()));

    // If we are not compiling with LWIP_DEBUG asserted, the unmapped
    // local value may go unused.

    (void) lError;

    return lwip_strerr(lError);
}

/**
 * Register a text error formatter for LwIP errors.
 */
void RegisterLwIPErrorFormatter()
{
    static ErrorFormatter sLwIPErrorFormatter = { FormatLwIPError, nullptr };

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
    if (err.IsRange(ChipError::Range::kLwIP))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            nullptr;
#else
            DescribeErrorLwIP(err);
#endif
        chip::FormatError(buf, bufSize, "LwIP", err, desc);
        return true;
    }
    return false;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace chip
