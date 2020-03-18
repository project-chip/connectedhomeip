/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ConnectivityManager.h>
#include <Weave/DeviceLayer/internal/NetworkProvisioningServer.h>
#include <Weave/DeviceLayer/internal/DeviceNetworkInfo.h>
#include <Weave/DeviceLayer/internal/ServiceTunnelAgent.h>
#include <Weave/DeviceLayer/internal/BLEManager.h>
#include <Weave/Profiles/WeaveProfiles.h>
#include <Weave/Profiles/common/CommonProfile.h>
#include <Warm/Warm.h>

#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/nd6.h>
#include <lwip/dns.h>

#include <new>


#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif


using namespace ::nl;
using namespace ::nl::Weave;
using namespace ::nl::Weave::TLV;
using namespace ::nl::Weave::Profiles::Common;
using namespace ::nl::Weave::Profiles::NetworkProvisioning;
using namespace ::nl::Weave::DeviceLayer::Internal;

using Profiles::kWeaveProfile_Common;
using Profiles::kWeaveProfile_NetworkProvisioning;

namespace nl {
namespace Weave {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;


WEAVE_ERROR ConnectivityManagerImpl::_Init()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Initialize the generic base classes that require it.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

    // Initialize the Weave Addressing and Routing Module.
    err = Warm::Init(FabricState);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const WeaveDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
