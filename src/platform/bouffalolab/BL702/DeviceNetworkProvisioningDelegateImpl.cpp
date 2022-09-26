/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "DeviceNetworkProvisioningDelegateImpl.h"

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "virt_net_spi.h"
extern virt_net_t vnet_spi;

#include <bl702Config.h>
using namespace ::chip::DeviceLayer::Internal;
#endif

namespace chip {
namespace DeviceLayer {

CHIP_ERROR
DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(ByteSpan threadData)
{
#if CHIP_ENABLE_OPENTHREAD
    CHIP_ERROR error = CHIP_NO_ERROR;

    SuccessOrExit(error = ThreadStackMgr().SetThreadEnabled(false));
    SuccessOrExit(error = ThreadStackMgr().SetThreadProvision(threadData));
    SuccessOrExit(error = ThreadStackMgr().SetThreadEnabled(true));
exit:
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_OPENTHREAD
}

CHIP_ERROR
DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * passwd)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    CHIP_ERROR err = CHIP_NO_ERROR;

    BL702Config::WriteWiFiInfo(ssid, passwd);

    virt_net_connect_ap(vnet_spi, ssid, passwd);

    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_WIFI
}

} // namespace DeviceLayer
} // namespace chip
