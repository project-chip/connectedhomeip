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

#include <app_preference.h>
#include <dns-sd.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR TizenToChipError(int tizenError)
{
    switch (tizenError)
    {
    case TIZEN_ERROR_NONE:
        return CHIP_NO_ERROR;
    case TIZEN_ERROR_OUT_OF_MEMORY:
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INTERNAL;

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
