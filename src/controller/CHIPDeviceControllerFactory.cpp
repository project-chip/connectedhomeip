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

#include <app/OperationalDeviceProxy.h>
#include <app/util/DataModelHandler.h>
#include <lib/support/ErrorStr.h>
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
    mListenPort               = params.listenPort;
    mFabricIndependentStorage = params.fabricIndependentStorage;
    mOperationalKeystore      = params.operationalKeystore;
    mOpCertStore              = params.opCertStore;
    mEnableServerInteractions = params.enableServerInteractions;

    CHIP_ERROR err = InitSystemState(params);

    return err;
}

CHIP_ERROR DeviceControllerFactory::InitSystemState()
{
    FactoryInitParams params;
    if (mSystemState != nullptr)
    {
        params.systemLayer        = mSystemState->SystemLayer();
        params.udpEndPointManager = mSystemState->UDPEndPointManager();
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        params.tcpEndPointManager = mSystemState->TCPEndPointManager();
#endif
#if CONFIG_NETWORK_LAYER_BLE
        params.bleLayer = mSystemState->BleLayer();
#endif
        params.listenPort               = mListenPort;
        params.fabricIndependentStorage = mFabricIndependentStorage;
        params.enableServerInteractions = mEnableServerInteractions;
        params.groupDataProvider        = mSystemState->GetGroupDataProvider();
        params.fabricTable              = mSystemState->Fabrics();
        params.operationalKeystore      = mOperationalKeystore;
        params.opCertStore              = mOpCertStore;
    }

    return InitSystemState(params);
}

