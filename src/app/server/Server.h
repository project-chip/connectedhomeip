/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/AppConfig.h>
#include <app/icd/server/ICDServerConfig.h>

#include <access/AccessControl.h>
#include <access/examples/ExampleAccessControlDelegate.h>
#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/FailSafeContext.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/SimpleSubscriptionResumptionStorage.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/server/AclStorage.h>
#include <app/server/AppDelegate.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/DefaultAclStorage.h>
#include <credentials/CertificateValidityPolicy.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/OperationalCertificateStore.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/OperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <messaging/ExchangeMgr.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/KvsPersistentStorageDelegate.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/secure_channel/UnsolicitedStatusHandler.h>
#if CHIP_CONFIG_ENABLE_SESSION_RESUMPTION
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>
#endif
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <system/SystemClock.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/TransportMgrBase.h>
#if CONFIG_NETWORK_LAYER_BLE
#include <transport/raw/BLE.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <transport/raw/WiFiPAF.h>
#endif
#include <app/TimerDelegates.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <transport/raw/UDP.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDManager.h> // nogncheck

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/icd/server/DefaultICDCheckInBackOffStrategy.h> // nogncheck
#include <app/icd/server/ICDCheckInBackOffStrategy.h>        // nogncheck
#endif                                                       // CHIP_CONFIG_ENABLE_ICD_CIP
#endif                                                       // CHIP_CONFIG_ENABLE_ICD_SERVER

namespace chip {

inline constexpr size_t kMaxBlePendingPackets = 1;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
inline constexpr size_t kMaxTcpActiveConnectionCount = CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS;

inline constexpr size_t kMaxTcpPendingPackets = CHIP_CONFIG_MAX_TCP_PENDING_PACKETS;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

//
// NOTE: Please do not alter the order of template specialization here as the logic
//       in the Server impl depends on this.
//
using ServerTransportMgr = chip::TransportMgr<chip::Transport::UDP
#if INET_CONFIG_ENABLE_IPV4
                                              ,
                                              chip::Transport::UDP
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                              ,
                                              chip::Transport::BLE<kMaxBlePendingPackets>
#endif
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                              ,
                                              chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                                              ,
                                              chip::Transport::WiFiPAFBase
#endif
                                              >;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
using UdcTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                     ,
                                     Transport::UDP /* IPv4 */
#endif
                                     >;
#endif

struct ServerInitParams
{
    ServerInitParams() = default;

    // Not copyable
    ServerInitParams(const ServerInitParams &)             = delete;
    ServerInitParams & operator=(const ServerInitParams &) = delete;

    // Application delegate to handle some commissioning lifecycle events
    AppDelegate * appDelegate = nullptr;
    // Port to use for Matter commissioning/operational traffic
    uint16_t operationalServicePort = CHIP_PORT;
    // Port to use for UDC if supported
    uint16_t userDirectedCommissioningPort = CHIP_UDC_PORT;
    // Interface on which to run daemon
    Inet::InterfaceId interfaceId = Inet::InterfaceId::Null();

    // Persistent storage delegate: MUST be injected. Used to maintain storage by much common code.
    // Must be initialized before being provided.
    PersistentStorageDelegate * persistentStorageDelegate = nullptr;
    // Session resumption storage: Optional. Support session resumption when provided.
    // Must be initialized before being provided.
    SessionResumptionStorage * sessionResumptionStorage = nullptr;
    // Session resumption storage: Optional. Support session resumption when provided.
    // Must be initialized before being provided.
    app::SubscriptionResumptionStorage * subscriptionResumptionStorage = nullptr;
    // Certificate validity policy: Optional. If none is injected, CHIPCert
    // enforces a default policy.
    Credentials::CertificateValidityPolicy * certificateValidityPolicy = nullptr;
    // Group data provider: MUST be injected. Used to maintain critical keys such as the Identity
    // Protection Key (IPK) for CASE. Must be initialized before being provided.
    Credentials::GroupDataProvider * groupDataProvider = nullptr;
    // Session keystore: MUST be injected. Used to derive and manage lifecycle of symmetric keys.
    Crypto::SessionKeystore * sessionKeystore = nullptr;
    // Access control delegate: MUST be injected. Used to look up access control rules. Must be
    // initialized before being provided.
    Access::AccessControl::Delegate * accessDelegate = nullptr;
    // ACL storage: MUST be injected. Used to store ACL entries in persistent storage. Must NOT
    // be initialized before being provided.
    app::AclStorage * aclStorage = nullptr;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Access Restriction implementation: MUST be injected if MNGD feature enabled. Used to enforce
    // access restrictions that are managed by the device.
    Access::AccessRestrictionProvider * accessRestrictionProvider = nullptr;
#endif

