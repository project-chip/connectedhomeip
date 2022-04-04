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
#include "NetworkCommissioningDriver.h"

#include <utils_log.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    // BL602Config::WriteWifiInfo(ssid, passwd);

    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(NetworkProvisioning, "BL602NetworkProvisioningDelegate: SSID: %s", ssid);
    err = NetworkCommissioning::BLWiFiDriver::GetInstance().ConnectWiFiNetwork(ssid, strlen(ssid), key, strlen(key));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
