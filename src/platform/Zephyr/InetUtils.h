/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inet/InetInterface.h>

struct in6_addr;
struct net_if;

namespace chip {
namespace DeviceLayer {
namespace InetUtils {

in6_addr ToZephyrAddr(const chip::Inet::IPAddress & address);
net_if * GetInterface(chip::Inet::InterfaceId ifaceId = chip::Inet::InterfaceId::Null());

} // namespace InetUtils
} // namespace DeviceLayer
} // namespace chip
