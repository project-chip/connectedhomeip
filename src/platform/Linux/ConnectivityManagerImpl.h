/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/wpa/DBusWpa.h>
#include <platform/Linux/dbus/wpa/DBusWpaBss.h>
#include <platform/Linux/dbus/wpa/DBusWpaInterface.h>
#include <platform/Linux/dbus/wpa/DBusWpaNetwork.h>
#include <system/SystemMutex.h>

#include <mutex>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <wifipaf/WiFiPAFEndPoint.h>
#include <wifipaf/WiFiPAFLayer.h>
#endif
#endif

#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/NetworkCommissioning.h>
#include <vector>

namespace chip {

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

template <>
struct GAutoPtrDeleter<WpaSupplicant1>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1BSS>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1Interface>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1Network>
{
    using deleter = GObjectDeleter;
};

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
struct GDBusWpaSupplicant
{
    enum class WpaState
    {
        INIT,
        CONNECTING,
        CONNECTED,
        NOT_CONNECTED,
        NO_INTERFACE_PATH,
        GOT_INTERFACE_PATH,
        INTERFACE_CONNECTED,
    };

    enum class WpaScanningState
    {
        IDLE,
        SCANNING,
    };

    WpaState state             = WpaState::INIT;
    WpaScanningState scanState = WpaScanningState::IDLE;
    GAutoPtr<WpaSupplicant1> proxy;
    GAutoPtr<WpaSupplicant1Interface> iface;
    GAutoPtr<WpaSupplicant1BSS> bss;
    GAutoPtr<char> interfacePath;
    GAutoPtr<char> networkPath;
};
#endif

/**
 * Concrete implementation of the ConnectivityManager singleton object for Linux platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
public:
    CHIP_ERROR ConnectWiFiNetworkAsync(ByteSpan ssid, ByteSpan credentials,
                                       NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    CHIP_ERROR ConnectWiFiNetworkWithPDCAsync(ByteSpan ssid, ByteSpan networkIdentity, ByteSpan clientIdentity,
                                              const Crypto::P256Keypair & clientIdentityKeypair,
                                              NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    CHIP_ERROR _WiFiPAFSubscribe(const uint16_t & connDiscriminator, void * appState, OnConnectionCompleteFunct onSuccess,
                                 OnConnectionErrorFunct onError);
    CHIP_ERROR _WiFiPAFCancelSubscribe(uint32_t SubscribeId);
    CHIP_ERROR _WiFiPAFCancelIncompleteSubscribe();
    void OnDiscoveryResult(GVariant * obj);
    void OnReplied(GVariant * obj);
    void OnNanReceive(GVariant * obj);
    void OnNanPublishTerminated(guint public_id, gchar * reason);
    void OnNanSubscribeTerminated(guint subscribe_id, gchar * reason);
    CHIP_ERROR _WiFiPAFSend(const WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msgBuf);
    void _WiFiPafSetApFreq(const uint16_t freq) { mApFreq = freq; }
    CHIP_ERROR _WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole role);
#endif

    void PostNetworkConnect();
    CHIP_ERROR CommitConfig();

    void StartWiFiManagement();
    bool IsWiFiManagementStarted();
    void StartNonConcurrentWiFiManagement();
    int32_t GetDisconnectReason();
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value);
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType);
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion);
    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & network);
    CHIP_ERROR StartWiFiScan(ByteSpan ssid, NetworkCommissioning::WiFiDriver::ScanCallback * callback);

private:
    bool _IsWiFiInterfaceEnabled() CHIP_REQUIRES(mWpaSupplicantMutex);
    CHIP_ERROR _ConnectWiFiNetworkAsync(GVariant * networkArgs,
                                        NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback)
        CHIP_REQUIRES(mWpaSupplicantMutex);
#endif

public:
    const char * GetEthernetIfName() { return (mEthIfName[0] == '\0') ? nullptr : mEthIfName; }
    void UpdateEthernetNetworkingStatus();

    void
    SetNetworkStatusChangeCallback(NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback)
    {
        mpStatusChangeCallback = statusChangeCallback;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    const char * GetWiFiIfName() { return (sWiFiIfName[0] == '\0') ? nullptr : sWiFiIfName; }
#endif

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    struct WiFiNetworkScanned
    {
        // The fields matches WiFiInterfaceScanResult::Type.
        uint8_t ssid[Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen;
        uint8_t bssid[6];
        int8_t rssi;
        uint16_t frequencyBand;
        uint8_t channel;
        uint8_t security;
    };

    CHIP_ERROR _Init();
    void _OnPlatformEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    System::Clock::Timeout _GetWiFiStationReconnectInterval();
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationEnabled();
    bool _IsWiFiStationConnected();
    bool _IsWiFiStationApplicationControlled();
    bool _IsWiFiStationProvisioned();
    void _ClearWiFiStationProvision();

    WiFiAPMode _GetWiFiAPMode();
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive();
    bool _IsWiFiAPApplicationControlled();
    void _DemandStartWiFiAP();
    void _StopOnDemandWiFiAP();
    void _MaintainOnDemandWiFiAP();
    System::Clock::Timeout _GetWiFiAPIdleTimeout();
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);
    void UpdateNetworkStatus();
    CHIP_ERROR StopAutoScan();

    void _OnWpaProxyReady(GObject * sourceObject, GAsyncResult * res);
    void _OnWpaInterfaceRemoved(WpaSupplicant1 * proxy, const char * path);
    void _OnWpaInterfaceAdded(WpaSupplicant1 * proxy, const char * path, GVariant * properties);
    void _OnWpaPropertiesChanged(WpaSupplicant1Interface * iface, GVariant * properties);
    void _OnWpaInterfaceScanDone(WpaSupplicant1Interface * iface, gboolean success);
    void _OnWpaInterfaceReady(GObject * sourceObject, GAsyncResult * res);
    void _OnWpaInterfaceProxyReady(GObject * sourceObject, GAsyncResult * res);
    void _OnWpaBssProxyReady(GObject * sourceObject, GAsyncResult * res);
    CHIP_ERROR StartWiFiManagementSync();
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    OnConnectionCompleteFunct mOnPafSubscribeComplete;
    OnConnectionErrorFunct mOnPafSubscribeError;
    WiFiPAF::WiFiPAFEndPoint mWiFiPAFEndPoint;
    void * mAppState;
    uint16_t mApFreq;
    CHIP_ERROR _WiFiPAFPublish(WiFiPAFAdvertiseParam & args);
    CHIP_ERROR _WiFiPAFCancelPublish(uint32_t PublishId);
    bool _WiFiPAFResourceAvailable() { return mPafChannelAvailable; };
    // The resource checking is needed right before sending data packets that they are initialized and connected.
    bool mPafChannelAvailable = true;
#endif

    bool _GetBssInfo(const gchar * bssPath, NetworkCommissioning::WiFiScanResponse & result);

    CHIP_ERROR _StartWiFiManagement();

    bool mAssociationStarted = false;
    BitFlags<ConnectivityFlags> mConnectivityFlag;
    GDBusWpaSupplicant mWpaSupplicant CHIP_GUARDED_BY(mWpaSupplicantMutex);
    // Access to mWpaSupplicant has to be protected by a mutex because it is accessed from
    // the CHIP event loop thread and dedicated D-Bus thread started by platform manager.
    std::mutex mWpaSupplicantMutex;

#endif
    NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;

    // ==================== ConnectivityManager Private Methods ====================

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void DriveAPState();
    CHIP_ERROR ConfigureWiFiAP();
    void ChangeWiFiAPState(WiFiAPState newState);
    static void DriveAPState(::chip::System::Layer * aLayer, void * aAppState);
#endif

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr();
    friend ConnectivityManagerImpl & ConnectivityMgrImpl();

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    char mEthIfName[Inet::InterfaceId::kMaxIfNameLength];

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    System::Clock::Timestamp mLastAPDemandTime;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    System::Clock::Timeout mWiFiAPIdleTimeout;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    char sWiFiIfName[Inet::InterfaceId::kMaxIfNameLength];
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    uint8_t sInterestedSSID[Internal::kMaxWiFiSSIDLength];
    uint8_t sInterestedSSIDLen;
#endif
    NetworkCommissioning::WiFiDriver::ScanCallback * mpScanCallback;
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
inline ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode()
{
    return mWiFiAPMode;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPActive()
{
    return mWiFiAPState == kWiFiAPState_Active;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPApplicationControlled()
{
    return mWiFiAPMode == kWiFiAPMode_ApplicationControlled;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout()
{
    return mWiFiAPIdleTimeout;
}

#endif

/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr()
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl()
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
