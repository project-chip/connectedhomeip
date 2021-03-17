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

#include <gen/att-storage.h>
#include <gen/attribute-id.h>
#include <gen/attribute-type.h>
#include <gen/callback.h>
#include <gen/cluster-id.h>
#include <gen/command-id.h>
#include <gen/enums.h>
#include <util/af.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ReturnMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#include <transport/NetworkProvisioning.h>

// Include DeviceNetworkProvisioningDelegateImpl for WiFi provisioning.
// TODO: Enable wifi network should be done by ConnectivityManager. (Or other platform neutral interfaces)
#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif

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
constexpr uint8_t kMaxNetworks           = 4;

// The temporary network id which will be used by thread networks in CHIP before we have the API for getting extpanid from dataset
// tlv.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
constexpr uint8_t kTemporaryThreadNetworkId[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
#endif

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
        ThreadNetworkInfo mThread;
        WiFiNetworkInfo mWiFi;
    } mData;
};

namespace {
// The internal network info containing credentials. Need to find some better place to save these info.
NetworkInfo sNetworks[kMaxNetworks];
} // namespace

EmberAfNetworkCommissioningError OnAddThreadNetworkCommandCallbackInternal(app::Command *, EndpointId,
                                                                           const uint8_t * operationalDataset,
                                                                           size_t operationalDatasetLen, uint64_t breadcrumb,
                                                                           uint32_t timeoutMs)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            VerifyOrExit(operationalDatasetLen <= sizeof(ThreadNetworkInfo::mDataset),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mData.mThread.mDataset, operationalDataset, operationalDatasetLen);

            using ThreadDatasetLenType = decltype(sNetworks[i].mData.mThread.mDatasetLen);
            VerifyOrExit(chip::CanCastTo<ThreadDatasetLenType>(operationalDatasetLen),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mData.mThread.mDatasetLen = static_cast<ThreadDatasetLenType>(operationalDatasetLen);

            // A 64bit id for thread networks, currently, we are missing some API for getting the thread network extpanid from the
            // dataset tlv.
            static_assert(sizeof(kTemporaryThreadNetworkId) <= sizeof(sNetworks[i].mNetworkID),
                          "TemporaryThreadNetworkId too long");
            memcpy(sNetworks[i].mNetworkID, kTemporaryThreadNetworkId, sizeof(kTemporaryThreadNetworkId));
            sNetworks[i].mNetworkIDLen = sizeof(kTemporaryThreadNetworkId);

            sNetworks[i].mNetworkType = NetworkType::kThread;
            sNetworks[i].mEnabled     = false;

            err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
            break;
        }
    }

exit:
    // TODO: We should encode response command here.

    ChipLogDetail(Zcl, "AddThreadNetwork: %d", err);
    return err;
#else
    // The target does not supports ThreadNetwork. We should not add AddThreadNetwork command in that case then the upper layer will
    // return "Command not found" error.
    return EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
#endif
}

EmberAfNetworkCommissioningError OnAddWiFiNetworkCommandCallbackInternal(app::Command *, EndpointId, const uint8_t * ssid,
                                                                         size_t ssidLen, const uint8_t * credentials,
                                                                         size_t credentialsLen, uint64_t breadcrumb,
                                                                         uint32_t timeoutMs)
{
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            VerifyOrExit(ssidLen <= sizeof(sNetworks[i].mData.mWiFi.mSSID),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mData.mWiFi.mSSID, ssid, ssidLen);

            using WiFiSSIDLenType = decltype(sNetworks[i].mData.mWiFi.mSSIDLen);
            VerifyOrExit(chip::CanCastTo<WiFiSSIDLenType>(ssidLen), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mData.mWiFi.mSSIDLen = static_cast<WiFiSSIDLenType>(ssidLen);

            VerifyOrExit(credentialsLen <= sizeof(sNetworks[i].mData.mWiFi.mCredentials),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mData.mWiFi.mCredentials, credentials, credentialsLen);

            using WiFiCredentialsLenType = decltype(sNetworks[i].mData.mWiFi.mCredentialsLen);
            VerifyOrExit(chip::CanCastTo<WiFiCredentialsLenType>(ssidLen),
                         err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mData.mWiFi.mCredentialsLen = static_cast<WiFiCredentialsLenType>(credentialsLen);

            VerifyOrExit(ssidLen <= sizeof(sNetworks[i].mNetworkID), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            memcpy(sNetworks[i].mNetworkID, sNetworks[i].mData.mWiFi.mSSID, ssidLen);

            using NetworkIDLenType = decltype(sNetworks[i].mNetworkIDLen);
            VerifyOrExit(chip::CanCastTo<NetworkIDLenType>(ssidLen), err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE);
            sNetworks[i].mNetworkIDLen = static_cast<NetworkIDLenType>(ssidLen);

            sNetworks[i].mNetworkType = NetworkType::kWiFi;
            sNetworks[i].mEnabled     = false;

            err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
            break;
        }
    }

    VerifyOrExit(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS, );

    ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %s", ssid);
exit:
    // TODO: We should encode response command here.

    ChipLogDetail(Zcl, "AddWiFiNetwork: %d", err);
    return err;
}

namespace {
CHIP_ERROR DoEnableNetwork(NetworkInfo * network)
{
    switch (network->mNetworkType)
    {
    case NetworkType::kThread:
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
        ReturnErrorOnFailure(
            DeviceLayer::ThreadStackMgr().SetThreadProvision(network->mData.mThread.mDataset, network->mData.mThread.mDatasetLen));
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

EmberAfNetworkCommissioningError OnEnableNetworkCommandCallbackInternal(app::Command *, EndpointId, const uint8_t * networkID,
                                                                        size_t networkIDLen, uint64_t breadcrumb,
                                                                        uint32_t timeoutMs)
{
    size_t networkSeq;
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_NETWORK_ID_NOT_FOUND;

    for (networkSeq = 0; networkSeq < kMaxNetworks; networkSeq++)
    {
        if (sNetworks[networkSeq].mNetworkIDLen == networkIDLen && sNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(sNetworks[networkSeq].mNetworkID, networkID, networkIDLen) == 0)
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
    return err;
}

} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
