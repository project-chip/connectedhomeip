/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#include <platform/silabs/SilabsConfig.h>

#include <limits>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::Silabs;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
NetworkCommissioning::WiFiScanResponse * sScanResult;
SlScanResponseIterator<NetworkCommissioning::WiFiScanResponse> mScanResponseIter(sScanResult);
} // namespace

SlWiFiDriver * SlWiFiDriver::mDriver = nullptr;

CHIP_ERROR SlWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;
    mDriver                = this;
    // TODO: default to SL_WIFI_SSID and SL_WIFI_PSK if not defined
#ifdef SL_ONNETWORK_PAIRING
    memcpy(&mSavedNetwork.ssid[0], SL_WIFI_SSID, sizeof(SL_WIFI_SSID));
    memcpy(&mSavedNetwork.key[0], SL_WIFI_PSK, sizeof(SL_WIFI_PSK));
    mSavedNetwork.keyLen  = sizeof(SL_WIFI_PSK);
    mSavedNetwork.ssidLen = sizeof(SL_WIFI_SSID);
    err                   = CHIP_NO_ERROR;
#else
    // If reading fails, wifi is not provisioned, no need to go further.
    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                           mSavedNetwork.ssidLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_WiFiPSK, mSavedNetwork.key, sizeof(mSavedNetwork.key),
                                           mSavedNetwork.keyLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

#endif // SL_ONNETWORK_PAIRING
    mStagingNetwork = mSavedNetwork;
    err             = ConnectWiFiNetwork(reinterpret_cast<const char *>(mSavedNetwork.ssid), mSavedNetwork.ssidLen,
                                         reinterpret_cast<const char *>(mSavedNetwork.key), mSavedNetwork.keyLen);
    return err;
}

CHIP_ERROR SlWiFiDriver::CommitConfiguration()
{
    constexpr uint8_t kDefaultSecurityBitmap =
        static_cast<uint8_t>(chip::app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kWpa2Personal);

    ReturnErrorOnFailure(SilabsConfig::WriteConfigValueBin(
        SilabsConfig::kConfigKey_WiFiSSID, reinterpret_cast<const uint8_t *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValueBin(
        SilabsConfig::kConfigKey_WiFiPSK, reinterpret_cast<const uint8_t *>(mStagingNetwork.key), mStagingNetwork.keyLen));
    ReturnErrorOnFailure(SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_WiFiSEC, &kDefaultSecurityBitmap,
                                                           sizeof(kDefaultSecurityBitmap)));

    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SlWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool SlWiFiDriver::NetworkMatch(const Silabs::WifiInterface::WiFiCredentials & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status SlWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                        uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.key), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    VerifyOrReturnError(ssid.data() != nullptr, Status::kNetworkNotFound);
    MutableByteSpan ssidSpan(mStagingNetwork.ssid, sizeof(mStagingNetwork.ssid));
    VerifyOrReturnError(CopySpanToMutableSpan(ssid, ssidSpan) == CHIP_NO_ERROR, Status::kBoundsExceeded);
    mStagingNetwork.ssidLen = ssid.size();

    VerifyOrReturnError(credentials.data() != nullptr, Status::kNetworkNotFound);
    MutableByteSpan keySpan(mStagingNetwork.key, sizeof(mStagingNetwork.key));
    VerifyOrReturnError(CopySpanToMutableSpan(credentials, keySpan) == CHIP_NO_ERROR, Status::kBoundsExceeded);
    mStagingNetwork.keyLen = credentials.size();

    return Status::kSuccess;
}

Status SlWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status SlWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR SlWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Disconnecting for current wifi");
        WifiInterface::GetInstance().TriggerDisconnection();
    }
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    // Set the wifi configuration
    WifiInterface::WiFiCredentials wifiConfig;

    VerifyOrReturnError(ssidLen <= kMaxWiFiSSIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(wifiConfig.ssid, ssid, ssidLen);
    wifiConfig.ssidLen = ssidLen;

    VerifyOrReturnError(keyLen < kMaxWiFiKeyLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(wifiConfig.key, key, keyLen);
    wifiConfig.keyLen = keyLen;

    wifiConfig.security.Set(chip::app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kWpa2Personal);

    ChipLogProgress(NetworkProvisioning, "Setting up connection for WiFi SSID: %s", NullTerminated(ssid, ssidLen).c_str());
    // Resetting the retry connection state machine for a new access point connection
    WifiInterface::GetInstance().ResetConnectionRetryInterval();
    ReturnErrorOnFailure(WifiInterface::GetInstance().SetWifiCredentials(wifiConfig));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));
    return CHIP_NO_ERROR;
}

