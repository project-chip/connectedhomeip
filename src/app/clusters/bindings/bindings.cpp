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
#include <app/clusters/bindings/bindings.h>
#include <app/util/attribute-storage.h>
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

    CHIP_ERROR NotifyBindingsChanged();
};

BindingTableAccess gAttrAccess;

bool IsValidBinding(const TargetStructType & entry)
{
    return (!entry.group.HasValue() && entry.endpoint.HasValue() && entry.node.HasValue()) ||
        (!entry.endpoint.HasValue() && !entry.node.HasValue() && entry.group.HasValue());
}

CHIP_ERROR CheckValidBindingList(const DecodableBindingListType & bindingList, FabricIndex accessingFabricIndex)
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
    ReturnErrorCodeIf(BindingTable::GetInstance().Size() - oldListSize + listSize > EMBER_BINDING_TABLE_SIZE,
                      CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    return CHIP_NO_ERROR;
}

void CreateBindingEntry(const TargetStructType & entry, EndpointId localEndpoint)
{
    EmberBindingTableEntry bindingEntry;

    if (entry.group.HasValue())
    {
        bindingEntry = EmberBindingTableEntry::ForGroup(entry.fabricIndex, entry.group.Value(), localEndpoint, entry.cluster);
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(entry.fabricIndex, entry.node.Value(), localEndpoint, entry.endpoint.Value(),
                                                       entry.cluster);
    }

    AddBindingEntry(bindingEntry);
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
            if (entry.local == endpoint && entry.type == EMBER_UNICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .node        = MakeOptional(entry.nodeId),
                    .group       = NullOptional,
                    .endpoint    = MakeOptional(entry.remote),
                    .cluster     = entry.clusterId,
                    .fabricIndex = entry.fabricIndex,
                };
                ReturnErrorOnFailure(subEncoder.Encode(value));
            }
            else if (entry.local == endpoint && entry.type == EMBER_MULTICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .node        = NullOptional,
                    .group       = MakeOptional(entry.groupId),
                    .endpoint    = NullOptional,
                    .cluster     = entry.clusterId,
                    .fabricIndex = entry.fabricIndex,
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

CHIP_ERROR BindingTableAccess::WriteBindingTable(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    FabricIndex accessingFabricIndex = decoder.AccessingFabricIndex();
    if (!path.IsListOperation() || path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        DecodableBindingListType newBindingList;

        ReturnErrorOnFailure(decoder.Decode(newBindingList));
        ReturnErrorOnFailure(CheckValidBindingList(newBindingList, accessingFabricIndex));

        // Clear all entries for the current accessing fabric and endpoint
        auto bindingTableIter = BindingTable::GetInstance().begin();
        while (bindingTableIter != BindingTable::GetInstance().end())
        {
            if (bindingTableIter->local == path.mEndpointId && bindingTableIter->fabricIndex == accessingFabricIndex)
            {
                if (bindingTableIter->type == EMBER_UNICAST_BINDING)
                {
                    BindingManager::GetInstance().UnicastBindingRemoved(bindingTableIter.GetIndex());
                }
                ReturnErrorOnFailure(BindingTable::GetInstance().RemoveAt(bindingTableIter));
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
            CreateBindingEntry(iter.GetValue(), path.mEndpointId);
        }
        LogErrorOnFailure(NotifyBindingsChanged());
        return CHIP_NO_ERROR;
    }
    if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        TargetStructType target;
        ReturnErrorOnFailure(decoder.Decode(target));
        if (!IsValidBinding(target))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        CreateBindingEntry(target, path.mEndpointId);
        LogErrorOnFailure(NotifyBindingsChanged());
        return CHIP_NO_ERROR;
    }
    return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
}

CHIP_ERROR BindingTableAccess::NotifyBindingsChanged()
{
    DeviceLayer::ChipDeviceEvent event;
    event.Type = DeviceLayer::DeviceEventType::kBindingsChangedViaCluster;
    return chip::DeviceLayer::PlatformMgr().PostEvent(&event);
}

} // namespace

void MatterBindingPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}

void AddBindingEntry(const EmberBindingTableEntry & entry)
{
    if (entry.type == EMBER_UNICAST_BINDING)
    {
        CHIP_ERROR err = BindingManager::GetInstance().UnicastBindingCreated(entry.fabricIndex, entry.nodeId);
        if (err != CHIP_NO_ERROR)
        {
            // Unicast connection failure can happen if peer is offline. We'll retry connection on-demand.
            ChipLogError(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(entry.nodeId), err.Format());
        }
    }

    BindingTable::GetInstance().Add(entry);
}
