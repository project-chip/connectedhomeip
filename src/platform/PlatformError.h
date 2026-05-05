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
/**
 * Maps a platform-specific error code to a CHIP_ERROR.
 *
 * This function encapsulates platform error codes into the CHIP error system.
 * The resulting error will be in the kPlatformExtended range and can be formatted
 * using platform-specific error formatters.
 *
 * @note Each platform implementation is responsible for registering a platform
 *       error formatter (via RegisterPlatformErrorFormatter() or similar) to
 *       provide human-readable descriptions of platform-specific error codes.
 *       For example:
 *       - Silabs: RegisterSilabsPlatformErrorFormatter() in PlatformManagerImpl
 *       - Tizen: RegisterTizenPlatformErrorFormatter() in PlatformManagerImpl
 *
 * @param aError Platform-specific error code (0 indicates success)
 * @param location Source location (when CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION is enabled)
 * @return CHIP_ERROR in the kPlatformExtended range
 */
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
inline ::chip::ChipError MapPlatformError(int aError, std::source_location location)
{
    return CHIP_ERROR(ChipError::Range::kPlatformExtended, static_cast<ChipError::ValueType>(aError), location);
}
#elif CHIP_CONFIG_ERROR_SOURCE
inline ::chip::ChipError MapPlatformError(int aError, const char * file, unsigned int line)
{
    return CHIP_ERROR(ChipError::Range::kPlatformExtended, static_cast<ChipError::ValueType>(aError), file, line);
}
#else
inline ::chip::ChipError MapPlatformError(int aError)
{
    return CHIP_ERROR(ChipError::Range::kPlatformExtended, static_cast<ChipError::ValueType>(aError));
}
#endif

} // namespace Internal
} // namespace Platform
} // namespace chip
