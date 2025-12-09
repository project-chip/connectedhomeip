/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ErrorUtils.h"

#include <PlatformError.h>

#include <app_preference.h>
#include <dns-sd.h>
#include <tizen.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

bool FormatTizenPlatformError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    if (!err.IsRange(ChipError::Range::kPlatform))
    {
        return false;
    }

    const char * desc = nullptr;
#if !CHIP_CONFIG_SHORT_ERROR_STR
    // The get_error_message() returns a pointer to a thread local storage. Subsequent
    // call will override it, however, FormatError() should consume it before the next
    // call to FormatTizenPlatformError() on the same thread.
    desc = get_error_message(static_cast<int>(err.GetValue()));
#endif

    FormatError(buf, bufSize, "Platform", err, desc);
    return true;
}

}; // namespace

void RegisterTizenPlatformErrorFormatter()
{
    static ErrorFormatter sTizenPlatformErrorFormatter = { FormatTizenPlatformError, nullptr };
    RegisterErrorFormatter(&sTizenPlatformErrorFormatter);
}

CHIP_ERROR TizenToChipError(int tizenError)
{
    switch (tizenError)
    {
    case TIZEN_ERROR_NONE:
        return CHIP_NO_ERROR;
    case TIZEN_ERROR_OUT_OF_MEMORY:
        return CHIP_ERROR_NO_MEMORY;
    default:
        return MATTER_PLATFORM_ERROR(tizenError);

    // Tizen DNSSD API errors
    case DNSSD_ERROR_NAME_CONFLICT:
        return CHIP_ERROR_MDNS_COLLISION;

    // Tizen Preference API errors
    case PREFERENCE_ERROR_NO_KEY:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
