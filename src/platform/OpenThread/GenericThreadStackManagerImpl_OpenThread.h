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
#include <openthread/link.h>
#include <openthread/netdata.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
#include <openthread/srp_client.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#include <openthread/dns_client.h>
#endif

#include <app/AttributeAccessInterface.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <platform/NetworkCommissioning.h>

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
    inline void OverrunErrorTally(void);
    void
    SetNetworkStatusChangeCallback(NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback)
    {
        mpStatusChangeCallback = statusChangeCallback;
    }

protected:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    void _ProcessThreadActivity(void);
    bool _HaveRouteToAddress(const Inet::IPAddress & destAddr);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    bool _IsThreadEnabled(void);
    CHIP_ERROR _SetThreadEnabled(bool val);

    bool _IsThreadProvisioned(void);
    bool _IsThreadAttached(void);
    CHIP_ERROR _GetThreadProvision(Thread::OperationalDataset & dataset);
    CHIP_ERROR _SetThreadProvision(ByteSpan netInfo);
    CHIP_ERROR _AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                      NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback);
    void _OnThreadAttachFinished(void);
    void _ErasePersistentInfo(void);
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType(void);
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
    CHIP_ERROR _StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback);
    static void _OnNetworkScanFinished(otActiveScanResult * aResult, void * aContext);
    void _OnNetworkScanFinished(otActiveScanResult * aResult);
    void _UpdateNetworkStatus();

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    CHIP_ERROR _GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _RequestSEDActiveMode(bool onOff, bool delayIdle);
    CHIP_ERROR SEDUpdateMode();
    static void RequestSEDModeUpdate(chip::System::Layer * apSystemLayer, void * apAppState);
#endif

    bool _HaveMeshConnectivity(void);
    CHIP_ERROR _GetAndLogThreadStatsCounters(void);
    CHIP_ERROR _GetAndLogThreadTopologyMinimal(void);
    CHIP_ERROR _GetAndLogThreadTopologyFull(void);
    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR _GetExternalIPv6Address(chip::Inet::IPAddress & addr);
    void _ResetThreadNetworkDiagnosticsCounts(void);
    CHIP_ERROR _WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);
    CHIP_ERROR _GetPollPeriod(uint32_t & buf);
    void _SetRouterPromotion(bool val);
    void _OnWoBLEAdvertisingStart(void);
    void _OnWoBLEAdvertisingStop(void);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    CHIP_ERROR _AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                              const Span<const char * const> & aSubTypes, const Span<const Dnssd::TextEntry> & aTxtEntries,
                              uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR _RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR _InvalidateAllSrpServices();
    CHIP_ERROR _RemoveInvalidSrpServices();

    CHIP_ERROR _SetupSrpHost(const char * aHostName);
    CHIP_ERROR _ClearSrpHost(const char * aHostName);
    CHIP_ERROR _SetSrpDnsCallbacks(DnsAsyncReturnCallback aInitCallback, DnsAsyncReturnCallback aErrorCallback, void * aContext);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    CHIP_ERROR _DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext);
    CHIP_ERROR _DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback, void * aContext);
    static void DispatchResolve(intptr_t context);
    static void DispatchResolveNoMemory(intptr_t context);
    static void DispatchAddressResolve(intptr_t context);
    static void DispatchBrowseEmpty(intptr_t context);
    static void DispatchBrowse(intptr_t context);
    static void DispatchBrowseNoMemory(intptr_t context);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    // ===== Members available to the implementation subclass.

    CHIP_ERROR DoInit(otInstance * otInst);
    bool IsThreadAttachedNoLock(void);
    bool IsThreadInterfaceUpNoLock(void);

    CHIP_ERROR _JoinerStart(void);

