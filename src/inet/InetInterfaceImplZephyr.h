/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 * @file
 *  This file defines the <tt>Inet::InterfaceId</tt> type alias and related
 *  classes for iterating on the list of system network interfaces and the list
 *  of system interface addresses.
 */

#pragma once

#include <device.h>

struct net_if;
struct net_if_ipv4;
struct net_if_ipv6;

namespace chip {
namespace Inet {
namespace PlatformNetworkInterface {

constexpr size_t kMaxNameLength = Z_DEVICE_MAX_NAME_LEN;
using Type                      = int;

inline Type FromInterfaceId(InterfaceId interface)
{
    return static_cast<Type>(interface.GetInternalId());
}

inline InterfaceId ToInterfaceId(Type interface)
{
    return InterfaceId::FromInternalId(static_cast<uintptr_t>(interface));
}

CHIP_ERROR GetInterfaceName(int interface, char * nameBuf, size_t nameBufSize);

inline CHIP_ERROR GetInterfaceName(InterfaceId interface, char * nameBuf, size_t nameBufSize)
{
    return GetInterfaceName(FromInterfaceId(interface), nameBuf, nameBufSize);
}

CHIP_ERROR GetLinkLocalAddr(int interface, IPAddress * llAddr);

inline CHIP_ERROR GetLinkLocalAddr(InterfaceId interface, IPAddress * llAddr)
{
    return GetLinkLocalAddr(FromInterfaceId(interface), llAddr);
}

} // namespace PlatformNetworkInterface

class InterfaceIteratorImplZephyr : public InterfaceIteratorBase
{
public:
    InterfaceIteratorImplZephyr();
    ~InterfaceIteratorImplZephyr() = default;

    // InterfaceIteratorBase overrides.
    bool HasCurrent() override;
    bool Next() override;
    InterfaceId GetInterfaceId() override;
    CHIP_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize) override;
    bool IsUp() override;
    bool SupportsMulticast() override;
    bool HasBroadcastAddress() override;
    CHIP_ERROR GetInterfaceType(InterfaceType & type) override;
    CHIP_ERROR GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize) override;

private:
    PlatformNetworkInterface::Type mCurrentId = 1;
    net_if * mCurrentInterface                = nullptr;
};

using InterfaceIterator = InterfaceIteratorImplZephyr;

class InterfaceAddressIteratorImplZephyr : public InterfaceAddressIteratorBase
{
public:
    InterfaceAddressIteratorImplZephyr();
    ~InterfaceAddressIteratorImplZephyr() = default;

    bool HasCurrent() override;
    bool Next() override;
    CHIP_ERROR GetAddress(IPAddress & outIPAddress) override;
    uint8_t GetPrefixLength() override;
    InterfaceId GetInterfaceId() override;
    CHIP_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize) override;
    bool IsUp() override;
    bool SupportsMulticast() override;
    bool HasBroadcastAddress() override;

private:
    InterfaceIterator mIntfIter;
    net_if_ipv6 * mIpv6 = nullptr;
    int mCurAddrIndex   = -1;
};

using InterfaceAddressIterator = InterfaceAddressIteratorImplZephyr;

} // namespace Inet
} // namespace chip
