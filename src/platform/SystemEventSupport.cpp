/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides implementations of the CHIP System Layer platform
 *          event functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

using namespace ::chip::DeviceLayer;

System::Error PostEvent(System::Layer & aLayer, void * aContext, System::Object & aTarget, System::EventType aType,
                        uintptr_t aArgument)
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kChipSystemLayerEvent;
    event.ChipSystemLayerEvent.Type     = aType;
    event.ChipSystemLayerEvent.Target   = &aTarget;
    event.ChipSystemLayerEvent.Argument = aArgument;

    PlatformMgr().PostEvent(&event);

    return CHIP_SYSTEM_NO_ERROR;
}

System::Error DispatchEvents(Layer & aLayer, void * aContext)
{
    PlatformMgr().RunEventLoop();

    return CHIP_SYSTEM_NO_ERROR;
}

System::Error DispatchEvent(System::Layer & aLayer, void * aContext, const ChipDeviceEvent * aEvent)
{
    PlatformMgr().DispatchEvent(aEvent);

    return CHIP_SYSTEM_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
