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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace chip {

/**
 * This is the common key allocation policy for all classes using
 * PersistentStorageDelegate storage.
 *
 * Keys should have the following formats:
 *
 * * Keys that are not tied to a specific fabric: "g/....".
 * * Keys that are tied to a specific fabric: "f/%x/...." where the %x gets
 *   replaced by the fabric index.
 */
class DefaultStorageKeyAllocator
{
public:
    DefaultStorageKeyAllocator() = default;
    const char * KeyName() { return mKeyName; }

    // Fabric Table
    const char * FabricIndexInfo() { return SetConst("g/fidx"); }
    const char * FabricNOC(FabricIndex fabric) { return Format("f/%x/n", fabric); }
    const char * FabricICAC(FabricIndex fabric) { return Format("f/%x/i", fabric); }
    const char * FabricRCAC(FabricIndex fabric) { return Format("f/%x/r", fabric); }
    const char * FabricMetadata(FabricIndex fabric) { return Format("f/%x/m", fabric); }
    const char * FabricOpKey(FabricIndex fabric) { return Format("f/%x/o", fabric); }

    // Fail-safe handling
    const char * FailSafeCommitMarkerKey() { return SetConst("g/fs/c"); }
    const char * FailSafeNetworkConfig() { return SetConst("g/fs/n"); }

    // LastKnownGoodTime
    const char * LastKnownGoodTimeKey() { return SetConst("g/lkgt"); }

    // Session resumption
    const char * FabricSession(FabricIndex fabric, NodeId nodeId)
    {
        return Format("f/%x/s/%08" PRIX32 "%08" PRIX32, fabric, static_cast<uint32_t>(nodeId >> 32), static_cast<uint32_t>(nodeId));
    }
    const char * SessionResumptionIndex() { return SetConst("g/sri"); }
    const char * SessionResumption(const char * resumptionIdBase64) { return Format("g/s/%s", resumptionIdBase64); }

    // Access Control
    const char * AccessControlAclEntry(FabricIndex fabric, size_t index)
    {
        return Format("f/%x/ac/0/%x", fabric, static_cast<unsigned>(index));
    }
    const char * AccessControlExtensionEntry(FabricIndex fabric) { return Format("f/%x/ac/1", fabric); }

    // Group Message Counters
    const char * GroupDataCounter() { return SetConst("g/gdc"); }
    const char * GroupControlCounter() { return SetConst("g/gcc"); }

    // Device Information Provider
    const char * UserLabelLengthKey(EndpointId endpoint) { return Format("g/userlbl/%x", endpoint); }
    const char * UserLabelIndexKey(EndpointId endpoint, uint32_t index) { return Format("g/userlbl/%x/%" PRIx32, endpoint, index); }

    // Group Data Provider

    // List of fabric indices that have endpoint-to-group associations defined.
    const char * GroupFabricList() { return SetConst("g/gfl"); }
    const char * FabricGroups(chip::FabricIndex fabric) { return Format("f/%x/g", fabric); }
    const char * FabricGroup(chip::FabricIndex fabric, chip::GroupId group) { return Format("f/%x/g/%x", fabric, group); }
    const char * FabricGroupKey(chip::FabricIndex fabric, uint16_t index) { return Format("f/%x/gk/%x", fabric, index); }
    const char * FabricGroupEndpoint(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId endpoint)
    {
        return Format("f/%x/g/%x/e/%x", fabric, group, endpoint);
    }
    const char * FabricKeyset(chip::FabricIndex fabric, uint16_t keyset) { return Format("f/%x/k/%x", fabric, keyset); }

    const char * AttributeValue(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
    {
        // Needs at most 26 chars: 6 for "g/a///", 4 for the endpoint id, 8 each
        // for the cluster and attribute ids.
        return Format("g/a/%x/%" PRIx32 "/%" PRIx32, endpointId, clusterId, attributeId);
    }

    // TODO: Should store fabric-specific parts of the binding list under keys
    // starting with "f/%x/".
    const char * BindingTable() { return SetConst("g/bt"); }
    const char * BindingTableEntry(uint8_t index) { return Format("g/bt/%x", index); }

    const char * OTADefaultProviders() { return SetConst("g/o/dp"); }
    const char * OTACurrentProvider() { return SetConst("g/o/cp"); }
    const char * OTAUpdateToken() { return SetConst("g/o/ut"); }
    const char * OTACurrentUpdateState() { return SetConst("g/o/us"); }
    const char * OTATargetVersion() { return SetConst("g/o/tv"); }

    // Event number counter.
    const char * IMEventNumber() { return SetConst("g/im/ec"); }

protected:
    // The ENFORCE_FORMAT args are "off by one" because this is a class method,
    // with an implicit "this" as first arg.
    const char * ENFORCE_FORMAT(2, 3) Format(const char * format, ...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(mKeyNameBuffer, sizeof(mKeyNameBuffer), format, args);
        va_end(args);
        return mKeyName = mKeyNameBuffer;
    }

    const char * SetConst(const char * keyName) { return mKeyName = keyName; }

private:
    const char * mKeyName                                             = nullptr;
    char mKeyNameBuffer[PersistentStorageDelegate::kKeyLengthMax + 1] = { 0 };
};

} // namespace chip
