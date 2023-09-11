/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements the concrete class
 *      <tt>Inet::IPPrefix</tt>, which comprise two member fields:
 *      a) a IP address and b) a length field. The CHIP Inet Layer
 *      uses objects of this class to represent Internet protocol
 *      address prefixes of both IPv4 and IPv6 address families.
 *
 */

#include "IPPrefix.h"
#include <lib/core/CHIPEncoding.h>

namespace chip {
namespace Inet {

IPPrefix IPPrefix::Zero;

bool IPPrefix::IsZero() const
{
    return IPAddr.Addr[0] == 0 && IPAddr.Addr[1] == 0 && IPAddr.Addr[2] == 0 && IPAddr.Addr[3] == 0 && Length == 0;
}

bool IPPrefix::operator==(const IPPrefix & other) const
{
    return IPAddr == other.IPAddr && Length == other.Length;
}

bool IPPrefix::operator!=(const IPPrefix & other) const
{
    return IPAddr != other.IPAddr || Length != other.Length;
}

bool IPPrefix::MatchAddress(const IPAddress & addr) const
{
    uint8_t l = (Length <= 128) ? Length : 128;
    int i;

    for (i = 0; l >= 32; i++, l = static_cast<uint8_t>(l - 32u))
        if (IPAddr.Addr[i] != addr.Addr[i])
            return false;

    if (l == 0)
        return true;

    uint32_t mask = chip::Encoding::BigEndian::HostSwap32(0xFFFFFFFF << (32 - l));
    return (IPAddr.Addr[i] & mask) == (addr.Addr[i] & mask);
}

} // namespace Inet
} // namespace chip
