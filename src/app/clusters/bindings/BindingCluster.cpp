/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Binding/Metadata.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

// TODO: add binding table to the persistent storage
namespace {

CHIP_ERROR CreateBindingEntry(const TargetStructType & entry, EndpointId localEndpoint)
{
    Binding::TableEntry bindingEntry;

    if (entry.group.HasValue())
    {
        bindingEntry = Binding::TableEntry(entry.fabricIndex, entry.group.Value(), localEndpoint, entry.cluster.std_optional());
    }
    else
    {
        bindingEntry = Binding::TableEntry(entry.fabricIndex, entry.node.Value(), localEndpoint, entry.endpoint.Value(),
                                           entry.cluster.std_optional());
    }

    return AddBindingEntry(bindingEntry);
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {

bool BindingCluster::IsValidBinding(const EndpointId localEndpoint, const TargetStructType & entry)
{
    // Entry has endpoint, node id and no group id
    if (!entry.group.HasValue() && entry.endpoint.HasValue() && entry.node.HasValue())
    {
        VerifyOrReturnValue(entry.cluster.HasValue(), true); // valid node/endopint binding without a clusterid
        // Valid node/endpoint/cluster binding
        ReadOnlyBufferBuilder<ClusterId> clientClusters;
        // TODO: is this a correct validation?
        VerifyOrReturnValue(mContext->provider.ClientClusters(localEndpoint, clientClusters) == CHIP_NO_ERROR, false);
        for (auto & client : clientClusters.TakeBuffer())
        {
            VerifyOrReturnValue(client != entry.cluster.Value(), true);
        }
    }
    // Entry has group id and no endpoint and node id
    return (!entry.endpoint.HasValue() && !entry.node.HasValue() && entry.group.HasValue());
}

CHIP_ERROR BindingCluster::CheckValidBindingList(const EndpointId localEndpoint, const DecodableBindingListType & bindingList,
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
    for (const auto & entry : mClusterContext.bindingTable)
    {
        if (entry.local == localEndpoint && entry.fabricIndex == accessingFabricIndex)
        {
            oldListSize++;
        }
    }
    VerifyOrReturnError(mClusterContext.bindingTable.Size() - oldListSize + listSize <= Binding::Table::kMaxBindingEntries,
                        CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus BindingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                            AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Binding::Attributes::Binding::Id: {
        return encoder.EncodeList([&](const auto & subEncoder) {
            for (auto & entry : mClusterContext.bindingTable)
            {
                if (entry.local != request.path.mEndpointId)
                {
                    continue;
                }
                if (entry.type == Binding::MATTER_UNICAST_BINDING)
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
                else if (entry.type == Binding::MATTER_MULTICAST_BINDING)
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
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(Binding::kRevision);
    default:
        break;
    }
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus BindingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                             AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Binding::Attributes::Binding::Id: {
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            DecodableBindingListType newBindingList;

            ReturnErrorOnFailure(decoder.Decode(newBindingList));
            ReturnErrorOnFailure(
                CheckValidBindingList(request.path.mEndpointId, newBindingList, request.GetAccessingFabricIndex()));

            // Clear all entries for the current accessing fabric and endpoint
            auto bindingTableIter = mClusterContext.bindingTable.begin();
            while (bindingTableIter != mClusterContext.bindingTable.end())
            {
                if (bindingTableIter->local == request.path.mEndpointId &&
                    bindingTableIter->fabricIndex == request.GetAccessingFabricIndex())
                {
                    if (bindingTableIter->type == Binding::MATTER_UNICAST_BINDING)
                    {
                        TEMPORARY_RETURN_IGNORED mClusterContext.bindingManager.UnicastBindingRemoved(bindingTableIter.GetIndex());
                    }
                    ReturnErrorOnFailure(mClusterContext.bindingTable.RemoveAt(bindingTableIter));
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
                err = CreateBindingEntry(iter.GetValue(), request.path.mEndpointId);
            }

            // If this was not caused by a list operation, OnListWriteEnd is not going to be triggered
            // so a notification is sent here.
            if (!request.path.IsListOperation())
            {
                // Notify binding table has changed
                LogErrorOnFailure(NotifyBindingsChanged(request.GetAccessingFabricIndex()));
            }
            return err;
        }
        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            TargetStructType target;
            ReturnErrorOnFailure(decoder.Decode(target));
            if (!IsValidBinding(request.path.mEndpointId, target))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
            return CreateBindingEntry(target, request.path.mEndpointId);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }
    break;
    default:
        break;
    }
    return Protocols::InteractionModel::Status::UnsupportedWrite;
}

void BindingCluster::ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                                    FabricIndex accessingFabric)
{
    if (opType == DataModel::ListWriteOperation::kListWriteSuccess)
    {
        LogErrorOnFailure(NotifyBindingsChanged(accessingFabric));
    }
}

CHIP_ERROR BindingCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Binding::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR BindingCluster::NotifyBindingsChanged(FabricIndex accessingFabricIndex)
{
    DeviceLayer::ChipDeviceEvent event{ .Type            = DeviceLayer::DeviceEventType::kBindingsChangedViaCluster,
                                        .BindingsChanged = { .fabricIndex = accessingFabricIndex } };
    return mClusterContext.platformManager.PostEvent(&event);
}

} // namespace Clusters
} // namespace app
} // namespace chip
