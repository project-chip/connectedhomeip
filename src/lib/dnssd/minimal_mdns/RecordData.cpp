/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "RecordData.h"

#include <inet/arpa-inet-compatibility.h>
#include <stdio.h>

namespace mdns {
namespace Minimal {

bool ParseTxtRecord(const BytesRange & data, TxtRecordDelegate * callback)
{
    // FORMAT:
    //   length-prefixed strings of the form "foo=bar" where = may be missing
    const uint8_t * pos = data.Start();

    while (data.Contains(pos))
    {
        uint8_t length = *pos;

        if (!data.Contains(pos + length))
        {
            return false;
        }

        // name=value string of size length
        const uint8_t * equalPos = pos + 1;
        while (((equalPos - pos) < length) && (*equalPos != '='))
        {
            equalPos++;
        }

        if (pos + length == equalPos && *equalPos == '=')
        {
            // If there is an '=' sign with an empty value, just ignore it and position the end cursor directly onto
            // the position of the '='
            callback->OnRecord(BytesRange(pos + 1, equalPos), BytesRange());
        }
        else if (pos + length == equalPos && *equalPos != '=')
        {
            callback->OnRecord(BytesRange(pos + 1, equalPos + 1), BytesRange());
        }
        else
        {
            callback->OnRecord(BytesRange(pos + 1, equalPos), BytesRange(equalPos + 1, pos + 1 + length));
        }

        pos += 1 + length;
    }

    return pos == data.End();
}

bool SrvRecord::Parse(const BytesRange & data, const BytesRange & packet)
{
    // FORMAT:
    //   - priority
    //   - weight
    //   - port
    //   - target
    if (data.Size() < 7)
    {
        return false;
    }

    const uint8_t * p = data.Start();

    mPriority = chip::Encoding::BigEndian::Read16(p);
    mWeight   = chip::Encoding::BigEndian::Read16(p);
    mPort     = chip::Encoding::BigEndian::Read16(p);
    mName     = SerializedQNameIterator(packet, p);

    return true;
}

bool ParseARecord(const BytesRange & data, chip::Inet::IPAddress * addr)
{
#if INET_CONFIG_ENABLE_IPV4
    if (data.Size() != 4)
    {
        return false;
    }

    addr->Addr[0] = 0;
    addr->Addr[1] = 0;
    addr->Addr[2] = htonl(0xFFFF);
    addr->Addr[3] = htonl(chip::Encoding::BigEndian::Get32(data.Start()));

    return true;
#else
    // IPV4 support is disabled: IPAddress should never get IPv4 values.
    return false;
#endif
}

bool ParseAAAARecord(const BytesRange & data, chip::Inet::IPAddress * addr)
{
    if (data.Size() != 16)
    {
        return false;
    }
    const uint8_t * p = data.Start();
    chip::Inet::IPAddress::ReadAddress(p, *addr);
    return true;
}

bool ParsePtrRecord(const BytesRange & data, const BytesRange & packet, SerializedQNameIterator * name)
{
    if (data.Size() < 1)
    {
        return false;
    }

    *name = SerializedQNameIterator(packet, data.Start());

    return true;
}

} // namespace Minimal
} // namespace mdns
