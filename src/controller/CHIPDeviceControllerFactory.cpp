/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Implementation of CHIP Device Controller Factory, a utility/manager class
 *      that vends Controller objects
 */

#include <controller/CHIPDeviceControllerFactory.h>

#include <app/InteractionModelEngine.h>
#include <app/OperationalSessionSetup.h>
#include <app/TimerDelegates.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/util/DataModelHandler.h>
#include <lib/core/ErrorStr.h>
#include <messaging/ReliableMessageProtocolConfig.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif

#include <app/server/Dnssd.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Credentials;

namespace chip {
namespace Controller {

CHIP_ERROR DeviceControllerFactory::Init(FactoryInitParams params)
{

    // SystemState is only set the first time init is called, after that it is managed
    // internally. If SystemState is set then init has already completed.
    if (mSystemState != nullptr)
    {
        ChipLogError(Controller, "Device Controller Factory already initialized...");
        return CHIP_NO_ERROR;
    }

    // Save our initialization state that we can't recover later from a
    // created-but-shut-down system state.
    mListenPort                = params.listenPort;
    mFabricIndependentStorage  = params.fabricIndependentStorage;
    mOperationalKeystore       = params.operationalKeystore;
    mOpCertStore               = params.opCertStore;
    mCertificateValidityPolicy = params.certificateValidityPolicy;
    mSessionResumptionStorage  = params.sessionResumptionStorage;
    mEnableServerInteractions  = params.enableServerInteractions;

    // Initialize the system state. Note that it is left in a somewhat
    // special state where it is initialized, but has a ref count of 0.
    CHIP_ERROR err = InitSystemState(params);

    return err;
}

CHIP_ERROR DeviceControllerFactory::ReinitSystemStateIfNecessary()
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mSystemState->IsShutDown(), CHIP_NO_ERROR);

    FactoryInitParams params;
    params.systemLayer        = mSystemState->SystemLayer();
    params.udpEndPointManager = mSystemState->UDPEndPointManager();
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    params.tcpEndPointManager = mSystemState->TCPEndPointManager();
#endif
#if CONFIG_NETWORK_LAYER_BLE
    params.bleLayer = mSystemState->BleLayer();
#endif
    params.listenPort                = mListenPort;
    params.fabricIndependentStorage  = mFabricIndependentStorage;
    params.enableServerInteractions  = mEnableServerInteractions;
    params.groupDataProvider         = mSystemState->GetGroupDataProvider();
    params.sessionKeystore           = mSystemState->GetSessionKeystore();
    params.fabricTable               = mSystemState->Fabrics();
    params.operationalKeystore       = mOperationalKeystore;
    params.opCertStore               = mOpCertStore;
    params.certificateValidityPolicy = mCertificateValidityPolicy;
    params.sessionResumptionStorage  = mSessionResumptionStorage;

    // re-initialization keeps any previously initialized values. The only place where
    // a provider exists is in the InteractionModelEngine, so just say "keep it as is".
    params.dataModelProvider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();

    return InitSystemState(params);
}

CHIP_ERROR DeviceControllerFactory::InitSystemState(FactoryInitParams params)
{
    if (mSystemState != nullptr)
    {
        Platform::Delete(mSystemState);
        mSystemState = nullptr;
    }

    DeviceControllerSystemStateParams stateParams;
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());

    stateParams.systemLayer        = &DeviceLayer::SystemLayer();
    stateParams.udpEndPointManager = DeviceLayer::UDPEndPointManager();
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    stateParams.tcpEndPointManager = DeviceLayer::TCPEndPointManager();
#endif
#else
    stateParams.systemLayer        = params.systemLayer;
    stateParams.tcpEndPointManager = params.tcpEndPointManager;
    stateParams.udpEndPointManager = params.udpEndPointManager;
    ChipLogError(Controller, "Warning: Device Controller Factory should be with a CHIP Device Layer...");
