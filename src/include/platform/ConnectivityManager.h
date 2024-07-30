/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Defines the public interface for the Device Layer ConnectivityManager object.
 */

#pragma once
#include <memory>

#include <app/icd/server/ICDServerConfig.h>
#include <inet/UDPEndPoint.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>

#include <app/util/basic-types.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <inet/TCPEndPoint.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <transport/raw/WiFiPAF.h>
#endif

namespace chip {

namespace Ble {
class BleLayer;
class BLEEndPoint;
} // namespace Ble

namespace DeviceLayer {

namespace Internal {
template <class>
class GenericPlatformManagerImpl;
template <class>
class GenericPlatformManagerImpl_FreeRTOS;
template <class>
class GenericPlatformManagerImpl_POSIX;
} // namespace Internal

class ConnectivityManager;
class ConnectivityManagerImpl;

/**
 * Defines the delegate class of Connectivity Manager to notify connectivity updates.
 */
class ConnectivityManagerDelegate
{
public:
    virtual ~ConnectivityManagerDelegate() {}

    /**
     * @brief
     *   Called when any network interface on the Node is changed
     *
     */
    virtual void OnNetworkInfoChanged() {}
};

/**
 * Provides control of network connectivity for a chip device.
 */
class ConnectivityManager
{
    using ImplClass = ::chip::DeviceLayer::ConnectivityManagerImpl;

public:
    // ===== Members that define the public interface of the ConnectivityManager

    enum WiFiStationMode
    {
        kWiFiStationMode_NotSupported          = 0,
        kWiFiStationMode_ApplicationControlled = 1,
        kWiFiStationMode_Disabled              = 2,
        kWiFiStationMode_Enabled               = 3,
    };

    enum WiFiAPMode
    {
        kWiFiAPMode_NotSupported                = 0,
        kWiFiAPMode_ApplicationControlled       = 1,
        kWiFiAPMode_Disabled                    = 2,
        kWiFiAPMode_Enabled                     = 3,
        kWiFiAPMode_OnDemand                    = 4,
        kWiFiAPMode_OnDemand_NoStationProvision = 5,
    };

    enum WiFiStationState
    {
        kWiFiStationState_NotConnected,
        kWiFiStationState_Connecting,
        kWiFiStationState_Connecting_Succeeded,
        kWiFiStationState_Connecting_Failed,
        kWiFiStationState_Connected,
        kWiFiStationState_Disconnecting,
    };

    enum WiFiAPState
    {
        kWiFiAPState_NotActive,
        kWiFiAPState_Activating,
        kWiFiAPState_Active,
        kWiFiAPState_Deactivating,
    };

    enum CHIPoBLEServiceMode
    {
        kCHIPoBLEServiceMode_NotSupported = 0,
        kCHIPoBLEServiceMode_Enabled      = 1,
        kCHIPoBLEServiceMode_Disabled     = 2,
    };

    enum ThreadDeviceType
    {
        kThreadDeviceType_NotSupported                = 0,
        kThreadDeviceType_Router                      = 1,
        kThreadDeviceType_FullEndDevice               = 2,
        kThreadDeviceType_MinimalEndDevice            = 3,
        kThreadDeviceType_SleepyEndDevice             = 4,
        kThreadDeviceType_SynchronizedSleepyEndDevice = 5,
    };

    enum BLEAdvertisingMode
    {
        kFastAdvertising     = 0,
        kSlowAdvertising     = 1,
        kExtendedAdvertising = 2,
    };

    enum class SEDIntervalMode
    {
        Idle   = 0,
        Active = 1,
    };

    struct SEDIntervalsConfig;

    void SetDelegate(ConnectivityManagerDelegate * delegate) { mDelegate = delegate; }
    ConnectivityManagerDelegate * GetDelegate() const { return mDelegate; }

    chip::Inet::EndPointManager<Inet::UDPEndPoint> & UDPEndPointManager();

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    chip::Inet::EndPointManager<Inet::TCPEndPoint> & TCPEndPointManager();
#endif

