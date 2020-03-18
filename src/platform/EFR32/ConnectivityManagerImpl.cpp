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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/NetworkProvisioningServer.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <platform/internal/ServiceTunnelAgent.h>
#include <platform/internal/BLEManager.h>
#include <platform/Profiles/CHIPProfiles.h>
#include <platform/Profiles/common/CommonProfile.h>
#include <warm/Warm.h>

#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/nd6.h>
#include <lwip/dns.h>

#include <new>

#if CHIP_DEVICE_CONFIG_ENABLE_WOBLE
#include <DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <DeviceLayer/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

using namespace ::chip;
using namespace ::chip::Chip;
using namespace ::chip::TLV;
using namespace ::chip::Profiles::Common;
using namespace ::chip::Profiles::NetworkProvisioning;
using namespace ::chip::DeviceLayer::Internal;

using Profiles::kChipProfile_Common;
using Profiles::kChipProfile_NetworkProvisioning;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

    // Initialize the Chip Addressing and Routing Module.
    err = Warm::Init(FabricState);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent *event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

} // namespace DeviceLayer
} // namespace chip
