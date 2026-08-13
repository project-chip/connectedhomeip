/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/dnssd/uld/KeyRecord.h>

#include <cstring>

namespace chip {
namespace Dnssd {
namespace Uld {

bool KeyResourceRecord::WriteData(RecordWriter & out) const
{
    if (mPublicKey.size() != kP256RawPublicKeySize)
    {
        return false;
    }

    out.Put16(kKeyFlags).Put8(kKeyProtocolDnssec).Put8(kKeyAlgorithmEcdsaP256);
    out.Writer().Put(mPublicKey.data(), mPublicKey.size());
    return out.Fit();
}

bool KeyResourceRecord::ExtractRawP256PublicKey(ByteSpan uncompressedPoint, MutableByteSpan outRawKey)
{
    constexpr uint8_t kUncompressedPointPrefix = 0x04;
    constexpr size_t kUncompressedPointSize    = kP256RawPublicKeySize + 1;

    if ((uncompressedPoint.size() != kUncompressedPointSize) || (uncompressedPoint[0] != kUncompressedPointPrefix) ||
        (outRawKey.size() < kP256RawPublicKeySize))
    {
        return false;
    }

    memcpy(outRawKey.data(), uncompressedPoint.data() + 1, kP256RawPublicKeySize);
    outRawKey.reduce_size(kP256RawPublicKeySize);
    return true;
}

} // namespace Uld
} // namespace Dnssd
} // namespace chip
