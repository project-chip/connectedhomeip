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

#pragma once

#include <lib/core/CHIPError.h>

#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code, std::source_location::current())
#elif CHIP_CONFIG_ERROR_SOURCE
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code, __FILE__, __LINE__)
#else
#define MATTER_PLATFORM_ERROR(code) chip::Platform::Internal::MapPlatformError(code)
#endif // CHIP_CONFIG_ERROR_SOURCE

namespace chip {
namespace Platform {

namespace Internal {
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
::chip::ChipError MapPlatformError(int code, std::source_location location);
#elif CHIP_CONFIG_ERROR_SOURCE
::chip::ChipError MapPlatformError(int code, const char * file, unsigned int line);
#else
::chip::ChipError MapPlatformError(int code);
#endif

/**
 * This implements a function to return a NULL-terminated descriptive C string, associated with the specified, mapped
 * Platform error.
 *
 *  @param[in] aError  The mapped Platform-specific error to describe.
 *
 *  @return A NULL-terminated descriptive C string describing the error.
 */
const char * DescribePlatformError(CHIP_ERROR aError);

/**
 * @brief Registers the platform-specific error formatter.
 *
 * This function initializes and registers an error formatter that is responsible
 * for formatting platform-specific error codes. It ensures that errors originating
 * from the platform layer are properly formatted and reported by the error handling
 * system.
 *
 * This function is called during the initialization of the respective
 * platform layer implementation to ensure that platform-specific errors are
 * correctly handled and reported.
 */
void RegisterPlatformErrorFormatter();

/**
 * Given a platform error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] aError                The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not a Platform error.
 *
 */
bool FormatPlatformError(char * buf, uint16_t bufSize, CHIP_ERROR aError);

} // namespace Internal
} // namespace Platform
} // namespace chip
