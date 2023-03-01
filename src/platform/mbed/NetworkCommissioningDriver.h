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

#include <inet/IPAddress.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#include <netsocket/WiFiInterface.h>

namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
} // namespace

class MbedScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    MbedScanResponseIterator(const size_t size, const WiFiAccessPoint * scanResults) : mSize(size), mScanResults(scanResults) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }

        item.security.SetRaw(mScanResults[mIternum].get_security());
        static_assert(chip::DeviceLayer::Internal::kMaxWiFiSSIDLength <= UINT8_MAX, "Our length won't fit in ssidLen");
        item.ssidLen  = static_cast<uint8_t>(strnlen(reinterpret_cast<const char *>(mScanResults[mIternum].get_ssid()),
                                                    chip::DeviceLayer::Internal::kMaxWiFiSSIDLength));
        item.channel  = mScanResults[mIternum].get_channel();
        item.wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
        item.rssi     = mScanResults[mIternum].get_rssi();
        memcpy(item.ssid, mScanResults[mIternum].get_ssid(), item.ssidLen);
        memcpy(item.bssid, mScanResults[mIternum].get_bssid(), 6);

        mIternum++;
        return true;
    }
    void Release() override {}

private:
    const size_t mSize;
    const WiFiAccessPoint * mScanResults;
    size_t mIternum = 0;
};

class WiFiDriverImpl final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(WiFiDriverImpl * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        WiFiDriverImpl * mDriver;
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
    void DisconnectNetwork(ByteSpan networkId);

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                              uint8_t & outNetworkIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;

    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);
    int32_t GetLastDisconnectReason();

    void OnNetworkStatusChange();

    static WiFiDriverImpl & GetInstance()
    {
        static WiFiDriverImpl instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);

    static void OnScanNetwork(intptr_t arg);
    void ExecuteScanNetwork(void);

    static void OnConnectNetwork(intptr_t arg);
    void ExecuteConnectNetwork(void);

    static void OnWiFiInterfaceEvent(nsapi_event_t event, intptr_t data);
    void ExecuteWiFiInterfaceChange(nsapi_connection_status_t new_status);

    void OnNetworkConnected();
    void OnNetworkDisconnected();
    void OnNetworkConnecting();

    chip::DeviceLayer::Internal::WiFiAuthSecurityType NsapiToNetworkSecurity(nsapi_security_t nsapi_security);

    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
    ScanCallback * mScanCallback;
    ConnectCallback * mConnectCallback;
    char mScanSSID[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    bool mScanSpecific = false;

    WiFiInterface * mWiFiInterface = nullptr;
    nsapi_security_t mSecurityType = NSAPI_SECURITY_NONE;
    Inet::IPAddress mIp4Address    = Inet::IPAddress::Any;
    Inet::IPAddress mIp6Address    = Inet::IPAddress::Any;

    NetworkStatusChangeCallback * mStatusChangeCallback = nullptr;
    int32_t mLastDisconnectedReason;
};
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
