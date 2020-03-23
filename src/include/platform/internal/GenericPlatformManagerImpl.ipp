/*
 *
 *    <COPYRIGHT>
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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.h>
#include <platform/internal/EventLogging.h>
#include <platform/internal/BLEManager.h>
#include <new>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl<PlatformManagerImpl>;

extern CHIP_ERROR InitEntropy();

template<class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err;

    mMsgLayerWasActive = false;

    // Arrange for Device Layer errors to be translated to text.
    RegisterDeviceLayerErrorFormatter();

    // Initialize the source used by CHIP to get secure random data.
    err = InitEntropy();
    SuccessOrExit(err);

    // Initialize the Configuration Manager object.
    err = ConfigurationMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Configuration Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP system layer.
    new (&SystemLayer) System::Layer();
    err = SystemLayer.Init(NULL);
    if (err != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    new (&InetLayer) Inet::InetLayer();
    err = InetLayer.Init(SystemLayer, NULL);
    if (err != INET_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    {
        ChipMessageLayer::InitContext initContext;
        initContext.systemLayer = &SystemLayer;
        initContext.inet = &InetLayer;
        initContext.listenTCP = true;
        initContext.listenUDP = true;
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        initContext.ble = BLEMgr().GetBleLayer();
        initContext.listenBLE = true;
#endif
    }

    // Perform dynamic configuration of the core CHIP objects based on stored settings.
    //
    // NB: In general, initialization of Device Layer objects should happen *after* this call
    // as their initialization methods may rely on the proper initialization of the core
    // objects.
    //
    err = ConfigurationMgr().ConfigureChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ConfigureChipStack failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the BLE manager.
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    err = BLEMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);
#endif

    // Initialize the Connectivity Manager object.
    err = ConnectivityMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Connectivity Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize CHIP Event Logging.
    err = InitChipEventLogging();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Event Logging initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Time Sync Manager object.
    err = TimeSyncMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Time Sync Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Software Update Manager object.
#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
    err = SoftwareUpdateMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Software Update Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);
#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

exit:
    return err;
}

template<class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
    VerifyOrExit(eventHandler != NULL, err = CHIP_ERROR_NO_MEMORY);

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
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kCallWorkFunct;
    event.CallWorkFunct.WorkFunct = workFunct;
    event.CallWorkFunct.Arg = arg;

    Impl()->PostEvent(&event);
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_DispatchEvent(const ChipDeviceEvent * event)
{
#if CHIP_PROGRESS_LOGGING
    uint64_t startUS = System::Layer::GetClock_MonotonicHiRes();
#endif // CHIP_PROGRESS_LOGGING

    switch (event->Type)
    {
    case DeviceEventType::kNoOp:
        // Do nothing for no-op events.
        break;

    case DeviceEventType::kChipSystemLayerEvent:
        // If the event is a CHIP System or Inet Layer event, deliver it to the SystemLayer event handler.
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
#if CHIP_PROGRESS_LOGGING
    uint32_t delta = ((uint32_t)(System::Layer::GetClock_MonotonicHiRes() - startUS)) / 1000;
    if (delta > 100)
    {
        ChipLogError(DeviceLayer, "Long dispatch time: %" PRId32 " ms", delta);
    }
#endif // CHIP_PROGRESS_LOGGING
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToSystemLayer(const ChipDeviceEvent * event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Invoke the System Layer's event handler function.
    err = SystemLayer.HandleEvent(*event->ChipSystemLayerEvent.Target,
                                  event->ChipSystemLayerEvent.Type,
                                  event->ChipSystemLayerEvent.Argument);
    if (err != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error handling CHIP System Layer event (type %d): %s",
                event->Type, ErrorStr(err));
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToDeviceLayer(const ChipDeviceEvent * event)
{
    // Dispatch the event to all the components in the Device Layer.
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    BLEMgr().OnPlatformEvent(event);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().OnPlatformEvent(event);
#endif
    ConnectivityMgr().OnPlatformEvent(event);
    TimeSyncMgr().OnPlatformEvent(event);
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToApplication(const ChipDeviceEvent * event)
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
void GenericPlatformManagerImpl<ImplClass>::HandleSessionEstablished(ChipSecurityManager * sm, ChipConnection * con, void * reqState, uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType)
{
    // Get the auth mode for the newly established session key.
    ChipSessionKey * sessionKey;
    FabricState.GetSessionKey(sessionKeyId, peerNodeId, sessionKey);
    ChipAuthMode authMode = (sessionKey != NULL) ? sessionKey->AuthMode : (ChipAuthMode)kChipAuthMode_NotSpecified;

    // Post a SessionEstablished event for the new session.  If a PASE session is established
    // using the device's pairing code, presume that this is a commissioner and set the
    // IsCommissioner flag as a convenience to the application.
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kSessionEstablished;
    event.SessionEstablished.PeerNodeId = peerNodeId;
    event.SessionEstablished.SessionKeyId = sessionKeyId;
    event.SessionEstablished.EncType = encType;
    event.SessionEstablished.AuthMode = authMode;
    event.SessionEstablished.IsCommissioner = (authMode == kChipAuthMode_PASE_PairingCode);
    PlatformMgr().PostEvent(&event);

    if (event.SessionEstablished.IsCommissioner)
    {
        ChipLogProgress(DeviceLayer, "Commissioner session established");
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::HandleMessageLayerActivityChanged(bool messageLayerIsActive)
{
    GenericPlatformManagerImpl<ImplClass> & self = PlatformMgrImpl();

    if (messageLayerIsActive != self.mMsgLayerWasActive)
    {
        self.mMsgLayerWasActive = messageLayerIsActive;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ThreadStackMgr().OnMessageLayerActivityChanged(messageLayerIsActive);
#endif
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_IPP

