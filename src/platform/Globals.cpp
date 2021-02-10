/* See Project CHIP LICENSE file for licensing information. */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/BLEManager.h>

namespace chip {
namespace DeviceLayer {

chip::System::Layer SystemLayer;
chip::Inet::InetLayer InetLayer;

namespace Internal {

const char * const TAG = "CHIP[DL]";

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
