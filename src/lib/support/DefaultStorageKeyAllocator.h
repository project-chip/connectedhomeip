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
#include <lib/support/CHIPMemString.h>

#include <stdio.h>

namespace chip {

/**
 * Represents a key used for addressing a specific storage element.
 *
 * May contain generic fixed keys (e.g. "g/fidx") or formatted fabric-specific
 * keys ("f/%x/..." where %x is the fabric index).
 */
class StorageKeyName
{
public:
    StorageKeyName(const StorageKeyName & other) = default;
    StorageKeyName & operator=(const StorageKeyName & other) = default;

    ~StorageKeyName() { memset(mKeyNameBuffer, 0, sizeof(mKeyNameBuffer)); }

    const char * KeyName() const { return mKeyNameBuffer; }

    bool IsInitialized() const { return mKeyNameBuffer[0] != 0; }
    bool IsUninitialized() const { return mKeyNameBuffer[0] == 0; }
    bool operator!() const { return IsUninitialized(); }

    static StorageKeyName FromConst(const char * value)
    {
        StorageKeyName result;
        Platform::CopyString(result.mKeyNameBuffer, value);
        return result;
    }

    static StorageKeyName ENFORCE_FORMAT(1, 2) Formatted(const char * format, ...)
    {
        StorageKeyName result;

        va_list args;
        va_start(args, format);
        vsnprintf(result.mKeyNameBuffer, sizeof(result.mKeyNameBuffer), format, args);
        va_end(args);

        return result;
    }

    // Explicit 0-filled key. MUST be initialized later
    static StorageKeyName Uninitialized()
    {
        StorageKeyName result;
        return result;
    }

private:
    // May only be created by the underlying constructor methods
    StorageKeyName() {}