// TODO: Re-write implementation with proper driver based callback
void SlWiFiDriver::UpdateNetworkingStatus()
{
    if (mpStatusChangeCallback == nullptr)
    {
        ChipLogError(NetworkProvisioning, "networkStatusChangeCallback is nil");
        return;
    }

    if (mStagingNetwork.ssidLen == 0)
    {
        ChipLogError(NetworkProvisioning, "ssidLen is 0");
        return;
    }

    ByteSpan networkId = ByteSpan((const unsigned char *) mStagingNetwork.ssid, mStagingNetwork.ssidLen);
    if (!WifiInterface::GetInstance().IsStationConnected())
    {
        // TODO: https://github.com/project-chip/connectedhomeip/issues/26861
        mpStatusChangeCallback->OnNetworkingStatusChange(Status::kUnknownError, MakeOptional(networkId),
                                                         MakeOptional(static_cast<int32_t>(SL_STATUS_FAIL)));
        return;
    }
    mpStatusChangeCallback->OnNetworkingStatusChange(Status::kSuccess, MakeOptional(networkId), NullOptional);
}

void SlWiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
    {
        TEMPORARY_RETURN_IGNORED CommitConfiguration();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void SlWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kUnknownError;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);

    err = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                             reinterpret_cast<const char *>(mStagingNetwork.key), mStagingNetwork.keyLen);
    if (err == CHIP_NO_ERROR)
    {
        mpConnectCallback = callback;
        networkingStatus  = Status::kSuccess;
    }

exit:
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
        mpConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

uint32_t SlWiFiDriver::GetSupportedWiFiBandsMask() const
{
    return WifiInterface::GetInstance().GetSupportedWiFiBandsMask();
}

bool SlWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    ChipLogDetail(DeviceLayer, "Start Scan WiFi Networks");
    CHIP_ERROR err = WifiInterface::GetInstance().StartNetworkScan(ssid, OnScanWiFiNetworkDone);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "StartNetworkScan failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

void SlWiFiDriver::OnScanWiFiNetworkDone(NetworkCommissioning::WiFiScanResponse * aScanResult)
{
    SlWiFiDriver * nwDriver = NetworkCommissioning::SlWiFiDriver::GetInstance();
    // Cannot use the driver if the instance is not initialized.
    VerifyOrDie(nwDriver != nullptr); // should never be null

    if (!aScanResult)
    {
        ChipLogProgress(DeviceLayer, "OnScanWiFiNetworkDone: Receive all scanned networks information.");

        if (nwDriver->mpScanCallback != nullptr)
        {
            if (mScanResponseIter.Count() == 0)
            {
                // if there is no network found, return kNetworkNotFound
                TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([nwDriver]() {
                    nwDriver->mpScanCallback->OnFinished(NetworkCommissioning::Status::kNetworkNotFound, CharSpan(), nullptr);
                    nwDriver->mpScanCallback = nullptr;
                });
                return;
            }
            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([nwDriver]() {
                nwDriver->mpScanCallback->OnFinished(NetworkCommissioning::Status::kSuccess, CharSpan(), &mScanResponseIter);
                nwDriver->mpScanCallback = nullptr;
            });
        }
    }
    else
    {
        mScanResponseIter.Add(aScanResult);
    }
}

void SlWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (!StartScanWiFiNetworks(ssid))
        {
            ChipLogError(DeviceLayer, "ScanWiFiNetworks failed to start");
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    WifiInterface::WiFiCredentials wifiConfig;
    network.networkIDLen = 0;
    network.connected    = false;

    // we are able to fetch the wifi provision data and STA should be connected
    VerifyOrReturnError(WifiInterface::GetInstance().IsStationConnected(), CHIP_ERROR_NOT_CONNECTED);
    ReturnErrorOnFailure(WifiInterface::GetInstance().GetWifiCredentials(wifiConfig));
    VerifyOrReturnError(wifiConfig.ssidLen <= NetworkCommissioning::kMaxNetworkIDLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    network.connected = true;

    ByteSpan ssidSpan(wifiConfig.ssid, wifiConfig.ssidLen);
    MutableByteSpan networkIdSpan(network.networkID, NetworkCommissioning::kMaxNetworkIDLen);

    ReturnErrorOnFailure(CopySpanToMutableSpan(ssidSpan, networkIdSpan));
    network.networkIDLen = wifiConfig.ssidLen;

    return CHIP_NO_ERROR;
}

size_t SlWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool SlWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = false;
    mExhausted        = true;

    Network connectedNetwork;
    CHIP_ERROR err = GetConnectedNetwork(connectedNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (connectedNetwork.networkIDLen == item.networkIDLen &&
            memcmp(connectedNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }
    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
