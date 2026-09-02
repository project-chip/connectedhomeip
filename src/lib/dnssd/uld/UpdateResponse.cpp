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

#include <lib/dnssd/uld/UpdateResponse.h>

#include <lib/dnssd/uld/KeyRecord.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Dnssd {
namespace Uld {

CHIP_ERROR UpdateResponse::Parse(ByteSpan packet)
{
    mParseError   = CHIP_NO_ERROR;
    mMessageId    = 0;
    mResponseCode = 0;
    mOptSeen      = false;
    mKey.reset();

    VerifyOrReturnError(packet.size() >= HeaderRef::kSizeBytes, CHIP_ERROR_INVALID_ARGUMENT);

    const BytesRange packetData = BytesRange::BufferWithSize(packet.data(), packet.size());
    VerifyOrReturnError(ParseDnsPacket(packetData, this), CHIP_ERROR_INVALID_ARGUMENT);
    return mParseError;
}

void UpdateResponse::OnHeader(ConstHeaderRef & header)
{
    const BitPackedFlags flags = header.GetFlags();
    if (!flags.IsResponse() || (flags.GetOpcode() != Opcode::kUpdate))
    {
        mParseError = CHIP_ERROR_INVALID_ARGUMENT;
        return;
    }

    mMessageId    = header.GetMessageId();
    mResponseCode = flags.GetResponseCodeBits();
}

void UpdateResponse::OnResource(ResourceType type, const ResourceData & data)
{
    VerifyOrReturn(mParseError == CHIP_NO_ERROR);

    if (data.GetType() == QType::OPT)
    {
        if ((type != ResourceType::kAdditional) || mOptSeen)
        {
            mParseError = CHIP_ERROR_INVALID_ARGUMENT;
            return;
        }
        mOptSeen = true;

        // RFC 6891: OPT TTL = EXTENDED-RCODE (8) | VERSION (8) | flags (16).
        // 12-bit RCODE = (EXTENDED-RCODE << headerRcodeBitWidth) | header RCODE (4).
        constexpr uint8_t kOptTtlExtendedRcodeShift = 24;
        constexpr uint8_t kOptTtlExtendedRcodeMask  = 0xFF;
        constexpr uint8_t kHeaderRcodeBitWidth      = 4;

        const uint16_t extendedResponseCode =
            static_cast<uint16_t>((data.GetTtlSeconds() >> kOptTtlExtendedRcodeShift) & kOptTtlExtendedRcodeMask);
        mResponseCode = static_cast<uint16_t>((extendedResponseCode << kHeaderRcodeBitWidth) | mResponseCode);
        return;
    }

    if (data.GetType() == QType::KEY)
    {
        if ((type == ResourceType::kAnswer) || mKey.has_value())
        {
            mParseError = CHIP_ERROR_INVALID_ARGUMENT;
            return;
        }

        Crypto::P256PublicKey publicKey;
        mParseError = KeyResourceRecord::Parse(data.GetData().AsByteSpan(), publicKey);
        if (mParseError == CHIP_NO_ERROR)
        {
            mKey = publicKey;
        }
    }
}

} // namespace Uld
} // namespace Dnssd
} // namespace chip
