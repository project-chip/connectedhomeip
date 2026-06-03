/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/thread-network-diagnostics-server/NullThreadDiagnosticsProvider.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Encode.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app::Clusters::ThreadNetworkDiagnostics {

CHIP_ERROR NullThreadDiagnosticsProvider::WriteAttributeToTlv(AttributeId attributeId,
                                                              app::AttributeValueEncoder & encoder)
{
    CHIP_ERROR err;
    switch (attributeId)
    {
    case Attributes::NeighborTable::Id:
    case Attributes::RouteTable::Id:
    case Attributes::ActiveNetworkFaultsList::Id:
        err = encoder.EncodeEmptyList();
        break;
    case Attributes::Channel::Id:
    case Attributes::RoutingRole::Id:
    case Attributes::NetworkName::Id:
    case Attributes::PanId::Id:
    case Attributes::ExtendedPanId::Id:
    case Attributes::MeshLocalPrefix::Id:
    case Attributes::PartitionId::Id:
    case Attributes::Weighting::Id:
    case Attributes::DataVersion::Id:
    case Attributes::StableDataVersion::Id:
    case Attributes::LeaderRouterId::Id:
    case Attributes::ActiveTimestamp::Id:
    case Attributes::PendingTimestamp::Id:
    case Attributes::Delay::Id:
    case Attributes::ChannelPage0Mask::Id:
    case Attributes::SecurityPolicy::Id:
    case Attributes::OperationalDatasetComponents::Id:
    case Attributes::ExtAddress::Id:
    case Attributes::Rloc16::Id:
        err = encoder.EncodeNull();
        break;
    default:
        // The remaining attributes are generally unsigned integers.
        // We rely here on TLV encoding 0 of any type to a the same numeric 0
        err = encoder.Encode<uint32_t>(0u);
        break;
    }
    return err;
}

void NullThreadDiagnosticsProvider::ResetCounts() {}

} // namespace chip::app::Clusters::ThreadNetworkDiagnostics
