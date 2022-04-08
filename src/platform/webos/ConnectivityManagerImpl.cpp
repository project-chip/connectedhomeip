/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/BLEManager.h>

#include <cstdlib>
#include <new>
#include <utility>

#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#ifndef CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD
#define CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD "dhclient -nw %s"
#endif

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

using namespace ::chip::DeviceLayer::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
char ConnectivityManagerImpl::sWiFiIfName[];
#endif

WiFiDriver::ScanCallback * ConnectivityManagerImpl::mpScanCallback;
NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * ConnectivityManagerImpl::mpConnectCallback;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
bool ConnectivityManagerImpl::mAssociattionStarted = false;
BitFlags<Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>::ConnectivityFlags>
    ConnectivityManagerImpl::mConnectivityFlag;
struct GDBusWpaSupplicant ConnectivityManagerImpl::mWpaSupplicant;
std::mutex ConnectivityManagerImpl::mWpaSupplicantMutex;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval()
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    return true;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    return true;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled()
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned()
{
    return true;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision() {}

bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return true;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP() {}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP() {}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP() {}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val) {}

void ConnectivityManagerImpl::_OnWpaPropertiesChanged(WpaFiW1Wpa_supplicant1Interface * proxy, GVariant * changed_properties,
                                                      const gchar * const * invalidated_properties, gpointer user_data)
{}

void ConnectivityManagerImpl::_OnWpaInterfaceProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

void ConnectivityManagerImpl::_OnWpaBssProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

void ConnectivityManagerImpl::_OnWpaInterfaceReady(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

void ConnectivityManagerImpl::_OnWpaInterfaceAdded(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                   gpointer user_data)
{}

void ConnectivityManagerImpl::_OnWpaInterfaceRemoved(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                     gpointer user_data)
{}

void ConnectivityManagerImpl::_OnWpaProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

void ConnectivityManagerImpl::StartWiFiManagement() {}

bool ConnectivityManagerImpl::IsWiFiManagementStarted()
{
    return true;
}

void ConnectivityManagerImpl::DriveAPState() {}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState) {}

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveAPState();
}

CHIP_ERROR
ConnectivityManagerImpl::ConnectWiFiNetworkAsync(ByteSpan ssid, ByteSpan credentials,
                                                 NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::_ConnectWiFiNetworkAsyncCallback(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::PostNetworkConnect() {}

CHIP_ERROR ConnectivityManagerImpl::CommitConfig()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(ByteSpan & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiVersion(uint8_t & wiFiVersion)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::GetConnectedNetwork(NetworkCommissioning::Network & network)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::StopAutoScan()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl::StartWiFiScan(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConnectivityManagerImpl::_GetBssInfo(const gchar * bssPath, NetworkCommissioning::WiFiScanResponse & result)
{
    return true;
}

void ConnectivityManagerImpl::_OnWpaInterfaceScanDone(GObject * source_object, GAsyncResult * res, gpointer user_data) {}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace DeviceLayer
} // namespace chip
