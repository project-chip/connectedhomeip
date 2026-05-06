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

#include <inet/InetInterface.h>

#if defined(CONFIG_ZEPHYR_VERSION_3_3)
#include <version.h>
#else
#include <zephyr/version.h>
#endif

#if KERNEL_VERSION_MAJOR > 4 || (KERNEL_VERSION_MAJOR == 4 && KERNEL_VERSION_MINOR >= 3)
struct net_in6_addr;
#else
struct in6_addr;
#endif
struct net_if;

namespace chip {
namespace DeviceLayer {
namespace InetUtils {

#if KERNEL_VERSION_MAJOR > 4 || (KERNEL_VERSION_MAJOR == 4 && KERNEL_VERSION_MINOR >= 3)
using ZephyrIn6Addr = ::net_in6_addr;
#else
using ZephyrIn6Addr = ::in6_addr;
#endif

ZephyrIn6Addr ToZephyrAddr(const Inet::IPAddress & address);
net_if * GetInterface(Inet::InterfaceId ifaceId = Inet::InterfaceId::Null());
net_if * GetWiFiInterface();

} // namespace InetUtils
} // namespace DeviceLayer
} // namespace chip
