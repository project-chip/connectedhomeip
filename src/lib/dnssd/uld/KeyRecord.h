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
#pragma once

#include <lib/dnssd/uld/Constants.h>
#include <lib/dnssd/wire/records/ResourceRecord.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace Uld {

/**
 * @brief DNS KEY resource record (RFC 2535 / RFC 6605).
 *
 * RDATA: flags(2) | protocol(1) | algorithm(1) | public_key(64).
 * @p publicKey is the 64-byte raw X||Y material (no 0x04 prefix).
 */
class KeyResourceRecord : public ResourceRecord
{
public:
    // DNSKEY RR flags for SRP / ULD requesters (RFC 9665): must be all zeroes.
    static constexpr uint16_t kKeyFlags = 0x0000;

    // DNSKEY RR protocol value for DNSSEC (RFC 4034).
    static constexpr uint8_t kKeyProtocolDnssec = 3;

    KeyResourceRecord(const FullQName & name, ByteSpan publicKey) : ResourceRecord(QType::KEY, name), mPublicKey(publicKey) {}

    ByteSpan GetPublicKey() const { return mPublicKey; }

    /**
     * @brief Copies X||Y after checking uncompressed-point length (65) and 0x04 prefix.
     * Does not validate coordinates or curve membership.
     * @return false if length/prefix checks fail or @p outRawKey is smaller than
     *         #kP256RawPublicKeySize.
     */
    static bool ExtractRawP256PublicKey(ByteSpan uncompressedPoint, MutableByteSpan outRawKey);

protected:
    bool WriteData(RecordWriter & out) const override;

private:
    ByteSpan mPublicKey;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip
