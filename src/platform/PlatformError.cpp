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
 *      This file contains free functions for mapping Platform
 *      specific errors into CHIP errors
 */

#include <lib/core/CHIPConfig.h>
#include <platform/PlatformError.h>

#include <lib/core/ErrorStr.h>

#include <string.h>

namespace chip {
namespace Platform {

namespace Internal {
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
::chip::ChipError MapPlatformError(int aError, std::source_location location)
{
    return (aError == 0 ? CHIP_NO_ERROR
                        : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError), location));
}
#elif CHIP_CONFIG_ERROR_SOURCE
::chip::ChipError MapPlatformError(int aError, const char * file, unsigned int line)
{
    return (aError == 0 ? CHIP_NO_ERROR
                        : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError), file, line));
}
#else
::chip::ChipError MapPlatformError(int aError)
{
    return (aError == 0 ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError)));
}
#endif

/**
 * This implements a function to return an NULL-terminated descriptive C string, associated with the specified, mapped
 * Platform error.
 *
 *  @param[in] aError  The mapped Platform-specific error to describe.
 *
 *  @return A NULL-terminated descriptive C string describing the error.
 */
const char * DescribePlatformError(CHIP_ERROR aError)
{
    const int lError = static_cast<int>(aError.GetValue());
    return strerror(lError);
}

/**
 * Register a text error formatter for Platform errors.
 */
void RegisterPlatformErrorFormatter()
{
    static ErrorFormatter sPlatformErrorFormatter = { FormatPlatformError, nullptr };

    RegisterErrorFormatter(&sPlatformErrorFormatter);
}

/**
 * Given a platform error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not a Platform error.
 *
 */
bool FormatPlatformError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    if (err.IsRange(ChipError::Range::kPlatform))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            nullptr;
#else
            DescribePlatformError(err);
#endif
        FormatError(buf, bufSize, "Platform", err, desc);
        return true;
    }

    return false;
}

} // namespace Internal
} // namespace Platform
} // namespace chip
