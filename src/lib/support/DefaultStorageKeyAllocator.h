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

/*
#include <lib/core/GroupId.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
*/

namespace chip {

/**
 * Represents a key used for addressing a specific storage element.
 *
 * May contain generic fixed keys (e.g. "g/fidx") or formatted fabric-specific
 * keys ("f/%x/..." where %x is the fabric index).
 */
class StorageKey
{
public:
    StorageKey(const StorageKey & other)             = default;
    StorageKey & operator=(const StorageKey & other) = default;

    ~StorageKey() { memset(mKeyNameBuffer, 0, sizeof(mKeyNameBuffer)); }

    const char * KeyName() const { return mKeyNameBuffer; }

    bool IsUninitialized() const { return mKeyNameBuffer[0] == 0; }
    bool operator!() const { return IsUninitialized(); }

    operator const char *() const { return mKeyNameBuffer; }

    static StorageKey FromConst(const char * value)
    {
        StorageKey result;
        Platform::CopyString(result.mKeyNameBuffer, value);
        return result;
    }

    static StorageKey ENFORCE_FORMAT(1, 2) Formatted(const char * format, ...)
    {
        StorageKey result;

        va_list args;
        va_start(args, format);
        vsnprintf(result.mKeyNameBuffer, sizeof(result.mKeyNameBuffer), format, args);
        va_end(args);

        return result;
    }

    // Explicit 0-filled key. MUST be initialized later
    static StorageKey Uninitialized()
    {
        StorageKey result;
        return result;
    }

private:
    // May only be created by the underlying constructor methods
    StorageKey() {}

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
    static StorageKey FabricIndexInfo() { return StorageKey::FromConst("g/fidx"); }
    static StorageKey FabricNOC(FabricIndex fabric) { return StorageKey::Formatted("f/%x/n", fabric); }
    static StorageKey FabricICAC(FabricIndex fabric) { return StorageKey::Formatted("f/%x/i", fabric); }
    static StorageKey FabricRCAC(FabricIndex fabric) { return StorageKey::Formatted("f/%x/r", fabric); }
    static StorageKey FabricMetadata(FabricIndex fabric) { return StorageKey::Formatted("f/%x/m", fabric); }
    static StorageKey FabricOpKey(FabricIndex fabric) { return StorageKey::Formatted("f/%x/o", fabric); }

    // Fail-safe handling
    static StorageKey FailSafeCommitMarkerKey() { return StorageKey::FromConst("g/fs/c"); }
    static StorageKey FailSafeNetworkConfig() { return StorageKey::FromConst("g/fs/n"); }

    // LastKnownGoodTime
    static StorageKey LastKnownGoodTimeKey() { return StorageKey::FromConst("g/lkgt"); }

    // Session resumption
    static StorageKey FabricSession(FabricIndex fabric, NodeId nodeId)
    {
        return StorageKey::Formatted("f/%x/s/%08" PRIX32 "%08" PRIX32, fabric, static_cast<uint32_t>(nodeId >> 32),
                                     static_cast<uint32_t>(nodeId));
    }

    static StorageKey SessionResumptionIndex() { return StorageKey::FromConst("g/sri"); }
    static StorageKey SessionResumption(const char * resumptionIdBase64)
    {
        return StorageKey::Formatted("g/s/%s", resumptionIdBase64);
    }

    // Access Control
    static StorageKey AccessControlAclEntry(FabricIndex fabric, size_t index)
    {
        return StorageKey::Formatted("f/%x/ac/0/%x", fabric, static_cast<unsigned>(index));
    }

    static StorageKey AccessControlExtensionEntry(FabricIndex fabric) { return StorageKey::Formatted("f/%x/ac/1", fabric); }

    // Group Message Counters
    static StorageKey GroupDataCounter() { return StorageKey::FromConst("g/gdc"); }
    static StorageKey GroupControlCounter() { return StorageKey::FromConst("g/gcc"); }

    // Device Information Provider
    static StorageKey UserLabelLengthKey(EndpointId endpoint) { return StorageKey::Formatted("g/userlbl/%x", endpoint); }
    static StorageKey UserLabelIndexKey(EndpointId endpoint, uint32_t index)
    {
        return StorageKey::Formatted("g/userlbl/%x/%" PRIx32, endpoint, index);
    }

    // Group Data Provider

    // List of fabric indices that have endpoint-to-group associations defined.
    static StorageKey GroupFabricList() { return StorageKey::FromConst("g/gfl"); }
    static StorageKey FabricGroups(chip::FabricIndex fabric) { return StorageKey::Formatted("f/%x/g", fabric); }
    static StorageKey FabricGroup(chip::FabricIndex fabric, chip::GroupId group)
    {
        return StorageKey::Formatted("f/%x/g/%x", fabric, group);
    }
    static StorageKey FabricGroupKey(chip::FabricIndex fabric, uint16_t index)
    {
        return StorageKey::Formatted("f/%x/gk/%x", fabric, index);
    }
    static StorageKey FabricGroupEndpoint(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId endpoint)
    {
        return StorageKey::Formatted("f/%x/g/%x/e/%x", fabric, group, endpoint);
    }
    static StorageKey FabricKeyset(chip::FabricIndex fabric, uint16_t keyset)
    {
        return StorageKey::Formatted("f/%x/k/%x", fabric, keyset);
    }

    static StorageKey AttributeValue(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
    {
        // Needs at most 26 chars: 6 for "g/a///", 4 for the endpoint id, 8 each
        // for the cluster and attribute ids.
        return StorageKey::Formatted("g/a/%x/%" PRIx32 "/%" PRIx32, endpointId, clusterId, attributeId);
    }

    // TODO: Should store fabric-specific parts of the binding list under keys
    // starting with "f/%x/".
    static StorageKey BindingTable() { return StorageKey::FromConst("g/bt"); }
    static StorageKey BindingTableEntry(uint8_t index) { return StorageKey::Formatted("g/bt/%x", index); }

    static StorageKey OTADefaultProviders() { return StorageKey::FromConst("g/o/dp"); }
    static StorageKey OTACurrentProvider() { return StorageKey::FromConst("g/o/cp"); }
    static StorageKey OTAUpdateToken() { return StorageKey::FromConst("g/o/ut"); }
    static StorageKey OTACurrentUpdateState() { return StorageKey::FromConst("g/o/us"); }
    static StorageKey OTATargetVersion() { return StorageKey::FromConst("g/o/tv"); }

    // Event number counter.
    static StorageKey IMEventNumber() { return StorageKey::FromConst("g/im/ec"); }
};

} // namespace chip
