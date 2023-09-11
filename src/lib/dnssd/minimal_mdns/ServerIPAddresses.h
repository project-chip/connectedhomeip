/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace mdns {
namespace Minimal {

class IpAddressIterator
{
public:
    virtual ~IpAddressIterator() = default;

    /// Fetch the next IP address in the iterator.
    ///
    /// Returns true if an IP address is available, false if the iteration is complete.
    virtual bool Next(chip::Inet::IPAddress & dest) = 0;
};

} // namespace Minimal
} // namespace mdns
