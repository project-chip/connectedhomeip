/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/Darwin/NetworkCommissioningDriver.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

class DarwinWiFiDriver final : public WiFiDriver
{
public:
    // BaseDriver
    CHIP_ERROR Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;
    NetworkIterator * GetNetworks() override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override;
    uint8_t GetScanNetworkTimeoutSeconds() override;
    uint8_t GetConnectNetworkTimeoutSeconds() override;

    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;

    Status RemoveNetwork(ByteSpan ssid, MutableCharSpan & outText, uint8_t & outIndex) override;
    Status ReorderNetwork(ByteSpan ssid, uint8_t index, MutableCharSpan & outText) override;
    void ConnectNetwork(ByteSpan ssid, ConnectCallback * callback) override;

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outText, uint8_t & outIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;

private:
    void DispatchScanNetworksResponse(ScanCallback * callback, Status status, Iterator<WiFiScanResponse> * responses = nullptr);
    void DispatchConnectNetworkResponse(ConnectCallback * callback, Status status);

    WiFiNetworkInfos mNetwork;
    NetworkStatusChangeCallback * mStatusChangeCallback = nullptr;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
