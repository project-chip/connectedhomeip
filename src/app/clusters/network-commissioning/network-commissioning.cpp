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

#include <lib/support/CodeUtils.h>
#include <lib/support/ReturnMacros.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <transport/NetworkProvisioning.h>

#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif

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

// TODO: The network info should be stored in attribute storage instead of memory.
// TODO: We cannot get length of operational dataset here, should be fixed after we have byte string type.
// TODO: Should encode response for command responses.
enum class NetworkCommissioninError : uint8_t
{
    kSuccess                = 0,
    kOutOfRange             = 1,
    kBoundsExceeded         = 2,
    kNetworkIDNotFound      = 3,
    kDuplicateNetworkID     = 4,
    kNetworkNotFound        = 5,
    kRegulatoryError        = 6,
    kAuthFailure            = 7,
    kUnsupportedSecurity    = 8,
    kOtherConnectionFailure = 9,
    kIPV6Failed             = 10,
    kIPBindFailed           = 11,
    kLabel9                 = 12,
    kLabel10                = 13,
    kLabel11                = 14,
    kLabel12                = 15,
    kLabel13                = 16,
    kLabel14                = 17,
    kLabel15                = 18,
    kUnknownError           = 19,
};

enum class NetworkType : uint8_t
{
    kUndefined = 0,
    kWiFi      = 1,
    kThread    = 2,
    kEthernet  = 3,
};

enum class WiFiSecurityType : uint8_t
{
    kUnencrypted  = 0,
    kWEPPersonal  = 1,
    kWPAPersonal  = 2,
    kWPA2Personal = 3,
    kWPA3Personal = 4,
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
    WiFiSecurityType mSecurity;
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
NetworkInfo sNetworks[kMaxNetworks];
}

void OnAddThreadNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * operationalDataset, uint64_t breadcrumb,
                                       uint32_t timeoutMs)
{
    CHIP_ERROR err                = CHIP_ERROR_NO_MEMORY;
    size_t operationalDataset_Len = strlen(reinterpret_cast<const char *>(operationalDataset));

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            memcpy(sNetworks[i].mData.mThread.mDataset, operationalDataset, operationalDataset_Len);
            sNetworks[i].mData.mThread.mDatasetLen = operationalDataset_Len;

            sNetworks[i].mNetworkType = NetworkType::kThread;
            sNetworks[i].mEnabled     = false;

            memcpy(sNetworks[i].mNetworkID, kThreadNetworkName, sizeof(kThreadNetworkName));
            sNetworks[i].mNetworkIDLen = sizeof(kThreadNetworkName);

            err = CHIP_NO_ERROR;
            break;
        }
    }

    SuccessOrExit(err);
exit:
    ChipLogFunctError(err);
}

void OnAddWiFiNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * ssid, const uint8_t * credentials,
                                     uint64_t breadcrumb, uint32_t timeoutMs)
{
    CHIP_ERROR err         = CHIP_ERROR_NO_MEMORY;
    size_t ssid_Len        = strlen(reinterpret_cast<const char *>(ssid));
    size_t credentials_Len = strlen(reinterpret_cast<const char *>(credentials));

    for (size_t i = 0; i < kMaxNetworks; i++)
    {
        if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
        {
            memcpy(sNetworks[i].mData.mWiFi.mSSID, ssid, ssid_Len);
            sNetworks[i].mData.mWiFi.mSSIDLen = ssid_Len;
            memcpy(sNetworks[i].mData.mWiFi.mCredentials, credentials, credentials_Len);
            sNetworks[i].mData.mWiFi.mCredentialsLen = credentials_Len;
            memcpy(sNetworks[i].mNetworkID, sNetworks[i].mData.mWiFi.mSSID, ssid_Len);
            sNetworks[i].mNetworkIDLen = ssid_Len;

            sNetworks[i].mNetworkType = NetworkType::kWiFi;
            sNetworks[i].mEnabled     = false;

            err = CHIP_NO_ERROR;
            break;
        }
    }

    SuccessOrExit(err);

    ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %s", ssid);
exit:
    ChipLogFunctError(err);
}

void OnEnableNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t networkSeq;
    size_t networkID_Len = strlen(reinterpret_cast<const char *>(networkID));

    for (networkSeq = 0; networkSeq < kMaxNetworks; networkSeq++)
    {
        if (sNetworks[networkSeq].mNetworkIDLen == networkID_Len && sNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(sNetworks[networkSeq].mNetworkID, networkID, networkID_Len) == 0)
        {
            break;
        }
    }

    VerifyOrExit(networkSeq != kMaxNetworks, err = CHIP_ERROR_KEY_NOT_FOUND);

    ChipLogDetail(Zcl, "Get network: %" PRIuMAX, networkSeq);

    // Just ignore if the network is already enabled
    VerifyOrExit(!sNetworks[networkSeq].mEnabled, err = CHIP_NO_ERROR);

    ChipLogDetail(Zcl, "Enable network");

    switch (sNetworks[networkSeq].mNetworkType)
    {
    case NetworkType::kThread:
        // TODO(#5040): Add support for thread network provisioning.
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        break;
    case NetworkType::kWiFi:
#if defined(CHIP_DEVICE_LAYER_TARGET)
    {
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        SuccessOrExit(
            err = deviceDelegate.ProvisionWiFi(reinterpret_cast<const char *>(sNetworks[networkSeq].mData.mWiFi.mSSID),
                                               reinterpret_cast<const char *>(sNetworks[networkSeq].mData.mWiFi.mCredentials)));
    }
#else
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
#endif
    break;
    case NetworkType::kEthernet:
    case NetworkType::kUndefined:
    default:
        ExitNow(err = CHIP_ERROR_NOT_IMPLEMENTED);
    }

    sNetworks[networkSeq].mEnabled = true;

exit:
    ChipLogFunctError(err);
}

// TODO: The following callbacks needs to be finished.
void OnDisableNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{}
void OnGetLastNetworkCommissioningResultCommandCallback(app::Command *, EndpointId, uint32_t timeoutMs) {}
void OnRemoveNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * NetworkID, uint64_t Breadcrumb, uint32_t TimeoutMs)
{}
void OnScanNetworksCommandCallback(app::Command *, EndpointId, const uint8_t * ssid, uint64_t breadcrumb, uint32_t timeoutMs) {}
void OnUpdateWiFiNetworkCommandCallback(app::Command *, EndpointId, const uint8_t * ssid, const uint8_t * credentials,
                                        uint64_t breadcrumb, uint32_t timeoutMs)
{}

} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
