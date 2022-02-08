/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Binding Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/util/attribute-storage.h>
#include <app/util/binding-table.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Binding;
using namespace chip::app::Clusters::Binding::Attributes;

// TODO: add binding table to the persistent storage
namespace {

class BindingTableAccess : public AttributeAccessInterface
{
public:
    // Register for the User Label cluster on all endpoints.
    BindingTableAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Binding::Id) {}

    CHIP_ERROR Read(FabricIndex fabricIndex, const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override;
    CHIP_ERROR Write(FabricIndex fabricIndex, const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override;

private:
    CHIP_ERROR ReadBindingTable(FabricIndex fabricIndex, EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR WriteBindingTable(FabricIndex fabricIndex, EndpointId endpoint, AttributeValueDecoder & decoder);
};

BindingTableAccess gAttrAccess;

EmberStatus getUnusedBindingIndex(uint8_t * bindingIndex)
{
    EmberBindingTableEntry currentEntry;
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        emberGetBinding(i, &currentEntry);
        if (currentEntry.type == EMBER_UNUSED_BINDING)
        {
            *bindingIndex = i;
            return EMBER_SUCCESS;
        }
    }

    return EMBER_NOT_FOUND;
}

bool BindingEntryMatches(FabricIndex fabricIndex, EndpointId endpoint, const EmberBindingTableEntry & entry,
                         const Structs::BindingEntry::Type & value)
{
    if (entry.local != endpoint || entry.fabricIndex != fabricIndex || entry.clusterId != value.clusterId)
    {
        return false;
    }
    return (entry.type == EMBER_UNICAST_BINDING && entry.nodeId == value.nodeId && entry.remote == value.endpointId) ||
        (entry.type == EMBER_MULTICAST_BINDING && entry.groupId == value.groupId);
}

bool IsInBindingList(FabricIndex fabricIndex, EndpointId endpoint, const EmberBindingTableEntry & entry,
                     const BindingList::TypeInfo::DecodableType & bindingList)
{
    if (entry.type == EMBER_UNUSED_BINDING)
    {
        return false;
    }
    auto iter = bindingList.begin();
    while (iter.Next())
    {
        if (BindingEntryMatches(fabricIndex, endpoint, entry, iter.GetValue()))
        {
            return true;
        }
    }
    return false;
}

bool IsInBindingTable(FabricIndex fabricIndex, EndpointId endpoint,
                      const Binding::Structs::BindingEntry::DecodableType & bindingEntry)
{
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry currentEntry;
        emberGetBinding(i, &currentEntry);
        if (currentEntry.type != EMBER_UNUSED_BINDING)
        {
            if (BindingEntryMatches(fabricIndex, endpoint, currentEntry, bindingEntry))
            {
                return true;
            }
        }
    }
    return false;
}

CHIP_ERROR AddBindingEntry(const Binding::Structs::BindingEntry::DecodableType & entry, FabricIndex fabricIndex,
                           EndpointId localEndpoint, uint8_t * outBindingIndex)
{
    GroupId groupId           = entry.groupId;
    NodeId nodeId             = entry.nodeId;
    EndpointId remoteEndpoint = entry.endpointId;
    ClusterId clusterId       = entry.clusterId;
    EmberBindingTableEntry bindingEntry;

    ChipLogDetail(Zcl, "RX: BindCallback");

    if ((groupId != 0 && nodeId != 0) || (groupId == 0 && nodeId == 0) || (groupId != 0 && remoteEndpoint != 0))
    {
        ChipLogError(Zcl, "Binding: Invalid request");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (groupId)
    {
        bindingEntry = EmberBindingTableEntry::ForGroup(fabricIndex, groupId, localEndpoint, clusterId);
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(fabricIndex, nodeId, localEndpoint, remoteEndpoint, clusterId);
    }

    uint8_t bindingIndex;

    if (getUnusedBindingIndex(&bindingIndex) != EMBER_SUCCESS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    emberSetBinding(bindingIndex, &bindingEntry);
    if (nodeId)
    {
        CHIP_ERROR err = BindingManager::GetInstance().UnicastBindingCreated(bindingEntry);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogProgress(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(nodeId), err.Format());
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR BindingTableAccess::Read(FabricIndex fabricIndex, const ConcreteReadAttributePath & path,
                                    AttributeValueEncoder & encoder)
{
    switch (path.mAttributeId)
    {
    case BindingList::Id:
        return ReadBindingTable(fabricIndex, path.mEndpointId, encoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::ReadBindingTable(FabricIndex fabricIndex, EndpointId endpoint, AttributeValueEncoder & encoder)
{
    DeviceLayer::AttributeList<Structs::BindingEntry::Type, EMBER_BINDING_TABLE_SIZE> bindingTable;

    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(i, &entry);
        if (entry.type == EMBER_UNICAST_BINDING && entry.fabricIndex == fabricIndex)
        {
            Structs::BindingEntry::Type value = {
                .nodeId     = entry.nodeId,
                .groupId    = 0,
                .endpointId = entry.remote,
                .clusterId  = entry.clusterId,
            };
            bindingTable.add(value);
        }
        else if (entry.type == EMBER_MULTICAST_BINDING && entry.fabricIndex == fabricIndex)
        {
            Structs::BindingEntry::Type value = {
                .nodeId     = 0,
                .groupId    = entry.groupId,
                .endpointId = 0,
                .clusterId  = entry.clusterId,
            };
            bindingTable.add(value);
        }
    }
    return encoder.EncodeList([&bindingTable](const auto & subEncoder) {
        for (auto & value : bindingTable)
        {
            ReturnErrorOnFailure(subEncoder.Encode(value));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR BindingTableAccess::Write(FabricIndex fabricIndex, const ConcreteDataAttributePath & path,
                                     AttributeValueDecoder & decoder)
{
    switch (path.mAttributeId)
    {
    case BindingList::Id:
        return WriteBindingTable(fabricIndex, path.mEndpointId, decoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::WriteBindingTable(FabricIndex fabricIndex, EndpointId endpoint, AttributeValueDecoder & decoder)
{
    BindingList::TypeInfo::DecodableType newBindingList;

    ReturnErrorOnFailure(decoder.Decode(newBindingList));

    // Add entries currently not in the binding table
    auto iter      = newBindingList.begin();
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t addedBindingIndecies[EMBER_BINDING_TABLE_SIZE];
    uint8_t numAddedBindings = 0;
    while (iter.Next())
    {
        if (!IsInBindingTable(fabricIndex, endpoint, iter.GetValue()))
        {
            err = AddBindingEntry(iter.GetValue(), fabricIndex, endpoint, &addedBindingIndecies[numAddedBindings]);
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
            numAddedBindings++;
        }
    }
    // Revert the added entries upon error
    if (err != CHIP_NO_ERROR)
    {
        for (uint8_t bindingIndex : addedBindingIndecies)
        {
            emberDeleteBinding(bindingIndex);
        }
    }

    // Remove entries not in the new binding list
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(i, &entry);
        if (entry.type != EMBER_UNUSED_BINDING && entry.fabricIndex == fabricIndex &&
            !IsInBindingList(fabricIndex, endpoint, entry, newBindingList))
        {
            if (entry.type == EMBER_UNICAST_BINDING && BindingManager::GetInstance().UnicastBindingRemoved(i) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Binding: Failed to remove pending notification for unicast binding" ChipLogFormatX64 ": %s",
                             ChipLogValueX64(entry.nodeId), err.AsString());
            }
            emberDeleteBinding(i);
        }
    }
    return err;
}

void MatterBindingPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