    // Contains the storage for the key name because some strings may be formatted.
    char mKeyNameBuffer[PersistentStorageDelegate::kKeyLengthMax + 1] = { 0 };
};

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
private:
    DefaultStorageKeyAllocator() = default;

public:
    // Fabric Table
    static StorageKeyName FabricIndexInfo() { return StorageKeyName::FromConst("g/fidx"); }
    static StorageKeyName FabricNOC(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/n", fabric); }
    static StorageKeyName FabricICAC(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/i", fabric); }
    static StorageKeyName FabricRCAC(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/r", fabric); }
    static StorageKeyName FabricMetadata(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/m", fabric); }
    static StorageKeyName FabricOpKey(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/o", fabric); }

    // Fail-safe handling
    static StorageKeyName FailSafeCommitMarkerKey() { return StorageKeyName::FromConst("g/fs/c"); }
    static StorageKeyName FailSafeNetworkConfig() { return StorageKeyName::FromConst("g/fs/n"); }

    // LastKnownGoodTime
    static StorageKeyName LastKnownGoodTimeKey() { return StorageKeyName::FromConst("g/lkgt"); }

    // Session resumption
    static StorageKeyName FabricSession(FabricIndex fabric, NodeId nodeId)
    {
        return StorageKeyName::Formatted("f/%x/s/%08" PRIX32 "%08" PRIX32, fabric, static_cast<uint32_t>(nodeId >> 32),
                                         static_cast<uint32_t>(nodeId));
    }

    static StorageKeyName SessionResumptionIndex() { return StorageKeyName::FromConst("g/sri"); }
    static StorageKeyName SessionResumption(const char * resumptionIdBase64)
    {
        return StorageKeyName::Formatted("g/s/%s", resumptionIdBase64);
    }

    // Access Control
    static StorageKeyName AccessControlAclEntry(FabricIndex fabric, size_t index)
    {
        return StorageKeyName::Formatted("f/%x/ac/0/%x", fabric, static_cast<unsigned>(index));
    }

    static StorageKeyName AccessControlExtensionEntry(FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/ac/1", fabric); }

    // Group Message Counters
    static StorageKeyName GroupDataCounter() { return StorageKeyName::FromConst("g/gdc"); }
    static StorageKeyName GroupControlCounter() { return StorageKeyName::FromConst("g/gcc"); }

    // Device Information Provider
    static StorageKeyName UserLabelLengthKey(EndpointId endpoint) { return StorageKeyName::Formatted("g/userlbl/%x", endpoint); }
    static StorageKeyName UserLabelIndexKey(EndpointId endpoint, uint32_t index)
    {
        return StorageKeyName::Formatted("g/userlbl/%x/%" PRIx32, endpoint, index);
    }

    // Group Data Provider

    // List of fabric indices that have endpoint-to-group associations defined.
    static StorageKeyName GroupFabricList() { return StorageKeyName::FromConst("g/gfl"); }
    static StorageKeyName FabricGroups(chip::FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/g", fabric); }
    static StorageKeyName FabricGroup(chip::FabricIndex fabric, chip::GroupId group)
    {
        return StorageKeyName::Formatted("f/%x/g/%x", fabric, group);
    }
    static StorageKeyName FabricGroupKey(chip::FabricIndex fabric, uint16_t index)
    {
        return StorageKeyName::Formatted("f/%x/gk/%x", fabric, index);
    }
    static StorageKeyName FabricGroupEndpoint(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId endpoint)
    {
        return StorageKeyName::Formatted("f/%x/g/%x/e/%x", fabric, group, endpoint);
    }
    static StorageKeyName FabricKeyset(chip::FabricIndex fabric, uint16_t keyset)
    {
        return StorageKeyName::Formatted("f/%x/k/%x", fabric, keyset);
    }

    static StorageKeyName AttributeValue(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
    {
        // Needs at most 26 chars: 6 for "g/a///", 4 for the endpoint id, 8 each
        // for the cluster and attribute ids.
        return StorageKeyName::Formatted("g/a/%x/%" PRIx32 "/%" PRIx32, endpointId, clusterId, attributeId);
    }

    // TODO: Should store fabric-specific parts of the binding list under keys
    // starting with "f/%x/".
    static StorageKeyName BindingTable() { return StorageKeyName::FromConst("g/bt"); }
    static StorageKeyName BindingTableEntry(uint8_t index) { return StorageKeyName::Formatted("g/bt/%x", index); }

    // Client Monitoring

    static StorageKeyName ClientMonitoringTableEntry(chip::FabricIndex fabric)
    {
        return StorageKeyName::Formatted("f/%x/cm", fabric);
    }

    static StorageKeyName OTADefaultProviders() { return StorageKeyName::FromConst("g/o/dp"); }
    static StorageKeyName OTACurrentProvider() { return StorageKeyName::FromConst("g/o/cp"); }
    static StorageKeyName OTAUpdateToken() { return StorageKeyName::FromConst("g/o/ut"); }
    static StorageKeyName OTACurrentUpdateState() { return StorageKeyName::FromConst("g/o/us"); }
    static StorageKeyName OTATargetVersion() { return StorageKeyName::FromConst("g/o/tv"); }

    // Event number counter.
    static StorageKeyName IMEventNumber() { return StorageKeyName::FromConst("g/im/ec"); }

    // Subscription resumption
    static StorageKeyName SubscriptionResumption(size_t index)
    {
        return StorageKeyName::Formatted("g/su/%x", static_cast<unsigned>(index));
    }
    static StorageKeyName SubscriptionResumptionMaxCount() { return StorageKeyName::Formatted("g/sum"); }

    static StorageKeyName GlobalSceneCountKey() { return StorageKeyName::Formatted("g/scc"); }
    static StorageKeyName FabricSceneDataKey(chip::FabricIndex fabric) { return StorageKeyName::Formatted("f/%x/sc", fabric); }
    static StorageKeyName FabricSceneKey(chip::FabricIndex fabric, uint8_t id)
    {
        return StorageKeyName::Formatted("f/%x/sc/%x", fabric, id);
    }
};

} // namespace chip
