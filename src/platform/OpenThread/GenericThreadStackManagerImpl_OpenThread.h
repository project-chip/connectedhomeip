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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#include <openthread/dns_client.h>
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
    CHIP_ERROR _AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                              const Span<const char * const> & aSubTypes, const Span<const Mdns::TextEntry> & aTxtEntries,
                              uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR _RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR _RemoveAllSrpServices();
    CHIP_ERROR _SetupSrpHost(const char * aHostName);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    CHIP_ERROR _DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext);
    CHIP_ERROR _DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback, void * aContext);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
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
        static constexpr uint8_t kMaxServicesNumber      = CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES;
        static constexpr uint8_t kMaxHostNameSize        = 16;
        static constexpr const char * kDefaultDomainName = "default.service.arpa";
        static constexpr uint8_t kDefaultDomainNameSize  = 20;
        static constexpr uint8_t kMaxDomainNameSize      = 32;

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
        // Thread supports both operational and commissionable discovery, so buffers sizes must be worst case.
        static constexpr size_t kSubTypeMaxNumber   = Mdns::kSubTypeMaxNumber;
        static constexpr size_t kSubTypeTotalLength = Mdns::kSubTypeTotalLength;
        static constexpr size_t kTxtMaxNumber =
            std::max(Mdns::CommissionAdvertisingParameters::kTxtMaxNumber, Mdns::OperationalAdvertisingParameters::kTxtMaxNumber);
        static constexpr size_t kTxtTotalValueLength = std::max(Mdns::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                Mdns::OperationalAdvertisingParameters::kTxtTotalValueSize);
#else
        // Thread only supports operational discovery.
        static constexpr size_t kSubTypeMaxNumber    = 0;
        static constexpr size_t kSubTypeTotalLength  = 0;
        static constexpr size_t kTxtMaxNumber        = Mdns::OperationalAdvertisingParameters::kTxtMaxNumber;
        static constexpr size_t kTxtTotalValueLength = Mdns::OperationalAdvertisingParameters::kTxtTotalValueSize;
#endif

        static constexpr size_t kServiceBufferSize = Mdns::kMdnsInstanceNameMaxSize + 1 + // add null-terminator
            Mdns::kMdnsTypeAndProtocolMaxSize + 1 +                                       // add null-terminator
            kSubTypeTotalLength + kSubTypeMaxNumber +                                     // add null-terminator for each subtype
            kTxtTotalValueLength;

        struct Service
        {
            otSrpClientService mService;
            uint8_t mServiceBuffer[kServiceBufferSize];
#if OPENTHREAD_API_VERSION >= 132
            const char * mSubTypes[kSubTypeMaxNumber + 1]; // extra entry for null terminator
#endif
            otDnsTxtEntry mTxtEntries[kTxtMaxNumber];

            bool IsUsed() const { return mService.mInstanceName != nullptr; }
            bool Matches(const char * aInstanceName, const char * aName) const;
        };

        char mHostName[kMaxHostNameSize + 1];
        otIp6Address mHostAddress;
        Service mServices[kMaxServicesNumber];
    };

    SrpClient mSrpClient;

    static void OnSrpClientNotification(otError aError, const otSrpClientHostInfo * aHostInfo, const otSrpClientService * aServices,
                                        const otSrpClientService * aRemovedServices, void * aContext);
    static void OnSrpClientStateChange(const otSockAddr * aServerSockAddr, void * aContext);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
    // Thread supports both operational and commissionable discovery, so buffers sizes must be worst case.
    static constexpr uint8_t kMaxDnsServiceTxtEntriesNumber =
        std::max(Mdns::CommissionAdvertisingParameters::kTxtMaxNumber, Mdns::OperationalAdvertisingParameters::kTxtMaxNumber);
    static constexpr size_t kTotalDnsServiceTxtValueSize = std::max(Mdns::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                    Mdns::OperationalAdvertisingParameters::kTxtTotalValueSize);
    static constexpr size_t kTotalDnsServiceTxtKeySize =
        std::max(Mdns::CommissionAdvertisingParameters::kTxtTotalKeySize, Mdns::OperationalAdvertisingParameters::kTxtTotalKeySize);
#else
    // Thread only supports operational discovery.
    static constexpr uint8_t kMaxDnsServiceTxtEntriesNumber = Mdns::OperationalAdvertisingParameters::kTxtMaxNumber;
    static constexpr size_t kTotalDnsServiceTxtValueSize    = Mdns::OperationalAdvertisingParameters::kTxtTotalValueSize;
    static constexpr size_t kTotalDnsServiceTxtKeySize      = Mdns::OperationalAdvertisingParameters::kTxtTotalKeySize;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
    static constexpr size_t kTotalDnsServiceTxtBufferSize =
        kTotalDnsServiceTxtKeySize + kMaxDnsServiceTxtEntriesNumber + kTotalDnsServiceTxtValueSize;

    DnsBrowseCallback mDnsBrowseCallback;
    DnsResolveCallback mDnsResolveCallback;

    struct DnsServiceTxtEntries
    {
        uint8_t mBuffer[kTotalDnsServiceTxtBufferSize];
        chip::Mdns::TextEntry mTxtEntries[kMaxDnsServiceTxtEntriesNumber];
    };

    struct DnsResult
    {
        chip::Mdns::MdnsService mMdnsService;
        DnsServiceTxtEntries mServiceTxtEntry;
    };

    static void OnDnsBrowseResult(otError aError, const otDnsBrowseResponse * aResponse, void * aContext);
    static void OnDnsResolveResult(otError aError, const otDnsServiceResponse * aResponse, void * aContext);
    static CHIP_ERROR FromOtDnsResponseToMdnsData(otDnsServiceInfo & serviceInfo, const char * serviceType,
                                                  chip::Mdns::MdnsService & mdnsService, DnsServiceTxtEntries & serviceTxtEntries);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
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
