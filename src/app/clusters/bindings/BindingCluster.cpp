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
#include <lib/support/Defer.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace {
Binding::TableEntry ToBindingEntry(const TargetStructType & target, EndpointId localEndpoint)
{
    if (target.group.HasValue())
    {
        return Binding::TableEntry::ForGroup(target.fabricIndex, target.group.Value(), localEndpoint,
                                             target.cluster.std_optional());
    }
    return Binding::TableEntry::ForNode(target.fabricIndex, target.node.Value(), localEndpoint, target.endpoint.Value(),
                                        target.cluster.std_optional());
}
} // namespace

void BindingCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mListWrite.reset();
    DefaultServerCluster::Shutdown(shutdownType);
}

bool BindingCluster::BindingListEquals(EndpointId endpoint, FabricIndex fabric, const DecodableBindingListType & bindings)
{
    auto existing = mClusterContext.bindingTable.begin();
    auto incoming = bindings.begin();
    while (incoming.Next())
    {
        while (existing != mClusterContext.bindingTable.end() && (existing->local != endpoint || existing->fabricIndex != fabric))
        {
            ++existing;
        }
        if (existing == mClusterContext.bindingTable.end() || !(*existing == ToBindingEntry(incoming.GetValue(), endpoint)))
        {
            return false;
        }
        ++existing;
    }
    for (; existing != mClusterContext.bindingTable.end(); ++existing)
    {
        if (existing->local == endpoint && existing->fabricIndex == fabric)
        {
            return false;
        }
    }
    return incoming.GetStatus() == CHIP_NO_ERROR;
}

void BindingCluster::SnapshotBindingList(EndpointId endpoint, FabricIndex fabric)
{
    auto & snapshot = mListWrite.emplace();
    snapshot.fabric = fabric;
    for (const auto & entry : mClusterContext.bindingTable)
    {
        if (entry.local == endpoint && entry.fabricIndex == fabric)
        {
            ++snapshot.count;
        }
    }
    if (snapshot.count == 0)
    {
        return;
    }
    snapshot.valid = static_cast<bool>(snapshot.entries.Alloc(snapshot.count));
    VerifyOrReturn(snapshot.valid);
    size_t index = 0;
    for (const auto & entry : mClusterContext.bindingTable)
    {
        if (entry.local == endpoint && entry.fabricIndex == fabric)
        {
            snapshot.entries[index++] = entry;
        }
    }
}

bool BindingCluster::BindingListEqualsSnapshot(EndpointId endpoint)
{
    VerifyOrReturnValue(mListWrite.has_value() && mListWrite->valid, false);
    size_t index = 0;
    for (const auto & entry : mClusterContext.bindingTable)
    {
        if (entry.local == endpoint && entry.fabricIndex == mListWrite->fabric)
        {
            if (index == mListWrite->count || !(entry == mListWrite->entries[index++]))
            {
                return false;
            }
        }
    }
    return index == mListWrite->count;
}

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
    return (!entry.endpoint.HasValue() && !entry.node.HasValue() && entry.group.HasValue() && IsValidGroupId(entry.group.Value()));
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
            auto encodeBinding = [&](const Binding::TableEntry & entry) -> CHIP_ERROR {
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
                return CHIP_NO_ERROR;
            };
            const auto * snapshot = mListWrite.has_value() && mListWrite->valid ? &mListWrite.value() : nullptr;
            // Stable fabric ordering keeps unfiltered reads unchanged when entries are reinserted.
            uint16_t minimumFabric = 0;
            while (true)
            {
                uint16_t nextFabric = UINT16_MAX;
                if (snapshot != nullptr && snapshot->fabric >= minimumFabric)
                {
                    nextFabric = snapshot->fabric;
                }
                for (const auto & entry : mClusterContext.bindingTable)
                {
                    if (entry.local == request.path.mEndpointId && entry.fabricIndex >= minimumFabric &&
                        entry.fabricIndex < nextFabric)
                    {
                        nextFabric = entry.fabricIndex;
                    }
                }
                if (nextFabric == UINT16_MAX)
                {
                    break;
                }
                if (snapshot != nullptr && snapshot->fabric == nextFabric)
                {
                    for (size_t i = 0; i < snapshot->count; ++i)
                    {
                        ReturnErrorOnFailure(encodeBinding(snapshot->entries[i]));
                    }
                }
                else
                {
                    for (const auto & entry : mClusterContext.bindingTable)
                    {
                        if (entry.local == request.path.mEndpointId && entry.fabricIndex == nextFabric)
                        {
                            ReturnErrorOnFailure(encodeBinding(entry));
                        }
                    }
                }
                minimumFabric = static_cast<uint16_t>(nextFabric + 1);
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
    return Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus BindingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                             AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Binding::Attributes::Binding::Id: {
        VerifyOrReturnError(!mListWrite.has_value() || mListWrite->valid, CHIP_ERROR_NO_MEMORY);
        bool changed              = false;
        auto notify               = MakeDefer([&] {
            if (changed && !mListWrite.has_value())
            {
                LogErrorOnFailure(NotifyBindingsChanged(request.GetAccessingFabricIndex()));
            }
        });
        auto markChangedOnSuccess = [&](CHIP_ERROR error) {
            changed = changed || error == CHIP_NO_ERROR;
            return error;
        };
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            DecodableBindingListType newBindingList;

            ReturnErrorOnFailure(decoder.Decode(newBindingList));
            ReturnErrorOnFailure(
                CheckValidBindingList(request.path.mEndpointId, newBindingList, request.GetAccessingFabricIndex()));

            if (BindingListEquals(request.path.mEndpointId, request.GetAccessingFabricIndex(), newBindingList))
            {
                return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
            }

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
                    ReturnErrorOnFailure(markChangedOnSuccess(mClusterContext.bindingTable.RemoveAt(bindingTableIter)));
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
                err = markChangedOnSuccess(CreateBindingEntry(iter.GetValue(), request.path.mEndpointId));
            }

            return err;
        }
        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            TargetStructType target;
            ReturnErrorOnFailure(decoder.Decode(target));
            if (!IsValidBinding(request.path.mEndpointId, target))
            {
                return Status::ConstraintError;
            }
            return markChangedOnSuccess(CreateBindingEntry(target, request.path.mEndpointId));
        }
        return Status::UnsupportedWrite;
    }
    default:
        return Status::UnsupportedWrite;
    }
}

void BindingCluster::ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                                    FabricIndex accessingFabric)
{
    if (path.mAttributeId != Binding::Attributes::Binding::Id)
    {
        return;
    }
    if (opType == DataModel::ListWriteOperation::kListWriteBegin)
    {
        SnapshotBindingList(path.mEndpointId, accessingFabric);
        return;
    }
    const bool changed = mListWrite.has_value() && mListWrite->valid && !BindingListEqualsSnapshot(path.mEndpointId);
    mListWrite.reset();
    if (changed)
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
    NotifyAttributeChanged(Binding::Attributes::Binding::Id);
    DeviceLayer::ChipDeviceEvent event{ .Type            = DeviceLayer::DeviceEventType::kBindingsChangedViaCluster,
                                        .BindingsChanged = { .fabricIndex = accessingFabricIndex } };
    return mClusterContext.platformManager.PostEvent(&event);
}

CHIP_ERROR BindingCluster::CreateBindingEntry(const TargetStructType & entry, EndpointId localEndpoint)
{
    return mClusterContext.bindingManager.AddBindingEntry(ToBindingEntry(entry, localEndpoint));
}

} // namespace Clusters
} // namespace app
} // namespace chip
