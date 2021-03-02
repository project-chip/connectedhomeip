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
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
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
constexpr uint8_t kMaxThreadDatasetLen   = 128;
constexpr uint8_t kMaxWiFiSSIDLen        = 32;
constexpr uint8_t kMaxWiFiCredentialsLen = 64;
constexpr uint8_t kMaxNetworks           = 4;
constexpr char kThreadNetworkName[]      = "tmp-thread-network";

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

// TODO: We cannot get length of operational dataset here, should be fixed after we have byte string type.
// Note: The codegen for OCTET_STRING will be implemented later since no other clusters are using OCTET_STRING.
EmberAfNetworkCommissioningError OnAddThreadNetworkCommandCallbackInternal(app::Command *, EndpointId,
                                                                           const uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                           uint32_t timeoutMs)
{
    // TODO: Implement AddThreadNetworkCommand after OCTET_STRING is supported.
    return EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_NETWORK_NOT_FOUND;
}

// TODO: We cannot get length of ssid / credentials here, should be fixed after we have byte string type in ZCL codegen.
// Note: SSID and credentials are not necessarily to be string.
EmberAfNetworkCommissioningError OnAddWiFiNetworkCommandCallbackInternal(app::Command *, EndpointId, const uint8_t * ssid,
                                                                         const uint8_t * credentials, uint64_t breadcrumb,
                                                                         uint32_t timeoutMs)
{
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            // Assume they are null terminated, see notes above.
            size_t ssidLen        = strlen(reinterpret_cast<const char *>(ssid));
            size_t credentialsLen = strlen(reinterpret_cast<const char *>(credentials));

            VerifyOrExit(ssidLen <= kMaxWiFiSSIDLen, err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED);
            VerifyOrExit(credentialsLen <= kMaxWiFiCredentialsLen, err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED);

            memcpy(sNetworks[i].mData.mWiFi.mSSID, ssid, ssidLen);
            sNetworks[i].mData.mWiFi.mSSIDLen = ssidLen;
            memcpy(sNetworks[i].mData.mWiFi.mCredentials, credentials, credentialsLen);
            sNetworks[i].mData.mWiFi.mCredentialsLen = credentialsLen;
            memcpy(sNetworks[i].mNetworkID, sNetworks[i].mData.mWiFi.mSSID, ssidLen);
            sNetworks[i].mNetworkIDLen = ssidLen;

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
        // TODO(#5040): Add support for thread network provisioning.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case NetworkType::kWiFi:
#if defined(CHIP_DEVICE_LAYER_TARGET)
    {
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        return deviceDelegate.ProvisionWiFi(reinterpret_cast<const char *>(network->mData.mWiFi.mSSID),
                                            reinterpret_cast<const char *>(network->mData.mWiFi.mCredentials));
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
                                                                        uint64_t breadcrumb, uint32_t timeoutMs)
{
    size_t networkSeq;
    size_t networkIDLen                  = strlen(reinterpret_cast<const char *>(networkID));
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
