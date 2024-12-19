/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_CPP

#include <inttypes.h>
#include <new>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <platform/internal/BLEManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/EventLogging.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

extern CHIP_ERROR InitEntropy();

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_InitChipStack()
{
    CHIP_ERROR err;

    mMsgLayerWasActive = false;

    // Arrange for CHIP core errors to be translated to text
    RegisterCHIPLayerErrorFormatter();

    // Arrange for Device Layer errors to be translated to text.
    RegisterDeviceLayerErrorFormatter();

    err = InitEntropy();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Entropy initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    // Initialize the CHIP system layer.
    err = SystemLayer().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SystemLayer initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    // Initialize the Configuration Manager.
    err = ConfigurationMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Configuration Manager initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    // Initialize the CHIP UDP layer.
    err = UDPEndPointManager()->Init(SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "UDP initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Initialize the CHIP TCP layer.
    err = TCPEndPointManager()->Init(SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "TCP initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);
#endif

    // TODO Perform dynamic configuration of the core CHIP objects based on stored settings.

    // Initialize the CHIP BLE manager.
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    err = BLEMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManager initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);
#endif

    // Initialize the Connectivity Manager object.
    err = ConnectivityMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Connectivity Manager initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    // Initialize the NFC onboarding payload manager
#if CHIP_DEVICE_CONFIG_ENABLE_NFC_ONBOARDING_PAYLOAD
    err = NFCOnboardingPayloadMgr().Init();
    VerifyOrExit(
        err == CHIP_NO_ERROR,
        ChipLogError(DeviceLayer, "NFC onboarding payload manager initialization failed: %" CHIP_ERROR_FORMAT, err.Format()));
#endif

    // TODO Initialize CHIP Event Logging.

    // TODO Initialize the Time Sync Manager object.

    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_Shutdown()
{
    ChipLogProgress(DeviceLayer, "Inet Layer shutdown");
    UDPEndPointManager()->Shutdown();

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPointManager()->Shutdown();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    ChipLogProgress(DeviceLayer, "BLE Layer shutdown");
    BLEMgr().Shutdown();
#endif

    ChipLogProgress(DeviceLayer, "System Layer shutdown");
    SystemLayer().Shutdown();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AppEventHandler * eventHandler;

    // Do nothing if the event handler is already registered.
    for (eventHandler = mAppEventHandlerList; eventHandler != nullptr; eventHandler = eventHandler->Next)
    {
        if (eventHandler->Handler == handler && eventHandler->Arg == arg)
        {
            ExitNow();
        }
    }

    eventHandler = (AppEventHandler *) chip::Platform::MemoryAlloc(sizeof(AppEventHandler));
    VerifyOrExit(eventHandler != nullptr, err = CHIP_ERROR_NO_MEMORY);

    eventHandler->Next    = mAppEventHandlerList;
    eventHandler->Handler = handler;
    eventHandler->Arg     = arg;

    mAppEventHandlerList = eventHandler;

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_RemoveEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
{
    AppEventHandler ** eventHandlerIndirectPtr;

    for (eventHandlerIndirectPtr = &mAppEventHandlerList; *eventHandlerIndirectPtr != nullptr;)
    {
        AppEventHandler * eventHandler = (*eventHandlerIndirectPtr);

        if (eventHandler->Handler == handler && eventHandler->Arg == arg)
        {
            *eventHandlerIndirectPtr = eventHandler->Next;
            chip::Platform::MemoryFree(eventHandler);
        }
        else
        {
            eventHandlerIndirectPtr = &eventHandler->Next;
        }
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_HandleServerStarted()
{
    PlatformManagerDelegate * platformManagerDelegate = PlatformMgr().GetDelegate();

    if (platformManagerDelegate != nullptr)
    {
        uint32_t softwareVersion;

        if (ConfigurationMgr().GetSoftwareVersion(softwareVersion) == CHIP_NO_ERROR)
            platformManagerDelegate->OnStartUp(softwareVersion);
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_HandleServerShuttingDown()
{
    PlatformManagerDelegate * platformManagerDelegate = PlatformMgr().GetDelegate();

    if (platformManagerDelegate != nullptr)
    {
        platformManagerDelegate->OnShutDown();
    }
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kCallWorkFunct };
    event.CallWorkFunct = { .WorkFunct = workFunct, .Arg = arg };
    CHIP_ERROR err      = Impl()->PostEvent(&event);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_ScheduleBackgroundWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kCallWorkFunct };
    event.CallWorkFunct = { .WorkFunct = workFunct, .Arg = arg };
    CHIP_ERROR err      = Impl()->PostBackgroundEvent(&event);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to schedule background work: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_PostBackgroundEvent(const ChipDeviceEvent * event)
{
    // Impl class must override to implement background event processing
    return Impl()->PostEvent(event);
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_RunBackgroundEventLoop(void)
{
    // Impl class must override to implement background event processing
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_StartBackgroundEventLoopTask(void)
{
    // Impl class must override to implement background event processing
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl<ImplClass>::_StopBackgroundEventLoopTask(void)
{
    // Impl class must override to implement background event processing
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_DispatchEvent(const ChipDeviceEvent * event)
{
#if (CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS != 0)
    System::Clock::Timestamp start = System::SystemClock().GetMonotonicTimestamp();
#endif // CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS != 0

    switch (event->Type)
    {
    case DeviceEventType::kNoOp:
        // Do nothing for no-op events.
        break;

    case DeviceEventType::kChipLambdaEvent:
        event->LambdaEvent();
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

#if (CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS != 0)
    uint32_t deltaMs = System::Clock::Milliseconds32(System::SystemClock().GetMonotonicTimestamp() - start).count();
    if (deltaMs > CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS)
    {
        ChipLogError(DeviceLayer, "Long dispatch time: %" PRIu32 " ms, for event type %d", deltaMs, event->Type);
    }
#endif // CHIP_DISPATCH_EVENT_LONG_DISPATCH_TIME_WARNING_THRESHOLD_MS != 0
}

template <class ImplClass>
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
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToApplication(const ChipDeviceEvent * event)
{
    // Dispatch the event to each of the registered application event handlers.
    for (AppEventHandler * eventHandler = mAppEventHandlerList; eventHandler != nullptr;)
    {
        AppEventHandler * nextEventHandler = eventHandler->Next;
        eventHandler->Handler(event, eventHandler->Arg);
        eventHandler = nextEventHandler;
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::HandleMessageLayerActivityChanged(bool messageLayerIsActive)
{
    GenericPlatformManagerImpl<ImplClass> & self = PlatformMgrImpl();

    if (messageLayerIsActive != self.mMsgLayerWasActive)
    {
        self.mMsgLayerWasActive = messageLayerIsActive;
    }
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_CPP
