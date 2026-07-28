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

#if CHIP_LINUX_NETWORK_MANAGER_CONNMAN
#include "ConnectivityManagerImpl_NetworkManagementConnMan.h"
#elif CHIP_LINUX_NETWORK_MANAGER_WPA_SUPPLICANT
#include "ConnectivityManagerImpl_NetworkManagementWpaSupplicant.h"
#endif // CHIP_LINUX_NETWORK_MANAGER_CONNMAN

#if CHIP_LINUX_WIFI_PAF_MANAGER_WPA_SUPPLICANT
#include "ConnectivityManagerImpl_WiFiPafWpaSupplicant.h"
#endif // CHIP_LINUX_WIFI_PAF_MANAGER_WPA_SUPPLICANT

#include "ConnectivityManagerImpl.h"
#include "ConnectivityManagerImpl_NetworkManagementInterface.h"
#include "ConnectivityManagerImpl_WiFiPafInterface.h"
#include "ConnectivityUtils.h"

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::NetworkCommissioning;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
using namespace ::chip::WiFiPAF;
#endif

namespace chip {
namespace DeviceLayer {

// Matter Linux Platform Connectivity Manager Implementation

template <typename T>
void ConnectivityManagerImpl::Deleter<T>::operator()(T * inPointer) const noexcept
{
    delete inPointer;
}

// Explicit Instantiations of private implementation (PIMPL) pattern deleters.

template struct ConnectivityManagerImpl::Deleter<Internal::NetworkManagementInterface>;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
template struct ConnectivityManagerImpl::Deleter<Internal::WiFiPafInterface>;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

// Destruction

ConnectivityManagerImpl::~ConnectivityManagerImpl() = default;

// Singleton

ConnectivityManagerImpl & ConnectivityManagerImpl::GetDefaultInstance()
{
    static ConnectivityManagerImpl sInstance;
    return sInstance;
}

// Linux Implementation-specific Methods

// Mutation

void ConnectivityManagerImpl::SetOneShotConnectCallback(OneShotConnectCallback * inOneShotConnectCallback) noexcept
{
    mpOneShotConnectCallback = inOneShotConnectCallback;
}

void ConnectivityManagerImpl::SetOneShotScanCallback(OneShotScanCallback * inOneShotScanCallback) noexcept
{
    mpOneShotScanCallback = inOneShotScanCallback;
}

void ConnectivityManagerImpl::SetNetworkStatusChangeCallback(NetworkStatusChangeCallback * inStatusChangeCallback) noexcept
{
    mpStatusChangeCallback = inStatusChangeCallback;
}

// Generic

CHIP_ERROR ConnectivityManagerImpl::CommitConfig()
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->CommitConfig();
}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
// Ethernet Control Plane Management

// Observation

const char * ConnectivityManagerImpl::GetEthernetIfName()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, nullptr);

    return mNetworkManagementImplementation->GetEthernetIfName();
}

// Helper

void ConnectivityManagerImpl::UpdateEthernetNetworkingStatus()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->UpdateEthernetNetworkingStatus();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
// Wi-Fi Control Plane Management

// Wi-Fi Station Control Plane Management

// Introspection

bool ConnectivityManagerImpl::IsWiFiStationConnecting() const noexcept
{
    return mpOneShotConnectCallback != nullptr;
}

bool ConnectivityManagerImpl::IsWiFiStationScanning() const noexcept
{
    return mpOneShotScanCallback != nullptr;
}

bool ConnectivityManagerImpl::IsWiFiManagementStarted()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, false);

    return mNetworkManagementImplementation->IsWiFiManagementStarted();
}

// Observation

CHIP_ERROR ConnectivityManagerImpl::GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->GetConfiguredNetwork(outNetwork);
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(MutableByteSpan & outWiFiBssId)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->GetWiFiBssId(outWiFiBssId);
}

const char * ConnectivityManagerImpl::GetWiFiIfName()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, nullptr);

    return mNetworkManagementImplementation->GetWiFiIfName();
}

CHIP_ERROR
ConnectivityManagerImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outWiFiSecurityType)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->GetWiFiSecurityType(outWiFiSecurityType);
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outWiFiVersion)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->GetWiFiVersion(outWiFiVersion);
}

// Worker

void ConnectivityManagerImpl::StartWiFiManagement()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->StartWiFiManagement();
}

void ConnectivityManagerImpl::StopWiFiManagement()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->StopWiFiManagement();
}

