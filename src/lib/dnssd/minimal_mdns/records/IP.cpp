/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "IP.h"

namespace mdns {
namespace Minimal {

bool IPResourceRecord::WriteData(RecordWriter & out) const
{
    // IP address is already stored in network byte order, hence raw bytes put
    if (mIPAddress.IsIPv6())
    {
        out.Put(BytesRange::BufferWithSize(mIPAddress.Addr, 16));
    }
    else
    {
        out.Put(BytesRange::BufferWithSize(mIPAddress.Addr + 3, 4));
    }

    return out.Fit();
}

} // namespace Minimal
} // namespace mdns
