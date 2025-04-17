/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <list>

#include <controller/CommissioningDelegate.h>
#include <lib/support/DLLUtil.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/RendezvousParameters.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_BLE

#include <deque>

namespace chip {
namespace Controller {

class DeviceCommissioner;

class NetworkRecoverParameters : public RendezvousParameters
{
public:
    NetworkRecoverParameters() = default;
#if CONFIG_NETWORK_LAYER_BLE
    NetworkRecoverParameters(BLE_CONNECTION_OBJECT connObj);
#endif // CONFIG_NETWORK_LAYER_BLE
    NetworkRecoverParameters(uint64_t recoveryId);

    bool HasRecoveryId() { return mRecoveryId != 0; }
    uint64_t GetRecoveryId() { return mRecoveryId; }

private:
    uint64_t mRecoveryId;
};

/// Callbacks for CHIP network recovery
class DLL_EXPORT NetworkRecoverDelegate
{
public:
    virtual ~NetworkRecoverDelegate() {}
    virtual void OnNetworkRecoverDiscover(std::list<uint64_t> recoveryIds)   = 0;
    virtual void OnNetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error) = 0;
};

enum class NetworkRecoverBehaviour : uint8_t
{
    kDiscover,
    kRecover,
};

class DLL_EXPORT NetworkRecover
{
public:
    NetworkRecover(DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}
    virtual ~NetworkRecover() {}

    CHIP_ERROR Discover(uint16_t timeout);
    CHIP_ERROR Recover(NodeId remoteId, uint64_t recoveryId, WiFiCredentials wiFiCreds, uint64_t breadcrumb = 0);

    DeviceCommissioner * GetCommissioner() { return mCommissioner; }

    void SetSystemLayer(System::Layer * systemLayer) { mSystemLayer = systemLayer; }
#if CONFIG_NETWORK_LAYER_BLE
    void SetBleLayer(Ble::BleLayer * bleLayer) { mBleLayer = bleLayer; }
#endif
    void SetNetworkRecoverDelegate(NetworkRecoverDelegate * delegate) { mNetworkRecoverDelegate = delegate; }
    NetworkRecoverDelegate * GetNetworkRecoverDelegate() const { return mNetworkRecoverDelegate; }

    void NetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error);

private:
    CHIP_ERROR StartDiscoverOverBle(uint64_t recoveryId);
    CHIP_ERROR StopConnectOverBle();
    bool NotifyOrConnectToDiscoveredDevice();
    void NetworkRecoverDiscoverFinish();
    static void OnRecoverableDiscoveredTimeoutCallback(System::Layer * layer, void * context);

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
    void OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj);
    void OnDiscoveredDeviceOverBle(uint64_t recoveryId);
    void OnBLEDiscoveryError(CHIP_ERROR err);
    /////////// BLEConnectionDelegate Callbacks /////////
    static void OnDiscoveredDeviceOverBle(void * appState, uint64_t recoveryId);
    static void OnDiscoveredDeviceOverBleConnected(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
#endif // CONFIG_NETWORK_LAYER_BLE

    NodeId mRemoteId;
    Optional<WiFiCredentials> mWiFiCreds;
    Optional<ByteSpan> threadOperationalDataset;
    uint64_t mBreadcrumb;

    NetworkRecoverBehaviour mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kDiscover;
    NetworkRecoverDelegate * mNetworkRecoverDelegate = nullptr;
    DeviceCommissioner * mCommissioner               = nullptr;
    std::deque<NetworkRecoverParameters> mNetworkRecoverParameters;
    System::Layer * mSystemLayer = nullptr;
    std::list<uint64_t> mDiscoveredRecoveryIds;
    bool mDiscoverTimeout;
};

} // namespace Controller
} // namespace chip
