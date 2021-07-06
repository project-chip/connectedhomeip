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

#pragma once

// Include headers
#include <core/CHIPError.h>
#include <system/SystemConfig.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#ifdef __cplusplus

namespace chip {
namespace System {

extern void RegisterLayerErrorFormatter();
extern bool FormatLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

extern CHIP_ERROR MapErrorPOSIX(int code);
extern const char * DescribeErrorPOSIX(CHIP_ERROR code);
extern bool IsErrorPOSIX(CHIP_ERROR code);
extern void RegisterPOSIXErrorFormatter();
extern bool FormatPOSIXError(char * buf, uint16_t bufSize, CHIP_ERROR err);
extern CHIP_ERROR MapErrorZephyr(int code);

#if CHIP_SYSTEM_CONFIG_USE_LWIP

extern CHIP_ERROR MapErrorLwIP(err_t code);
extern const char * DescribeErrorLwIP(CHIP_ERROR code);
extern bool IsErrorLwIP(CHIP_ERROR code);
extern void RegisterLwIPErrorFormatter(void);
extern bool FormatLwIPError(char * buf, uint16_t bufSize, CHIP_ERROR err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

// clang-format off

// !!!!! IMPORTANT !!!!!
// These definitions are present temporarily in order to reduce breakage for PRs in flight.
// TODO: remove compatibility definitions
using Error = CHIP_ERROR;
#define CHIP_SYSTEM_NO_ERROR                    CHIP_NO_ERROR
#define CHIP_SYSTEM_ERROR_ACCESS_DENIED         CHIP_ERROR_ACCESS_DENIED
#define CHIP_SYSTEM_ERROR_BAD_ARGS              CHIP_ERROR_INVALID_ARGUMENT
#define CHIP_SYSTEM_ERROR_NOT_SUPPORTED         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
#define CHIP_SYSTEM_ERROR_NO_MEMORY             CHIP_ERROR_NO_MEMORY
#define CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED  CHIP_ERROR_REAL_TIME_NOT_SYNCED
#define CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT      CHIP_ERROR_UNEXPECTED_EVENT
#define CHIP_SYSTEM_ERROR_UNEXPECTED_STATE      CHIP_ERROR_INCORRECT_STATE

// clang-format on

} // namespace System
} // namespace chip

#endif // !defined(__cplusplus)
