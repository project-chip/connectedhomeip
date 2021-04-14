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

#pragma once

#include <cinttypes>
#include <cstdint>

#include <gen/enums.h>

#include <app/Command.h>
#include <app/common/gen/enums.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/internal/DeviceNetworkProvisioning.h>

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {

constexpr uint8_t kMaxNetworkIDLen       = 32;
constexpr uint8_t kMaxThreadDatasetLen   = 254; // As defined in Thread spec.
constexpr uint8_t kMaxWiFiSSIDLen        = 32;
constexpr uint8_t kMaxWiFiCredentialsLen = 64;
constexpr uint8_t kMaxNetworks           = 4;

namespace Internal {
enum class NetworkType : uint8_t
{
    kUndefined = 0,
    kWiFi      = 1,
    kThread    = 2,
    kEthernet  = 3,
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
        Thread::OperationalDataset mThread;
        WiFiNetworkInfo mWiFi;
    } mData;
};

enum class ClusterState : uint8_t
{
    kIdle            = 0,
    kEnablingNetwork = 1,
};

constexpr uint8_t kInvalidNetworkSeq = 0xFF;

void EncodeAndSendEnableNetworkResponse(CHIP_ERROR err);
void SetEnablingNetworkSeq(uint8_t seq);
ClusterState GetClusterState();
CHIP_ERROR __attribute__((warn_unused_result)) MoveClusterState(ClusterState newState);
void OnNetworkEnableStatusChangeCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
void SetCommissionerSecureSessionHandle(const SecureSessionHandle & handle);

NetworkInfo * GetNetworkBySeq(uint8_t seq);
} // namespace Internal

void OnAddThreadNetworkCommandCallbackInternal(app::Command *, EndpointId, ByteSpan operationalDataset, uint64_t breadcrumb,
                                               uint32_t timeoutMs);
void OnAddWiFiNetworkCommandCallbackInternal(app::Command *, EndpointId, ByteSpan ssid, ByteSpan credentials, uint64_t breadcrumb,
                                             uint32_t timeoutMs);
void OnEnableNetworkCommandCallbackInternal(app::Command *, EndpointId, ByteSpan networkID, uint64_t breadcrumb,
                                            uint32_t timeoutMs);

namespace Internal {
} // namespace Internal
} // namespace NetworkCommissioning

} // namespace clusters
} // namespace app
} // namespace chip
