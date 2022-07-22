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

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/ConfigurationManagerImpl.h>
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreWLAN/CoreWLAN.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::NetworkCommissioning;
using chip::DeviceLayer::ConfigurationManagerImpl;

constexpr size_t kMaxNetworks = 1;
constexpr size_t kScanNetworkTimeoutSeconds = 30;
constexpr size_t kConnectNetworkTimeoutSeconds = 30;

namespace {
class DarwinWiFiNetworkIterator final : public DarwinNetworkIterator {
public:
    DarwinWiFiNetworkIterator()
    {
        auto & connectivityMgr = ConnectivityMgrImpl();
        ReturnOnFailure(connectivityMgr.GetWiFiInterfaceName(mInterfaceName, sizeof(mInterfaceName)));
        ReturnOnFailure(connectivityMgr.GetInterfaceStatus(mInterfaceName, &mInterfaceStatus));
        mInterfaceCount++;
    }
};

class DarwinScanResponseIterator : public Iterator<WiFiScanResponse> {
public:
    DarwinScanResponseIterator(NSArray * networks) { mNetworks = networks; }

    bool Next(WiFiScanResponse & scanResponse) override
    {
        if (mIndex >= [mNetworks count]) {
            return false;
        }

        CWNetwork * network = mNetworks[mIndex];
        CopyNetworkInformationTo(scanResponse, network);

        mIndex++;
        return true;
    }

    size_t Count() override { return [mNetworks count]; }

    void Release() override { mNetworks = nil; }

private:
    NSArray * mNetworks = nil;
    size_t mIndex = 0;

    void CopyNetworkInformationTo(WiFiScanResponse & destination, CWNetwork * source)
    {
        destination.security = GetWiFiSecurity(source);
        destination.ssidLen = [source.ssid length];
        destination.channel = source.wlanChannel.channelNumber;
        destination.wiFiBand = GetWiFiBand(source.wlanChannel);
        destination.rssi = source.rssiValue;
        CopyStringTo(destination.ssid, source.ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
        CopyStringTo(destination.bssid, source.bssid, DeviceLayer::Internal::kWiFiBSSIDLength);
    }

    void CopyStringTo(uint8_t * destination, NSString * source, size_t maxLength)
    {
        NSData * data = [source dataUsingEncoding:NSUTF8StringEncoding];
        memcpy(destination, [data bytes], std::min([data length], maxLength));
    }

    WiFiSecurity GetWiFiSecurity(CWNetwork * network)
    {
        if ([network supportsSecurity:kCWSecurityNone]) {
            return WiFiSecurity::kUnencrypted;
        }

        if ([network supportsSecurity:kCWSecurityWEP]) {
            return WiFiSecurity::kWepPersonal;
        }

        if ([network supportsSecurity:kCWSecurityWPAPersonal]) {
            return WiFiSecurity::kWpaPersonal;
        }

        if ([network supportsSecurity:kCWSecurityWPA2Personal]) {
            return WiFiSecurity::kWpa2Personal;
        }

        if ([network supportsSecurity:kCWSecurityWPA3Personal]) {
            return WiFiSecurity::kWpa3Personal;
        }

        // If the value is not one of the preceding, it is unclear about how to map it.
        return WiFiSecurity::kUnencrypted;
    }

    WiFiBand GetWiFiBand(CWChannel * channel)
    {
        if (channel.channelBand == kCWChannelBand2GHz) {
            return WiFiBand::k2g4;
        }

        if (channel.channelBand == kCWChannelBand5GHz) {
            return WiFiBand::k5g;
        }

        // If the value is not one of the preceding, it is unclear about how to map it.
        return WiFiBand::k2g4;
    }
};

}

namespace chip {
namespace DeviceLayer {
    namespace NetworkCommissioning {
        CHIP_ERROR DarwinWiFiDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
        {
            auto & configurationMgr = ConfigurationManagerImpl::GetDefaultInstance();
            auto & connectivityMgr = ConnectivityMgrImpl();

            ReturnErrorOnFailure(configurationMgr.GetWiFiNetworkInformations(mNetwork));
            if (!connectivityMgr.IsWiFiStationConnected() && connectivityMgr.IsWiFiStationProvisioned()) {
                LogErrorOnFailure(connectivityMgr.ConnectWiFiNetwork(mNetwork));
            }

            mStatusChangeCallback = networkStatusChangeCallback;
            return CHIP_NO_ERROR;
        }

        void DarwinWiFiDriver::Shutdown() { mStatusChangeCallback = nullptr; }

        NetworkIterator * DarwinWiFiDriver::GetNetworks() { return new DarwinWiFiNetworkIterator(); }

