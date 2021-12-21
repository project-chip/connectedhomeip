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

#include <lwip/netif.h>

namespace chip {
namespace Inet {

namespace PlatformNetworkInterface {

using Type                      = struct netif *;
constexpr size_t kMaxNameLength = 13; // Names are formatted as %c%c%d

inline Type FromInterfaceId(InterfaceId interface)
{
    return reinterpret_cast<Type>(interface.GetInternalId());
}

inline InterfaceId ToInterfaceId(Type interface)
{
    return InterfaceId::FromInternalId(reinterpret_cast<uintptr_t>(interface));
}

CHIP_ERROR GetInterfaceName(struct netif * interface, char * nameBuf, size_t nameBufSize);

inline CHIP_ERROR GetInterfaceName(InterfaceId interface, char * nameBuf, size_t nameBufSize)
{
    return GetInterfaceName(FromInterfaceId(interface), nameBuf, nameBufSize);
}

CHIP_ERROR GetLinkLocalAddr(struct netif * interface, IPAddress * llAddr);

inline CHIP_ERROR GetLinkLocalAddr(InterfaceId interface, IPAddress * llAddr)
{
    return GetLinkLocalAddr(FromInterfaceId(interface), llAddr);
}

} // namespace PlatformNetworkInterface

class InterfaceIteratorImplLwIP : public InterfaceIteratorBase
{
public:
    InterfaceIteratorImplLwIP() { mCurNetif = netif_list; }
    ~InterfaceIteratorImplLwIP() {}

    // InterfaceIteratorBase overrides.
    bool HasCurrent() override { return mCurNetif != NULL; }
    bool Next() override;
    InterfaceId GetInterfaceId() override { return PlatformNetworkInterface::ToInterfaceId(mCurNetif); }
    CHIP_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize) override;
    bool IsUp() override;
    bool SupportsMulticast() override;
    bool HasBroadcastAddress() override;
    CHIP_ERROR GetInterfaceType(InterfaceType & type) override;
    CHIP_ERROR GetHardwareAddress(uint8_t * addressBuffer, uint8_t & addressSize, uint8_t addressBufferSize) override;

private:
    struct netif * mCurNetif;
};

using InterfaceIterator = InterfaceIteratorImplLwIP;

class InterfaceAddressIteratorImplLwIP : public InterfaceAddressIteratorBase
{
public:
    InterfaceAddressIteratorImplLwIP() : mCurAddrIndex(kBeforeStartIndex) {}
    ~InterfaceAddressIteratorImplLwIP() = default;

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
    enum
    {
        kBeforeStartIndex = -1
    };

    InterfaceIterator mIntfIter;
    int mCurAddrIndex;
};

using InterfaceAddressIterator = InterfaceAddressIteratorImplLwIP;

} // namespace Inet
} // namespace chip
