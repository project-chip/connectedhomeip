/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "../TelinkWiFiDriver.h"

#include <platform/KeyValueStoreManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::PersistedStorage;
using namespace ::chip::app::Clusters::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

size_t TelinkWiFiDriver::WiFiNetworkIterator::Count()
{
    VerifyOrReturnValue(mDriver != nullptr, 0);
    return mDriver->mStagingNetwork.IsConfigured() ? 1 : 0;
}

bool TelinkWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    // we assume only one network is actually supported
    // TODO: verify if this can be extended
    if (mExhausted || 0 == Count())
    {
        return false;
    }

    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = static_cast<uint8_t>(mDriver->mStagingNetwork.ssidLen);
    item.connected    = false;

    mExhausted = true;

    WiFiManager::WiFiInfo wifiInfo;
    if (CHIP_NO_ERROR == WiFiManager::Instance().GetWiFiInfo(wifiInfo))
    {
        if (WiFiManager::StationStatus::CONNECTED <= WiFiManager::Instance().GetStationStatus())
        {
            if (wifiInfo.mSsidLen == item.networkIDLen && 0 == memcmp(wifiInfo.mSsid, item.networkID, wifiInfo.mSsidLen))
            {
                item.connected = true;
            }
        }
    }
    return true;
}

bool TelinkWiFiScanResponseIterator::Next(WiFiScanResponse & item)
{
    if (mResultId < mResultCount)
    {
        item = mResults[mResultId++];
        return true;
    }
    return false;
}

void TelinkWiFiScanResponseIterator::Release()
{
    mResultId = mResultCount = 0;
    Platform::MemoryFree(mResults);
    mResults = nullptr;
}

void TelinkWiFiScanResponseIterator::Add(const WiFiScanResponse & result)
{
    void * newResults = Platform::MemoryRealloc(mResults, (mResultCount + 1) * sizeof(WiFiScanResponse));

    if (newResults)
    {
        mResults                 = static_cast<WiFiScanResponse *>(newResults);
        mResults[mResultCount++] = result;
    }
}

CHIP_ERROR TelinkWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    mpNetworkStatusChangeCallback = networkStatusChangeCallback;

    LoadFromStorage();

    if (mStagingNetwork.IsConfigured())
    {
        WiFiManager::ConnectionHandling handling{ [] { Instance().OnNetworkStatusChanged(Status::kSuccess); },
                                                  [] { Instance().OnNetworkStatusChanged(Status::kUnknownError); },
                                                  System::Clock::Seconds32{ kWiFiConnectNetworkTimeoutSeconds } };
        ReturnErrorOnFailure(
            WiFiManager::Instance().Connect(mStagingNetwork.GetSsidSpan(), mStagingNetwork.GetPassSpan(), handling));
    }

    return CHIP_NO_ERROR;
}

void TelinkWiFiDriver::OnNetworkStatusChanged(Status status)
{
    if (status == Status::kSuccess)
    {
        ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
    }

    if (mpNetworkStatusChangeCallback)
    {
        const uint8_t * ssid{};
        size_t ssidLen{};
        WiFiManager::WiFiInfo wifiInfo;

        if (CHIP_NO_ERROR == WiFiManager::Instance().GetWiFiInfo(wifiInfo))
        {
            ssid    = wifiInfo.mSsid;
            ssidLen = wifiInfo.mSsidLen;
        }
        else
        {
            ssid    = WiFiManager::Instance().GetWantedNetwork().ssid;
            ssidLen = WiFiManager::Instance().GetWantedNetwork().ssidLen;
        }
        mpNetworkStatusChangeCallback->OnNetworkingStatusChange(status, MakeOptional(ByteSpan(wifiInfo.mSsid, wifiInfo.mSsidLen)),
                                                                NullOptional);
    }

    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(status, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void TelinkWiFiDriver::Shutdown()
{
    mpNetworkStatusChangeCallback = nullptr;
}

CHIP_ERROR TelinkWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kPassKey, mStagingNetwork.pass, mStagingNetwork.passLen));
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kSsidKey, mStagingNetwork.ssid, mStagingNetwork.ssidLen));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TelinkWiFiDriver::RevertConfiguration()
{
    LoadFromStorage();

    if (WiFiManager::StationStatus::CONNECTING <= WiFiManager::Instance().GetStationStatus())
    {
        WiFiManager::WiFiInfo wifiInfo;
        ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(wifiInfo));
        if (mStagingNetwork.GetSsidSpan().data_equal(ByteSpan(wifiInfo.mSsid, wifiInfo.mSsidLen)))
        {
            // we are already connected to this network, so return prematurely
            return CHIP_NO_ERROR;
        }

        WiFiManager::Instance().Disconnect();
    }

    if (mStagingNetwork.IsConfigured())
    {
        WiFiManager::ConnectionHandling handling{ [] { Instance().OnNetworkStatusChanged(Status::kSuccess); },
                                                  [] { Instance().OnNetworkStatusChanged(Status::kUnknownError); },
                                                  System::Clock::Seconds32{ kWiFiConnectNetworkTimeoutSeconds } };
        ReturnErrorOnFailure(
            WiFiManager::Instance().Connect(mStagingNetwork.GetSsidSpan(), mStagingNetwork.GetPassSpan(), handling));
    }

    return CHIP_NO_ERROR;
}

