/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <pkg_protocol.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

void NetworkEventHandler(const ChipDeviceEvent * event, intptr_t arg);

namespace {
inline constexpr uint8_t kMaxWiFiNetworks                  = 1;
inline constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
inline constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
} // namespace

class BLScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    BLScanResponseIterator(size_t size, WiFiScanResponse * pScanResults) : mSize(size), mpScanResults(pScanResults) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }

        memcpy(&item, mpScanResults + mIternum, sizeof(WiFiScanResponse));

        mIternum++;
        return true;
    }
    void Release() override {}

private:
    const size_t mSize;
    const WiFiScanResponse * mpScanResults;
    size_t mIternum = 0;
};

class BLWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(BLWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        BLWiFiDriver * mDriver;
        bool mExhausted = false;
    };

    struct WiFiNetwork
    {
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        char credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
    };
    enum WiFiCredentialLength
    {
        kOpen      = 0,
        kWEP64     = 5,
        kMinWPAPSK = 8,
        kMaxWPAPSK = 63,
        kWPAPSKHex = 64,
    };
    // BaseDriver
    NetworkIterator * GetNetworks() override { return new WiFiNetworkIterator(this); }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown();

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
    void OnConnectWiFiNetwork(bool isConnected);
    void OnScanWiFiNetworkDone(void);
    void OnScanWiFiNetworkDone(void * opaque);
    void OnNetworkStatusChange(void);

    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);
    int32_t GetLastDisconnectReason();

    static BLWiFiDriver & GetInstance()
    {
        static BLWiFiDriver instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);

    WiFiScanResponse * mScanResponse = nullptr;
    size_t mScanResponseNum          = 0;

    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
    int32_t mLastDisconnectedReason;

    /** +1 byte for string termination */
    char mScanSSID[chip::DeviceLayer::Internal::kMaxWiFiSSIDLength + 1];
    bool mScanSpecific = false;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
