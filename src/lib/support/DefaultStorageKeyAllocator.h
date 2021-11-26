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

/**
 * This is the common key allocation policy for all classes using PersistentStorageDelegate storage
 */
class DefaultStorageKeyAllocator
{
public:
    DefaultStorageKeyAllocator() = default;

    // Fabric Table

    const char * FabricTable(chip::FabricIndex fabric) { return Format("f/%x/t", fabric); }

    // Group Data Provider

    const char * FabricGroups(chip::FabricIndex fabric) { return Format("f/%x/g", fabric); }
    const char * FabricEndpoint(chip::FabricIndex fabric, chip::EndpointId endpoint)
    {
        return Format("f/%x/e/%x", fabric, endpoint);
    }
    const char * FabricEndpointGroup(chip::FabricIndex fabric, chip::EndpointId endpoint, chip::GroupId group)
    {
        return Format("f/%x/e/%x/g/%x", fabric, endpoint, group);
    }
    const char * GroupStates() { return Format("g/s"); }
    const char * GroupState(uint16_t state_id) { return Format("g/s/%x", state_id); }
    const char * FabricKeySet(chip::FabricIndex fabric, uint16_t keyset_id) { return Format("f/%x/k/%x", fabric, keyset_id); }
    const char * KeyName() { return mKeyName; }

private:
    static const size_t kKeyLengthMax = 32;

    const char * Format(const char * format...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(mKeyName, sizeof(mKeyName), format, args);
        va_end(args);
        return mKeyName;
    }

    char mKeyName[kKeyLengthMax + 1] = { 0 };
};

} // namespace chip