private:
    // ===== Private members for use by this class only.

    otInstance * mOTInst;
    uint64_t mOverrunCount      = 0;
    bool mIsAttached            = false;
    bool mTemporaryRxOnWhenIdle = false;

    NetworkCommissioning::ThreadDriver::ScanCallback * mpScanCallback;
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;
    NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    ConnectivityManager::SEDIntervalsConfig mIntervalsConfig;
    ConnectivityManager::SEDIntervalMode mIntervalsMode = ConnectivityManager::SEDIntervalMode::Idle;
    uint32_t mActiveModeConsumers                       = 0;
    bool mDelayIdleTimerRunning                         = false;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    struct SrpClient
    {
        static constexpr uint8_t kMaxServicesNumber      = CHIP_DEVICE_CONFIG_THREAD_SRP_MAX_SERVICES;
        static constexpr const char * kDefaultDomainName = "default.service.arpa";
        static constexpr uint8_t kDefaultDomainNameSize  = 20;
        static constexpr uint8_t kMaxDomainNameSize      = 32;

        // SRP is used for both operational and commissionable services, so buffers sizes must be worst case.
        static constexpr size_t kSubTypeMaxNumber   = Dnssd::Common::kSubTypeMaxNumber;
        static constexpr size_t kSubTypeTotalLength = Dnssd::Common::kSubTypeTotalLength;
        static constexpr size_t kTxtMaxNumber =
            std::max(Dnssd::CommissionAdvertisingParameters::kTxtMaxNumber, Dnssd::OperationalAdvertisingParameters::kTxtMaxNumber);
        static constexpr size_t kTxtTotalValueLength = std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                Dnssd::OperationalAdvertisingParameters::kTxtTotalValueSize);

        static constexpr size_t kServiceBufferSize = Dnssd::Common::kInstanceNameMaxLength + 1 + // add null-terminator
            Dnssd::kDnssdTypeAndProtocolMaxSize + 1 +                                            // add null-terminator
            kSubTypeTotalLength + kSubTypeMaxNumber + // add null-terminator for each subtype
            kTxtTotalValueLength;

        struct Service
        {
            otSrpClientService mService;
            bool mIsInvalid;
            uint8_t mServiceBuffer[kServiceBufferSize];
            const char * mSubTypes[kSubTypeMaxNumber + 1]; // extra entry for null terminator
            otDnsTxtEntry mTxtEntries[kTxtMaxNumber];

            bool IsUsed() const { return mService.mInstanceName != nullptr; }
            bool Matches(const char * instanceName, const char * name) const;
            bool Matches(const char * instanceName, const char * name, uint16_t port, const Span<const char * const> & subTypes,
                         const Span<const Dnssd::TextEntry> & txtEntries) const;
        };

        char mHostName[Dnssd::kHostNameMaxLength + 1];
        otIp6Address mHostAddress;
        Service mServices[kMaxServicesNumber];
        bool mIsInitialized;
        DnsAsyncReturnCallback mInitializedCallback;
        void * mCallbackContext;
    };

    SrpClient mSrpClient;

    static void OnSrpClientNotification(otError aError, const otSrpClientHostInfo * aHostInfo, const otSrpClientService * aServices,
                                        const otSrpClientService * aRemovedServices, void * aContext);
    static void OnSrpClientStateChange(const otSockAddr * aServerSockAddr, void * aContext);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
    // Thread supports both operational and commissionable discovery, so buffers sizes must be worst case.
    static constexpr uint8_t kMaxDnsServiceTxtEntriesNumber =
        std::max(Dnssd::CommissionAdvertisingParameters::kTxtMaxNumber, Dnssd::OperationalAdvertisingParameters::kTxtMaxNumber);
    static constexpr size_t kTotalDnsServiceTxtValueSize = std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalValueSize,
                                                                    Dnssd::OperationalAdvertisingParameters::kTxtTotalValueSize);
    static constexpr size_t kTotalDnsServiceTxtKeySize   = std::max(Dnssd::CommissionAdvertisingParameters::kTxtTotalKeySize,
                                                                  Dnssd::OperationalAdvertisingParameters::kTxtTotalKeySize);
#else
    // Thread only supports operational discovery.
    static constexpr uint8_t kMaxDnsServiceTxtEntriesNumber = Dnssd::OperationalAdvertisingParameters::kTxtMaxNumber;
    static constexpr size_t kTotalDnsServiceTxtValueSize    = Dnssd::OperationalAdvertisingParameters::kTxtTotalValueSize;
    static constexpr size_t kTotalDnsServiceTxtKeySize      = Dnssd::OperationalAdvertisingParameters::kTxtTotalKeySize;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY
    static constexpr size_t kTotalDnsServiceTxtBufferSize =
        kTotalDnsServiceTxtKeySize + kMaxDnsServiceTxtEntriesNumber + kTotalDnsServiceTxtValueSize;

    DnsBrowseCallback mDnsBrowseCallback;
    DnsResolveCallback mDnsResolveCallback;

    struct DnsServiceTxtEntries
    {
        uint8_t mBuffer[kTotalDnsServiceTxtBufferSize];
        Dnssd::TextEntry mTxtEntries[kMaxDnsServiceTxtEntriesNumber];
    };

    struct DnsResult
    {
        void * context;
        chip::Dnssd::DnssdService mMdnsService;
        DnsServiceTxtEntries mServiceTxtEntry;
        CHIP_ERROR error;

        DnsResult(void * cbContext, CHIP_ERROR aError)
        {
            context = cbContext;
            error   = aError;
        }
    };

    static void OnDnsBrowseResult(otError aError, const otDnsBrowseResponse * aResponse, void * aContext);
    static void OnDnsResolveResult(otError aError, const otDnsServiceResponse * aResponse, void * aContext);
    static void OnDnsAddressResolveResult(otError aError, const otDnsAddressResponse * aResponse, void * aContext);

    static CHIP_ERROR ResolveAddress(intptr_t context, otDnsAddressCallback callback);

    static CHIP_ERROR FromOtDnsResponseToMdnsData(otDnsServiceInfo & serviceInfo, const char * serviceType,
                                                  chip::Dnssd::DnssdService & mdnsService, DnsServiceTxtEntries & serviceTxtEntries,
                                                  otError error);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    static void OnJoinerComplete(otError aError, void * aContext);
    void OnJoinerComplete(otError aError);

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    CHIP_ERROR SetSEDIntervalMode(ConnectivityManager::SEDIntervalMode intervalType);
#endif

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
inline void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OverrunErrorTally(void)
{
    mOverrunCount++;
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
