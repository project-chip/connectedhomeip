/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file contains declaration statements for Weave System
 *      Layer-specific errors.
 *
 *      Error types, ranges, and mappings overrides may be made by
 *      defining the appropriate WEAVE_SYSTEM_CONFIG_* or *
 *      _WEAVE_SYSTEM_CONFIG_* macros.
 *
 *      NOTE WELL: On some platforms, this header is included by
 *      C-language programs.
 */

#ifndef SYSTEMERROR_H
#define SYSTEMERROR_H

// Include headers
#include <SystemConfig.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

/**
 *  @def WEAVE_SYSTEM_NO_ERROR
 *
 *  @brief
 *      This macro expands to a constant expression representing the error code for success or no error. Its definition may be
 *      configured via the #WEAVE_SYSTEM_CONFIG_NO_ERROR configuration variable.
 */
#define WEAVE_SYSTEM_NO_ERROR WEAVE_SYSTEM_CONFIG_NO_ERROR

/**
 *  @def WEAVE_SYSTEM_CONFIG_ERROR_MIN
 *
 *  @brief
 *      This macro expands to a constant expression representing the minimum code number for error conditions in the Weave System
 *      Layer. This value may be configured via #WEAVE_SYSTEM_CONFIG_ERROR_MIN.
 */
#ifdef WEAVE_SYSTEM_CONFIG_ERROR_MIN
#define WEAVE_SYSTEM_ERROR_MIN WEAVE_SYSTEM_CONFIG_ERROR_MIN
#endif // WEAVE_SYSTEM_CONFIG_ERROR_MIN

/**
 *  @def WEAVE_SYSTEM_CONFIG_ERROR_MAX
 *
 *  @brief
 *      This macro expands to a constant expression representing the maximum code number for error conditions in the Weave System
 *      Layer. This value may be configured via #WEAVE_SYSTEM_CONFIG_ERROR_MAX.
 */
#ifdef WEAVE_SYSTEM_CONFIG_ERROR_MAX
#define WEAVE_SYSTEM_ERROR_MAX WEAVE_SYSTEM_CONFIG_ERROR_MAX
#endif // WEAVE_SYSTEM_CONFIG_ERROR_MAX

#ifdef _WEAVE_SYSTEM_CONFIG_ERROR

/**
 *  @def _WEAVE_SYSTEM_ERROR(e)
 *
 *  @brief
 *      This defines a mapping function for Weave System Layer errors that allows mapping such errors into a platform- or
 *      system-specific range. This function may be configured via #_WEAVE_SYSTEM_CONFIG_ERROR(e).
 *
 *  @param[in]  e  The Weave System Layer error to map.
 *
 *  @return The mapped Weave System Layer error.
 */
#define _WEAVE_SYSTEM_ERROR(e) _WEAVE_SYSTEM_CONFIG_ERROR(e)

/**
 *  @name Error Definitions
 *
 *  @{
 */

/**
 *  @def WEAVE_SYSTEM_ERROR_NOT_IMPLEMENTED
 *
 *  @brief
 *      A requested function has not been implemented.
 */
#define WEAVE_SYSTEM_ERROR_NOT_IMPLEMENTED _WEAVE_SYSTEM_ERROR(0)

/**
 *  @def WEAVE_SYSTEM_ERROR_NOT_SUPPORTED
 *
 *  @brief
 *      A requested function or feature is not supported.
 */
#define WEAVE_SYSTEM_ERROR_NOT_SUPPORTED _WEAVE_SYSTEM_ERROR(1)

/**
 *  @def WEAVE_SYSTEM_ERROR_BAD_ARGS
 *
 *  @brief
 *      An invalid argument or arguments were supplied.
 */
#define WEAVE_SYSTEM_ERROR_BAD_ARGS _WEAVE_SYSTEM_ERROR(2)

/**
 *  @def WEAVE_SYSTEM_ERROR_UNEXPECTED_STATE
 *
 *  @brief
 *      An unexpected state was encountered.
 */
#define WEAVE_SYSTEM_ERROR_UNEXPECTED_STATE _WEAVE_SYSTEM_ERROR(3)

/**
 *  @def WEAVE_SYSTEM_ERROR_UNEXPECTED_EVENT
 *
 *  @brief
 *      An unexpected event was encountered.
 */
#define WEAVE_SYSTEM_ERROR_UNEXPECTED_EVENT _WEAVE_SYSTEM_ERROR(4)

/**
 *  @def WEAVE_SYSTEM_ERROR_NO_MEMORY
 *
 *  @brief
 *      A request for memory could not be fulfilled.
 */
#define WEAVE_SYSTEM_ERROR_NO_MEMORY _WEAVE_SYSTEM_ERROR(5)

/**
 *  @def WEAVE_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED
 *
 *  @brief
 *      The system's real time clock is not synchronized to an accurate time source.
 */
#define WEAVE_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED _WEAVE_SYSTEM_ERROR(6)

/**
 *  @def WEAVE_SYSTEM_ERROR_ACCESS_DENIED
 *
 *  @brief
 *      The requestor is not privileged to perform the requested operation.
 */
#define WEAVE_SYSTEM_ERROR_ACCESS_DENIED _WEAVE_SYSTEM_ERROR(7)

//                        !!!!! IMPORTANT !!!!!
//
// If you add new Weave System Layer errors, please update the translation of error
// codes to strings in SystemError.cpp, and add them to unittest in test-apps/TestErrorStr.cpp

#endif // _WEAVE_SYSTEM_CONFIG_ERROR

/**
 *  @}
 */


#ifdef __cplusplus

namespace nl {
namespace Weave {
namespace System {

/**
 *  The basic type for all nl::Weave::System errors.
 *
 *  This is defined to a platform- or system-specific type.
 */
typedef WEAVE_SYSTEM_CONFIG_ERROR_TYPE Error;

extern void RegisterSystemLayerErrorFormatter(void);
extern bool FormatSystemLayerError(char * buf, uint16_t bufSize, int32_t err);

extern Error MapErrorPOSIX(int code);
extern const char* DescribeErrorPOSIX(Error code);
extern bool IsErrorPOSIX(Error code);
extern void RegisterPOSIXErrorFormatter(void);
extern bool FormatPOSIXError(char * buf, uint16_t bufSize, int32_t err);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

extern Error MapErrorLwIP(err_t code);
extern const char* DescribeErrorLwIP(Error code);
extern bool IsErrorLwIP(Error code);
extern void RegisterLwIPErrorFormatter(void);
extern bool FormatLwIPError(char * buf, uint16_t bufSize, int32_t err);

#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace Weave
} // namespace nl

#endif // !defined(__cplusplus)
#endif // defined(SYSTEMERROR_H)
