/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

class EthernetDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    EthernetDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead) override;
};

EthernetDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR EthernetDiagosticsAttrAccess::Read(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead)
{
    *aDataRead = false;
    return CHIP_NO_ERROR;
}

} // anonymous namespace

bool emberAfEthernetNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = EthernetNetworkDiagnostics::Attributes::SetPacketRxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketRxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetPacketTxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketTxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetTxErrCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset TxErrCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetCollisionCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset CollisionCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetOverrunCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset OverrunCount attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emberAfEthernetNetworkDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
}
