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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/ConnectivityManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;
using chip::DeviceLayer::ConnectivityManager;

namespace {

class EthernetDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    EthernetDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead) override;

private:
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(uint64_t &), const AttributeValueEncoder & aEncoder);
};

EthernetDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR EthernetDiagosticsAttrAccess::Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead)
{
    if (aClusterInfo.mClusterId != EthernetNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *aDataRead = true;
    switch (aClusterInfo.mFieldId)
    {
    case PacketRxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetEthPacketRxCount, aEncoder);
    }
    case PacketTxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetEthPacketTxCount, aEncoder);
    }
    case TxErrCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetEthTxErrCount, aEncoder);
    }
    case CollisionCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetEthCollisionCount, aEncoder);
    }
    case OverrunCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetEthOverrunCount, aEncoder);
    }
    default: {
        *aDataRead = false;
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EthernetDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(uint64_t &),
                                                         const AttributeValueEncoder & aEncoder)
{
    uint64_t data;
    CHIP_ERROR err = (DeviceLayer::ConnectivityMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aEncoder.Encode(data);
}
} // anonymous namespace

bool emberAfEthernetNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    VerifyOrExit(DeviceLayer::ConnectivityMgr().ResetEthNetworkDiagnosticsCounts() == CHIP_NO_ERROR,
                 status = EMBER_ZCL_STATUS_FAILURE);

    status = EthernetNetworkDiagnostics::Attributes::PacketRxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketRxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::PacketTxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketTxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::TxErrCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset TxErrCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::CollisionCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset CollisionCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::OverrunCount::Set(endpoint, 0);
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
