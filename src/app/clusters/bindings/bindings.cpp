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
using chip::app::DataModel::Nullable;
using TargetStructType         = Binding::Structs::TargetStruct::Type;
using DecodableBindingListType = Binding::Attributes::BindingList::TypeInfo::DecodableType;

// TODO: add binding table to the persistent storage
namespace {

class BindingTableAccess : public AttributeAccessInterface
{
public:
    // Register for the User Label cluster on all endpoints.
    BindingTableAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Binding::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override;

private:
    CHIP_ERROR ReadBindingTable(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR WriteBindingTable(EndpointId endpoint, AttributeValueDecoder & decoder);
};

BindingTableAccess gAttrAccess;

bool IsValidBinding(const TargetStructType & entry)
{
    return (!entry.groupId.IsNull() && entry.endpointId.IsNull() && entry.nodeId.IsNull()) ||
        (!entry.endpointId.IsNull() && !entry.nodeId.IsNull() && entry.groupId.IsNull());
}

CHIP_ERROR CheckValidBindingList(const DecodableBindingListType & bindingList)
{
    size_t listSize = 0;
    auto iter       = bindingList.begin();
    while (iter.Next())
    {
        VerifyOrReturnError(IsValidBinding(iter.GetValue()), CHIP_ERROR_INVALID_ARGUMENT);
        listSize++;
    }
    ReturnErrorOnFailure(iter.GetStatus());
    ReturnErrorCodeIf(listSize > EMBER_BINDING_TABLE_SIZE, CHIP_ERROR_NO_MEMORY);
    return CHIP_NO_ERROR;
}

bool BindingEntryMatches(EndpointId endpoint, const EmberBindingTableEntry & entry, const TargetStructType & value)
{
    if (entry.local != endpoint || entry.fabricIndex != value.fabricIdx || entry.clusterId != value.clusterId)
    {
        return false;
    }
    return (entry.type == EMBER_UNICAST_BINDING && !value.nodeId.IsNull() && entry.nodeId == value.nodeId.Value() &&
            !value.endpointId.IsNull() && entry.remote == value.endpointId.Value()) ||
        (entry.type == EMBER_MULTICAST_BINDING && !value.groupId.IsNull() && entry.groupId == value.groupId.Value());
}

bool IsInBindingList(EndpointId endpoint, const EmberBindingTableEntry & entry, const DecodableBindingListType & bindingList)
{
    if (entry.type == EMBER_UNUSED_BINDING)
    {
        return false;
    }
    auto iter = bindingList.begin();
    while (iter.Next())
    {
        if (BindingEntryMatches(endpoint, entry, iter.GetValue()))
        {
            return true;
        }
    }
    return false;
}

bool IsInBindingTable(EndpointId endpoint, const TargetStructType & bindingEntry, uint8_t * foundIndex)
{
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry currentEntry;
        emberGetBinding(i, &currentEntry);
        if (currentEntry.type != EMBER_UNUSED_BINDING)
        {
            if (BindingEntryMatches(endpoint, currentEntry, bindingEntry))
            {
                *foundIndex = i;
                return true;
            }
        }
    }
    return false;
}

void AddBindingEntry(const TargetStructType & entry, EndpointId localEndpoint, uint8_t index)
{
    EmberBindingTableEntry bindingEntry;

    if (!entry.groupId.IsNull())
    {
        bindingEntry = EmberBindingTableEntry::ForGroup(entry.fabricIdx, entry.groupId.Value(), localEndpoint, entry.clusterId);
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(entry.fabricIdx, entry.nodeId.Value(), localEndpoint,
                                                       entry.endpointId.Value(), entry.clusterId);
    }

    emberSetBinding(index, &bindingEntry);
}

CHIP_ERROR BindingTableAccess::Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder)
{
    switch (path.mAttributeId)
    {
    case Binding::Attributes::BindingList::Id:
        return ReadBindingTable(path.mEndpointId, encoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::ReadBindingTable(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([&](const auto & subEncoder) {
        for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
        {
            EmberBindingTableEntry entry;
            emberGetBinding(i, &entry);
            if (entry.type == EMBER_UNICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .fabricIdx  = entry.fabricIndex,
                    .nodeId     = Nullable<NodeId>(entry.nodeId),
                    .endpointId = Nullable<EndpointId>(entry.remote),
                    .clusterId  = entry.clusterId,
                };
                ReturnErrorOnFailure(subEncoder.Encode(value));
            }
            else if (entry.type == EMBER_MULTICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .groupId   = Nullable<GroupId>(entry.groupId),
                    .clusterId = entry.clusterId,
                };
                ReturnErrorOnFailure(subEncoder.Encode(value));
            }
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR BindingTableAccess::Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    switch (path.mAttributeId)
    {
    case Binding::Attributes::BindingList::Id:
        return WriteBindingTable(path.mEndpointId, decoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::WriteBindingTable(EndpointId endpoint, AttributeValueDecoder & decoder)
{
    DecodableBindingListType newBindingList;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(decoder.Decode(newBindingList));
    ReturnErrorOnFailure(CheckValidBindingList(newBindingList));

    // Remove entries not in the new binding list
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(i, &entry);
        if (entry.type != EMBER_UNUSED_BINDING && !IsInBindingList(endpoint, entry, newBindingList))
        {

            if (entry.type == EMBER_UNICAST_BINDING)
            {
                err = BindingManager::GetInstance().UnicastBindingRemoved(i);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl,
                                 "Binding: Failed to remove pending notification for unicast binding " ChipLogFormatX64
                                 ": %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(entry.nodeId), err.Format());
                }
            }
            emberDeleteBinding(i);
        }
    }

    // Add entries currently not in the binding table
    uint8_t bindingIndex = 0;
    for (auto iter = newBindingList.begin(); iter.Next();)
    {
        uint8_t oldIndex = 0;
        if (!iter.GetValue().nodeId.IsNull() && IsInBindingTable(endpoint, iter.GetValue(), &oldIndex))
        {
            BindingManager::GetInstance().UnicastBindingMoved(oldIndex, bindingIndex);
        }
        else if (!iter.GetValue().nodeId.IsNull())
        {
            err = BindingManager::GetInstance().UnicastBindingCreated(iter.GetValue().fabricIdx, iter.GetValue().nodeId.Value());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(Zcl,
                                "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64
                                ": %" CHIP_ERROR_FORMAT,
                                ChipLogValueX64(iter.GetValue().nodeId.Value()), err.Format());
            }
        }
        bindingIndex++;
    }
    bindingIndex = 0;
    for (auto iter = newBindingList.begin(); iter.Next();)
    {
        AddBindingEntry(iter.GetValue(), endpoint, bindingIndex);
        bindingIndex++;
    }
    return err;
}
} // namespace

void MatterBindingPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
