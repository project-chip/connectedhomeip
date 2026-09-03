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

#include <pw_unit_test/framework.h>

#include <array>
#include <cstring>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/uld/KeyRecord.h>
#include <lib/dnssd/uld/UpdateResponse.h>

namespace {

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

constexpr size_t kKeyFlagsOffset     = 34;
constexpr size_t kKeyProtocolOffset  = 36;
constexpr size_t kKeyAlgorithmOffset = 37;
constexpr size_t kKeyBytesOffset     = 38;
constexpr size_t kPacketSize         = 113;

std::array<uint8_t, kPacketSize> MakeUpdateResponse()
{
    std::array<uint8_t, kPacketSize> packet = {};

    packet[0]  = 0x12;
    packet[1]  = 0x34; // message id
    packet[2]  = 0xA8;
    packet[3]  = 0x0B; // response, opcode UPDATE, header RCODE 11
    packet[8]  = 0x00;
    packet[9]  = 0x01; // one authority record
    packet[10] = 0x00;
    packet[11] = 0x01; // one additional record

    size_t offset    = HeaderRef::kSizeBytes;
    packet[offset++] = 4;
    memcpy(packet.data() + offset, "host", 4);
    offset += 4;
    packet[offset++] = 5;
    memcpy(packet.data() + offset, "local", 5);
    offset += 5;
    packet[offset++] = 0;
    packet[offset++] = 0;
    packet[offset++] = static_cast<uint8_t>(QType::KEY);
    packet[offset++] = 0;
    packet[offset++] = static_cast<uint8_t>(QClass::IN);
    offset += 4; // TTL
    packet[offset++] = 0;
    packet[offset++] = 68; // RDLENGTH
    packet[offset++] = 0;
    packet[offset++] = 0; // flags
    packet[offset++] = KeyResourceRecord::kKeyProtocolDnssec;
    packet[offset++] = kKeyAlgorithmEcdsaP256;
    for (size_t i = 0; i < kP256RawPublicKeySize; ++i)
    {
        packet[offset++] = static_cast<uint8_t>(i + 1);
    }

    packet[offset++] = 0; // root owner name
    packet[offset++] = 0;
    packet[offset++] = static_cast<uint8_t>(QType::OPT);
    packet[offset++] = 0x04;
    packet[offset++] = 0xD0; // UDP payload size 1232
    packet[offset++] = 0x02;
    packet[offset++] = 0;
    packet[offset++] = 0;
    packet[offset++] = 0; // extended RCODE 2
    packet[offset++] = 0;
    packet[offset++] = 0; // empty OPT RDATA

    EXPECT_EQ(offset, packet.size());
    return packet;
}

TEST(TestUpdateResponse, ParsesExtendedResponseCodeAndKey)
{
    const auto packet = MakeUpdateResponse();
    UpdateResponse response;

    ASSERT_EQ(response.Parse(ByteSpan(packet)), CHIP_NO_ERROR);
    EXPECT_EQ(response.GetMessageId(), 0x1234);
    EXPECT_EQ(response.GetResponseCode(), 0x2B);
    const std::optional<Crypto::P256PublicKey> & key = response.GetKey();
    ASSERT_TRUE(key.has_value());
    if (key.has_value())
    {
        EXPECT_EQ(key->ConstBytes()[0], 0x04);
        EXPECT_EQ(memcmp(key->ConstBytes() + 1, packet.data() + kKeyBytesOffset, kP256RawPublicKeySize), 0);
    }
}

TEST(TestUpdateResponse, RejectsUnsupportedKeyMetadata)
{
    auto packet = MakeUpdateResponse();
    UpdateResponse response;

    packet[kKeyFlagsOffset] = 1;
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);

    packet                     = MakeUpdateResponse();
    packet[kKeyProtocolOffset] = 2;
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);

    packet                      = MakeUpdateResponse();
    packet[kKeyAlgorithmOffset] = 14;
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestUpdateResponse, RejectsEmptyOrTruncatedKey)
{
    auto packet = MakeUpdateResponse();
    UpdateResponse response;

    memset(packet.data() + kKeyBytesOffset, 0, kP256RawPublicKeySize);
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);

    packet = MakeUpdateResponse();
    EXPECT_EQ(response.Parse(ByteSpan(packet.data(), packet.size() - 1)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestUpdateResponse, ParsesHeaderOnlyResponse)
{
    const uint8_t packet[] = {
        0x12, 0x34, // message id
        0xA8, 0x00, // response, opcode UPDATE, RCODE 0
        0,    0,    // no zone
        0,    0,    // no prerequisites
        0,    0,    // no updates
        0,    0,    // no additional records
    };
    UpdateResponse response;

    ASSERT_EQ(response.Parse(ByteSpan(packet)), CHIP_NO_ERROR);
    EXPECT_EQ(response.GetMessageId(), 0x1234);
    EXPECT_EQ(response.GetResponseCode(), 0);
    EXPECT_FALSE(response.GetKey().has_value());
}

TEST(TestUpdateResponse, RejectsNonUpdateMessage)
{
    auto packet = MakeUpdateResponse();
    UpdateResponse response;

    packet[2] = 0x80; // response with QUERY opcode
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);

    packet    = MakeUpdateResponse();
    packet[2] = 0x28; // UPDATE query, not a response
    EXPECT_EQ(response.Parse(ByteSpan(packet)), CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