    // Network native params can be injected depending on the
    // selected Endpoint implementation
    void * endpointNativeParams = nullptr;
    // Optional. Support test event triggers when provided. Must be initialized before being
    // provided.
    TestEventTriggerDelegate * testEventTriggerDelegate = nullptr;
    // Operational keystore with access to the operational keys: MUST be injected.
    Crypto::OperationalKeystore * operationalKeystore = nullptr;
    // Operational certificate store with access to the operational certs in persisted storage:
    // must not be null at timne of Server::Init().
    Credentials::OperationalCertificateStore * opCertStore = nullptr;
    // Required, if not provided, the Server::Init() WILL fail.
    app::reporting::ReportScheduler * reportScheduler = nullptr;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    // Optional. Support for the ICD Check-In BackOff strategy. Must be initialized before being provided.
    // If the ICD Check-In protocol use-case is supported and no strategy is provided, server will use the default strategy.
    app::ICDCheckInBackOffStrategy * icdCheckInBackOffStrategy = nullptr;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    // MUST NOT be null during initialization: every application must define the
    // data model it wants to use. Backwards-compatibility can use `CodegenDataModelProviderInstance`
    // for ember/zap-generated models.
    chip::app::DataModel::Provider * dataModelProvider = nullptr;
};

/**
 * Transitional version of ServerInitParams to assist SDK integrators in
 * transitioning to injecting product/platform-owned resources. This version
 * of `ServerInitParams` statically owns and initializes (via the
 * `InitializeStaticResourcesBeforeServerInit()` method) the persistent storage
 * delegate, the group data provider, and the access control delegate. This is to reduce
 * the amount of copied boilerplate in all the example initializations (e.g. AppTask.cpp,
 * main.cpp).
 *
 * This version SHOULD BE USED ONLY FOR THE IN-TREE EXAMPLES.
 *
 * ACTION ITEMS FOR TRANSITION from a example in-tree to a product:
 *
 * While this could be used indefinitely, it does not exemplify orderly management of
 * application-injected resources. It is recommended for actual products to instead:
 *   - Use the basic ServerInitParams in the application
 *   - Have the application own an instance of the resources being injected in its own
 *     state (e.g. an implementation of PersistentStorageDelegate and GroupDataProvider
 *     interfaces).
 *   - Initialize the injected resources prior to calling Server::Init()
 *   - De-initialize the injected resources after calling Server::Shutdown()
 *
 * WARNING: DO NOT replicate the pattern shown here of having a subclass of ServerInitParams
 *          own the resources outside of examples. This was done to reduce the amount of change
 *          to existing examples while still supporting non-example versions of the
 *          resources to be injected.
 */
struct CommonCaseDeviceServerInitParams : public ServerInitParams
{
    CommonCaseDeviceServerInitParams() = default;

    // Not copyable
    CommonCaseDeviceServerInitParams(const CommonCaseDeviceServerInitParams &)             = delete;
    CommonCaseDeviceServerInitParams & operator=(const CommonCaseDeviceServerInitParams &) = delete;

