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

#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#include <ncs_version.h>
#endif

#if KERNEL_VERSION_MAJOR > 4 || (KERNEL_VERSION_MAJOR == 4 && KERNEL_VERSION_MINOR >= 4)
#define INET_UTILS_USE_NET_IN6_ADDR
// nRF Connect SDK 3.3.0 supports Zephyr 4.3.99 version, so unfortunately it needs a separate check
#elif CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#if NCS_VERSION_MAJOR > 3 || (NCS_VERSION_MAJOR == 3 && NCS_VERSION_MINOR >= 3)
#define INET_UTILS_USE_NET_IN6_ADDR
#endif // NCS_VERSION_MAJOR > 3 || (NCS_VERSION_MAJOR == 3 && NCS_VERSION_MINOR >= 3)
#endif // KERNEL_VERSION_MAJOR > 4 || (KERNEL_VERSION_MAJOR == 4 && KERNEL_VERSION_MINOR >= 4)

#ifdef INET_UTILS_USE_NET_IN6_ADDR
struct net_in6_addr;
#else
struct in6_addr;
#endif // INET_UTILS_USE_NET_IN6_ADDR
struct net_if;

namespace chip {
namespace DeviceLayer {
namespace InetUtils {

#ifdef INET_UTILS_USE_NET_IN6_ADDR
using ZephyrIn6Addr = ::net_in6_addr;
#else
using ZephyrIn6Addr = ::in6_addr;
#endif // INET_UTILS_USE_NET_IN6_ADDR

ZephyrIn6Addr ToZephyrAddr(const Inet::IPAddress & address);
net_if * GetInterface(Inet::InterfaceId ifaceId = Inet::InterfaceId::Null());
net_if * GetWiFiInterface();

} // namespace InetUtils
} // namespace DeviceLayer
} // namespace chip
