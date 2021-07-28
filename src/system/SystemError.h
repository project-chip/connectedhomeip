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

extern CHIP_ERROR MapErrorPOSIX(int code);
extern const char * DescribeErrorPOSIX(CHIP_ERROR code);
extern void RegisterPOSIXErrorFormatter();
extern bool FormatPOSIXError(char * buf, uint16_t bufSize, CHIP_ERROR err);

#if __ZEPHYR__
extern CHIP_ERROR MapErrorZephyr(int code);
#endif // __ZEPHYR__

#if CHIP_SYSTEM_CONFIG_USE_LWIP

extern CHIP_ERROR MapErrorLwIP(err_t code);
extern const char * DescribeErrorLwIP(CHIP_ERROR code);
extern void RegisterLwIPErrorFormatter(void);
extern bool FormatLwIPError(char * buf, uint16_t bufSize, CHIP_ERROR err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

// clang-format off

} // namespace System
} // namespace chip

#endif // !defined(__cplusplus)