        uint8_t DarwinWiFiDriver::GetMaxNetworks() { return kMaxNetworks; }

        uint8_t DarwinWiFiDriver::GetScanNetworkTimeoutSeconds() { return kScanNetworkTimeoutSeconds; }

        uint8_t DarwinWiFiDriver::GetConnectNetworkTimeoutSeconds() { return kConnectNetworkTimeoutSeconds; }

        CHIP_ERROR DarwinWiFiDriver::CommitConfiguration()
        {
            return ConfigurationManagerImpl::GetDefaultInstance().StoreWiFiNetworkInformations(mNetwork);
        }

        CHIP_ERROR DarwinWiFiDriver::RevertConfiguration()
        {
            mNetwork.Reset();
            return ConfigurationManagerImpl::GetDefaultInstance().GetWiFiNetworkInformations(mNetwork);
        }

        Status DarwinWiFiDriver::AddOrUpdateNetwork(
            ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outText, uint8_t & outIndex)
        {
            outText.reduce_size(0);
            outIndex = 0;
            VerifyOrReturnError(CHIP_NO_ERROR == mNetwork.Update(ssid, credentials), Status::kUnknownError);

            return Status::kSuccess;
        }

        Status DarwinWiFiDriver::RemoveNetwork(ByteSpan ssid, MutableCharSpan & outText, uint8_t & outIndex)
        {
            outText.reduce_size(0);
            outIndex = 0;
            VerifyOrReturnError(mNetwork.Match(ssid), Status::kNetworkIDNotFound);

            mNetwork.Reset();

            return Status::kSuccess;
        }

        Status DarwinWiFiDriver::ReorderNetwork(ByteSpan ssid, uint8_t index, MutableCharSpan & outText)
        {
            outText.reduce_size(0);
            VerifyOrReturnError(index == 0, Status::kOutOfRange);
            VerifyOrReturnError(mNetwork.Match(ssid), Status::kNetworkIDNotFound);

            // We only support one network, so reorder is actually no-op.
            return Status::kSuccess;
        }

        void DarwinWiFiDriver::ConnectNetwork(ByteSpan ssid, ConnectCallback * callback)
        {
            VerifyOrReturn(mNetwork.Match(ssid), DispatchConnectNetworkResponse(callback, Status::kNetworkIDNotFound));

            auto err = ConnectivityMgrImpl().ConnectWiFiNetwork(mNetwork);
            VerifyOrReturn(CHIP_ERROR_KEY_NOT_FOUND != err, DispatchConnectNetworkResponse(callback, Status::kNetworkNotFound));
            VerifyOrReturn(CHIP_ERROR_NOT_CONNECTED != err, DispatchConnectNetworkResponse(callback, Status::kAuthFailure));
            VerifyOrReturn(CHIP_NO_ERROR == err, DispatchConnectNetworkResponse(callback, Status::kUnknownError));

            DispatchConnectNetworkResponse(callback, Status::kSuccess);
        }

        void DarwinWiFiDriver::DispatchConnectNetworkResponse(ConnectCallback * callback, Status status)
        {
            if (callback) {
                callback->OnResult(status, CharSpan(), 0);
            }
        }

        void DarwinWiFiDriver::ScanNetworks(ByteSpan ssid, ScanCallback * callback)
        {
            VerifyOrReturn(callback != nullptr); // Not much we can do.

            auto WiFiClient = [CWWiFiClient sharedWiFiClient];
            VerifyOrReturn(WiFiClient != nil, DispatchScanNetworksResponse(callback, Status::kUnknownError));

            CWInterface * interface = [WiFiClient interface];
            VerifyOrReturn(interface != nil, DispatchScanNetworksResponse(callback, Status::kUnknownError));

            NSError * error = nil;
            NSData * ssidData = ssid.size() ? [NSData dataWithBytes:ssid.data() length:ssid.size()] : nil;
            auto networks = [[interface scanForNetworksWithSSID:ssidData error:&error] allObjects];
            VerifyOrReturn(error == nil, DispatchScanNetworksResponse(callback, Status::kUnknownError));
            VerifyOrReturn([networks count] != 0, DispatchScanNetworksResponse(callback, Status::kSuccess));

            DarwinScanResponseIterator iter(networks);
            DispatchScanNetworksResponse(callback, Status::kSuccess, &iter);
        }

        void DarwinWiFiDriver::DispatchScanNetworksResponse(
            ScanCallback * callback, Status status, Iterator<WiFiScanResponse> * responses)
        {
            if (callback) {
                callback->OnFinished(status, CharSpan(), responses);
            }
        }
    } // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
