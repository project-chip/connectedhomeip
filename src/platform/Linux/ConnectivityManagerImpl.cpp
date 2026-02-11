/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    Copyright (c) 2025 NXP
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

#include <string.h>

#include <lib/core/Optional.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <credentials/CHIPCert.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include "ConnectivityManagerImpl.h"
#include "ConnectivityUtils.h"

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::NetworkCommissioning;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
using namespace ::chip::WiFiPAF;
#endif

namespace chip {
namespace DeviceLayer {

// MARK: Singleton

ConnectivityManagerImpl & ConnectivityManagerImpl::GetDefaultInstance()
{
    static ConnectivityManagerImpl sInstance;
    return sInstance;
}

void ConnectivityManagerImpl::UpdateEthernetNetworkingStatus()
{
    if (mpStatusChangeCallback != nullptr)
    {
        if (mEthIfName[0] != '\0')
        {
            ByteSpan ifNameSpan(reinterpret_cast<unsigned char *>(mEthIfName),
                                strnlen(mEthIfName, Inet::InterfaceId::kMaxIfNameLength));
            mpStatusChangeCallback->OnNetworkingStatusChange(Status::kSuccess, MakeOptional(ifNameSpan), NullOptional);
        }
    }
}

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
#endif
    mpConnectCallback = nullptr;
    mpScanCallback    = nullptr;

    if (ConnectivityUtils::GetEthInterfaceName(mEthIfName, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got Ethernet interface: %s", mEthIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get Ethernet interface");
        mEthIfName[0] = '\0';
    }

    if (GetDiagnosticDataProvider().ResetEthNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset Ethernet statistic counts");
    }

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityUtils::GetWiFiInterfaceName(sWiFiIfName, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got WiFi interface: %s", sWiFiIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get WiFi interface");
        sWiFiIfName[0] = '\0';
    }

    if (GetDiagnosticDataProvider().ResetWiFiNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset WiFi statistic counts");
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    return WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());
#else
    return CHIP_NO_ERROR;
#endif
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    WiFiPAFLayer & WiFiPafLayer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoWiFiPAFReceived: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFReceived");
        WiFiPAFSession RxInfo;
        memcpy(&RxInfo, &event->CHIPoWiFiPAFReceived.SessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
        WiFiPafLayer.OnWiFiPAFMessageReceived(RxInfo, System::PacketBufferHandle::Adopt(event->CHIPoWiFiPAFReceived.Data));
        break;
    }
    case DeviceEventType::kCHIPoWiFiPAFConnected: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFConnected");
        WiFiPAF::WiFiPAFSession SessionInfo;
        memcpy(&SessionInfo, &event->CHIPoWiFiPAFReceived.SessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
        TEMPORARY_RETURN_IGNORED WiFiPafLayer.HandleTransportConnectionInitiated(SessionInfo, mOnPafSubscribeComplete, mAppState,
                                                                                 mOnPafSubscribeError);
        break;
    }
    case DeviceEventType::kCHIPoWiFiPAFCancelConnect: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFCancelConnect");
        if (mOnPafSubscribeError != nullptr)
        {
            mOnPafSubscribeError(mAppState, CHIP_ERROR_CANCELLED);
            mOnPafSubscribeError = nullptr;
        }
        break;
    }
    case DeviceEventType::kCHIPoWiFiPAFWriteDone: {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: event: kCHIPoWiFiPAFWriteDone");
        WiFiPAF::WiFiPAFSession TxInfo;
        memcpy(&TxInfo, &event->CHIPoWiFiPAFReceived.SessionInfo, sizeof(WiFiPAF::WiFiPAFSession));
        TEMPORARY_RETURN_IGNORED WiFiPafLayer.HandleWriteConfirmed(TxInfo, event->CHIPoWiFiPAFReceived.result);
        break;
    }
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
}

ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