    /**
     * Call this before Server::Init() to initialize the internally-owned resources.
     * Server::Init() will fail if this is not done, since several params required to
     * be non-null will be null without calling this method. ** See the transition method
     * in the outer comment of this class **.
     *
     * @return CHIP_NO_ERROR on success or a CHIP_ERROR value from APIs called to initialize
     *         resources on failure.
     */
    CHIP_ERROR InitializeStaticResourcesBeforeServerInit()
    {
        // KVS-based persistent storage delegate injection
        if (persistentStorageDelegate == nullptr)
        {
            chip::DeviceLayer::PersistedStorage::KeyValueStoreManager & kvsManager =
                DeviceLayer::PersistedStorage::KeyValueStoreMgr();
            ReturnErrorOnFailure(sKvsPersistenStorageDelegate.Init(&kvsManager));
            this->persistentStorageDelegate = &sKvsPersistenStorageDelegate;
        }

        // PersistentStorageDelegate "software-based" operational key access injection
        if (this->operationalKeystore == nullptr)
        {
            // WARNING: PersistentStorageOperationalKeystore::Finish() is never called. It's fine for
            //          for examples and for now.
            ReturnErrorOnFailure(sPersistentStorageOperationalKeystore.Init(this->persistentStorageDelegate));
            this->operationalKeystore = &sPersistentStorageOperationalKeystore;
        }

        // OpCertStore can be injected but default to persistent storage default
        // for simplicity of the examples.
        if (this->opCertStore == nullptr)
        {
            // WARNING: PersistentStorageOpCertStore::Finish() is never called. It's fine for
            //          for examples and for now, since all storage is immediate for that impl.
            ReturnErrorOnFailure(sPersistentStorageOpCertStore.Init(this->persistentStorageDelegate));
            this->opCertStore = &sPersistentStorageOpCertStore;
        }

        // Injection of report scheduler WILL lead to two schedulers being allocated. As recommended above, this should only be used
        // for IN-TREE examples. If a default scheduler is desired, the basic ServerInitParams should be used by the application and
        // CommonCaseDeviceServerInitParams should not be allocated.
        if (this->reportScheduler == nullptr)
        {
            reportScheduler = &sReportScheduler;
        }

        // Session Keystore injection
        this->sessionKeystore = &sSessionKeystore;

        // Group Data provider injection
        sGroupDataProvider.SetStorageDelegate(this->persistentStorageDelegate);
        sGroupDataProvider.SetSessionKeystore(this->sessionKeystore);
        ReturnErrorOnFailure(sGroupDataProvider.Init());
        this->groupDataProvider = &sGroupDataProvider;

#if CHIP_CONFIG_ENABLE_SESSION_RESUMPTION
        ReturnErrorOnFailure(sSessionResumptionStorage.Init(this->persistentStorageDelegate));
        this->sessionResumptionStorage = &sSessionResumptionStorage;
#else
        this->sessionResumptionStorage = nullptr;
#endif

        // Inject access control delegate
        this->accessDelegate = Access::Examples::GetAccessControlDelegate();

        // Inject ACL storage. (Don't initialize it.)
        this->aclStorage = &sAclStorage;

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
        ChipLogProgress(AppServer, "Initializing subscription resumption storage...");
        ReturnErrorOnFailure(sSubscriptionResumptionStorage.Init(this->persistentStorageDelegate));
        this->subscriptionResumptionStorage = &sSubscriptionResumptionStorage;
#else
        ChipLogProgress(AppServer, "Subscription persistence not supported");
#endif

#if CHIP_CONFIG_ENABLE_ICD_CIP
        if (this->icdCheckInBackOffStrategy == nullptr)
        {
            this->icdCheckInBackOffStrategy = &sDefaultICDCheckInBackOffStrategy;
        }
#endif

        return CHIP_NO_ERROR;
    }

private:
    static KvsPersistentStorageDelegate sKvsPersistenStorageDelegate;
    static PersistentStorageOperationalKeystore sPersistentStorageOperationalKeystore;
    static Credentials::PersistentStorageOpCertStore sPersistentStorageOpCertStore;
    static Credentials::GroupDataProviderImpl sGroupDataProvider;
    static chip::app::DefaultTimerDelegate sTimerDelegate;
    static app::reporting::ReportSchedulerImpl sReportScheduler;

#if CHIP_CONFIG_ENABLE_SESSION_RESUMPTION
    static SimpleSessionResumptionStorage sSessionResumptionStorage;
#endif
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    static app::SimpleSubscriptionResumptionStorage sSubscriptionResumptionStorage;
#endif
    static app::DefaultAclStorage sAclStorage;
    static Crypto::DefaultSessionKeystore sSessionKeystore;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    static app::DefaultICDCheckInBackOffStrategy sDefaultICDCheckInBackOffStrategy;
#endif
};

/**
 * The `Server` singleton class is an aggregate for all the resources needed to run a
 * Node that is both Commissionable and mainly used as an end-node with server clusters.
 * In other words, it aggregates the state needed for the type of Node used for most
 * products that are not mainly controller/administrator role.
 *
 * This sington class expects `ServerInitParams` initialization parameters but does not
 * own the resources injected from `ServerInitParams`. Any object pointers/references
 * passed in ServerInitParams must be pre-initialized externally, and shutdown/finalized
 * after `Server::Shutdown()` is called.
 *
 * TODO: Separate lifecycle ownership for some more capabilities that should not belong to
 *       common logic, such as `GenerateShutDownEvent`.
 *
 * TODO: Replace all uses of GetInstance() to "reach in" to this state from all cluster
 *       server common logic that deal with global node state with either a common NodeState
 *       compatible with OperationalDeviceProxy/DeviceProxy, or with injection at common
 *       SDK logic init.
 */
class Server
{
public:
    CHIP_ERROR Init(const ServerInitParams & initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR
    SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner,
                                         Protocols::UserDirectedCommissioning::IdentificationDeclaration & id);

    Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient * GetUserDirectedCommissioningClient()
    {
        return gUDCClient;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    /**
     * @brief Call this function to rejoin existing groups found in the GroupDataProvider
     */
    void RejoinExistingMulticastGroups();

    FabricTable & GetFabricTable() { return mFabrics; }

    CASESessionManager * GetCASESessionManager() { return &mCASESessionManager; }

    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeMgr; }

