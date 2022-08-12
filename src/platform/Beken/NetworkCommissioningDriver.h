/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "matter_pal.h"
#include <platform/NetworkCommissioning.h>

#define NC_SECURITYCONVERT(security) ((security < 3) ? security : (security == 3) ? 2 : (security < 7) ? 3 : 4)

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
} // namespace

class BKScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    BKScanResponseIterator(const size_t size, const wifi_scan_result_t * scanResults) : mSize(size), mpScanResults(scanResults) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }
        uint8_t ssidlenth = strlen(mpScanResults->aps[mIternum].ssid);
        item.security.SetRaw(NC_SECURITYCONVERT(mpScanResults->aps[mIternum].security));
        item.ssidLen  = ssidlenth;
        item.channel  = mpScanResults->aps[mIternum].channel;
        item.wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
        item.rssi     = mpScanResults->aps[mIternum].rssi;
        memcpy(item.ssid, mpScanResults->aps[mIternum].ssid, ssidlenth);
        memcpy(item.bssid, mpScanResults->aps[mIternum].bssid, 6);

        mIternum++;
        return true;
    }
    void Release() override {}

private:
    const size_t mSize;
    const wifi_scan_result_t * mpScanResults;
    size_t mIternum = 1;
};

class BekenWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(BekenWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        BekenWiFiDriver * mDriver;
        bool mExhausted = false;
    };

    struct WiFiNetwork
    {
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        char credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
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

    CHIP_ERROR ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen);
    void OnConnectWiFiNetwork();
    void OnScanWiFiNetworkDone();
    void OnNetworkStatusChange();

    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);
    int32_t GetLastDisconnectReason();

    static BekenWiFiDriver & GetInstance()
    {
        static BekenWiFiDriver instance;
        return instance;
    }
    CHIP_ERROR GetSavedNetWorkConfig(WiFiNetwork * WifiNetconf);

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);
    CHIP_ERROR StartScanWiFiNetworks(ByteSpan ssid);

    WiFiNetworkIterator mWiFiIterator = WiFiNetworkIterator(this);
    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
    int32_t mLastDisconnectedReason;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
