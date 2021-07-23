/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "network-commissioning.h"
#include "NetworkCommissioningCluster-Gen.h"
#include "NetworkCommissioningCluster.h"
#include "core/CHIPError.h"
#include "protocols/secure_channel/Constants.h"

#include <cstring>
#include <device/SchemaUtils.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceControlServer.h>
#include <type_traits>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

// Include DeviceNetworkProvisioningDelegateImpl for WiFi provisioning.
// TODO: Enable wifi network should be done by ConnectivityManager. (Or other platform neutral interfaces)
#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif

// TODO: Configuration should move to build-time configuration

using namespace chip::app::Cluster;

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {

void DispatchServerCommand(app::Command * apCommandObj, CommandId aCommandId, EndpointId aEndpointId, TLV::TLVReader & aDataTlv) {}

} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip

NetworkCommissioningServer::NetworkCommissioningServer() : ClusterServer(NetworkCommissioningCluster::kClusterId) {}

CHIP_ERROR NetworkCommissioningServer::OnInvokeRequest(CommandParams & commandParams, InvokeResponder & invokeInteraction,
                                                       TLV::TLVReader * payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (commandParams.CommandId)
    {
    case NetworkCommissioningCluster::kAddThreadNetworkRequestCommandId: {
        NetworkCommissioningCluster::AddThreadNetworkCommand::Type req;
        uint8_t DataSet[200];

        req.OperationalDataSet = chip::ByteSpan{ DataSet };

        err = DecodeSchemaElement(req, *payload);
        SuccessOrExit(err);

        err = AddThreadNetwork(req);
        break;
    }

    case NetworkCommissioningCluster::kAddWifiNetworkRequestCommandId: {
        NetworkCommissioningCluster::AddWifiNetworkCommand::Type req;
        uint8_t Ssid[200];
        uint8_t Credentials[200];

        req.Ssid        = chip::ByteSpan{ Ssid };
        req.Credentials = chip::ByteSpan{ Credentials };

        err = DecodeSchemaElement(req, *payload);
        SuccessOrExit(err);

        err = AddWifiNetwork(req);
        break;
    }

    case NetworkCommissioningCluster::kEnableNetworkRequestCommandId: {
        NetworkCommissioningCluster::EnableNetworkCommand::Type req;
        uint8_t NetworkId[64];

        req.NetworkId = chip::ByteSpan{ NetworkId };

        err = DecodeSchemaElement(req, *payload);
        SuccessOrExit(err);

        err = EnableNetwork(req);
        break;
    }
    }

exit:
    //
    // This is unfortunate that we're diluting errors returned down to a singular code. We should in-fact, be sending
    // back either IM or Cluster status codes within each of the 'handlers' above.
    //
    err = invokeInteraction.AddStatusCode(
        commandParams, { Protocols::SecureChannel::GeneralStatusCode::kSuccess, Protocols::InteractionModel::Id, (uint16_t) err });
    return err;
}

CHIP_ERROR NetworkCommissioningServer::AddThreadNetwork(NetworkCommissioningCluster::AddThreadNetworkCommand::Type & request)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (mNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            Thread::OperationalDataset & dataset = mNetworks[i].mData.mThread;
            CHIP_ERROR error                     = dataset.Init(request.OperationalDataSet);

            if (error != CHIP_NO_ERROR)
            {
                ChipLogDetail(Zcl, "Failed to parse Thread operational dataset: %s", ErrorStr(error));
                err = CHIP_ERROR_INVALID_ARGUMENT;
                break;
            }

            uint8_t extendedPanId[Thread::kSizeExtendedPanId];

            static_assert(sizeof(mNetworks[i].mNetworkID) >= sizeof(extendedPanId),
                          "Network ID must be larger than Thread extended PAN ID!");
            SuccessOrExit(dataset.GetExtendedPanId(extendedPanId));
            memcpy(mNetworks[i].mNetworkID, extendedPanId, sizeof(extendedPanId));
            mNetworks[i].mNetworkIDLen = sizeof(extendedPanId);

            mNetworks[i].mNetworkType = NetworkType::kThread;
            mNetworks[i].mEnabled     = false;

            err = CHIP_NO_ERROR;
            break;
        }
    }

exit:
    // TODO: We should encode response command here.
    ChipLogFunctError(err);
    return err;
#else
    // The target does not supports ThreadNetwork. We should not add AddThreadNetwork command in that case then the upper layer will
    // return "Command not found" error.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR
