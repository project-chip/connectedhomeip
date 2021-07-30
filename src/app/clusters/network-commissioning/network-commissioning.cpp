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

#include <cstring>
#include <type_traits>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceControlServer.h>

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
#ifndef CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS
#define CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS 4
#endif // CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS

using namespace chip;
using namespace chip::app;
using namespace chip::app::clusters;
using namespace chip::app::clusters::NetworkCommissioning;

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {

constexpr uint8_t kMaxNetworkIDLen       = 32;
constexpr uint8_t kMaxThreadDatasetLen   = 254; // As defined in Thread spec.
constexpr uint8_t kMaxWiFiSSIDLen        = 32;
constexpr uint8_t kMaxWiFiCredentialsLen = 64;
constexpr uint8_t kMaxNetworks           = CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS;

enum class NetworkType : uint8_t
{
    kUndefined = 0,
    kWiFi      = 1,
    kThread    = 2,
    kEthernet  = 3,
};

struct ThreadNetworkInfo
{
    uint8_t mDataset[kMaxThreadDatasetLen];
    uint8_t mDatasetLen;
};

struct WiFiNetworkInfo
{
    uint8_t mSSID[kMaxWiFiSSIDLen + 1];
    uint8_t mSSIDLen;
    uint8_t mCredentials[kMaxWiFiCredentialsLen];
    uint8_t mCredentialsLen;
};

struct NetworkInfo
{
    uint8_t mNetworkID[kMaxNetworkIDLen];
    uint8_t mNetworkIDLen;
    uint8_t mEnabled;
    NetworkType mNetworkType;
    union NetworkData
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        Thread::OperationalDataset mThread;
#endif
#if defined(CHIP_DEVICE_LAYER_TARGET)
        WiFiNetworkInfo mWiFi;
#endif
    } mData;
};

namespace {
// The internal network info containing credentials. Need to find some better place to save these info.
NetworkInfo sNetworks[kMaxNetworks];
} // namespace

EmberAfNetworkCommissioningError OnAddThreadNetworkCommandCallbackInternal(app::CommandHandler *, EndpointId,
                                                                           ByteSpan operationalDataset, uint64_t breadcrumb,
                                                                           uint32_t timeoutMs)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            Thread::OperationalDataset & dataset = sNetworks[i].mData.mThread;
            CHIP_ERROR error                     = dataset.Init(operationalDataset);

            if (error != CHIP_NO_ERROR)
            {
                ChipLogDetail(Zcl, "Failed to parse Thread operational dataset: %s", ErrorStr(error));
                err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
                break;
            }

            uint8_t extendedPanId[Thread::kSizeExtendedPanId];

            static_assert(sizeof(sNetworks[i].mNetworkID) >= sizeof(extendedPanId),
                          "Network ID must be larger than Thread extended PAN ID!");
            SuccessOrExit(dataset.GetExtendedPanId(extendedPanId));
            memcpy(sNetworks[i].mNetworkID, extendedPanId, sizeof(extendedPanId));
            sNetworks[i].mNetworkIDLen = sizeof(extendedPanId);

            sNetworks[i].mNetworkType = NetworkType::kThread;
            sNetworks[i].mEnabled     = false;

            err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
            break;
        }
    }

exit:
    // TODO: We should encode response command here.

    ChipLogDetail(Zcl, "AddThreadNetwork: %" PRIu8, err);
    return err;
#else
    // The target does not supports ThreadNetwork. We should not add AddThreadNetwork command in that case then the upper layer will
    // return "Command not found" error.
    return EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
#endif
}

EmberAfNetworkCommissioningError OnAddWiFiNetworkCommandCallbackInternal(app::CommandHandler *, EndpointId, ByteSpan ssid,
                                                                         ByteSpan credentials, uint64_t breadcrumb,
                                                                         uint32_t timeoutMs)
{
#if defined(CHIP_DEVICE_LAYER_TARGET)
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            VerifyOrExit(ssid.size() <= sizeof(sNetworks[i].mData.mWiFi.mSSID),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mData.mWiFi.mSSID, ssid.data(), ssid.size());

            using WiFiSSIDLenType = decltype(sNetworks[i].mData.mWiFi.mSSIDLen);
            VerifyOrExit(CanCastTo<WiFiSSIDLenType>(ssid.size()), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mData.mWiFi.mSSIDLen = static_cast<WiFiSSIDLenType>(ssid.size());

            VerifyOrExit(credentials.size() <= sizeof(sNetworks[i].mData.mWiFi.mCredentials),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mData.mWiFi.mCredentials, credentials.data(), credentials.size());

            using WiFiCredentialsLenType = decltype(sNetworks[i].mData.mWiFi.mCredentialsLen);
            VerifyOrExit(CanCastTo<WiFiCredentialsLenType>(ssid.size()), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mData.mWiFi.mCredentialsLen = static_cast<WiFiCredentialsLenType>(credentials.size());

            VerifyOrExit(ssid.size() <= sizeof(sNetworks[i].mNetworkID), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mNetworkID, sNetworks[i].mData.mWiFi.mSSID, ssid.size());

            using NetworkIDLenType = decltype(sNetworks[i].mNetworkIDLen);
            VerifyOrExit(CanCastTo<NetworkIDLenType>(ssid.size()), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mNetworkIDLen = static_cast<NetworkIDLenType>(ssid.size());

            sNetworks[i].mNetworkType = NetworkType::kWiFi;
            sNetworks[i].mEnabled     = false;

            err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
            break;
        }
    }

    VerifyOrExit(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS, );

    ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %.*s", static_cast<int>(ssid.size()), ssid.data());
exit:
    // TODO: We should encode response command here.

    ChipLogDetail(Zcl, "AddWiFiNetwork: %" PRIu8, err);
    return err;
#else
    // The target does not supports WiFiNetwork.
    // return "Command not found" error.
    return EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
#endif
}

namespace {
CHIP_ERROR DoEnableNetwork(NetworkInfo * network)
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
} // namespace

EmberAfNetworkCommissioningError OnEnableNetworkCommandCallbackInternal(app::CommandHandler *, EndpointId, ByteSpan networkID,
                                                                        uint64_t breadcrumb, uint32_t timeoutMs)
{
    size_t networkSeq;
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_NETWORK_ID_NOT_FOUND;
    // TODO(cecille): This is very dangerous - need to check against real netif name, ensure no password.
    constexpr char ethernetNetifMagicCode[] = "ETH0";
    if (networkID.size() == sizeof(ethernetNetifMagicCode) &&
        memcmp(networkID.data(), ethernetNetifMagicCode, networkID.size()) == 0)
    {
        ChipLogProgress(Zcl, "Wired network enabling requested. Assuming success.");
        ExitNow(err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS);
    }

    for (networkSeq = 0; networkSeq < kMaxNetworks; networkSeq++)
    {
        if (sNetworks[networkSeq].mNetworkIDLen == networkID.size() &&
            sNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(sNetworks[networkSeq].mNetworkID, networkID.data(), networkID.size()) == 0)
        {
            // TODO: Currently, we cannot figure out the detailed error from network provisioning on DeviceLayer, we should
            // implement this in device layer.
            VerifyOrExit(DoEnableNetwork(&sNetworks[networkSeq]) == CHIP_NO_ERROR,
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR);
            ExitNow(err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS);
        }
    }
    // TODO: We should encode response command here.
exit:
    if (err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS)
    {
        DeviceLayer::Internal::DeviceControlServer::DeviceControlSvr().EnableNetworkForOperational(networkID);
    }
    return err;
}

} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
