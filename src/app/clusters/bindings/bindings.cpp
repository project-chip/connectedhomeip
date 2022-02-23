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
#include <app/AttributePersistenceProvider.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/bindings.h>
#include <app/util/attribute-storage.h>
#include <app/util/binding-table.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using TargetStructType         = Binding::Structs::TargetStruct::Type;
using DecodableBindingListType = Binding::Attributes::Binding::TypeInfo::DecodableType;

// TODO: add binding table to the persistent storage
namespace {

class BindingTableAccess : public AttributeAccessInterface
{
public:
    // Register for the Binding cluster on all endpoints.
    BindingTableAccess() : AttributeAccessInterface(NullOptional, Binding::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override;

private:
    CHIP_ERROR ReadBindingTable(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR WriteBindingTable(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder);
};

BindingTableAccess gAttrAccess;

bool IsValidBinding(const TargetStructType & entry)
{
    return (!entry.group.HasValue() && entry.endpoint.HasValue() && entry.node.HasValue()) ||
        (!entry.endpoint.HasValue() && !entry.node.HasValue() && entry.group.HasValue());
}

EmberBindingTableEntry BindingTableEntryFromTargetStruct(const TargetStructType & entry, EndpointId localEndpoint)
{
    if (entry.group.HasValue())
    {
        return EmberBindingTableEntry::ForGroup(entry.fabricIndex, entry.group.Value(), localEndpoint, entry.cluster);
    }
    else
    {
        return EmberBindingTableEntry::ForNode(entry.fabricIndex, entry.node.Value(), localEndpoint, entry.endpoint.Value(),
                                               entry.cluster);
    }
}

CHIP_ERROR CheckValidBindingList(const DecodableBindingListType & bindingList, FabricIndex accessingFabricIndex, size_t * newSize)
{
    size_t listSize = 0;
    auto iter       = bindingList.begin();
    while (iter.Next())
    {
        VerifyOrReturnError(IsValidBinding(iter.GetValue()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        listSize++;
    }
    ReturnErrorOnFailure(iter.GetStatus());

    // Check binding table size
    uint8_t oldListSize = 0;
    for (const auto & entry : BindingTable::GetInstance())
    {
        if (entry.fabricIndex == accessingFabricIndex)
        {
            oldListSize++;
        }
    }
    *newSize = BindingTable::GetInstance().Size() - oldListSize + listSize;
    ReturnErrorCodeIf(*newSize > EMBER_BINDING_TABLE_SIZE, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder)
{
    switch (path.mAttributeId)
    {
    case Binding::Attributes::Binding::Id:
        return ReadBindingTable(path.mEndpointId, encoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTableAccess::ReadBindingTable(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([&](const auto & subEncoder) {
        for (const EmberBindingTableEntry & entry : BindingTable::GetInstance())
        {
            if (entry.type == EMBER_UNICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .fabricIndex = entry.fabricIndex,
                    .node        = MakeOptional(entry.nodeId),
                    .group       = NullOptional,
                    .endpoint    = MakeOptional(entry.remote),
                    .cluster     = entry.clusterId,
                };
                ReturnErrorOnFailure(subEncoder.Encode(value));
            }
            else if (entry.type == EMBER_MULTICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .fabricIndex = entry.fabricIndex,
                    .node        = NullOptional,
                    .group       = MakeOptional(entry.groupId),
                    .endpoint    = NullOptional,
                    .cluster     = entry.clusterId,
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
    case Binding::Attributes::Binding::Id:
        return WriteBindingTable(path, decoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SaveWithDecodableBindingList(const DecodableBindingListType & bindingList, size_t newTableSize,
                                        FabricIndex accessingFabricIndex, EndpointId localEndpoint)
{
    // The binding table can be at most 480 bytes, allocate on heap to reduce static stack size allocation
    // which cannot be reused.
    EmberBindingTableEntry * table =
        static_cast<EmberBindingTableEntry *>(Platform::MemoryAlloc(newTableSize * sizeof(EmberBindingTableEntry)));
    uint8_t idx = 0;

    if (table == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    for (const EmberBindingTableEntry & entry : BindingTable::GetInstance())
    {
        if (entry.fabricIndex != accessingFabricIndex)
        {
            table[idx++] = entry;
        }
    }
    auto iter = bindingList.begin();
    while (iter.Next())
    {
        table[idx++] = BindingTableEntryFromTargetStruct(iter.GetValue(), localEndpoint);
    }
    CHIP_ERROR error = Server::GetInstance().GetPersistentStorage().SyncSetKeyValue(
        kBindingStoargeKey, table, static_cast<uint16_t>(newTableSize * sizeof(EmberBindingTableEntry)));

    Platform::MemoryFree(table);
    return error;
}

CHIP_ERROR SaveWithNewEntry(const TargetStructType & newEntry, EndpointId localEndpoint)
{
    size_t newTableSize = BindingTable::GetInstance().Size() + 1;
    EmberBindingTableEntry * table =
        static_cast<EmberBindingTableEntry *>(Platform::MemoryAlloc(newTableSize * sizeof(EmberBindingTableEntry)));
    uint8_t idx = 0;

    if (table == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    for (const EmberBindingTableEntry & entry : BindingTable::GetInstance())
    {
        table[idx++] = entry;
    }
    table[idx++] = BindingTableEntryFromTargetStruct(newEntry, localEndpoint);

    CHIP_ERROR error = Server::GetInstance().GetPersistentStorage().SyncSetKeyValue(
        kBindingStoargeKey, table, static_cast<uint16_t>(newTableSize * sizeof(EmberBindingTableEntry)));

    Platform::MemoryFree(table);
    return error;
}

CHIP_ERROR BindingTableAccess::WriteBindingTable(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    FabricIndex accessingFabricIndex = decoder.AccessingFabricIndex();
    if (!path.IsListOperation() || path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        DecodableBindingListType newBindingList;
        size_t newTableSize;

        ReturnErrorOnFailure(decoder.Decode(newBindingList));
        ReturnErrorOnFailure(CheckValidBindingList(newBindingList, accessingFabricIndex, &newTableSize));

        // Try saving to storage first, if saving fails we won't update the content.
        ReturnErrorOnFailure(SaveWithDecodableBindingList(newBindingList, newTableSize, accessingFabricIndex, path.mEndpointId));

        // Clear all entries for the current accessing fabric
        auto bindingTableIter = BindingTable::GetInstance().begin();
        while (bindingTableIter != BindingTable::GetInstance().end())
        {
            if (bindingTableIter->fabricIndex == accessingFabricIndex)
            {
                if (bindingTableIter->type == EMBER_UNICAST_BINDING)
                {
                    BindingManager::GetInstance().UnicastBindingRemoved(bindingTableIter.GetIndex());
                }
                bindingTableIter = BindingTable::GetInstance().RemoveAt(bindingTableIter);
            }
            else
            {
                ++bindingTableIter;
            }
        }

        // Add new entries
        auto iter = newBindingList.begin();
        while (iter.Next())
        {
            AddBindingEntry(BindingTableEntryFromTargetStruct(iter.GetValue(), path.mEndpointId));
        }
        return CHIP_NO_ERROR;
    }
    else if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        TargetStructType target;
        ReturnErrorOnFailure(decoder.Decode(target));
        if (!IsValidBinding(target))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        if (BindingTable::GetInstance().Size() == EMBER_BINDING_TABLE_SIZE)
        {
            return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
        }
        ReturnErrorOnFailure(SaveWithNewEntry(target, path.mEndpointId));
        AddBindingEntry(BindingTableEntryFromTargetStruct(target, path.mEndpointId));
        return CHIP_NO_ERROR;
    }
    return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
}
} // namespace

namespace chip {

void AddBindingEntry(const EmberBindingTableEntry & entry)
{
    if (entry.type == EMBER_UNICAST_BINDING)
    {
        CHIP_ERROR err = BindingManager::GetInstance().UnicastBindingCreated(entry.fabricIndex, entry.nodeId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogProgress(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(entry.nodeId), err.Format());
        }
    }
    BindingTable::GetInstance().Add(entry);
}

} // namespace chip

void MatterBindingPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