NetworkCommissioningServer::AddWifiNetwork(chip::app::Cluster::NetworkCommissioningCluster::AddWifiNetworkCommand::Type & request)
{
#if defined(CHIP_DEVICE_LAYER_TARGET)
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (mNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            VerifyOrExit(request.Ssid.size() <= sizeof(mNetworks[i].mData.mWiFi.mSSID), err = CHIP_ERROR_INVALID_ARGUMENT);
            memcpy(mNetworks[i].mData.mWiFi.mSSID, request.Ssid.data(), request.Ssid.size());

            using WiFiSSIDLenType = decltype(mNetworks[i].mData.mWiFi.mSSIDLen);
            VerifyOrExit(chip::CanCastTo<WiFiSSIDLenType>(request.Ssid.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
            mNetworks[i].mData.mWiFi.mSSIDLen = static_cast<WiFiSSIDLenType>(request.Ssid.size());

            VerifyOrExit(request.Credentials.size() <= sizeof(mNetworks[i].mData.mWiFi.mCredentials),
                         err = CHIP_ERROR_INVALID_ARGUMENT);
            memcpy(mNetworks[i].mData.mWiFi.mCredentials, request.Credentials.data(), request.Credentials.size());

            using WiFiCredentialsLenType = decltype(mNetworks[i].mData.mWiFi.mCredentialsLen);
            VerifyOrExit(chip::CanCastTo<WiFiCredentialsLenType>(request.Ssid.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
            mNetworks[i].mData.mWiFi.mCredentialsLen = static_cast<WiFiCredentialsLenType>(request.Credentials.size());

            VerifyOrExit(request.Ssid.size() <= sizeof(mNetworks[i].mNetworkID), err = CHIP_ERROR_INVALID_ARGUMENT);
            memcpy(mNetworks[i].mNetworkID, mNetworks[i].mData.mWiFi.mSSID, request.Ssid.size());

            using NetworkIDLenType = decltype(mNetworks[i].mNetworkIDLen);
            VerifyOrExit(chip::CanCastTo<NetworkIDLenType>(request.Ssid.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
            mNetworks[i].mNetworkIDLen = static_cast<NetworkIDLenType>(request.Ssid.size());

            mNetworks[i].mNetworkType = NetworkType::kWiFi;
            mNetworks[i].mEnabled     = false;

            err = CHIP_NO_ERROR;
            break;
        }
    }

    VerifyOrExit(err == CHIP_NO_ERROR, );

    ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %.*s", static_cast<int>(request.Ssid.size()), request.Ssid.data());
exit:
    // TODO: We should encode response command here.

    ChipLogDetail(Zcl, "AddWiFiNetwork: %s", ErrorStr(err));
    return err;
#else
    // The target does not supports WiFiNetwork.
    // return "Command not found" error.
    return EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
#endif
}

CHIP_ERROR NetworkCommissioningServer::DoEnableNetwork(NetworkInfo * network)
{
    switch (network->mNetworkType)
    {
    case NetworkType::kThread:
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
// TODO: On linux, we are using Reset() instead of Detach() to disable thread network, which is not expected.
// Upstream issue: https://github.com/openthread/ot-br-posix/issues/755
#if !CHIP_DEVICE_LAYER_TARGET_LINUX
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
#endif
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadProvision(network->mData.mThread.AsByteSpan()));
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(true));
#else
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
        break;
    case NetworkType::kWiFi:
#if defined(CHIP_DEVICE_LAYER_TARGET)
    {
        // TODO: Currently, DeviceNetworkProvisioningDelegateImpl assumes that ssid and credentials are null terminated strings,
        // which is not correct, this should be changed once we have better method for commissioning wifi networks.
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        ReturnErrorOnFailure(deviceDelegate.ProvisionWiFi(reinterpret_cast<const char *>(network->mData.mWiFi.mSSID),
                                                          reinterpret_cast<const char *>(network->mData.mWiFi.mCredentials)));
        break;
    }
#else
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
    break;
    case NetworkType::kEthernet:
    case NetworkType::kUndefined:
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    network->mEnabled = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkCommissioningServer::EnableNetwork(NetworkCommissioningCluster::EnableNetworkCommand::Type & request)
{
    size_t networkSeq;
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    // TODO(cecille): This is very dangerous - need to check against real netif name, ensure no password.
    constexpr char ethernetNetifMagicCode[] = "ETH0";
    if (request.NetworkId.size() == sizeof(ethernetNetifMagicCode) &&
        memcmp(request.NetworkId.data(), ethernetNetifMagicCode, request.NetworkId.size()) == 0)
    {
        ChipLogProgress(Zcl, "Wired network enabling requested. Assuming success.");
        ExitNow(err = CHIP_NO_ERROR);
    }

    for (networkSeq = 0; networkSeq < kMaxNetworks; networkSeq++)
    {
        if (mNetworks[networkSeq].mNetworkIDLen == request.NetworkId.size() &&
            mNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(mNetworks[networkSeq].mNetworkID, request.NetworkId.data(), request.NetworkId.size()) == 0)
        {
            // TODO: Currently, we cannot figure out the detailed error from network provisioning on DeviceLayer, we should
            // implement this in device layer.
            VerifyOrExit(DoEnableNetwork(&mNetworks[networkSeq]) == CHIP_NO_ERROR, err = CHIP_ERROR_INVALID_ARGUMENT);
            ExitNow(err = CHIP_NO_ERROR);
        }
    }

    // TODO: We should encode response command here.
exit:
    if (err == CHIP_NO_ERROR)
    {
        DeviceLayer::Internal::DeviceControlServer::DeviceControlSvr().EnableNetworkForOperational(request.NetworkId);
    }
    return err;
}
