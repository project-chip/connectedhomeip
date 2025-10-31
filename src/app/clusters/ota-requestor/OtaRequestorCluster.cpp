/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/ota-requestor/OtaRequestorCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

namespace chip::app::Clusters {

OtaRequestorCluster::OtaRequestorCluster(EndpointId endpointId)
    : DefaultServerCluster(ConcreteClusterPath(endpointId, OtaSoftwareUpdateRequestor::Id))
{
}

DataModel::ActionReturnStatus OtaRequestorCluster::ReadAttribute(
    const DataModel::ReadAttributeRequest & request,
    AttributeValueEncoder & encoder)
{
    return Protocols::InteractionModel::Status::UnsupportedRead;
}

CHIP_ERROR OtaRequestorCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(OtaSoftwareUpdateRequestor::Attributes::kMandatoryMetadata), {});
}

}  // namespace chip::app::Clusters
