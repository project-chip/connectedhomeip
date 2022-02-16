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
                                                      stateParams.messageCounterManager, params.fabricIndependentStorage));
    ReturnErrorOnFailure(stateParams.exchangeMgr->Init(stateParams.sessionMgr));
    ReturnErrorOnFailure(stateParams.messageCounterManager->Init(stateParams.exchangeMgr));

    InitDataModelHandler(stateParams.exchangeMgr);

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->Init(stateParams.exchangeMgr));

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    ReturnErrorOnFailure(Dnssd::Resolver::Instance().Init(stateParams.udpEndPointManager));
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    // store the system state
    mSystemState = chip::Platform::New<DeviceControllerSystemState>(stateParams);
    ChipLogDetail(Controller, "System State Initialized...");
    return CHIP_NO_ERROR;
}

void DeviceControllerFactory::PopulateInitParams(ControllerInitParams & controllerParams, const SetupParams & params)
{
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    controllerParams.deviceAddressUpdateDelegate = params.deviceAddressUpdateDelegate;
#endif
    controllerParams.operationalCredentialsDelegate = params.operationalCredentialsDelegate;
    controllerParams.operationalKeypair             = params.operationalKeypair;
    controllerParams.controllerNOC                  = params.controllerNOC;
    controllerParams.controllerICAC                 = params.controllerICAC;
    controllerParams.controllerRCAC                 = params.controllerRCAC;
    controllerParams.storageDelegate                = params.storageDelegate;

    controllerParams.systemState        = mSystemState;
    controllerParams.controllerVendorId = params.controllerVendorId;
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

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    Dnssd::Resolver::Instance().Shutdown();
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

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
