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

#include <dns-sd.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR TizenToChipError(int tizenError)
{
    switch (tizenError)
    {
    case DNSSD_ERROR_NONE:
        return CHIP_NO_ERROR;
    case DNSSD_ERROR_NAME_CONFLICT:
        return CHIP_ERROR_MDNS_COLLISION;
    case DNSSD_ERROR_OUT_OF_MEMORY:
        return CHIP_ERROR_NO_MEMORY;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
