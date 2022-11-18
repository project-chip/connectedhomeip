/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "InetUtils.h"

namespace chip {
namespace DeviceLayer {
namespace InetUtils {

in6_addr ToZephyrAddr(const chip::Inet::IPAddress & address)
{
    in6_addr zephyrAddr;

    static_assert(sizeof(zephyrAddr.s6_addr) == sizeof(address.Addr), "Unexpected address size");
    memcpy(zephyrAddr.s6_addr, address.Addr, sizeof(address.Addr));

    return zephyrAddr;
}

net_if * GetInterface(chip::Inet::InterfaceId ifaceId)
{
    return ifaceId.IsPresent() ? net_if_get_by_index(ifaceId.GetPlatformInterface()) : net_if_get_default();
}

} // namespace InetUtils
} // namespace DeviceLayer
} // namespace chip
