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

#pragma once

#include <platform/internal/GenericDeviceNetworkProvisioningDelegateImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

template <class ImplClass>
class GenericDeviceNetworkProvisioningDelegateImpl;

} // namespace Internal

class DeviceNetworkProvisioningDelegateImpl final
    : public Internal::GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>
{
private:
    friend class GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>;

    CHIP_ERROR _ProvisionWiFiNetwork(const char * ssid, const char * passwd);
    CHIP_ERROR _ProvisionThreadNetwork(ByteSpan threadData) { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

} // namespace DeviceLayer
} // namespace chip
