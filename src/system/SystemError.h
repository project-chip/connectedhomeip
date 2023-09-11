/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include <lib/core/CHIPError.h>
#include <system/SystemConfig.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/err.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#ifdef __cplusplus

#if CHIP_CONFIG_ERROR_SOURCE
#define CHIP_ERROR_POSIX(code) chip::System::Internal::MapErrorPOSIX(code, __FILE__, __LINE__)
#else // CHIP_CONFIG_ERROR_SOURCE
#define CHIP_ERROR_POSIX(code) chip::System::Internal::MapErrorPOSIX(code)
#endif // CHIP_CONFIG_ERROR_SOURCE

namespace chip {
namespace System {

namespace Internal {
extern CHIP_ERROR MapErrorPOSIX(int code);
extern CHIP_ERROR MapErrorPOSIX(int code, const char * file, unsigned int line);
} // namespace Internal

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