CHIP_ERROR DeviceControllerFactory::InitSystemState(FactoryInitParams params)
{
    if (mSystemState != nullptr && mSystemState->IsInitialized())
    {
        return CHIP_NO_ERROR;
    }

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

    VerifyOrReturnError(stateParams.systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(stateParams.udpEndPointManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // OperationalCertificateStore needs to be provided to init the fabric table if fabric table is
    // not provided wholesale.
    ReturnErrorCodeIf((params.fabricTable == nullptr) && (params.opCertStore == nullptr), CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_NETWORK_LAYER_BLE
#if CONFIG_DEVICE_LAYER
    stateParams.bleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#else
    stateParams.bleLayer = params.bleLayer;
#endif // CONFIG_DEVICE_LAYER
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
                                                            ));

    // TODO(#16231): All the new'ed state above/below in this method is never properly released or null-checked!
    stateParams.sessionMgr                = chip::Platform::New<SessionManager>();
    stateParams.certificateValidityPolicy = params.certificateValidityPolicy;
    stateParams.unsolicitedStatusHandler  = Platform::New<Protocols::SecureChannel::UnsolicitedStatusHandler>();
    stateParams.exchangeMgr               = chip::Platform::New<Messaging::ExchangeManager>();
    stateParams.messageCounterManager     = chip::Platform::New<secure_channel::MessageCounterManager>();
    stateParams.groupDataProvider         = params.groupDataProvider;

    // if no fabricTable was provided, create one and track it in stateParams for cleanup
    stateParams.fabricTable = params.fabricTable;

    FabricTable * tempFabricTable = nullptr;
    if (stateParams.fabricTable == nullptr)
    {
        // TODO(#16231): Previously (and still) the objects new-ed in this entire method seem expected to last forever...
        auto newFabricTable = Platform::MakeUnique<FabricTable>();
        ReturnErrorCodeIf(!newFabricTable, CHIP_ERROR_NO_MEMORY);

        FabricTable::InitParams fabricTableInitParams;
        fabricTableInitParams.storage             = params.fabricIndependentStorage;
        fabricTableInitParams.operationalKeystore = params.operationalKeystore;
        fabricTableInitParams.opCertStore         = params.opCertStore;
        ReturnErrorOnFailure(newFabricTable->Init(fabricTableInitParams));
        stateParams.fabricTable = newFabricTable.release();
        tempFabricTable         = stateParams.fabricTable;
    }

    auto sessionResumptionStorage = chip::Platform::MakeUnique<SimpleSessionResumptionStorage>();
    ReturnErrorOnFailure(sessionResumptionStorage->Init(params.fabricIndependentStorage));
    stateParams.sessionResumptionStorage = std::move(sessionResumptionStorage);

    auto delegate = chip::Platform::MakeUnique<ControllerFabricDelegate>();
    ReturnErrorOnFailure(delegate->Init(stateParams.sessionResumptionStorage.get(), stateParams.groupDataProvider));
    stateParams.fabricTableDelegate = delegate.get();
    ReturnErrorOnFailure(stateParams.fabricTable->AddFabricDelegate(stateParams.fabricTableDelegate));
    delegate.release();

    ReturnErrorOnFailure(stateParams.sessionMgr->Init(stateParams.systemLayer, stateParams.transportMgr,
                                                      stateParams.messageCounterManager, params.fabricIndependentStorage,
                                                      stateParams.fabricTable));
    ReturnErrorOnFailure(stateParams.exchangeMgr->Init(stateParams.sessionMgr));
    ReturnErrorOnFailure(stateParams.messageCounterManager->Init(stateParams.exchangeMgr));
    ReturnErrorOnFailure(stateParams.unsolicitedStatusHandler->Init(stateParams.exchangeMgr));

    InitDataModelHandler(stateParams.exchangeMgr);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->Init(stateParams.exchangeMgr, stateParams.fabricTable));

    ReturnErrorOnFailure(Dnssd::Resolver::Instance().Init(stateParams.udpEndPointManager));

    if (params.enableServerInteractions)
    {
        stateParams.caseServer = chip::Platform::New<CASEServer>();

        // Enable listening for session establishment messages.
        ReturnErrorOnFailure(stateParams.caseServer->ListenForSessionEstablishment(
            stateParams.exchangeMgr, stateParams.sessionMgr, stateParams.fabricTable, stateParams.sessionResumptionStorage.get(),
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

        //
        // Start up the DNS-SD server.  We are not giving it a
        // CommissioningModeProvider, so it will not claim we are in
        // commissioning mode.
        //
        chip::app::DnssdServer::Instance().StartServer();
    }

    stateParams.operationalDevicePool = Platform::New<DeviceControllerSystemStateParams::OperationalDevicePool>();
    stateParams.caseClientPool        = Platform::New<DeviceControllerSystemStateParams::CASEClientPool>();

    DeviceProxyInitParams deviceInitParams = {
        .sessionManager           = stateParams.sessionMgr,
        .sessionResumptionStorage = stateParams.sessionResumptionStorage.get(),
        .exchangeMgr              = stateParams.exchangeMgr,
        .fabricTable              = stateParams.fabricTable,
        .clientPool               = stateParams.caseClientPool,
        .groupDataProvider        = stateParams.groupDataProvider,
        .mrpLocalConfig           = GetLocalMRPConfig(),
    };

    CASESessionManagerConfig sessionManagerConfig = {
        .sessionInitParams = deviceInitParams,
        .devicePool        = stateParams.operationalDevicePool,
    };

    // TODO: Need to be able to create a CASESessionManagerConfig here!
    stateParams.caseSessionManager = Platform::New<CASESessionManager>();
    ReturnErrorOnFailure(stateParams.caseSessionManager->Init(stateParams.systemLayer, sessionManagerConfig));

    // store the system state
    mSystemState = chip::Platform::New<DeviceControllerSystemState>(std::move(stateParams));
    mSystemState->SetTempFabricTable(tempFabricTable);
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

    controllerParams.systemState        = mSystemState;
    controllerParams.controllerVendorId = params.controllerVendorId;

    controllerParams.enableServerInteractions = params.enableServerInteractions;
}

CHIP_ERROR DeviceControllerFactory::SetupController(SetupParams params, DeviceController & controller)
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(params.controllerVendorId != VendorId::Unspecified, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(InitSystemState());

    ControllerInitParams controllerParams;
    PopulateInitParams(controllerParams, params);

    CHIP_ERROR err = controller.Init(controllerParams);
    return err;
}

CHIP_ERROR DeviceControllerFactory::SetupCommissioner(SetupParams params, DeviceCommissioner & commissioner)
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(params.controllerVendorId != VendorId::Unspecified, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(InitSystemState());

    CommissionerInitParams commissionerParams;

    // PopulateInitParams works against ControllerInitParams base class of CommissionerInitParams only
    PopulateInitParams(commissionerParams, params);

    // Set commissioner-specific fields not in ControllerInitParams
    commissionerParams.pairingDelegate           = params.pairingDelegate;
    commissionerParams.defaultCommissioner       = params.defaultCommissioner;
    commissionerParams.deviceAttestationVerifier = params.deviceAttestationVerifier;

    CHIP_ERROR err = commissioner.Init(commissionerParams);
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

DeviceControllerFactory::~DeviceControllerFactory()
{
    Shutdown();
}

void DeviceControllerFactory::Shutdown()
{
    if (mSystemState != nullptr)
    {
        Platform::Delete(mSystemState);
        mSystemState = nullptr;
    }
    mFabricIndependentStorage = nullptr;
    mOperationalKeystore      = nullptr;
    mOpCertStore              = nullptr;
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

    if (mFabricTableDelegate != nullptr)
    {
        if (mFabrics != nullptr)
        {
            mFabrics->RemoveFabricDelegate(mFabricTableDelegate);
        }

        chip::Platform::Delete(mFabricTableDelegate);
        mFabricTableDelegate = nullptr;
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

    // mCASEClientPool and mDevicePool must be deallocated
    // after mCASESessionManager, which uses them.

    if (mOperationalDevicePool != nullptr)
    {
        Platform::Delete(mOperationalDevicePool);
        mOperationalDevicePool = nullptr;
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

    if (mTempFabricTable != nullptr)
    {
        chip::Platform::Delete(mTempFabricTable);
        mTempFabricTable = nullptr;
        // if we created a temp fabric table, then mFabrics points to it.
        // if we did not create a temp fabric table, then keep the reference
        // so that SetupController/Commissioner can use it
        mFabrics = nullptr;
    }
}

} // namespace Controller
} // namespace chip
