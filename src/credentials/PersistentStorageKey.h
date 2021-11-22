/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <string.h>

namespace chip {
namespace Credentials {

class PersistentStorageKey
{
public:
    PersistentStorageKey() = default;

    const char * FabricTable(chip::FabricIndex fabric) { return Format("f/%u/t", fabric); }
    const char * FabricGroups(chip::FabricIndex fabric) { return Format("f/%u/g", fabric); }
    const char * FabricEndpoint(chip::FabricIndex fabric, chip::EndpointId endpoint)
    {
        return Format("f/%u/e/%u", fabric, endpoint);
    }
    const char * FabricEndpointGroup(chip::FabricIndex fabric, chip::EndpointId endpoint, chip::GroupId group)
    {
        return Format("f/%u/e/%u/g/%u", fabric, endpoint, group);
    }
    const char * GroupStates() { return Format("g/s"); }
    const char * GroupState(uint16_t state_index) { return Format("g/s/%u", state_index); }
    const char * FabricKeyset(chip::FabricIndex fabric, uint16_t keyset_id) { return Format("f/%u/k/%u", fabric, keyset_id); }
    const char * Value() { return mValue; }

private:
    static const size_t kKeyLengthMax = 32;

    const char * Format(const char * format...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(mValue, sizeof(mValue), format, args);
        va_end(args);
        return mValue;
    }

    char mValue[kKeyLengthMax] = { 0 };
};

} // namespace Credentials
} // namespace chip
