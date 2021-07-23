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

#include <app/CommandHandler.h>
#include <app/common/gen/enums.h>
#include <lib/core/CHIPCore.h>
#include <platform/internal/DeviceNetworkProvisioning.h>
#include <Cluster.h>
#include <NetworkCommissioningCluster-Gen.h>
#include <platform/CHIPDeviceLayer.h>
#include <lib/support/ThreadOperationalDataset.h>

namespace chip {
namespace app {
namespace Cluster {

#ifndef CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS
#define CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS 4
#endif // CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS

class NetworkCommissioningServer : public ClusterServer {
public:
    NetworkCommissioningServer();

private:
    CHIP_ERROR OnInvokeRequest(CommandParams &commandParams, InvokeResponder &invokeInteraction, TLV::TLVReader *payload) override;
    CHIP_ERROR AddThreadNetwork(chip::app::Cluster::NetworkCommissioningCluster::AddThreadNetworkCommand::Type &request);
    CHIP_ERROR AddWifiNetwork(chip::app::Cluster::NetworkCommissioningCluster::AddWifiNetworkCommand::Type &request);
    CHIP_ERROR EnableNetwork(chip::app::Cluster::NetworkCommissioningCluster::EnableNetworkCommand::Type &request);

private:
    constexpr static uint8_t kMaxNetworkIDLen       = 32;
    constexpr static uint8_t kMaxThreadDatasetLen   = 254; // As defined in Thread spec.
    constexpr static uint8_t kMaxWiFiSSIDLen        = 32;
    constexpr static uint8_t kMaxWiFiCredentialsLen = 64;
    constexpr static uint8_t kMaxNetworks           = CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS;

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

    CHIP_ERROR DoEnableNetwork(NetworkInfo *network);

private:
    NetworkInfo mNetworks[kMaxNetworks];

};

} // namespace clusters
} // namespace app
} // namespace chip
