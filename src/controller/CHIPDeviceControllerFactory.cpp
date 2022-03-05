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

#include <app/util/DataModelHandler.h>
#include <lib/support/ErrorStr.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif

#include <app/server/Dnssd.h>
#include <protocols/secure_channel/CASEServer.h>

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

    mListenPort               = params.listenPort;
    mFabricStorage            = params.fabricStorage;
    mFabricIndependentStorage = params.fabricIndependentStorage;

    CHIP_ERROR err = InitSystemState(params);

    return err;
}

CHIP_ERROR DeviceControllerFactory::InitSystemState()
{
    FactoryInitParams params;
    if (mSystemState != nullptr)
    {
        params.systemLayer        = mSystemState->SystemLayer();
        params.tcpEndPointManager = mSystemState->TCPEndPointManager();
        params.udpEndPointManager = mSystemState->UDPEndPointManager();
#if CONFIG_NETWORK_LAYER_BLE
        params.bleLayer = mSystemState->BleLayer();
#endif
    }

    params.fabricIndependentStorage = mFabricIndependentStorage;

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
        mSystemState->Release();
        chip::Platform::Delete(mSystemState);
        mSystemState = nullptr;
    }

    DeviceControllerSystemStateParams stateParams;
#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());

    stateParams.systemLayer        = &DeviceLayer::SystemLayer();
    stateParams.tcpEndPointManager = DeviceLayer::TCPEndPointManager();
    stateParams.udpEndPointManager = DeviceLayer::UDPEndPointManager();
#else
    stateParams.systemLayer        = params.systemLayer;
    stateParams.tcpEndPointManager = params.tcpEndPointManager;
    stateParams.udpEndPointManager = params.udpEndPointManager;
    ChipLogError(Controller, "Warning: Device Controller Factory should be with a CHIP Device Layer...");
#endif // CONFIG_DEVICE_LAYER

    VerifyOrReturnError(stateParams.systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(stateParams.udpEndPointManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

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
                                                            .SetListenPort(mListenPort)
#if INET_CONFIG_ENABLE_IPV4
                                                            ,
                                                        Transport::UdpListenParameters(stateParams.udpEndPointManager)
                                                            .SetAddressType(Inet::IPAddressType::kIPv4)
                                                            .SetListenPort(mListenPort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                                            ,
                                                        Transport::BleListenParameters(stateParams.bleLayer)
#endif
                                                            ));

    stateParams.fabricTable           = chip::Platform::New<FabricTable>();
    stateParams.sessionMgr            = chip::Platform::New<SessionManager>();
    stateParams.exchangeMgr           = chip::Platform::New<Messaging::ExchangeManager>();
    stateParams.messageCounterManager = chip::Platform::New<secure_channel::MessageCounterManager>();

    ReturnErrorOnFailure(stateParams.fabricTable->Init(mFabricStorage));

    ReturnErrorOnFailure(stateParams.sessionMgr->Init(stateParams.systemLayer, stateParams.transportMgr,
                                                      stateParams.messageCounterManager, params.fabricIndependentStorage,
                                                      stateParams.fabricTable));
    ReturnErrorOnFailure(stateParams.exchangeMgr->Init(stateParams.sessionMgr));
    ReturnErrorOnFailure(stateParams.messageCounterManager->Init(stateParams.exchangeMgr));

    InitDataModelHandler(stateParams.exchangeMgr);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->Init(stateParams.exchangeMgr));

    ReturnErrorOnFailure(Dnssd::Resolver::Instance().Init(stateParams.udpEndPointManager));

    if (params.enableServerInteractions)
    {
        stateParams.caseServer = chip::Platform::New<CASEServer>();

        //
        // Enable listening for session establishment messages.
        //
        // We pass in a nullptr for the BLELayer since we're not permitting usage of BLE in this server modality for the controller,
        // especially since it will interrupt other potential usages of BLE by the controller acting in a commissioning capacity.
        //
        ReturnErrorOnFailure(stateParams.caseServer->ListenForSessionEstablishment(
            stateParams.exchangeMgr, stateParams.transportMgr, nullptr, stateParams.sessionMgr, stateParams.fabricTable));

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

    // store the system state
    mSystemState = chip::Platform::New<DeviceControllerSystemState>(stateParams);
    ChipLogDetail(Controller, "System State Initialized...");
    return CHIP_NO_ERROR;
}

void DeviceControllerFactory::PopulateInitParams(ControllerInitParams & controllerParams, const SetupParams & params)
{
    controllerParams.operationalCredentialsDelegate = params.operationalCredentialsDelegate;
    controllerParams.operationalKeypair             = params.operationalKeypair;
    controllerParams.controllerNOC                  = params.controllerNOC;
    controllerParams.controllerICAC                 = params.controllerICAC;
    controllerParams.controllerRCAC                 = params.controllerRCAC;
    controllerParams.storageDelegate                = params.storageDelegate;

    controllerParams.systemState        = mSystemState;
    controllerParams.controllerVendorId = params.controllerVendorId;

    controllerParams.enableServerInteractions = params.enableServerInteractions;
}

CHIP_ERROR DeviceControllerFactory::SetupController(SetupParams params, DeviceController & controller)
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(InitSystemState());

    ControllerInitParams controllerParams;
    PopulateInitParams(controllerParams, params);

    CHIP_ERROR err = controller.Init(controllerParams);
    return err;
}

CHIP_ERROR DeviceControllerFactory::SetupCommissioner(SetupParams params, DeviceCommissioner & commissioner)
{
    VerifyOrReturnError(mSystemState != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(InitSystemState());

    CommissionerInitParams commissionerParams;
    PopulateInitParams(commissionerParams, params);
    commissionerParams.pairingDelegate     = params.pairingDelegate;
    commissionerParams.defaultCommissioner = params.defaultCommissioner;

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
        mSystemState->Release();
        chip::Platform::Delete(mSystemState);
        mSystemState = nullptr;
    }
    mFabricStorage            = nullptr;
    mFabricIndependentStorage = nullptr;
}

CHIP_ERROR DeviceControllerSystemState::Shutdown()
{
    VerifyOrReturnError(mRefCount == 1, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the System State, this will teardown the CHIP Stack");

    if (mCASEServer != nullptr)
    {
        chip::Platform::Delete(mCASEServer);
        mCASEServer = nullptr;
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
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().Shutdown());
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

    if (mSessionMgr != nullptr)
    {
        chip::Platform::Delete(mSessionMgr);
        mSessionMgr = nullptr;
    }

    if (mFabrics != nullptr)
    {
        chip::Platform::Delete(mFabrics);
        mFabrics = nullptr;
    }

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
