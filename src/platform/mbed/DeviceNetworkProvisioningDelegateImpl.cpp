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

#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NetworkProvisioning, "MbedNetworkProvisioningDelegate: SSID: %s", ssid);
    err = ConnectivityMgrImpl().ProvisionWiFiNetwork(ssid, key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