    SessionManager & GetSecureSessionManager() { return mSessions; }

    SessionResumptionStorage * GetSessionResumptionStorage() { return mSessionResumptionStorage; }

    app::SubscriptionResumptionStorage * GetSubscriptionResumptionStorage() { return mSubscriptionResumptionStorage; }

    TransportMgrBase & GetTransportManager() { return mTransports; }

    Credentials::GroupDataProvider * GetGroupDataProvider() { return mGroupsProvider; }

    Crypto::SessionKeystore * GetSessionKeystore() const { return mSessionKeystore; }

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * GetBleLayerObject() { return mBleLayer; }
#endif

    CommissioningWindowManager & GetCommissioningWindowManager() { return mCommissioningWindowManager; }

    PersistentStorageDelegate & GetPersistentStorage() { return *mDeviceStorage; }

    app::FailSafeContext & GetFailSafeContext() { return mFailSafeContext; }

    TestEventTriggerDelegate * GetTestEventTriggerDelegate() { return mTestEventTriggerDelegate; }

    Crypto::OperationalKeystore * GetOperationalKeystore() { return mOperationalKeystore; }

    Credentials::OperationalCertificateStore * GetOpCertStore() { return mOpCertStore; }

    app::DefaultSafeAttributePersistenceProvider & GetDefaultAttributePersister() { return mAttributePersister; }

    app::reporting::ReportScheduler * GetReportScheduler() { return mReportScheduler; }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::ICDManager & GetICDManager() { return mICDManager; }

#if CHIP_CONFIG_ENABLE_ICD_CIP
    /**
     * @brief Function to determine if a Check-In message would be sent at Boot up
     *
     * @param aFabricIndex client fabric index
     * @param subjectID client subject ID
     * @return true Check-In message would be sent on boot up.
     * @return false Device has a persisted subscription with the client. See CHIP_CONFIG_PERSIST_SUBSCRIPTIONS.
     */
    bool ShouldCheckInMsgsBeSentAtBootFunction(FabricIndex aFabricIndex, NodeId subjectID);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    /**
     * This function causes the ShutDown event to be generated async on the
     * Matter event loop.  Should be called before stopping the event loop.
     */
    void GenerateShutDownEvent();

    void Shutdown();

    void ScheduleFactoryReset();

    System::Clock::Microseconds64 TimeSinceInit() const
    {
        return System::SystemClock().GetMonotonicMicroseconds64() - mInitTimestamp;
    }

    static Server & GetInstance() { return sServer; }

private:
    Server() {}

    static Server sServer;

    void InitFailSafe();
    void OnPlatformEvent(const DeviceLayer::ChipDeviceEvent & event);
    void CheckServerReadyEvent();

    void PostFactoryResetEvent();

    static void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t);

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    /**
     * @brief Called at Server::Init time to resume persisted subscriptions if the feature flag is enabled
     */
    void ResumeSubscriptions();
#endif