CHIP_ERROR ConnectivityManagerImpl::ConnectWiFiNetworkAsync(
    ByteSpan inSsid, ByteSpan inCredentials, NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->ConnectWiFiNetworkAsync(inSsid, inCredentials, inConnectCallback);
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
CHIP_ERROR ConnectivityManagerImpl::ConnectWiFiNetworkWithPDCAsync(
    ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity, const Crypto::P256Keypair & inClientIdentityKeypair,
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->ConnectWiFiNetworkWithPDCAsync(inSsid, inNetworkIdentity, inClientIdentity,
                                                                            inClientIdentityKeypair, inConnectCallback);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

CHIP_ERROR ConnectivityManagerImpl::StartWiFiScan(ByteSpan inSsid, NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->StartWiFiScan(inSsid, inScanCallback);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

// Curiously-recurring template pattern (CRTP) "Public" Connectivity Manager Members

// Initialization

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    VerifyOrReturnError(mNetworkManagementImplementation == nullptr, CHIP_ERROR_ALREADY_INITIALIZED);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    VerifyOrReturnError(mWiFiPafImplementation == nullptr, CHIP_ERROR_ALREADY_INITIALIZED);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

    SetNetworkStatusChangeCallback(nullptr);
    SetOneShotConnectCallback(nullptr);
    SetOneShotScanCallback(nullptr);

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    if (GetDiagnosticDataProvider().ResetEthNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset Ethernet statistic counts");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (GetDiagnosticDataProvider().ResetWiFiNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset Wi-Fi statistic counts");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    {
        Internal::NetworkManagementBasis * basis = nullptr;

#if CHIP_LINUX_NETWORK_MANAGER_CONNMAN
        ConnectivityManagerImpl_NetworkManagementConnMan * instance =
            chip::Platform::New<ConnectivityManagerImpl_NetworkManagementConnMan>();
        basis = static_cast<Internal::NetworkManagementBasis *>(instance);
        mNetworkManagementImplementation.reset(instance);
#elif CHIP_LINUX_NETWORK_MANAGER_WPA_SUPPLICANT
        ConnectivityManagerImpl_NetworkManagementWpaSupplicant * instance =
            chip::Platform::New<ConnectivityManagerImpl_NetworkManagementWpaSupplicant>();
        basis = static_cast<Internal::NetworkManagementBasis *>(instance);
        mNetworkManagementImplementation.reset(instance);
#endif
        VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(mNetworkManagementImplementation->Init(*this));

        basis->SetDelegate(this);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#if CHIP_LINUX_WIFI_PAF_MANAGER_WPA_SUPPLICANT
    mWiFiPafImplementation.reset(chip::Platform::New<ConnectivityManagerImpl_WiFiPafWpaSupplicant>());
#endif // CHIP_LINUX_WIFI_PAF_MANAGER_WPA_SUPPLICANT
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(mWiFiPafImplementation->Init(*this));

    ReturnErrorOnFailure(WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer()));
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    return CHIP_NO_ERROR;
}

// Event Handling

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * inDeviceEvent)
{
    // Forward the event to the generic base classes as needed.

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(inDeviceEvent);
#endif

    if ((inDeviceEvent != nullptr) && (mNetworkManagementImplementation != nullptr))
        mNetworkManagementImplementation->OnPlatformEvent(*inDeviceEvent);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if ((inDeviceEvent != nullptr) && (mWiFiPafImplementation != nullptr))
        mWiFiPafImplementation->OnPlatformEvent(*inDeviceEvent);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
// Wi-Fi Control Plane Management

// Wi-Fi Station Control Plane Management

// Introspection

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, false);

    return mNetworkManagementImplementation->IsWiFiStationApplicationControlled();
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, false);

    return mNetworkManagementImplementation->IsWiFiStationConnected();
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, false);

    return mNetworkManagementImplementation->IsWiFiStationEnabled();
}

// Observation

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, ConnectivityManager::kWiFiStationMode_NotSupported);

    return mNetworkManagementImplementation->GetWiFiStationMode();
}

// Mutation

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode inWiFiStationMode)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->SetWiFiStationMode(inWiFiStationMode);
}

// Wi-Fi Soft Access Point Control Plane Management

// Observation

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode()
{
    VerifyOrReturnValue(mNetworkManagementImplementation != nullptr, ConnectivityManager::kWiFiAPMode_NotSupported);

    return mNetworkManagementImplementation->GetWiFiApMode();
}

