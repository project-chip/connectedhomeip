/**
 *
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-logic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <clusters/GeneralDiagnostics/Metadata.h>

using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace Clusters {

template <typename LOGIC>
void GeneralDiagnosticsCluster<LOGIC>::OnDeviceReboot(BootReasonEnum bootReason)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(BootReason::Id);

    Events::BootReason::Type event{ bootReason };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

template <typename LOGIC>
void GeneralDiagnosticsCluster<LOGIC>::OnHardwareFaultsDetect(const GeneralFaults<kMaxHardwareFaults> & previous,
                                                       const GeneralFaults<kMaxHardwareFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveHardwareFaults::Id);

    // Record HardwareFault event
    DataModel::List<const HardwareFaultEnum> currentList(reinterpret_cast<const HardwareFaultEnum *>(current.data()),
                                                         current.size());
    DataModel::List<const HardwareFaultEnum> previousList(reinterpret_cast<const HardwareFaultEnum *>(previous.data()),
                                                          previous.size());
    Events::HardwareFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

template <typename LOGIC>
void GeneralDiagnosticsCluster<LOGIC>::OnRadioFaultsDetect(const GeneralFaults<kMaxRadioFaults> & previous,
                                                    const GeneralFaults<kMaxRadioFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveRadioFaults::Id);

    // Record RadioFault event
    DataModel::List<const RadioFaultEnum> currentList(reinterpret_cast<const RadioFaultEnum *>(current.data()), current.size());
    DataModel::List<const RadioFaultEnum> previousList(reinterpret_cast<const RadioFaultEnum *>(previous.data()), previous.size());
    Events::RadioFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

template <typename LOGIC>
void GeneralDiagnosticsCluster<LOGIC>::OnNetworkFaultsDetect(const GeneralFaults<kMaxNetworkFaults> & previous,
                                                      const GeneralFaults<kMaxNetworkFaults> & current)
{
    VerifyOrReturn(mContext != nullptr);
    NotifyAttributeChanged(ActiveNetworkFaults::Id);

    // Record NetworkFault event
    DataModel::List<const NetworkFaultEnum> currentList(reinterpret_cast<const NetworkFaultEnum *>(current.data()), current.size());
    DataModel::List<const NetworkFaultEnum> previousList(reinterpret_cast<const NetworkFaultEnum *>(previous.data()),
                                                         previous.size());
    Events::NetworkFaultChange::Type event{ currentList, previousList };

    (void) mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

} // namespace Clusters
} // namespace app
} // namespace chip
