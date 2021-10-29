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

#include <inttypes.h>
#include <new>
#include <platform/GenericPlatformManagerImpl.h>
#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/EventLogging.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

extern CHIP_ERROR InitEntropy();

CHIP_ERROR GenericPlatformManagerImpl::InitChipStackInner()
{
    CHIP_ERROR err;

    // Arrange for CHIP core errors to be translated to text
    RegisterCHIPLayerErrorFormatter();

    // Arrange for Device Layer errors to be translated to text.
    RegisterDeviceLayerErrorFormatter();

    err = InitEntropy();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Entropy initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP system layer.
    err = SystemLayer().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the Configuration Manager.
    err = ConfigurationMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Configuration Manager initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = InetLayer().Init(SystemLayer(), nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // TODO Perform dynamic configuration of the core CHIP objects based on stored settings.

    // Initialize the CHIP BLE manager.
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

    // Initialize the NFC Manager.
#if CHIP_DEVICE_CONFIG_ENABLE_NFC
    err = NFCMgr().Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "NFC Manager initialization failed: %s", ErrorStr(err)));
#endif

    // TODO Initialize CHIP Event Logging.

    // TODO Initialize the Time Sync Manager object.

    SuccessOrExit(err);

    // TODO Initialize the Software Update Manager object.

exit:
    return err;
}

CHIP_ERROR GenericPlatformManagerImpl::ShutdownInner()
{
    CHIP_ERROR err;
    ChipLogError(DeviceLayer, "Inet Layer shutdown");
    err = InetLayer().Shutdown();

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    ChipLogError(DeviceLayer, "BLE shutdown");
    err = BLEMgr().Shutdown();
#endif

    ChipLogError(DeviceLayer, "System Layer shutdown");
    err = SystemLayer().Shutdown();

    return err;
}

CHIP_ERROR GenericPlatformManagerImpl::AddEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
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

void GenericPlatformManagerImpl::RemoveEventHandler(PlatformManager::EventHandlerFunct handler, intptr_t arg)
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

void GenericPlatformManagerImpl::ScheduleWork(AsyncWorkFunct workFunct, intptr_t arg)
{
    ChipDeviceEvent event;
    event.Type                    = DeviceEventType::kCallWorkFunct;
    event.CallWorkFunct.WorkFunct = workFunct;
    event.CallWorkFunct.Arg       = arg;

    CHIP_ERROR status = PostEvent(&event);
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to schedule work: %" CHIP_ERROR_FORMAT, status.Format());
    }
}

void GenericPlatformManagerImpl::DispatchEvent(const ChipDeviceEvent * event)
{
#if CHIP_PROGRESS_LOGGING
    System::Clock::Timestamp start = System::SystemClock().GetMonotonicTimestamp();
#endif // CHIP_PROGRESS_LOGGING

    switch (event->Type)
    {
    case DeviceEventType::kNoOp:
        // Do nothing for no-op events.
        break;

    case DeviceEventType::kChipSystemLayerEvent:
        // If the event is a CHIP System or Inet Layer event, deliver it to the System::Layer event handler.
        DispatchEventToSystemLayer(event);
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
        DispatchEventToDeviceLayer(event);

        // If the event is not an internal event, also deliver it to the application's registered
        // event handlers.
        if (!event->IsInternal())
        {
            DispatchEventToApplication(event);
        }

        break;
    }

    // TODO: make this configurable
#if CHIP_PROGRESS_LOGGING
    uint32_t deltaMs = System::Clock::Milliseconds32(System::SystemClock().GetMonotonicTimestamp() - start).count();
    if (deltaMs > 100)
    {
        ChipLogError(DeviceLayer, "Long dispatch time: %" PRIu32 " ms, for event type %d", deltaMs, event->Type);
    }
#endif // CHIP_PROGRESS_LOGGING
}

void GenericPlatformManagerImpl::DispatchEventToSystemLayer(const ChipDeviceEvent * event)
{
    // TODO(#788): remove ifdef LWIP once System::Layer event APIs are generally available
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Invoke the System Layer's event handler function.
    err = static_cast<System::LayerImplLwIP &>(SystemLayer())
              .HandleEvent(*event->ChipSystemLayerEvent.Target, event->ChipSystemLayerEvent.Type,
                           event->ChipSystemLayerEvent.Argument);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error handling CHIP System Layer event (type %d): %s", event->Type, ErrorStr(err));
    }
#endif
}

void GenericPlatformManagerImpl::DispatchEventToDeviceLayer(const ChipDeviceEvent * event)
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

void GenericPlatformManagerImpl::DispatchEventToApplication(const ChipDeviceEvent * event)
{
    // Dispatch the event to each of the registered application event handlers.
    for (AppEventHandler * eventHandler = mAppEventHandlerList; eventHandler != nullptr; eventHandler = eventHandler->Next)
    {
        eventHandler->Handler(event, eventHandler->Arg);
    }
}

CHIP_ERROR GenericPlatformManagerImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetRebootCount(uint16_t & rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetUpTime(uint64_t & upTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR GenericPlatformManagerImpl::GetBootReasons(uint8_t & bootReasons)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
