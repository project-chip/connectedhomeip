/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "WiFiManager.h"

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 35;

class NrfWiFiScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    size_t Count() override { return mResultCount; }
    bool Next(WiFiScanResponse & item) override;
    void Release() override;
    void Add(const WiFiScanResponse & result);

private:
    size_t mResultId            = 0;
    size_t mResultCount         = 0;
    WiFiScanResponse * mResults = nullptr;
};

class NrfWiFiDriver final : public WiFiDriver
{
public:
    // Define non-volatile storage keys for SSID and password.
    // The naming convention is aligned with DefaultStorageKeyAllocator class.
    static constexpr const char * kSsidKey = "g/wi/s";
    static constexpr const char * kPassKey = "g/wi/p";

    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(NrfWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        NrfWiFiDriver * mDriver;
        bool mExhausted{ false };
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new WiFiNetworkIterator(this); }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return kMaxWiFiNetworks; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return kWiFiScanNetworksTimeOutSeconds; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return kWiFiConnectNetworkTimeoutSeconds; }

    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;

    Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override;
    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override;

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                              uint8_t & outNetworkIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;

    static NrfWiFiDriver & Instance()
    {
        static NrfWiFiDriver sInstance;
        return sInstance;
    }

    void OnNetworkStatusChanged(Status status);
    void OnScanWiFiNetworkResult(const WiFiScanResponse & result);
    void OnScanWiFiNetworkDone(WiFiManager::WiFiRequestStatus status);

private:
    void LoadFromStorage();

    ConnectCallback * mpConnectCallback{ nullptr };
    NetworkStatusChangeCallback * mpNetworkStatusChangeCallback{ nullptr };
    WiFiManager::WiFiNetwork mStagingNetwork;
    NrfWiFiScanResponseIterator mScanResponseIterator;
    ScanCallback * mScanCallback{ nullptr };
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