    class GroupDataProviderListener final : public Credentials::GroupDataProvider::GroupListener
    {
    public:
        GroupDataProviderListener() {}

        CHIP_ERROR Init(Server * server)
        {
            VerifyOrReturnError(server != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mServer = server;
            return CHIP_NO_ERROR;
        };

        void OnGroupAdded(chip::FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & new_group) override
        {
            const FabricInfo * fabric = mServer->GetFabricTable().FindFabricWithIndex(fabric_index);
            if (fabric == nullptr)
            {
                ChipLogError(AppServer, "Group added to nonexistent fabric?");
                return;
            }

            if (mServer->GetTransportManager().MulticastGroupJoinLeave(
                    Transport::PeerAddress::Multicast(fabric->GetFabricId(), new_group.group_id), true) != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Unable to listen to group");
            }
        };

        void OnGroupRemoved(chip::FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & old_group) override
        {
            const FabricInfo * fabric = mServer->GetFabricTable().FindFabricWithIndex(fabric_index);
            if (fabric == nullptr)
            {
                ChipLogError(AppServer, "Group removed from nonexistent fabric?");
                return;
            }

            mServer->GetTransportManager().MulticastGroupJoinLeave(
                Transport::PeerAddress::Multicast(fabric->GetFabricId(), old_group.group_id), false);
        };

    private:
        Server * mServer;
    };

    class ServerFabricDelegate final : public chip::FabricTable::Delegate
    {
    public:
        ServerFabricDelegate() {}

        CHIP_ERROR Init(Server * server)
        {
            VerifyOrReturnError(server != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mServer = server;
            return CHIP_NO_ERROR;
        }

        void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            ClearCASEResumptionStateOnFabricChange(fabricIndex);
            ClearSubscriptionResumptionStateOnFabricChange(fabricIndex);

            Credentials::GroupDataProvider * groupDataProvider = mServer->GetGroupDataProvider();
            if (groupDataProvider != nullptr)
            {
                CHIP_ERROR err = groupDataProvider->RemoveFabric(fabricIndex);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                 "Warning, failed to delete GroupDataProvider state for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                                 static_cast<unsigned>(fabricIndex), err.Format());
                }
            }

            // Remove access control entries in reverse order (it could be any order, but reverse order
            // will cause less churn in persistent storage).
            CHIP_ERROR aclErr = Access::GetAccessControl().DeleteAllEntriesForFabric(fabricIndex);
            if (aclErr != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Warning, failed to delete access control state for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(fabricIndex), aclErr.Format());
            }

            //  Remove ACL extension entry for the given fabricIndex.
            auto & storage = mServer->GetPersistentStorage();
            aclErr = storage.SyncDeleteKeyValue(DefaultStorageKeyAllocator::AccessControlExtensionEntry(fabricIndex).KeyName());

            if (aclErr != CHIP_NO_ERROR && aclErr != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                ChipLogError(AppServer, "Warning, failed to delete ACL extension entry for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(fabricIndex), aclErr.Format());
            }

            mServer->GetCommissioningWindowManager().OnFabricRemoved(fabricIndex);
        }

        void OnFabricUpdated(const FabricTable & fabricTable, chip::FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            ClearCASEResumptionStateOnFabricChange(fabricIndex);
        }

    private:
        void ClearCASEResumptionStateOnFabricChange(chip::FabricIndex fabricIndex)
        {
            auto * sessionResumptionStorage = mServer->GetSessionResumptionStorage();
            VerifyOrReturn(sessionResumptionStorage != nullptr);
            CHIP_ERROR err = sessionResumptionStorage->DeleteAll(fabricIndex);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "Warning, failed to delete session resumption state for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(fabricIndex), err.Format());
            }
        }

