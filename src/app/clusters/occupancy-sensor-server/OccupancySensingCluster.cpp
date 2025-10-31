/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Commands.h>
#include <clusters/OccupancySensing/Metadata.h>

namespace chip::app::Clusters {

using namespace OccupancySensing::Attributes;
using namespace chip::app::Clusters::OccupancySensing;

OccupancySensingCluster::OccupancySensingCluster(EndpointId endpointId) :
    DefaultServerCluster({ endpointId, OccupancySensing::Id })
{}

DataModel::ActionReturnStatus OccupancySensingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(OccupancySensing::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

} // namespace chip::app::Clusters