#endif // CONFIG_DEVICE_LAYER

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    auto tcpListenParams = Transport::TcpListenParameters(stateParams.tcpEndPointManager)
                               .SetAddressType(IPAddressType::kIPv6)
                               .SetListenPort(params.listenPort)
                               .SetServerListenEnabled(false); // Initialize as a TCP Client
#endif

    if (params.dataModelProvider == nullptr)
    {
        ChipLogError(AppServer, "Device Controller Factory requires a `dataModelProvider` value.");
        ChipLogError(AppServer, "For backwards compatibility, you likely can use `CodegenDataModelProviderInstance(...)`");
    }

    VerifyOrReturnError(params.dataModelProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(stateParams.systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(stateParams.udpEndPointManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // OperationalCertificateStore needs to be provided to init the fabric table if fabric table is
    // not provided wholesale.
    VerifyOrReturnError((params.fabricTable != nullptr) || (params.opCertStore != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.sessionKeystore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_NETWORK_LAYER_BLE
#if CONFIG_DEVICE_LAYER
    stateParams.bleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#else
    stateParams.bleLayer = params.bleLayer;
#endif // CONFIG_DEVICE_LAYER
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    stateParams.wifipaf_layer = params.wifipaf_layer;
#endif
    VerifyOrReturnError(stateParams.bleLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#endif

    stateParams.transportMgr = chip::Platform::New<DeviceTransportMgr>();

    //
    // The logic below expects IPv6 to be at index 0 of this tuple. Please do not alter that.
    //
    ReturnErrorOnFailure(stateParams.transportMgr->Init(Transport::UdpListenParameters(stateParams.udpEndPointManager)
                                                            .SetAddressType(Inet::IPAddressType::kIPv6)
                                                            .SetListenPort(params.listenPort)
#if INET_CONFIG_ENABLE_IPV4
                                                            ,
                                                        Transport::UdpListenParameters(stateParams.udpEndPointManager)
                                                            .SetAddressType(Inet::IPAddressType::kIPv4)
                                                            .SetListenPort(params.listenPort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                                            ,
                                                        Transport::BleListenParameters(stateParams.bleLayer)
#endif
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                                            ,
                                                        tcpListenParams
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                                                        ,
                                                        Transport::WiFiPAFListenParameters()
#endif
                                                            ));

    // TODO(#16231): All the new'ed state above/below in this method is never properly released or null-checked!
    stateParams.sessionMgr                = chip::Platform::New<SessionManager>();
    stateParams.certificateValidityPolicy = params.certificateValidityPolicy;
    stateParams.unsolicitedStatusHandler  = Platform::New<Protocols::SecureChannel::UnsolicitedStatusHandler>();
    stateParams.exchangeMgr               = chip::Platform::New<Messaging::ExchangeManager>();
    stateParams.messageCounterManager     = chip::Platform::New<secure_channel::MessageCounterManager>();
    stateParams.groupDataProvider         = params.groupDataProvider;
    stateParams.timerDelegate             = chip::Platform::New<chip::app::DefaultTimerDelegate>();
    stateParams.reportScheduler           = chip::Platform::New<app::reporting::ReportSchedulerImpl>(stateParams.timerDelegate);
    stateParams.sessionKeystore           = params.sessionKeystore;
    stateParams.bdxTransferServer         = chip::Platform::New<bdx::BDXTransferServer>();

    // if no fabricTable was provided, create one and track it in stateParams for cleanup
    stateParams.fabricTable = params.fabricTable;

    FabricTable * tempFabricTable = nullptr;
    if (stateParams.fabricTable == nullptr)
    {
        // TODO(#16231): Previously (and still) the objects new-ed in this entire method seem expected to last forever...
        auto newFabricTable = Platform::MakeUnique<FabricTable>();
        VerifyOrReturnError(newFabricTable, CHIP_ERROR_NO_MEMORY);

        FabricTable::InitParams fabricTableInitParams;
        fabricTableInitParams.storage             = params.fabricIndependentStorage;
        fabricTableInitParams.operationalKeystore = params.operationalKeystore;
        fabricTableInitParams.opCertStore         = params.opCertStore;
        ReturnErrorOnFailure(newFabricTable->Init(fabricTableInitParams));
        stateParams.fabricTable = newFabricTable.release();
        tempFabricTable         = stateParams.fabricTable;
    }

    SessionResumptionStorage * sessionResumptionStorage;
    if (params.sessionResumptionStorage == nullptr)
    {
        auto ownedSessionResumptionStorage = chip::Platform::MakeUnique<SimpleSessionResumptionStorage>();
        ReturnErrorOnFailure(ownedSessionResumptionStorage->Init(params.fabricIndependentStorage));
        stateParams.ownedSessionResumptionStorage    = std::move(ownedSessionResumptionStorage);
        stateParams.externalSessionResumptionStorage = nullptr;
        sessionResumptionStorage                     = stateParams.ownedSessionResumptionStorage.get();
    }
    else
    {
        stateParams.ownedSessionResumptionStorage    = nullptr;
        stateParams.externalSessionResumptionStorage = params.sessionResumptionStorage;
        sessionResumptionStorage                     = stateParams.externalSessionResumptionStorage;
    }

    auto delegate = chip::Platform::MakeUnique<ControllerFabricDelegate>();
    ReturnErrorOnFailure(delegate->Init(sessionResumptionStorage, stateParams.groupDataProvider));
    stateParams.fabricTableDelegate = delegate.get();
    ReturnErrorOnFailure(stateParams.fabricTable->AddFabricDelegate(stateParams.fabricTableDelegate));
    delegate.release();

    ReturnErrorOnFailure(stateParams.sessionMgr->Init(stateParams.systemLayer, stateParams.transportMgr,
                                                      stateParams.messageCounterManager, params.fabricIndependentStorage,
                                                      stateParams.fabricTable, *stateParams.sessionKeystore));
    ReturnErrorOnFailure(stateParams.exchangeMgr->Init(stateParams.sessionMgr));
    ReturnErrorOnFailure(stateParams.messageCounterManager->Init(stateParams.exchangeMgr));
    ReturnErrorOnFailure(stateParams.unsolicitedStatusHandler->Init(stateParams.exchangeMgr));
    ReturnErrorOnFailure(stateParams.bdxTransferServer->Init(stateParams.systemLayer, stateParams.exchangeMgr));

    chip::app::InteractionModelEngine * interactionModelEngine = chip::app::InteractionModelEngine::GetInstance();

    // Initialize the data model now that everything cluster implementations might
    // depend on is initalized.
    interactionModelEngine->SetDataModelProvider(params.dataModelProvider);

    ReturnErrorOnFailure(Dnssd::Resolver::Instance().Init(stateParams.udpEndPointManager));

    if (params.enableServerInteractions)
    {
        stateParams.caseServer = chip::Platform::New<CASEServer>();

        // Enable listening for session establishment messages.
        ReturnErrorOnFailure(stateParams.caseServer->ListenForSessionEstablishment(
            stateParams.exchangeMgr, stateParams.sessionMgr, stateParams.fabricTable, sessionResumptionStorage,
            stateParams.certificateValidityPolicy, stateParams.groupDataProvider));

        //
        // We need to advertise the port that we're listening to for unsolicited messages over UDP. However, we have both a IPv4
        // and IPv6 endpoint to pick from. Given that the listen port passed in may be set to 0 (which then has the kernel select
        // a valid port at bind time), that will result in two possible ports being provided back from the resultant endpoint
        // initializations. Since IPv6 is POR for Matter, let's go ahead and pick that port.
        //
        app::DnssdServer::Instance().SetSecuredPort(stateParams.transportMgr->GetTransport().GetImplAtIndex<0>().GetBoundPort());

        //
        // TODO: This is a hack to workaround the fact that we have a bi-polar stack that has controller and server modalities that
        // are mutually exclusive in terms of initialization of key stack singletons. Consequently, DnssdServer accesses
        // Server::GetInstance().GetFabricTable() to access the fabric table, but we don't want to do that when we're initializing
        // the controller logic since the factory here has its own fabric table.
        //
        // Consequently, reach in set the fabric table pointer to point to the right version.
        //
        app::DnssdServer::Instance().SetFabricTable(stateParams.fabricTable);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        // Disable the TCP Server based on the TCPListenParameters setting.
        app::DnssdServer::Instance().SetTCPServerEnabled(tcpListenParams.IsServerListenEnabled());
#endif
    }

    stateParams.sessionSetupPool = Platform::New<DeviceControllerSystemStateParams::SessionSetupPool>();
    stateParams.caseClientPool   = Platform::New<DeviceControllerSystemStateParams::CASEClientPool>();

    CASEClientInitParams sessionInitParams = {
        .sessionManager            = stateParams.sessionMgr,
        .sessionResumptionStorage  = sessionResumptionStorage,
        .certificateValidityPolicy = stateParams.certificateValidityPolicy,
        .exchangeMgr               = stateParams.exchangeMgr,
        .fabricTable               = stateParams.fabricTable,
        .groupDataProvider         = stateParams.groupDataProvider,
        // Don't provide an MRP local config, so each CASE initiation will use
        // the then-current value.
        .mrpLocalConfig = NullOptional,
    };

    CASESessionManagerConfig sessionManagerConfig = {
        .sessionInitParams = sessionInitParams,
        .clientPool        = stateParams.caseClientPool,
        .sessionSetupPool  = stateParams.sessionSetupPool,
    };

    // TODO: Need to be able to create a CASESessionManagerConfig here!
    stateParams.caseSessionManager = Platform::New<CASESessionManager>();
    ReturnErrorOnFailure(stateParams.caseSessionManager->Init(stateParams.systemLayer, sessionManagerConfig));

    ReturnErrorOnFailure(interactionModelEngine->Init(stateParams.exchangeMgr, stateParams.fabricTable, stateParams.reportScheduler,
                                                      stateParams.caseSessionManager));

    // store the system state
    mSystemState = chip::Platform::New<DeviceControllerSystemState>(std::move(stateParams));
    mSystemState->SetTempFabricTable(tempFabricTable, params.enableServerInteractions);
    ChipLogDetail(Controller, "System State Initialized...");
    return CHIP_NO_ERROR;
}

void DeviceControllerFactory::PopulateInitParams(ControllerInitParams & controllerParams, const SetupParams & params)
{
    controllerParams.operationalCredentialsDelegate       = params.operationalCredentialsDelegate;
    controllerParams.operationalKeypair                   = params.operationalKeypair;
    controllerParams.hasExternallyOwnedOperationalKeypair = params.hasExternallyOwnedOperationalKeypair;
    controllerParams.controllerNOC                        = params.controllerNOC;
    controllerParams.controllerICAC                       = params.controllerICAC;
    controllerParams.controllerRCAC                       = params.controllerRCAC;
    controllerParams.permitMultiControllerFabrics         = params.permitMultiControllerFabrics;
    controllerParams.removeFromFabricTableOnShutdown      = params.removeFromFabricTableOnShutdown;
    controllerParams.deleteFromFabricTableOnShutdown      = params.deleteFromFabricTableOnShutdown;

    controllerParams.systemState        = mSystemState;
    controllerParams.controllerVendorId = params.controllerVendorId;

    controllerParams.enableServerInteractions = params.enableServerInteractions;
    if (params.fabricIndex.HasValue())
    {
        controllerParams.fabricIndex.SetValue(params.fabricIndex.Value());
    }
}

void DeviceControllerFactory::ControllerInitialized(const DeviceController & controller)
{
    if (mEnableServerInteractions && controller.GetFabricIndex() != kUndefinedFabricIndex)
    {
        // Restart DNS-SD advertising, because initialization of this controller could
        // have modified whether a particular fabric identity should be
        // advertised.  Just calling AdvertiseOperational() is not good enough
        // here, since we might be removing advertising.
        app::DnssdServer::Instance().StartServer();
    }
}

CHIP_ERROR DeviceControllerFactory::SetupController(SetupParams params, DeviceController & controller)
{
    VerifyOrReturnError(params.controllerVendorId != VendorId::Unspecified, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(ReinitSystemStateIfNecessary());

    ControllerInitParams controllerParams;
    PopulateInitParams(controllerParams, params);

    CHIP_ERROR err = controller.Init(controllerParams);

    if (err == CHIP_NO_ERROR)
    {
        ControllerInitialized(controller);
    }

    return err;
}

CHIP_ERROR DeviceControllerFactory::SetupCommissioner(SetupParams params, DeviceCommissioner & commissioner)
{
    VerifyOrReturnError(params.controllerVendorId != VendorId::Unspecified, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(ReinitSystemStateIfNecessary());

    CommissionerInitParams commissionerParams;

    // PopulateInitParams works against ControllerInitParams base class of CommissionerInitParams only
    PopulateInitParams(commissionerParams, params);

    // Set commissioner-specific fields not in ControllerInitParams
    commissionerParams.pairingDelegate           = params.pairingDelegate;
    commissionerParams.defaultCommissioner       = params.defaultCommissioner;
    commissionerParams.deviceAttestationVerifier = params.deviceAttestationVerifier;

    CHIP_ERROR err = commissioner.Init(commissionerParams);

    if (err == CHIP_NO_ERROR)
    {
        ControllerInitialized(commissioner);
    }

    return err;
}

CHIP_ERROR DeviceControllerFactory::ServiceEvents()
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().StartEventLoopTask());
#endif // CONFIG_DEVICE_LAYER

    return CHIP_NO_ERROR;
}

void DeviceControllerFactory::RetainSystemState()
{
    (void) mSystemState->Retain();
}

bool DeviceControllerFactory::ReleaseSystemState()
{
    return mSystemState->Release();
}

CHIP_ERROR DeviceControllerFactory::EnsureAndRetainSystemState()
{
    ReturnErrorOnFailure(ReinitSystemStateIfNecessary());
    RetainSystemState();
    return CHIP_NO_ERROR;
}

DeviceControllerFactory::~DeviceControllerFactory()
{
    Shutdown();
}

void DeviceControllerFactory::Shutdown()
{
    if (mSystemState != nullptr)
    {
        // ~DeviceControllerSystemState will call Shutdown(),
        // which in turn ensures that the reference count is 0.
        Platform::Delete(mSystemState);
        mSystemState = nullptr;
    }
    mFabricIndependentStorage  = nullptr;
    mOperationalKeystore       = nullptr;
    mOpCertStore               = nullptr;
    mCertificateValidityPolicy = nullptr;
    mSessionResumptionStorage  = nullptr;
}

void DeviceControllerSystemState::Shutdown()
{
    VerifyOrDie(mRefCount == 0);
    if (mHaveShutDown)
    {
        // Nothing else to do here.
        return;
    }
    mHaveShutDown = true;

    ChipLogDetail(Controller, "Shutting down the System State, this will teardown the CHIP Stack");

    if (mTempFabricTable && mEnableServerInteractions)
    {
        // The DnssdServer is holding a reference to our temp fabric table,
        // which we are about to destroy.  Stop it, so that it will stop trying
        // to use it.
        app::DnssdServer::Instance().StopServer();
    }

    if (mFabricTableDelegate != nullptr)
    {
        if (mFabrics != nullptr)
        {
            mFabrics->RemoveFabricDelegate(mFabricTableDelegate);
        }

        chip::Platform::Delete(mFabricTableDelegate);
        mFabricTableDelegate = nullptr;
    }

    if (mBDXTransferServer != nullptr)
    {
        mBDXTransferServer->Shutdown();
        chip::Platform::Delete(mBDXTransferServer);
        mBDXTransferServer = nullptr;
    }

    if (mCASEServer != nullptr)
    {
        mCASEServer->Shutdown();
        chip::Platform::Delete(mCASEServer);
        mCASEServer = nullptr;
    }

    if (mCASESessionManager != nullptr)
    {
        mCASESessionManager->Shutdown();
        Platform::Delete(mCASESessionManager);
        mCASESessionManager = nullptr;
    }

    // The above took care of CASE handshakes, and shutting down all the
    // controllers should have taken care of the PASE handshakes.  Clean up any
    // outstanding secure sessions (shouldn't really be any, since controllers
    // should have handled that, but just in case).
    if (mSessionMgr != nullptr)
    {
        mSessionMgr->ExpireAllSecureSessions();
    }

    // mCASEClientPool and mSessionSetupPool must be deallocated
    // after mCASESessionManager, which uses them.

    if (mSessionSetupPool != nullptr)
    {
        Platform::Delete(mSessionSetupPool);
        mSessionSetupPool = nullptr;
    }

    if (mCASEClientPool != nullptr)
    {
        Platform::Delete(mCASEClientPool);
        mCASEClientPool = nullptr;
    }

    Dnssd::Resolver::Instance().Shutdown();

    // Shut down the interaction model
    app::InteractionModelEngine::GetInstance()->Shutdown();

    // Shut down the TransportMgr. This holds Inet::UDPEndPoints so it must be shut down
    // before PlatformMgr().Shutdown() shuts down Inet.
    if (mTransportMgr != nullptr)
    {
        mTransportMgr->Close();
        chip::Platform::Delete(mTransportMgr);
        mTransportMgr = nullptr;
    }

    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->Shutdown();
    }
    if (mSessionMgr != nullptr)
    {
        mSessionMgr->Shutdown();
    }

    mSystemLayer        = nullptr;
    mUDPEndPointManager = nullptr;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    mTCPEndPointManager = nullptr;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = nullptr;
#endif // CONFIG_NETWORK_LAYER_BLE

    if (mMessageCounterManager != nullptr)
    {
        chip::Platform::Delete(mMessageCounterManager);
        mMessageCounterManager = nullptr;
    }

    if (mExchangeMgr != nullptr)
    {
        chip::Platform::Delete(mExchangeMgr);
        mExchangeMgr = nullptr;
    }

    if (mUnsolicitedStatusHandler != nullptr)
    {
        Platform::Delete(mUnsolicitedStatusHandler);
        mUnsolicitedStatusHandler = nullptr;
    }

    if (mSessionMgr != nullptr)
    {
        chip::Platform::Delete(mSessionMgr);
        mSessionMgr = nullptr;
    }

    if (mReportScheduler != nullptr)
    {
        chip::Platform::Delete(mReportScheduler);
        mReportScheduler = nullptr;
    }

    if (mTimerDelegate != nullptr)
    {
        chip::Platform::Delete(mTimerDelegate);
        mTimerDelegate = nullptr;
    }

    if (mTempFabricTable != nullptr)
    {
        mTempFabricTable->Shutdown();
        chip::Platform::Delete(mTempFabricTable);
        mTempFabricTable = nullptr;
        // if we created a temp fabric table, then mFabrics points to it.
        // if we did not create a temp fabric table, then keep the reference
        // so that SetupController/Commissioner can use it
        mFabrics = nullptr;
    }

#if CONFIG_DEVICE_LAYER
    //
    // We can safely call PlatformMgr().Shutdown(), which like DeviceController::Shutdown(),
    // expects to be called with external thread synchronization and will not try to acquire the
    // stack lock.
    //
    // Actually stopping the event queue is a separable call that applications will have to sequence.
    // Consumers are expected to call PlaformMgr().StopEventLoopTask() before calling
    // DeviceController::Shutdown() in the CONFIG_DEVICE_LAYER configuration
    //
    DeviceLayer::PlatformMgr().Shutdown();
#endif
}

} // namespace Controller
} // namespace chip
