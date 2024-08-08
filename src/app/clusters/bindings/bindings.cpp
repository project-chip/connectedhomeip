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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/bindings/bindings.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

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
    void OnListWriteEnd(const app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override;

private:
    CHIP_ERROR ReadBindingTable(EndpointId endpoint, AttributeValueEncoder & encoder);
    CHIP_ERROR WriteBindingTable(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder);

    CHIP_ERROR NotifyBindingsChanged();

    FabricIndex mAccessingFabricIndex;
};

BindingTableAccess gAttrAccess;

bool IsValidBinding(const EndpointId localEndpoint, const TargetStructType & entry)
{
    bool isValid = false;

    // Entry has endpoint, node id and no group id
    if (!entry.group.HasValue() && entry.endpoint.HasValue() && entry.node.HasValue())
    {
        if (entry.cluster.HasValue())
        {
            if (emberAfContainsClient(localEndpoint, entry.cluster.Value()))
            {
                // Valid node/endpoint/cluster binding
                isValid = true;
            }
        }
        else
        {
            // Valid node/endpoint (no cluster id) binding
            isValid = true;
        }
    }
    // Entry has group id and no endpoint and node id
    else if (!entry.endpoint.HasValue() && !entry.node.HasValue() && entry.group.HasValue())
    {
        // Valid group binding
        isValid = true;
    }

    return isValid;
}

CHIP_ERROR CheckValidBindingList(const EndpointId localEndpoint, const DecodableBindingListType & bindingList,
                                 FabricIndex accessingFabricIndex)
{
    size_t listSize = 0;
    auto iter       = bindingList.begin();
    while (iter.Next())
    {
        VerifyOrReturnError(IsValidBinding(localEndpoint, iter.GetValue()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
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
    ReturnErrorCodeIf(BindingTable::GetInstance().Size() - oldListSize + listSize > MATTER_BINDING_TABLE_SIZE,
                      CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CreateBindingEntry(const TargetStructType & entry, EndpointId localEndpoint)
{
    EmberBindingTableEntry bindingEntry;

    if (entry.group.HasValue())
    {
        bindingEntry =
            EmberBindingTableEntry::ForGroup(entry.fabricIndex, entry.group.Value(), localEndpoint, entry.cluster.std_optional());
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(entry.fabricIndex, entry.node.Value(), localEndpoint, entry.endpoint.Value(),
                                                       entry.cluster.std_optional());
    }

    return AddBindingEntry(bindingEntry);
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
            if (entry.local == endpoint && entry.type == MATTER_UNICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .node        = MakeOptional(entry.nodeId),
                    .group       = NullOptional,
                    .endpoint    = MakeOptional(entry.remote),
                    .cluster     = FromStdOptional(entry.clusterId),
                    .fabricIndex = entry.fabricIndex,
                };
                ReturnErrorOnFailure(subEncoder.Encode(value));
            }
            else if (entry.local == endpoint && entry.type == MATTER_MULTICAST_BINDING)
            {
                Binding::Structs::TargetStruct::Type value = {
                    .node        = NullOptional,
                    .group       = MakeOptional(entry.groupId),
                    .endpoint    = NullOptional,
                    .cluster     = FromStdOptional(entry.clusterId),
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

void BindingTableAccess::OnListWriteEnd(const app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful)
{
    // Notify binding table has changed
    LogErrorOnFailure(NotifyBindingsChanged());
}

CHIP_ERROR BindingTableAccess::WriteBindingTable(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    mAccessingFabricIndex = decoder.AccessingFabricIndex();
    if (!path.IsListOperation() || path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        DecodableBindingListType newBindingList;

        ReturnErrorOnFailure(decoder.Decode(newBindingList));
        ReturnErrorOnFailure(CheckValidBindingList(path.mEndpointId, newBindingList, mAccessingFabricIndex));

        // Clear all entries for the current accessing fabric and endpoint
        auto bindingTableIter = BindingTable::GetInstance().begin();
        while (bindingTableIter != BindingTable::GetInstance().end())
        {
            if (bindingTableIter->local == path.mEndpointId && bindingTableIter->fabricIndex == mAccessingFabricIndex)
            {
                if (bindingTableIter->type == MATTER_UNICAST_BINDING)
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
        auto iter      = newBindingList.begin();
        CHIP_ERROR err = CHIP_NO_ERROR;
        while (iter.Next() && err == CHIP_NO_ERROR)
        {
            err = CreateBindingEntry(iter.GetValue(), path.mEndpointId);
        }

        // If this was not caused by a list operation, OnListWriteEnd is not going to be triggered
        // so a notification is sent here.
        if (!path.IsListOperation())
        {
            // Notify binding table has changed
            LogErrorOnFailure(NotifyBindingsChanged());
        }
        return err;
    }
    if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        TargetStructType target;
        ReturnErrorOnFailure(decoder.Decode(target));
        if (!IsValidBinding(path.mEndpointId, target))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        return CreateBindingEntry(target, path.mEndpointId);
    }
    return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
}

CHIP_ERROR BindingTableAccess::NotifyBindingsChanged()
{
    DeviceLayer::ChipDeviceEvent event{ .Type            = DeviceLayer::DeviceEventType::kBindingsChangedViaCluster,
                                        .BindingsChanged = { .fabricIndex = mAccessingFabricIndex } };
    return chip::DeviceLayer::PlatformMgr().PostEvent(&event);
}

} // namespace

void MatterBindingPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}

CHIP_ERROR AddBindingEntry(const EmberBindingTableEntry & entry)
{
    CHIP_ERROR err = BindingTable::GetInstance().Add(entry);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (entry.type == MATTER_UNICAST_BINDING)
    {
        err = BindingManager::GetInstance().UnicastBindingCreated(entry.fabricIndex, entry.nodeId);
        if (err != CHIP_NO_ERROR)
        {
            // Unicast connection failure can happen if peer is offline. We'll retry connection on-demand.
            ChipLogError(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(entry.nodeId), err.Format());
        }
    }

    return CHIP_NO_ERROR;
}
