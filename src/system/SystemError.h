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
 *      This file contains declaration statements for CHIP System
 *      Layer-specific errors.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate CHIP_SYSTEM_CONFIG_* or *
 *      _CHIP_SYSTEM_CONFIG_* macros.
 *
 *      NOTE WELL: On some platforms, this header is included by
 *      C-language programs.
 */

#ifndef SYSTEMERROR_H
#define SYSTEMERROR_H

// Include headers
#include <system/SystemConfig.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  @def CHIP_SYSTEM_NO_ERROR
 *
 *  @brief
 *      This macro expands to a constant expression representing the error code for success or no error. Its definition may be
 *      configured via the #CHIP_SYSTEM_CONFIG_NO_ERROR configuration variable.
 */
#define CHIP_SYSTEM_NO_ERROR CHIP_SYSTEM_CONFIG_NO_ERROR

/**
 *  @def CHIP_SYSTEM_CONFIG_ERROR_MIN
 *
 *  @brief
 *      This macro expands to a constant expression representing the minimum code number for error conditions in the CHIP System
 *      Layer. This value may be configured via #CHIP_SYSTEM_CONFIG_ERROR_MIN.
 */
#ifdef CHIP_SYSTEM_CONFIG_ERROR_MIN
#define CHIP_SYSTEM_ERROR_MIN CHIP_SYSTEM_CONFIG_ERROR_MIN
#endif // CHIP_SYSTEM_CONFIG_ERROR_MIN

/**
 *  @def CHIP_SYSTEM_CONFIG_ERROR_MAX
 *
 *  @brief
 *      This macro expands to a constant expression representing the maximum code number for error conditions in the CHIP System
 *      Layer. This value may be configured via #CHIP_SYSTEM_CONFIG_ERROR_MAX.
 */
#ifdef CHIP_SYSTEM_CONFIG_ERROR_MAX
#define CHIP_SYSTEM_ERROR_MAX CHIP_SYSTEM_CONFIG_ERROR_MAX
#endif // CHIP_SYSTEM_CONFIG_ERROR_MAX

#ifdef _CHIP_SYSTEM_CONFIG_ERROR

/**
 *  @def _CHIP_SYSTEM_ERROR(e)
 *
 *  @brief
 *      This defines a mapping function for CHIP System Layer errors that allows mapping such errors into a platform- or
 *      system-specific range. This function may be configured via #_CHIP_SYSTEM_CONFIG_ERROR(e).
 *
 *  @param[in]  e  The CHIP System Layer error to map.
 *
 *  @return The mapped CHIP System Layer error.
 */
#define _CHIP_SYSTEM_ERROR(e) _CHIP_SYSTEM_CONFIG_ERROR(e)

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def CHIP_SYSTEM_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *      A requested function has not been implemented.
 */
#define CHIP_SYSTEM_ERROR_NOT_IMPLEMENTED _CHIP_SYSTEM_ERROR(0)

/**
 *  @def CHIP_SYSTEM_ERROR_NOT_SUPPORTED
 *
 *  @brief
 *      A requested function or feature is not supported.
 */
#define CHIP_SYSTEM_ERROR_NOT_SUPPORTED _CHIP_SYSTEM_ERROR(1)

/**
 *  @def CHIP_SYSTEM_ERROR_BAD_ARGS
 *
 *  @brief
 *      An invalid argument or arguments were supplied.
 */
#define CHIP_SYSTEM_ERROR_BAD_ARGS _CHIP_SYSTEM_ERROR(2)

/**
 *  @def CHIP_SYSTEM_ERROR_UNEXPECTED_STATE
 *
 *  @brief
 *      An unexpected state was encountered.
 */
#define CHIP_SYSTEM_ERROR_UNEXPECTED_STATE _CHIP_SYSTEM_ERROR(3)

/**
 *  @def CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT
 *
 *  @brief
 *      An unexpected event was encountered.
 */
#define CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT _CHIP_SYSTEM_ERROR(4)

/**
 *  @def CHIP_SYSTEM_ERROR_NO_MEMORY
 *
 *  @brief
 *      A request for memory could not be fulfilled.
 */
#define CHIP_SYSTEM_ERROR_NO_MEMORY _CHIP_SYSTEM_ERROR(5)

/**
 *  @def CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED
 *
 *  @brief
 *      The system's real time clock is not synchronized to an accurate time source.
 */
#define CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED _CHIP_SYSTEM_ERROR(6)

/**
 *  @def CHIP_SYSTEM_ERROR_ACCESS_DENIED
 *
 *  @brief
 *      The requestor is not privileged to perform the requested operation.
 */
#define CHIP_SYSTEM_ERROR_ACCESS_DENIED _CHIP_SYSTEM_ERROR(7)

//                        !!!!! IMPORTANT !!!!!
//
// If you add new CHIP System Layer errors, please update the translation of error
// codes to strings in SystemError.cpp, and add them to unittest in test-apps/TestErrorStr.cpp

#endif // _CHIP_SYSTEM_CONFIG_ERROR

/**
 *  @}
 */

#ifdef __cplusplus

namespace chip {
namespace System {

/**
 *  The basic type for all chip::System errors.
 *
 *  This is defined to a platform- or system-specific type.
 */
typedef CHIP_SYSTEM_CONFIG_ERROR_TYPE Error;

extern void RegisterLayerErrorFormatter(void);
extern bool FormatLayerError(char * buf, uint16_t bufSize, int32_t err);

extern Error MapErrorPOSIX(int code);
extern const char * DescribeErrorPOSIX(Error code);
extern bool IsErrorPOSIX(Error code);
extern void RegisterPOSIXErrorFormatter(void);
extern bool FormatPOSIXError(char * buf, uint16_t bufSize, int32_t err);
extern Error MapErrorZephyr(int code);

#if CHIP_SYSTEM_CONFIG_USE_LWIP

extern Error MapErrorLwIP(err_t code);
extern const char * DescribeErrorLwIP(Error code);
extern bool IsErrorLwIP(Error code);
extern void RegisterLwIPErrorFormatter(void);
extern bool FormatLwIPError(char * buf, uint16_t bufSize, int32_t err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace chip

#endif // !defined(__cplusplus)
#endif // defined(SYSTEMERROR_H)
