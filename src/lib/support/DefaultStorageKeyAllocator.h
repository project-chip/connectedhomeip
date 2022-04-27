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
    static constexpr size_t kKeyLengthMax = 32;

    DefaultStorageKeyAllocator() = default;
    const char * KeyName() const { return mKeyName; }

    // Fabric Table
    auto & FabricIndexInfo() { return Format("g/fidx"); }
    auto & FabricNOC(FabricIndex fabric) { return Format("f/%x/n", fabric); }
    auto & FabricICAC(FabricIndex fabric) { return Format("f/%x/i", fabric); }
    auto & FabricRCAC(FabricIndex fabric) { return Format("f/%x/r", fabric); }
    auto & FabricMetadata(FabricIndex fabric) { return Format("f/%x/m", fabric); }
    auto & FabricOpKey(FabricIndex fabric) { return Format("f/%x/o", fabric); }

    // FailSafeContext
    auto & FailSafeContextKey() { return Format("g/fs/c"); }
    auto & FailSafeNetworkConfig() { return Format("g/fs/n"); }

    // Session resumption
    auto & FabricSession(FabricIndex fabric, NodeId nodeId)
    {
        return Format("f/%x/s/%08" PRIX32 "%08" PRIX32, fabric, static_cast<uint32_t>(nodeId >> 32), static_cast<uint32_t>(nodeId));
    }
    auto & SessionResumptionIndex() { return Format("g/sri"); }
    auto & SessionResumption(const char * resumptionIdBase64) { return Format("g/s/%s", resumptionIdBase64); }

    // Access Control
    auto & AccessControlExtensionEntry(FabricIndex fabric) { return Format("f/%x/ac/1", fabric); }

    // TODO: We should probably store the fabric-specific parts of the ACL list
    // under keys starting with "f/%x/".
    auto & AccessControlList() { return Format("g/acl"); }
    auto & AccessControlEntry(size_t index)
    {
        // This cast will never overflow because the number of ACL entries will be low.
        return Format("g/acl/%x", static_cast<unsigned int>(index));
    }

    // Group Message Counters
    auto & GroupDataCounter() { return Format("g/gdc"); }
    auto & GroupControlCounter() { return Format("g/gcc"); }

    // Device Information Provider
    auto & UserLabelLengthKey(EndpointId endpoint) { return Format("g/userlbl/%x", endpoint); }
    auto & UserLabelIndexKey(EndpointId endpoint, uint32_t index) { return Format("g/userlbl/%x/%" PRIx32, endpoint, index); }

    // Group Data Provider

    // List of fabric indices that have endpoint-to-group associations defined.
    auto & GroupFabricList() { return Format("g/gfl"); }
    auto & FabricGroups(chip::FabricIndex fabric) { return Format("f/%x/g", fabric); }
    auto & FabricGroup(chip::FabricIndex fabric, chip::GroupId group) { return Format("f/%x/g/%x", fabric, group); }
    auto & FabricGroupKey(chip::FabricIndex fabric, uint16_t index) { return Format("f/%x/gk/%x", fabric, index); }
    auto & FabricGroupEndpoint(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId endpoint)
    {
        return Format("f/%x/g/%x/e/%x", fabric, group, endpoint);
    }
    auto & FabricKeyset(chip::FabricIndex fabric, uint16_t keyset) { return Format("f/%x/k/%x", fabric, keyset); }

    auto & AttributeValue(const app::ConcreteAttributePath & aPath)
    {
        // Needs at most 26 chars: 6 for "g/a///", 4 for the endpoint id, 8 each
        // for the cluster and attribute ids.
        return Format("g/a/%x/%" PRIx32 "/%" PRIx32, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    }

    // TODO: Should store fabric-specific parts of the binding list under keys
    // starting with "f/%x/".
    auto & BindingTable() { return Format("g/bt"); }
    auto & BindingTableEntry(uint8_t index) { return Format("g/bt/%x", index); }

    auto & OTADefaultProviders() { return Format("g/o/dp"); }
    auto & OTACurrentProvider() { return Format("g/o/cp"); }
    auto & OTAUpdateToken() { return Format("g/o/ut"); }
    auto & OTACurrentUpdateState() { return Format("g/o/us"); }
    auto & OTATargetVersion() { return Format("g/o/tv"); }

    // Event number counter.
    auto & IMEventNumber() { return Format("g/im/e"); }

private:
    // The ENFORCE_FORMAT args are "off by one" because this is a class method,
    // with an implicit "this" as first arg.
    const DefaultStorageKeyAllocator & ENFORCE_FORMAT(2, 3) Format(const char * format, ...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(mKeyName, sizeof(mKeyName), format, args);
        va_end(args);
        return *this;
    }

    char mKeyName[kKeyLengthMax + 1] = { 0 };
};

} // namespace chip
