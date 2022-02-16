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

#include <app/ConcreteAttributePath.h>
#include <app/util/basic-types.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <string.h>

namespace chip {

/**
 * This is the common key allocation policy for all classes using PersistentStorageDelegate storage
 */
class DefaultStorageKeyAllocator
{
public:
    DefaultStorageKeyAllocator() = default;
    const char * KeyName() { return mKeyName; }

    // Fabric Table

    const char * FabricTable(chip::FabricIndex fabric) { return Format("f/%x/t", fabric); }

    // Access Control List

    const char * AccessControlList() { return Format("acl"); }
    const char * AccessControlEntry(size_t index)
    {
        // This cast will never overflow because the number of ACL entries will be low.
        return Format("acl/%x", static_cast<unsigned int>(index));
    }

    // Group Message Counters
    const char * GroupDataCounter() { return Format("gdc"); }
    const char * GroupControlCounter() { return Format("gcc"); }

    // Group Data Provider

    const char * FabricTable() { return Format("f/t"); }
    const char * FabricGroups(chip::FabricIndex fabric) { return Format("f/%x/g", fabric); }
    const char * FabricGroup(chip::FabricIndex fabric, chip::GroupId group) { return Format("f/%x/g/%x", fabric, group); }
    const char * FabricGroupKey(chip::FabricIndex fabric, uint16_t index) { return Format("f/%x/gk/%x", fabric, index); }
    const char * FabricGroupEndpoint(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId endpoint)
    {
        return Format("f/%x/g/%x/e/%x", fabric, group, endpoint);
    }
    const char * FabricKeyset(chip::FabricIndex fabric, uint16_t keyset) { return Format("f/%x/k/%x", fabric, keyset); }

    const char * AttributeValue(const app::ConcreteAttributePath & aPath)
    {
        // Needs at most 24 chars: 4 for "a///", 4 for the endpoint id, 8 each
        // for the cluster and attribute ids.
        return Format("a/%" PRIx16 "/%" PRIx32 "/%" PRIx32, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    }

private:
    static const size_t kKeyLengthMax = 32;

    // The ENFORCE_FORMAT args are "off by one" because this is a class method,
    // with an implicit "this" as first arg.
    const char * ENFORCE_FORMAT(2, 3) Format(const char * format, ...)
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
