/* See Project CHIP LICENSE file for licensing information. */


#include "IP.h"

namespace mdns {
namespace Minimal {

bool IPResourceRecord::WriteData(chip::Encoding::BigEndian::BufferWriter & out) const
{
    // IP address is already stored in network byte order, hence raw bytes put
    if (mIPAddress.IsIPv6())
    {
        out.Put(mIPAddress.Addr, 16);
    }
    else
    {
        out.Put(mIPAddress.Addr + 3, 4);
    }

    return out.Fit();
}

} // namespace Minimal
} // namespace mdns