Status TelinkWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                            uint8_t & outNetworkIndex)
{
    outDebugText    = {};
    outNetworkIndex = 0;

    VerifyOrReturnError(!mStagingNetwork.IsConfigured() || ssid.data_equal(mStagingNetwork.GetSsidSpan()), Status::kBoundsExceeded);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.pass), Status::kOutOfRange);

    mStagingNetwork.Erase();
    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    memcpy(mStagingNetwork.pass, credentials.data(), credentials.size());
    mStagingNetwork.ssidLen = ssid.size();
    mStagingNetwork.passLen = credentials.size();

    return Status::kSuccess;
}

Status TelinkWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText    = {};
    outNetworkIndex = 0;

    VerifyOrReturnError(networkId.data_equal(mStagingNetwork.GetSsidSpan()), Status::kNetworkIDNotFound);
    mStagingNetwork.Clear();

    return Status::kSuccess;
}

Status TelinkWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText = {};

    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(networkId.data_equal(mStagingNetwork.GetSsidSpan()), Status::kNetworkIDNotFound);

    return Status::kSuccess;
}

void TelinkWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    Status status = Status::kSuccess;
    WiFiManager::ConnectionHandling handling{ [] { Instance().OnNetworkStatusChanged(Status::kSuccess); },
                                              [] { Instance().OnNetworkStatusChanged(Status::kUnknownError); },
                                              System::Clock::Seconds32{ kWiFiConnectNetworkTimeoutSeconds } };

    VerifyOrExit(mpConnectCallback == nullptr, status = Status::kUnknownError);
    mpConnectCallback = callback;

    VerifyOrExit(WiFiManager::StationStatus::CONNECTING != WiFiManager::Instance().GetStationStatus(),
                 status = Status::kOtherConnectionFailure);
    VerifyOrExit(networkId.data_equal(mStagingNetwork.GetSsidSpan()), status = Status::kNetworkIDNotFound);

    WiFiManager::Instance().Connect(mStagingNetwork.GetSsidSpan(), mStagingNetwork.GetPassSpan(), handling);

exit:
    if (status != Status::kSuccess && mpConnectCallback)
    {
        mpConnectCallback->OnResult(status, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void TelinkWiFiDriver::LoadFromStorage()
{
    WiFiManager::WiFiNetwork network;

    mStagingNetwork = {};
    ReturnOnFailure(KeyValueStoreMgr().Get(kSsidKey, network.ssid, sizeof(network.ssid), &network.ssidLen));
    ReturnOnFailure(KeyValueStoreMgr().Get(kPassKey, network.pass, sizeof(network.pass), &network.passLen));
    mStagingNetwork = network;
}

void TelinkWiFiDriver::OnScanWiFiNetworkDone(const WiFiManager::ScanDoneStatus & status)
{
    VerifyOrReturn(mScanCallback != nullptr);
    mScanCallback->OnFinished(status ? Status::kUnknownError : Status::kSuccess, CharSpan(), &mScanResponseIterator);
    mScanCallback = nullptr;
}

void TelinkWiFiDriver::OnScanWiFiNetworkResult(const WiFiScanResponse & response)
{
    mScanResponseIterator.Add(response);
}

void TelinkWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    mScanCallback    = callback;
    CHIP_ERROR error = WiFiManager::Instance().Scan(
        ssid, [](const WiFiScanResponse & response) { Instance().OnScanWiFiNetworkResult(response); },
        [](const WiFiManager::ScanDoneStatus & status) { Instance().OnScanWiFiNetworkDone(status); });

    if (error != CHIP_NO_ERROR)
    {
        mScanCallback = nullptr;
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

uint32_t TelinkWiFiDriver::GetSupportedWiFiBandsMask() const
{
    uint32_t bands = static_cast<uint32_t>(1UL << chip::to_underlying(WiFiBandEnum::k2g4));
    return bands;
}

void TelinkWiFiDriver::StartDefaultWiFiNetwork(void)
{
    chip::ByteSpan ssidSpan = ByteSpan(Uint8::from_const_char(CONFIG_DEFAULT_WIFI_SSID), strlen(CONFIG_DEFAULT_WIFI_SSID));
    chip::ByteSpan passwordSpan =
        ByteSpan(Uint8::from_const_char(CONFIG_DEFAULT_WIFI_PASSWORD), strlen(CONFIG_DEFAULT_WIFI_PASSWORD));

    char debugBuffer[1] = { 0 };
    MutableCharSpan debugText(debugBuffer, 0);
    uint8_t outNetworkIndex = 0;

    AddOrUpdateNetwork(ssidSpan, passwordSpan, debugText, outNetworkIndex);
    CommitConfiguration();
    RevertConfiguration();
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
