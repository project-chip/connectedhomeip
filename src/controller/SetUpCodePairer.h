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

/**
 *    @file
 *      Declaration of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/DLLUtil.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_BLE

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
#include <controller/DeviceDiscoveryDelegate.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

namespace chip {
namespace Controller {

class DeviceCommissioner;

class DLL_EXPORT SetUpCodePairer
{
public:
    SetUpCodePairer(DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}
    virtual ~SetUpCodePairer() {}

    CHIP_ERROR PairDevice(chip::NodeId remoteId, const char * setUpCode);

// Called by the DeviceCommissioner to notify that we have discovered a new device.
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    void NotifyCommissionableDeviceDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData);
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

#if CONFIG_NETWORK_LAYER_BLE
    void SetBleLayer(Ble::BleLayer * bleLayer) { mBleLayer = bleLayer; };
#endif // CONFIG_NETWORK_LAYER_BLE

private:
    CHIP_ERROR Connect(SetupPayload & paload);
    CHIP_ERROR StartDiscoverOverBle(SetupPayload & payload);
    CHIP_ERROR StopConnectOverBle();
    CHIP_ERROR StartDiscoverOverIP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverIP();
    CHIP_ERROR StartDiscoverOverSoftAP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverSoftAP();

    void OnDeviceDiscovered(RendezvousParameters & params);

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
    void OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj);
    /////////// BLEConnectionDelegate Callbacks /////////
    static void OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
#endif // CONFIG_NETWORK_LAYER_BLE

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    bool NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData);
    Dnssd::DiscoveryFilter currentFilter;
#endif

    DeviceCommissioner * mCommissioner = nullptr;
    chip::NodeId mRemoteId;
    uint32_t mSetUpPINCode = 0;
};

} // namespace Controller
} // namespace chip