    // WiFi station methods
    WiFiStationMode GetWiFiStationMode();
    CHIP_ERROR SetWiFiStationMode(WiFiStationMode val);
    bool IsWiFiStationEnabled();
    bool IsWiFiStationApplicationControlled();
    bool IsWiFiStationConnected();
    System::Clock::Timeout GetWiFiStationReconnectInterval();
    CHIP_ERROR SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool IsWiFiStationProvisioned();
    void ClearWiFiStationProvision();
    CHIP_ERROR GetAndLogWiFiStatsCounters();
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    struct WiFiPAFAdvertiseParam;

    CHIP_ERROR SetWiFiPAFAdvertisingEnabled(WiFiPAFAdvertiseParam & args);
    typedef void (*OnConnectionCompleteFunct)(void * appState);
    typedef void (*OnConnectionErrorFunct)(void * appState, CHIP_ERROR err);
    CHIP_ERROR WiFiPAFConnect(const SetupDiscriminator & connDiscriminator, void * appState, OnConnectionCompleteFunct onSuccess,
                              OnConnectionErrorFunct onError);
    CHIP_ERROR WiFiPAFCancelConnect();
    CHIP_ERROR WiFiPAFSend(System::PacketBufferHandle && msgBuf);
    Transport::WiFiPAFBase * GetWiFiPAF();
    void SetWiFiPAF(Transport::WiFiPAFBase * pmWiFiPAF);
#endif

    // WiFi AP methods
    WiFiAPMode GetWiFiAPMode();
    CHIP_ERROR SetWiFiAPMode(WiFiAPMode val);
    bool IsWiFiAPActive();
    bool IsWiFiAPApplicationControlled();
    void DemandStartWiFiAP();
    void StopOnDemandWiFiAP();
    void MaintainOnDemandWiFiAP();
    System::Clock::Timeout GetWiFiAPIdleTimeout();
    void SetWiFiAPIdleTimeout(System::Clock::Timeout val);
    CHIP_ERROR DisconnectNetwork();

    // Thread Methods
    bool IsThreadEnabled();
    ThreadDeviceType GetThreadDeviceType();
    CHIP_ERROR SetThreadDeviceType(ThreadDeviceType deviceType);
    bool IsThreadAttached();
    bool IsThreadProvisioned();
    void ErasePersistentInfo();
    void ResetThreadNetworkDiagnosticsCounts();

    CHIP_ERROR SetPollingInterval(System::Clock::Milliseconds32 pollingInterval);

    // CHIPoBLE service methods
    Ble::BleLayer * GetBleLayer();
    bool IsBLEAdvertisingEnabled();
    /**
     * Enable or disable BLE advertising.
     *
     * @return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if BLE advertising is not
     * supported or other error on other failures.
     */
    CHIP_ERROR SetBLEAdvertisingEnabled(bool val);
    bool IsBLEAdvertising();
    CHIP_ERROR SetBLEAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR GetBLEDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR SetBLEDeviceName(const char * deviceName);
    uint16_t NumBLEConnections();

    // User selected mode methods
    bool IsUserSelectedModeActive();
    void SetUserSelectedMode(bool val);
    uint16_t GetUserSelectedModeTimeout();
    void SetUserSelectedModeTimeout(uint16_t val);

    // Support methods
    static const char * WiFiStationModeToStr(WiFiStationMode mode);
    static const char * WiFiAPModeToStr(WiFiAPMode mode);
    static const char * WiFiStationStateToStr(WiFiStationState state);
    static const char * WiFiAPStateToStr(WiFiAPState state);
    static const char * CHIPoBLEServiceModeToStr(CHIPoBLEServiceMode mode);

private:
    ConnectivityManagerDelegate * mDelegate = nullptr;

    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_POSIX;

    CHIP_ERROR Init();
    void OnPlatformEvent(const ChipDeviceEvent * event);
    bool CanStartWiFiScan();
    void OnWiFiScanDone();
    void OnWiFiStationProvisionChange();

protected:
    // Construction/destruction limited to subclasses.
    ConnectivityManager()  = default;
    ~ConnectivityManager() = default;

