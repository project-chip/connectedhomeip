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

#ifndef _DEVICE_NETWORK_PROVISIONING_DELEGATE_H_
#define _DEVICE_NETWORK_PROVISIONING_DELEGATE_H_

#include <core/CHIPError.h>
#include <transport/NetworkProvisioning.h>

using namespace ::chip;

class ESP32NetworkProvisioningDelegate : public DeviceNetworkProvisioningDelegate
{
public:
    /**
     * @brief
     *   Called to provision WiFi credentials in a device
     *
     * @param ssid WiFi SSID
     * @param passwd WiFi password
     */
    void ProvisionNetwork(const char * ssid, const char * passwd) override;
};

#endif // _DEVICE_NETWORK_PROVISIONING_DELEGATE_H_
