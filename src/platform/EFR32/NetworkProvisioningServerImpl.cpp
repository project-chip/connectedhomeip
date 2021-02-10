/* See Project CHIP LICENSE file for licensing information. */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/NetworkProvisioningServer.h>

#include <core/CHIPTLV.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <platform/internal/GenericNetworkProvisioningServerImpl.cpp>
#include <protocols/Protocols.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

NetworkProvisioningServerImpl NetworkProvisioningServerImpl::sInstance;

CHIP_ERROR NetworkProvisioningServerImpl::_Init(void)
{
    return GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>::DoInit();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