        void ClearSubscriptionResumptionStateOnFabricChange(chip::FabricIndex fabricIndex)
        {
            auto * subscriptionResumptionStorage = mServer->GetSubscriptionResumptionStorage();
            VerifyOrReturn(subscriptionResumptionStorage != nullptr);
            CHIP_ERROR err = subscriptionResumptionStorage->DeleteAll(fabricIndex);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "Warning, failed to delete subscription resumption state for fabric index 0x%x: %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(fabricIndex), err.Format());
            }
        }

        Server * mServer = nullptr;
    };

    /**
     * Since root certificates for Matter nodes cannot be updated in a reasonable
     * way, it doesn't make sense to enforce expiration time on root certificates.
     * This policy allows through root certificates, even if they're expired, and
     * otherwise delegates to the provided policy, or to the default policy if no
     * policy is provided.
     */
    class IgnoreRootExpirationValidityPolicy : public Credentials::CertificateValidityPolicy
    {
    public:
        IgnoreRootExpirationValidityPolicy() {}

        void Init(Credentials::CertificateValidityPolicy * providedPolicy) { mProvidedPolicy = providedPolicy; }

        CHIP_ERROR ApplyCertificateValidityPolicy(const Credentials::ChipCertificateData * cert, uint8_t depth,
                                                  Credentials::CertificateValidityResult result) override
        {
            switch (result)
            {
            case Credentials::CertificateValidityResult::kExpired:
            case Credentials::CertificateValidityResult::kExpiredAtLastKnownGoodTime:
            case Credentials::CertificateValidityResult::kTimeUnknown: {
                Credentials::CertType certType;
                ReturnErrorOnFailure(cert->mSubjectDN.GetCertType(certType));
                if (certType == Credentials::CertType::kRoot)
                {
                    return CHIP_NO_ERROR;
                }

                break;
            }
            default:
                break;
            }

            if (mProvidedPolicy)
            {
                return mProvidedPolicy->ApplyCertificateValidityPolicy(cert, depth, result);
            }

            return Credentials::CertificateValidityPolicy::ApplyDefaultPolicy(cert, depth, result);
        }

    private:
        Credentials::CertificateValidityPolicy * mProvidedPolicy = nullptr;
    };

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif

    // By default, use a certificate validation policy compatible with non-wall-clock-time-synced
    // embedded systems.
    static Credentials::IgnoreCertificateValidityPeriodPolicy sDefaultCertValidityPolicy;

    ServerTransportMgr mTransports;
    SessionManager mSessions;
    CASEServer mCASEServer;

    CASESessionManager mCASESessionManager;
    CASEClientPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS> mCASEClientPool;
    OperationalSessionSetupPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES> mSessionSetupPool;

    Protocols::SecureChannel::UnsolicitedStatusHandler mUnsolicitedStatusHandler;
    Messaging::ExchangeManager mExchangeMgr;
    FabricTable mFabrics;
    secure_channel::MessageCounterManager mMessageCounterManager;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient * gUDCClient = nullptr;
    // mUdcTransportMgr is for insecure communication (ex. user directed commissioning)
    // specifically, the commissioner declaration message (sent by commissioner to commissionee)
    UdcTransportMgr * mUdcTransportMgr = nullptr;
    uint16_t mCdcListenPort            = CHIP_UDC_COMMISSIONEE_PORT;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CommissioningWindowManager mCommissioningWindowManager;

    IgnoreRootExpirationValidityPolicy mCertificateValidityPolicy;

    PersistentStorageDelegate * mDeviceStorage;
    SessionResumptionStorage * mSessionResumptionStorage;
    app::SubscriptionResumptionStorage * mSubscriptionResumptionStorage;
    Credentials::GroupDataProvider * mGroupsProvider;
    Crypto::SessionKeystore * mSessionKeystore;
    app::DefaultSafeAttributePersistenceProvider mAttributePersister;
    GroupDataProviderListener mListener;
    ServerFabricDelegate mFabricDelegate;
    app::reporting::ReportScheduler * mReportScheduler;

    Access::AccessControl mAccessControl;
    app::AclStorage * mAclStorage;

    TestEventTriggerDelegate * mTestEventTriggerDelegate;
    Crypto::OperationalKeystore * mOperationalKeystore;
    Credentials::OperationalCertificateStore * mOpCertStore;
    app::FailSafeContext mFailSafeContext;

    bool mIsDnssdReady = false;
    uint16_t mOperationalServicePort;
    uint16_t mUserDirectedCommissioningPort;
    Inet::InterfaceId mInterfaceId;

    System::Clock::Microseconds64 mInitTimestamp;
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::ICDManager mICDManager;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
};

} // namespace chip
