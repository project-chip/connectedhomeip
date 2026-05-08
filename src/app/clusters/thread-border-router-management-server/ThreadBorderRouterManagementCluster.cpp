/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ThreadBorderRouterManagementCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <clusters/ThreadBorderRouterManagement/Metadata.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters {
namespace ThreadBorderRouterManagement {

DataModel::ActionReturnStatus ThreadBorderRouterManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        // Return cluster revision 1 as boilerplate.
        return encoder.Encode<uint16_t>(1);
    case Globals::Attributes::FeatureMap::Id: {
        BitFlags<Feature> featureMap;
        if (mDelegate.GetPanChangeSupported())
        {
            featureMap.Set(Feature::kPANChange);
        }
        return encoder.Encode(featureMap);
    }
    case Attributes::BorderRouterName::Id: {
        char buffer[kBorderRouterNameMaxLength];
        MutableCharSpan name(buffer, sizeof(buffer));
        mDelegate.GetBorderRouterName(name);
        return encoder.Encode(name);
    }
    case Attributes::BorderAgentID::Id: {
        uint8_t buffer[kBorderAgentIdLength];
        MutableByteSpan agentId(buffer, sizeof(buffer));
        ReturnErrorOnFailure(mDelegate.GetBorderAgentId(agentId));
        return encoder.Encode(agentId);
    }
    case Attributes::ThreadVersion::Id:
        return encoder.Encode(mDelegate.GetThreadVersion());
    case Attributes::InterfaceEnabled::Id:
        return encoder.Encode(mDelegate.GetInterfaceEnabled());
    case Attributes::ActiveDatasetTimestamp::Id: {
        uint64_t timestamp = 0;
        Thread::OperationalDataset dataset;
        if ((mDelegate.GetDataset(dataset, Delegate::DatasetType::kActive) == CHIP_NO_ERROR) &&
            (dataset.GetActiveTimestamp(timestamp) == CHIP_NO_ERROR))
        {
            return encoder.Encode(DataModel::Nullable<uint64_t>(timestamp));
        }
        return encoder.Encode(DataModel::Nullable<uint64_t>());
    }
    case Attributes::PendingDatasetTimestamp::Id: {
        uint64_t timestamp = 0;
        Thread::OperationalDataset dataset;
        if ((mDelegate.GetDataset(dataset, Delegate::DatasetType::kPending) == CHIP_NO_ERROR) &&
            (dataset.GetActiveTimestamp(timestamp) == CHIP_NO_ERROR))
        {
            return encoder.Encode(DataModel::Nullable<uint64_t>(timestamp));
        }
        return encoder.Encode(DataModel::Nullable<uint64_t>());
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ThreadBorderRouterManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), {});
}

} // namespace ThreadBorderRouterManagement
} // namespace chip::app::Clusters