    // No copy, move or assignment.
    ConnectivityManager(const ConnectivityManager &)             = delete;
    ConnectivityManager(const ConnectivityManager &&)            = delete;
    ConnectivityManager & operator=(const ConnectivityManager &) = delete;
};

/**
 * Information describing the desired intervals for a sleepy end device (SED).
 */
struct ConnectivityManager::SEDIntervalsConfig
{
    /** Interval at which the device is able to communicate with its parent when there are active chip exchanges in progress. Only
     * meaningful when the device is acting as a sleepy end node.  */
    System::Clock::Milliseconds32 ActiveIntervalMS;

    /** Interval at which the device is able to communicate with its parent when there are NO active chip exchanges in progress.
     * Only meaningful when the device is acting as a sleepy end node. */
    System::Clock::Milliseconds32 IdleIntervalMS;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
struct ConnectivityManager::WiFiPAFAdvertiseParam
{
    /* To enable/disable WiFiPAF Commissioning */
    bool enable;
    /* The optional commands */
    const char * ExtCmds;
};
#endif

/**
 * Returns a reference to the public interface of the ConnectivityManager singleton object.
 *
 * chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
extern ConnectivityManager & ConnectivityMgr();

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the selected platform.
 */
extern ConnectivityManagerImpl & ConnectivityMgrImpl();

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the ConfigurationManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_CONNECTIVITYMANAGERIMPL_HEADER
#include EXTERNAL_CONNECTIVITYMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define CONNECTIVITYMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/ConnectivityManagerImpl.h>
#include CONNECTIVITYMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

inline chip::Inet::EndPointManager<Inet::UDPEndPoint> & ConnectivityManager::UDPEndPointManager()
{
    return static_cast<ImplClass *>(this)->_UDPEndPointManager();
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
inline chip::Inet::EndPointManager<Inet::TCPEndPoint> & ConnectivityManager::TCPEndPointManager()
{
    return static_cast<ImplClass *>(this)->_TCPEndPointManager();
}
#endif

inline ConnectivityManager::WiFiStationMode ConnectivityManager::GetWiFiStationMode()
{
    return static_cast<ImplClass *>(this)->_GetWiFiStationMode();
}

inline CHIP_ERROR ConnectivityManager::SetWiFiStationMode(WiFiStationMode val)
{
    return static_cast<ImplClass *>(this)->_SetWiFiStationMode(val);
}

inline bool ConnectivityManager::IsWiFiStationEnabled()
{
    return static_cast<ImplClass *>(this)->_IsWiFiStationEnabled();
}

inline bool ConnectivityManager::IsWiFiStationApplicationControlled()
{
    return static_cast<ImplClass *>(this)->_IsWiFiStationApplicationControlled();
}

inline bool ConnectivityManager::IsWiFiStationConnected()
{
    return static_cast<ImplClass *>(this)->_IsWiFiStationConnected();
}

inline System::Clock::Timeout ConnectivityManager::GetWiFiStationReconnectInterval()
{
    return static_cast<ImplClass *>(this)->_GetWiFiStationReconnectInterval();
}

inline CHIP_ERROR ConnectivityManager::SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    return static_cast<ImplClass *>(this)->_SetWiFiStationReconnectInterval(val);
}

inline bool ConnectivityManager::IsWiFiStationProvisioned()
{
    return static_cast<ImplClass *>(this)->_IsWiFiStationProvisioned();
}

inline void ConnectivityManager::ClearWiFiStationProvision()
{
    static_cast<ImplClass *>(this)->_ClearWiFiStationProvision();
}

inline ConnectivityManager::WiFiAPMode ConnectivityManager::GetWiFiAPMode()
{
    return static_cast<ImplClass *>(this)->_GetWiFiAPMode();
}

inline CHIP_ERROR ConnectivityManager::SetWiFiAPMode(WiFiAPMode val)
{
    return static_cast<ImplClass *>(this)->_SetWiFiAPMode(val);
}

inline bool ConnectivityManager::IsWiFiAPActive()
{
    return static_cast<ImplClass *>(this)->_IsWiFiAPActive();
}

inline bool ConnectivityManager::IsWiFiAPApplicationControlled()
{
    return static_cast<ImplClass *>(this)->_IsWiFiAPApplicationControlled();
}

inline void ConnectivityManager::DemandStartWiFiAP()
{
    static_cast<ImplClass *>(this)->_DemandStartWiFiAP();
}

inline void ConnectivityManager::StopOnDemandWiFiAP()
{
    static_cast<ImplClass *>(this)->_StopOnDemandWiFiAP();
}

inline void ConnectivityManager::MaintainOnDemandWiFiAP()
{
    static_cast<ImplClass *>(this)->_MaintainOnDemandWiFiAP();
}

inline System::Clock::Timeout ConnectivityManager::GetWiFiAPIdleTimeout()
{
    return static_cast<ImplClass *>(this)->_GetWiFiAPIdleTimeout();
}

inline void ConnectivityManager::SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{
    static_cast<ImplClass *>(this)->_SetWiFiAPIdleTimeout(val);
}

inline CHIP_ERROR ConnectivityManager::GetAndLogWiFiStatsCounters()
{
    return static_cast<ImplClass *>(this)->_GetAndLogWiFiStatsCounters();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
inline CHIP_ERROR ConnectivityManager::SetWiFiPAFAdvertisingEnabled(WiFiPAFAdvertiseParam & args)
{
    return static_cast<ImplClass *>(this)->_SetWiFiPAFAdvertisingEnabled(args);
}

inline CHIP_ERROR ConnectivityManager::WiFiPAFConnect(const SetupDiscriminator & connDiscriminator, void * appState,
                                                      OnConnectionCompleteFunct onSuccess, OnConnectionErrorFunct onError)
{
    return static_cast<ImplClass *>(this)->_WiFiPAFConnect(connDiscriminator, appState, onSuccess, onError);
}

inline CHIP_ERROR ConnectivityManager::WiFiPAFCancelConnect()
{
    return static_cast<ImplClass *>(this)->_WiFiPAFCancelConnect();
}

inline CHIP_ERROR ConnectivityManager::WiFiPAFSend(chip::System::PacketBufferHandle && msgBuf)
{
    return static_cast<ImplClass *>(this)->_WiFiPAFSend(std::move(msgBuf));
}
#endif

inline bool ConnectivityManager::IsThreadEnabled()
{
    return static_cast<ImplClass *>(this)->_IsThreadEnabled();
}

inline ConnectivityManager::ThreadDeviceType ConnectivityManager::GetThreadDeviceType()
{
    return static_cast<ImplClass *>(this)->_GetThreadDeviceType();
}

inline CHIP_ERROR ConnectivityManager::SetThreadDeviceType(ThreadDeviceType deviceType)
{
    return static_cast<ImplClass *>(this)->_SetThreadDeviceType(deviceType);
}

inline CHIP_ERROR ConnectivityManager::SetPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    return static_cast<ImplClass *>(this)->_SetPollingInterval(pollingInterval);
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

inline bool ConnectivityManager::IsThreadAttached()
{
    return static_cast<ImplClass *>(this)->_IsThreadAttached();
}

inline bool ConnectivityManager::IsThreadProvisioned()
{
    return static_cast<ImplClass *>(this)->_IsThreadProvisioned();
}

inline void ConnectivityManager::ErasePersistentInfo()
{
    static_cast<ImplClass *>(this)->_ErasePersistentInfo();
}

inline void ConnectivityManager::ResetThreadNetworkDiagnosticsCounts()
{
    static_cast<ImplClass *>(this)->_ResetThreadNetworkDiagnosticsCounts();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
inline Transport::WiFiPAFBase * ConnectivityManager::GetWiFiPAF()
{
    return static_cast<ImplClass *>(this)->_GetWiFiPAF();
}

inline void ConnectivityManager::SetWiFiPAF(Transport::WiFiPAFBase * pWiFiPAF)
{
    return static_cast<ImplClass *>(this)->_SetWiFiPAF(pWiFiPAF);
}
#endif

inline Ble::BleLayer * ConnectivityManager::GetBleLayer()
{
    return static_cast<ImplClass *>(this)->_GetBleLayer();
}

inline bool ConnectivityManager::IsBLEAdvertisingEnabled()
{
    return static_cast<ImplClass *>(this)->_IsBLEAdvertisingEnabled();
}

inline CHIP_ERROR ConnectivityManager::SetBLEAdvertisingEnabled(bool val)
{
    return static_cast<ImplClass *>(this)->_SetBLEAdvertisingEnabled(val);
}

inline bool ConnectivityManager::IsBLEAdvertising()
{
    return static_cast<ImplClass *>(this)->_IsBLEAdvertising();
}

inline CHIP_ERROR ConnectivityManager::SetBLEAdvertisingMode(BLEAdvertisingMode mode)
{
    return static_cast<ImplClass *>(this)->_SetBLEAdvertisingMode(mode);
}

inline CHIP_ERROR ConnectivityManager::GetBLEDeviceName(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetBLEDeviceName(buf, bufSize);
}

inline CHIP_ERROR ConnectivityManager::SetBLEDeviceName(const char * deviceName)
{
    return static_cast<ImplClass *>(this)->_SetBLEDeviceName(deviceName);
}

inline uint16_t ConnectivityManager::NumBLEConnections()
{
    return static_cast<ImplClass *>(this)->_NumBLEConnections();
}

inline bool ConnectivityManager::IsUserSelectedModeActive()
{
    return static_cast<ImplClass *>(this)->_IsUserSelectedModeActive();
}

inline void ConnectivityManager::SetUserSelectedMode(bool val)
{
    static_cast<ImplClass *>(this)->_SetUserSelectedMode(val);
}

inline uint16_t ConnectivityManager::GetUserSelectedModeTimeout()
{
    return static_cast<ImplClass *>(this)->_GetUserSelectedModeTimeout();
}

inline void ConnectivityManager::SetUserSelectedModeTimeout(uint16_t val)
{
    static_cast<ImplClass *>(this)->_SetUserSelectedModeTimeout(val);
}

inline const char * ConnectivityManager::WiFiStationModeToStr(WiFiStationMode mode)
{
    return ImplClass::_WiFiStationModeToStr(mode);
}

inline const char * ConnectivityManager::WiFiAPModeToStr(WiFiAPMode mode)
{
    return ImplClass::_WiFiAPModeToStr(mode);
}

inline const char * ConnectivityManager::WiFiStationStateToStr(WiFiStationState state)
{
    return ImplClass::_WiFiStationStateToStr(state);
}

inline const char * ConnectivityManager::WiFiAPStateToStr(WiFiAPState state)
{
    return ImplClass::_WiFiAPStateToStr(state);
}

inline const char * ConnectivityManager::CHIPoBLEServiceModeToStr(CHIPoBLEServiceMode mode)
{
    return ImplClass::_CHIPoBLEServiceModeToStr(mode);
}

inline CHIP_ERROR ConnectivityManager::Init()
{
    return static_cast<ImplClass *>(this)->_Init();
}

inline void ConnectivityManager::OnPlatformEvent(const ChipDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_OnPlatformEvent(event);
}

inline bool ConnectivityManager::CanStartWiFiScan()
{
    return static_cast<ImplClass *>(this)->_CanStartWiFiScan();
}

inline void ConnectivityManager::OnWiFiScanDone()
{
    static_cast<ImplClass *>(this)->_OnWiFiScanDone();
}

inline void ConnectivityManager::OnWiFiStationProvisionChange()
{
    static_cast<ImplClass *>(this)->_OnWiFiStationProvisionChange();
}

inline CHIP_ERROR ConnectivityManager::DisconnectNetwork()
{
    return static_cast<ImplClass *>(this)->_DisconnectNetwork();
}

} // namespace DeviceLayer
} // namespace chip
