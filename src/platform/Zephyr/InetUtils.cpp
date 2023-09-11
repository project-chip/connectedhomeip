/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
