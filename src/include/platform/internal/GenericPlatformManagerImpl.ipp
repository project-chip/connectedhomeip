/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl<> template.
 */


#ifndef GENERIC_PLATFORM_MANAGER_IMPL_IPP
#define GENERIC_PLATFORM_MANAGER_IMPL_IPP

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/PlatformManager.h>
#include <Weave/DeviceLayer/internal/GenericPlatformManagerImpl.h>
#include <Weave/DeviceLayer/internal/DeviceControlServer.h>
#include <Weave/DeviceLayer/internal/DeviceDescriptionServer.h>
#include <Weave/DeviceLayer/internal/NetworkProvisioningServer.h>
#include <Weave/DeviceLayer/internal/FabricProvisioningServer.h>
#include <Weave/DeviceLayer/internal/ServiceProvisioningServer.h>
#include <Weave/DeviceLayer/internal/ServiceDirectoryManager.h>
#include <Weave/DeviceLayer/internal/EchoServer.h>
#include <Weave/DeviceLayer/internal/EventLogging.h>
#include <Weave/DeviceLayer/internal/BLEManager.h>
#include <new>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl<PlatformManagerImpl>;

extern WEAVE_ERROR InitCASEAuthDelegate();
extern WEAVE_ERROR InitEntropy();

template<class ImplClass>
WEAVE_ERROR GenericPlatformManagerImpl<ImplClass>::_InitWeaveStack(void)
{
    WEAVE_ERROR err;

    mMsgLayerWasActive = false;

    // Arrange for Device Layer errors to be translated to text.
    RegisterDeviceLayerErrorFormatter();

    // Initialize the source used by Weave to get secure random data.
    err = InitEntropy();
    SuccessOrExit(err);

    // Initialize the Configuration Manager object.
    err = ConfigurationMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Configuration Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Weave system layer.
    new (&SystemLayer) System::Layer();
    err = SystemLayer.Init(NULL);
    if (err != WEAVE_SYSTEM_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Weave Inet layer.
    new (&InetLayer) Inet::InetLayer();
    err = InetLayer.Init(SystemLayer, NULL);
    if (err != INET_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Weave fabric state object.
    new (&FabricState) WeaveFabricState();
    err = FabricState.Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "FabricState initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    FabricState.DefaultSubnet = kWeaveSubnetId_PrimaryWiFi;

#if WEAVE_CONFIG_SECURITY_TEST_MODE
    FabricState.LogKeys = true;
#endif

    {
        WeaveMessageLayer::InitContext initContext;
        initContext.systemLayer = &SystemLayer;
        initContext.inet = &InetLayer;
        initContext.listenTCP = true;
        initContext.listenUDP = true;
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
        initContext.ble = BLEMgr().GetBleLayer();
        initContext.listenBLE = true;
#endif
        initContext.fabricState = &FabricState;

        // Initialize the Weave message layer.
        new (&MessageLayer) WeaveMessageLayer();
        err = MessageLayer.Init(&initContext);
        if (err != WEAVE_NO_ERROR) {
            WeaveLogError(DeviceLayer, "MessageLayer initialization failed: %s", ErrorStr(err));
        }
        SuccessOrExit(err);
    }

    // Hook the MessageLayer activity changed callback.
    MessageLayer.SetSignalMessageLayerActivityChanged(ImplClass::HandleMessageLayerActivityChanged);

    // Initialize the Weave exchange manager.
    err = ExchangeMgr.Init(&MessageLayer);
    if (err != WEAVE_NO_ERROR) {
        WeaveLogError(DeviceLayer, "ExchangeMgr initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Weave security manager.
    new (&SecurityMgr) WeaveSecurityManager();
    err = SecurityMgr.Init(ExchangeMgr, SystemLayer);
    if (err != WEAVE_NO_ERROR) {
        WeaveLogError(DeviceLayer, "SecurityMgr initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Arrange for the security manager to call a handler function whenever
    // a new secure session is established.
    SecurityMgr.OnSessionEstablished = ImplClass::HandleSessionEstablished;

    // Initialize the CASE auth delegate object.
    err = InitCASEAuthDelegate();
    SuccessOrExit(err);

#if WEAVE_CONFIG_SECURITY_TEST_MODE
    SecurityMgr.CASEUseKnownECDHKey = true;
#endif

    // Perform dynamic configuration of the core Weave objects based on stored settings.
    //
    // NB: In general, initialization of Device Layer objects should happen *after* this call
    // as their initialization methods may rely on the proper initialization of the core
    // objects.
    //
    err = ConfigurationMgr().ConfigureWeaveStack();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "ConfigureWeaveStack failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

#if WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    // Initialize the service directory manager.
    err = InitServiceDirectoryManager();
    SuccessOrExit(err);
#endif

    // Initialize the BLE manager.
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
    err = BLEMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "BLEManager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);
#endif

    // Initialize the Connectivity Manager object.
    err = ConnectivityMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Connectivity Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Device Control server.
    err = DeviceControlSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Device Control server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Device Description server.
    err = DeviceDescriptionSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Device Description server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Network Provisioning server.
    err = NetworkProvisioningSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Network Provisioning server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Fabric Provisioning server.
    err = FabricProvisioningSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Fabric Provisioning server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Service Provisioning server.
    err = ServiceProvisioningSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Service Provisioning server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Echo server.
    err = EchoSvr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Weave Echo server initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize Weave Event Logging.
    err = InitWeaveEventLogging();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Event Logging initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Trait Manager object.
#if WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
    err = TraitMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Trait Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);
#endif // WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER

    // Initialize the Time Sync Manager object.
    err = TimeSyncMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Time Sync Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Software Update Manager object.
#if WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
    err = SoftwareUpdateMgr().Init();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Software Update Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);
#endif // WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

exit:
    return err;
}

template<class ImplClass>
WEAVE_ERROR GenericPlatformManagerImpl<ImplClass>::_AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    AppEventHandler * eventHandler;

    // Do nothing if the event handler is already registered.
    for (eventHandler = mAppEventHandlerList; eventHandler != NULL; eventHandler = eventHandler->Next)
    {
        if (eventHandler->Handler == handler && eventHandler->Arg == arg)
        {
            ExitNow();
        }
    }

    eventHandler = (AppEventHandler *)malloc(sizeof(AppEventHandler));
    VerifyOrExit(eventHandler != NULL, err = WEAVE_ERROR_NO_MEMORY);

    eventHandler->Next = mAppEventHandlerList;
    eventHandler->Handler = handler;
    eventHandler->Arg = arg;

    mAppEventHandlerList = eventHandler;

exit:
    return err;
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_RemoveEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
{
    AppEventHandler ** eventHandlerIndirectPtr;

    for (eventHandlerIndirectPtr = &mAppEventHandlerList; *eventHandlerIndirectPtr != NULL; )
    {
        AppEventHandler * eventHandler = (*eventHandlerIndirectPtr);

        if (eventHandler->Handler == handler && eventHandler->Arg == arg)
        {
            *eventHandlerIndirectPtr = eventHandler->Next;
            free(eventHandler);
        }
        else
        {
            eventHandlerIndirectPtr = &eventHandler->Next;
        }
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    WeaveDeviceEvent event;
    event.Type = DeviceEventType::kCallWorkFunct;
    event.CallWorkFunct.WorkFunct = workFunct;
    event.CallWorkFunct.Arg = arg;

    Impl()->PostEvent(&event);
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_DispatchEvent(const WeaveDeviceEvent * event)
{
#if WEAVE_PROGRESS_LOGGING
    uint64_t startUS = System::Layer::GetClock_MonotonicHiRes();
#endif // WEAVE_PROGRESS_LOGGING

    switch (event->Type)
    {
    case DeviceEventType::kNoOp:
        // Do nothing for no-op events.
        break;

    case DeviceEventType::kWeaveSystemLayerEvent:
        // If the event is a Weave System or Inet Layer event, deliver it to the SystemLayer event handler.
        Impl()->DispatchEventToSystemLayer(event);
        break;

    case DeviceEventType::kCallWorkFunct:
        // If the event is a "call work function" event, call the specified function.
        event->CallWorkFunct.WorkFunct(event->CallWorkFunct.Arg);
        break;

    default:
        // For all other events, deliver the event to each of the components in the Device Layer.
        Impl()->DispatchEventToDeviceLayer(event);

        // If the event is not an internal event, also deliver it to the application's registered
        // event handlers.
        if (!event->IsInternal())
        {
            Impl()->DispatchEventToApplication(event);
        }

        break;
    }

    // TODO: make this configurable
#if WEAVE_PROGRESS_LOGGING
    uint32_t delta = ((uint32_t)(System::Layer::GetClock_MonotonicHiRes() - startUS)) / 1000;
    if (delta > 100)
    {
        WeaveLogError(DeviceLayer, "Long dispatch time: %" PRId32 " ms", delta);
    }
#endif // WEAVE_PROGRESS_LOGGING
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToSystemLayer(const WeaveDeviceEvent * event)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Invoke the System Layer's event handler function.
    err = SystemLayer.HandleEvent(*event->WeaveSystemLayerEvent.Target,
                                  event->WeaveSystemLayerEvent.Type,
                                  event->WeaveSystemLayerEvent.Argument);
    if (err != WEAVE_SYSTEM_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Error handling Weave System Layer event (type %d): %s",
                event->Type, nl::ErrorStr(err));
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToDeviceLayer(const WeaveDeviceEvent * event)
{
    // Dispatch the event to all the components in the Device Layer.
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
    BLEMgr().OnPlatformEvent(event);
#endif
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().OnPlatformEvent(event);
#endif
    ConnectivityMgr().OnPlatformEvent(event);
    DeviceControlSvr().OnPlatformEvent(event);
    DeviceDescriptionSvr().OnPlatformEvent(event);
    NetworkProvisioningSvr().OnPlatformEvent(event);
    FabricProvisioningSvr().OnPlatformEvent(event);
    ServiceProvisioningSvr().OnPlatformEvent(event);
#if WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
    TraitMgr().OnPlatformEvent(event);
#endif // WEAVE_DEVICE_CONFIG_ENABLE_TRAIT_MANAGER
    TimeSyncMgr().OnPlatformEvent(event);
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToApplication(const WeaveDeviceEvent * event)
{
    // Dispatch the event to each of the registered application event handlers.
    for (AppEventHandler * eventHandler = mAppEventHandlerList;
         eventHandler != NULL;
         eventHandler = eventHandler->Next)
    {
        eventHandler->Handler(event, eventHandler->Arg);
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::HandleSessionEstablished(WeaveSecurityManager * sm, WeaveConnection * con, void * reqState, uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType)
{
    // Get the auth mode for the newly established session key.
    WeaveSessionKey * sessionKey;
    FabricState.GetSessionKey(sessionKeyId, peerNodeId, sessionKey);
    WeaveAuthMode authMode = (sessionKey != NULL) ? sessionKey->AuthMode : (WeaveAuthMode)kWeaveAuthMode_NotSpecified;

    // Post a SessionEstablished event for the new session.  If a PASE session is established
    // using the device's pairing code, presume that this is a commissioner and set the
    // IsCommissioner flag as a convenience to the application.
    WeaveDeviceEvent event;
    event.Type = DeviceEventType::kSessionEstablished;
    event.SessionEstablished.PeerNodeId = peerNodeId;
    event.SessionEstablished.SessionKeyId = sessionKeyId;
    event.SessionEstablished.EncType = encType;
    event.SessionEstablished.AuthMode = authMode;
    event.SessionEstablished.IsCommissioner = (authMode == kWeaveAuthMode_PASE_PairingCode);
    PlatformMgr().PostEvent(&event);

    if (event.SessionEstablished.IsCommissioner)
    {
        WeaveLogProgress(DeviceLayer, "Commissioner session established");
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::HandleMessageLayerActivityChanged(bool messageLayerIsActive)
{
    GenericPlatformManagerImpl<ImplClass> & self = PlatformMgrImpl();

    if (messageLayerIsActive != self.mMsgLayerWasActive)
    {
        self.mMsgLayerWasActive = messageLayerIsActive;

#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
        ThreadStackMgr().OnMessageLayerActivityChanged(messageLayerIsActive);
#endif
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_PLATFORM_MANAGER_IMPL_IPP

