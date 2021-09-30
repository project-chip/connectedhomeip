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
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using chip::DeviceLayer::ConnectivityManager;

namespace {

class WiFiDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the WiFiNetworkDiagnostics cluster on all endpoints.
    WiFiDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), WiFiNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(T &), const AttributeValueEncoder & aEncoder);
};

template <typename T>
CHIP_ERROR WiFiDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(T &),
                                                     const AttributeValueEncoder & aEncoder)
{
    T data;
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

WiFiDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR WiFiDiagosticsAttrAccess::Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead)
{
    if (aClusterInfo.mClusterId != WiFiNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *aDataRead = true;
    switch (aClusterInfo.mFieldId)
    {
    case SecurityType::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiSecurityType, aEncoder);
    }
    case WiFiVersion::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiVersion, aEncoder);
    }
    case ChannelNumber::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiChannelNumber, aEncoder);
    }
    case Rssi::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiRssi, aEncoder);
    }
    case BeaconLostCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiBeaconLostCount, aEncoder);
    }
    case BeaconRxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiBeaconRxCount, aEncoder);
    }
    case PacketMulticastRxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiPacketMulticastRxCount, aEncoder);
    }
    case PacketMulticastTxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiPacketMulticastTxCount, aEncoder);
    }
    case PacketUnicastRxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiPacketUnicastRxCount, aEncoder);
    }
    case PacketUnicastTxCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiPacketUnicastTxCount, aEncoder);
    }
    case CurrentMaxRate::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiCurrentMaxRate, aEncoder);
    }
    case OverrunCount::Id: {
        return ReadIfSupported(&ConnectivityManager::GetWiFiOverrunCount, aEncoder);
    }
    default: {
        *aDataRead = false;
        break;
    }
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

bool emberAfWiFiNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    VerifyOrExit(DeviceLayer::ConnectivityMgr().ResetWiFiNetworkDiagnosticsCounts() == CHIP_NO_ERROR,
                 status = EMBER_ZCL_STATUS_FAILURE);

    status = WiFiNetworkDiagnostics::Attributes::BeaconLostCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset BeaconLostCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::BeaconRxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset BeaconRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::PacketMulticastRxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketMulticastRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::PacketMulticastTxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketMulticastTxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::PacketUnicastRxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketUnicastRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::PacketUnicastTxCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketUnicastTxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::OverrunCount::Set(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset OverrunCount attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);

    return true;
}

void emberAfWiFiNetworkDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
}
