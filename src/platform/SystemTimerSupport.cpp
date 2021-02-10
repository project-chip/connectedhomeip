/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides implementations of the CHIP System Layer platform
 *          timer functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

using namespace ::chip::DeviceLayer;

System::Error StartTimer(System::Layer & aLayer, void * aContext, uint32_t aMilliseconds)
{
    return PlatformMgr().StartChipTimer(aMilliseconds);
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
