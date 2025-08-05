/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file contains declaration statements for CHIP Platform
 *      Layer-specific errors.
 */

#pragma once

// Include headers
#include <lib/core/CHIPError.h>

#ifdef __cplusplus

#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code, location)
#elif CHIP_CONFIG_ERROR_SOURCE
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code, __FILE__, __LINE__)
#else
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code)
#endif // CHIP_CONFIG_ERROR_SOURCE

namespace chip {
namespace Platform {

namespace Internal {
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
extern ::chip::ChipError MapPlatformError(int code, std::source_location location);
#elif CHIP_CONFIG_ERROR_SOURCE
extern ::chip::ChipError MapPlatformError(int code, const char * file, unsigned int line);
#else
extern ::chip::ChipError MapPlatformError(int code);
#endif
} // namespace Internal

extern const char * DescribePlatformError(CHIP_ERROR code);
extern void RegisterPlatformErrorFormatter();
extern bool FormatPlatformError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace Platform
} // namespace chip

#endif // ifdef __cplusplus
