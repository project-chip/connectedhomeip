/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/ConnectivityUtils.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

NetworkIterator * LinuxEthernetDriver::GetNetworks()
{
    auto ret = new EthernetNetworkIterator();
    ConnectivityUtils::GetEthInterfaceName(SafePointerCast<char *>(ret->interfaceName), sizeof(ret->interfaceName));
    ret->interfaceNameLen = static_cast<uint8_t>(strnlen(SafePointerCast<char *>(ret->interfaceName), sizeof(ret->interfaceName)));
    return ret;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
