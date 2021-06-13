/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides a generic implementation of ThreadStackManager features
 *          for use on platforms that use OpenThread.
 */

#pragma once

#include <openthread/instance.h>
#include <openthread/netdata.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
#include <openthread/srp_client.h>
#endif

#include <lib/mdns/Advertiser.h>
#include <lib/mdns/platform/Mdns.h>

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl;

namespace Internal {

/**
 * Provides a generic implementation of ThreadStackManager features that works in conjunction
 * with OpenThread.
 *
 * This class contains implementations of select features from the ThreadStackManager abstract
 * interface that are suitable for use on devices that employ OpenThread.  It is intended to
 * be inherited, directly or indirectly, by the ThreadStackManagerImpl class, which also appears
 * as the template's ImplClass parameter.
 *
 * The class is designed to be independent of the choice of host OS (e.g. RTOS or posix) and
 * network stack (e.g. LwIP or other IP stack).
 */
template <class ImplClass>
class GenericThreadStackManagerImpl_OpenThread
{
public:
    // ===== Platform-specific methods directly callable by the application.

    otInstance * OTInstance() const;
    static void OnOpenThreadStateChange(uint32_t flags, void * context);

protected:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    void _ProcessThreadActivity(void);
    bool _HaveRouteToAddress(const Inet::IPAddress & destAddr);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    bool _IsThreadEnabled(void);
    CHIP_ERROR _SetThreadEnabled(bool val);

    bool _IsThreadProvisioned(void);
    bool _IsThreadAttached(void);
    CHIP_ERROR _GetThreadProvision(ByteSpan & netInfo);
    CHIP_ERROR _SetThreadProvision(ByteSpan netInfo);
    void _ErasePersistentInfo(void);
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType(void);
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
    void _GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig);
    CHIP_ERROR _SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig);
    bool _HaveMeshConnectivity(void);
    void _OnMessageLayerActivityChanged(bool messageLayerIsActive);
    CHIP_ERROR _GetAndLogThreadStatsCounters(void);
    CHIP_ERROR _GetAndLogThreadTopologyMinimal(void);
    CHIP_ERROR _GetAndLogThreadTopologyFull(void);
    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR _GetExternalIPv6Address(chip::Inet::IPAddress & addr);
    CHIP_ERROR _GetPollPeriod(uint32_t & buf);
    void _OnWoBLEAdvertisingStart(void);
    void _OnWoBLEAdvertisingStop(void);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    CHIP_ERROR _AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort, chip::Mdns::TextEntry * aTxtEntries,
                              size_t aTxtEntiresSize, uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR _RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR _RemoveAllSrpServices();
    CHIP_ERROR _SetupSrpHost(const char * aHostName);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    // ===== Members available to the implementation subclass.

    CHIP_ERROR DoInit(otInstance * otInst);
    bool IsThreadAttachedNoLock(void);
    bool IsThreadInterfaceUpNoLock(void);
    CHIP_ERROR AdjustPollingInterval(void);

    CHIP_ERROR _JoinerStart(void);

private:
    // ===== Private members for use by this class only.

    otInstance * mOTInst;
    ConnectivityManager::ThreadPollingConfig mPollingConfig;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    struct SrpClient
    {
        static constexpr uint8_t kMaxServicesNumber   = CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES;
        static constexpr uint8_t kMaxInstanceNameSize = chip::Mdns::kMdnsInstanceNameMaxSize;
        static constexpr uint8_t kMaxNameSize         = chip::Mdns::kMdnsTypeAndProtocolMaxSize;
        static constexpr uint8_t kMaxHostNameSize     = 16;
        // Thread only supports operational discovery
        static constexpr uint8_t kMaxTxtEntriesNumber = chip::Mdns::OperationalAdvertisingParameters::kNumAdvertisingTxtEntries;
        static constexpr uint8_t kMaxTxtValueSize     = chip::Mdns::OperationalAdvertisingParameters::kTxtMaxValueSize;
        static constexpr uint8_t kMaxTxtKeySize       = chip::Mdns::OperationalAdvertisingParameters::kTxtMaxKeySize;

        struct Service
        {
            otSrpClientService mService;
            char mInstanceName[kMaxInstanceNameSize + 1];
            char mName[kMaxNameSize + 1];
            otDnsTxtEntry mTxtEntries[kMaxTxtEntriesNumber];
            uint8_t mTxtValueBuffers[kMaxTxtEntriesNumber][kMaxTxtValueSize];
            char mTxtKeyBuffers[kMaxTxtEntriesNumber][kMaxTxtKeySize];
        };

        char mHostName[kMaxHostNameSize + 1];
        otIp6Address mHostAddress;
        Service mServices[kMaxServicesNumber];
    };

    SrpClient mSrpClient;

    static void OnSrpClientNotification(otError aError, const otSrpClientHostInfo * aHostInfo, const otSrpClientService * aServices,
                                        const otSrpClientService * aRemovedServices, void * aContext);
    static void OnSrpClientStateChange(const otSockAddr * aServerSockAddr, void * aContext);

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    static void OnJoinerComplete(otError aError, void * aContext);
    void OnJoinerComplete(otError aError);

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;

/**
 * Returns the underlying OpenThread instance object.
 */
template <class ImplClass>
inline otInstance * GenericThreadStackManagerImpl_OpenThread<ImplClass>::OTInstance() const
{
    return mOTInst;
}

template <class ImplClass>
inline void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnWoBLEAdvertisingStart(void)
{
    // Do nothing by default.
}

template <class ImplClass>
inline void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnWoBLEAdvertisingStop(void)
{
    // Do nothing by default.
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
