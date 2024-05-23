/*
 *
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

/**
 *    @file
 *          Provides an implementation of the network commissioning drivers
 *          for Open IOT SDK platform.
 */

#include <lwip/netif.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/openiotsdk/NetworkCommissioningDriver.h>

using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

NetworkIterator * OpenIoTSDKEthernetDriver::GetNetworks()
{
    auto ret = new EthernetNetworkIterator();

    char buf[NETIF_NAMESIZE];
    char * ifname = netif_index_to_name(0, buf);

    ret->interfaceNameLen = static_cast<uint8_t>(strlen(ifname));
    memcpy(ret->interfaceName, ifname, ret->interfaceNameLen);

    return ret;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
