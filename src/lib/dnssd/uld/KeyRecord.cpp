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

#include <algorithm>
#include <cstring>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Dnssd {
namespace Uld {

bool KeyResourceRecord::WriteData(RecordWriter & out) const
{
    static_assert(Crypto::kP256_PublicKey_Length == kP256RawPublicKeySize + 1);
    VerifyOrReturnValue(mPublicKey.IsUncompressed(), false);

    const uint8_t * rawBegin = mPublicKey.ConstBytes() + 1;
    const uint8_t * rawEnd   = rawBegin + kP256RawPublicKeySize;
    VerifyOrReturnValue(std::any_of(rawBegin, rawEnd, [](uint8_t value) { return value != 0; }), false);

    out.Put16(kKeyFlags).Put8(kKeyProtocolDnssec).Put8(kKeyAlgorithmEcdsaP256);
    out.Writer().Put(rawBegin, kP256RawPublicKeySize);
    return out.Fit();
}

CHIP_ERROR KeyResourceRecord::Parse(ByteSpan rdata, Crypto::P256PublicKey & publicKey)
{
    static_assert(Crypto::kP256_PublicKey_Length == kP256RawPublicKeySize + 1);
    // KEY RDATA prefix: flags (u16) | protocol (u8) | algorithm (u8).
    constexpr size_t kKeyMetadataSize = sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t);

    VerifyOrReturnError(rdata.size() == kKeyMetadataSize + kP256RawPublicKeySize, CHIP_ERROR_INVALID_ARGUMENT);

    const uint8_t * cursor = rdata.data();
    // Verify the validity of the Key metadata.
    VerifyOrReturnError(Encoding::BigEndian::Read16(cursor) == kKeyFlags, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*cursor++ == kKeyProtocolDnssec, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*cursor++ == kKeyAlgorithmEcdsaP256, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(std::any_of(cursor, rdata.end(), [](uint8_t value) { return value != 0; }), CHIP_ERROR_INVALID_ARGUMENT);

    constexpr uint8_t kUncompressedPointMarker = 0x04;
    publicKey.Bytes()[0]                       = kUncompressedPointMarker;
    memcpy(publicKey.Bytes() + 1, cursor, kP256RawPublicKeySize);
    return CHIP_NO_ERROR;
}

} // namespace Uld
} // namespace Dnssd
} // namespace chip
