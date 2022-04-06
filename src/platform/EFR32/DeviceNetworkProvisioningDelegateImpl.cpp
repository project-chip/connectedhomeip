/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "NetworkCommissioningWiFiDriver.h"
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NetworkProvisioning, "EFR Wifi provision: SSID: %s", ssid);
    err = NetworkCommissioning::SlWiFiDriver::GetInstance().ConnectWiFiNetwork(ssid, strlen(ssid), key, strlen(key));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "ERR:WiFi:Provision network: %s", chip::ErrorStr(err));
    }

    return err;
}
#endif

} // namespace DeviceLayer
} // namespace chip