// Mutation

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode inWiFiApMode)
{
    VerifyOrReturnError(mNetworkManagementImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mNetworkManagementImplementation->SetWiFiApMode(inWiFiApMode);
}

// Control

void ConnectivityManagerImpl::_DemandStartWiFiAP()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->DemandStartWiFiAp();
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->MaintainOnDemandWiFiAp();
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP()
{
    VerifyOrReturn(mNetworkManagementImplementation != nullptr);

    mNetworkManagementImplementation->StopOnDemandWiFiAp();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
// Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
// Service Discovery (USD) / Public Action Frame (PAF)
// Commissioning Transport

// Initialization

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFShutdown(uint32_t inPublishId, WiFiPAF::WiFiPafRole inWiFiPafRole)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafShutdown(inPublishId, inWiFiPafRole);
}

// Introspection

bool ConnectivityManagerImpl::_WiFiPAFResourceAvailable()
{
    VerifyOrReturnValue(mWiFiPafImplementation != nullptr, false);

    return mWiFiPafImplementation->IsWiFiPafResourceAvailable();
}

// Mutation

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiPAFAdvertisingEnabled(bool inEnabled, uint32_t & inOutPublishId)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafSetAdvertisingEnabled(inEnabled, inOutPublishId);
}

void ConnectivityManagerImpl::_WiFiPafSetApFreq(const uint16_t inFrequency)
{
    VerifyOrReturn(mWiFiPafImplementation != nullptr);

    mWiFiPafImplementation->WiFiPafSetSubscribeFreq(inFrequency);
}

void ConnectivityManagerImpl::_WiFiPAFSetParam(const WiFiPAFAdvertiseParam & inWiFiPAFAdvertiseParams)
{
    VerifyOrReturn(mWiFiPafImplementation != nullptr);

    mWiFiPafImplementation->WiFiPafSetParam(inWiFiPAFAdvertiseParams);
}

// Publish-and-subscribe

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFPublish(WiFiPAFAdvertiseParam & inWiFiPafAdvertiseParams)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafPublish(inWiFiPafAdvertiseParams);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelPublish(uint32_t inPublishId)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafCancelPublish(inPublishId);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSubscribe(const uint16_t & inDiscriminator, void * inContext,
                                                      OnConnectionCompleteFunct onSuccessFunc, OnConnectionErrorFunct onErrorFunc)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafSubscribe(inDiscriminator, inContext, onSuccessFunc, onErrorFunc);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelSubscribe(uint32_t inSubscribeId)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafCancelSubscribe(inSubscribeId);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelIncompleteSubscribe()
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafCancelIncompleteSubscribe();
}

// Data Transmission

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSend(const WiFiPAF::WiFiPAFSession & inWiFiPafSession,
                                                 chip::System::PacketBufferHandle && inOutMessageBuffer)
{
    VerifyOrReturnError(mWiFiPafImplementation != nullptr, CHIP_ERROR_UNINITIALIZED);

    return mWiFiPafImplementation->WiFiPafSend(inWiFiPafSession, std::move(inOutMessageBuffer));
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

// Network Commissioning Action Delegation Methods

void ConnectivityManagerImpl::OnScanFinished(NetworkCommissioning::Status inStatus, CharSpan inDebugText,
                                             NetworkCommissioning::WiFiScanResponseIterator * inNetworks) noexcept
{
    if (mpOneShotScanCallback != nullptr)
    {
        mpOneShotScanCallback->OnFinished(inStatus, inDebugText, inNetworks);

        mpOneShotScanCallback = nullptr;
    }
}

void ConnectivityManagerImpl::OnConnectResult(NetworkCommissioning::Status inCommissioningError,
                                              CharSpan inDebugText, int32_t inConnectStatus) noexcept
{
    if (mpOneShotConnectCallback != nullptr)
    {
        mpOneShotConnectCallback->OnResult(inCommissioningError, inDebugText, inConnectStatus);

        mpOneShotConnectCallback = nullptr;
    }
}

void ConnectivityManagerImpl::OnStatusChange(NetworkCommissioning::Status inCommissioningError,
                                             Optional<ByteSpan> inNetworkId,
                                             Optional<int32_t> inConnectStatus) noexcept
{
    if (mpStatusChangeCallback != nullptr)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(inCommissioningError, inNetworkId, inConnectStatus);
    }
}

// Network Management Delegate Method

void ConnectivityManagerImpl::OnWiFiMediumAvailable(Internal::NetworkManagementBasis & inOutNetworkManagement, bool inAvailable)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if (mWiFiPafImplementation != nullptr)
    {
        RETURN_SAFELY_IGNORED mWiFiPafImplementation->WiFiPafSetResourceAvailable(inAvailable);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
}

ConnectivityManagerImpl & ConnectivityMgrImpl()
{
    return ConnectivityManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
