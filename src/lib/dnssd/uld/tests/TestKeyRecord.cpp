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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/uld/KeyRecord.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::Encoding::BigEndian;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

const QNamePart kNames[] = { "host", "local" };

TEST(TestKeyRecord, WritesFlagsProtocolAlgorithmAndRawKey)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[256];

    Crypto::P256Keypair keypair;
    ASSERT_EQ(keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    KeyResourceRecord record(kNames, keypair.Pubkey());
    record.SetTtl(120);

    EXPECT_TRUE(record.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAuthorityCount(), 1);

    const size_t nameBytes = 1 + 4 + 1 + 5 + 1;
    // type KEY
    EXPECT_EQ(dataBuffer[nameBytes], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 1], static_cast<uint8_t>(QType::KEY));
    // class IN
    EXPECT_EQ(dataBuffer[nameBytes + 2], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 3], static_cast<uint8_t>(QClass::IN));
    // rdlength = 4 + 64
    EXPECT_EQ(dataBuffer[nameBytes + 8], 0);
    EXPECT_EQ(dataBuffer[nameBytes + 9], 68);

    const uint8_t * rdata = dataBuffer + nameBytes + 10;
    EXPECT_EQ(rdata[0], 0x00);
    EXPECT_EQ(rdata[1], 0x00);
    EXPECT_EQ(rdata[2], KeyResourceRecord::kKeyProtocolDnssec);
    EXPECT_EQ(rdata[3], kKeyAlgorithmEcdsaP256);
    EXPECT_EQ(memcmp(rdata + 4, keypair.Pubkey().ConstBytes() + 1, kP256RawPublicKeySize), 0);
}

TEST(TestKeyRecord, RejectsNonUncompressedKey)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];
    Crypto::P256PublicKey publicKey;
    memset(publicKey.Bytes(), 0, publicKey.Length());

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    KeyResourceRecord record(kNames, publicKey);
    EXPECT_FALSE(record.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAuthorityCount(), 0);
}

TEST(TestKeyRecord, ParseReconstructsUncompressedPublicKey)
{
    uint8_t rdata[4 + kP256RawPublicKeySize] = {};
    rdata[2]                                 = KeyResourceRecord::kKeyProtocolDnssec;
    rdata[3]                                 = kKeyAlgorithmEcdsaP256;
    for (size_t i = 0; i < kP256RawPublicKeySize; ++i)
    {
        rdata[4 + i] = static_cast<uint8_t>(i + 1);
    }

    Crypto::P256PublicKey publicKey;
    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata), publicKey), CHIP_NO_ERROR);
    EXPECT_EQ(publicKey.ConstBytes()[0], 0x04);
    EXPECT_EQ(memcmp(publicKey.ConstBytes() + 1, rdata + 4, kP256RawPublicKeySize), 0);
}

TEST(TestKeyRecord, ParseRejectsInvalidRdata)
{
    uint8_t rdata[4 + kP256RawPublicKeySize] = {};
    rdata[2]                                 = KeyResourceRecord::kKeyProtocolDnssec;
    rdata[3]                                 = kKeyAlgorithmEcdsaP256;
    rdata[4]                                 = 1;

    Crypto::P256PublicKey publicKey;

    rdata[0] = 1;
    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata), publicKey), CHIP_ERROR_INVALID_ARGUMENT);

    rdata[0] = 0;
    rdata[2] = 2;
    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata), publicKey), CHIP_ERROR_INVALID_ARGUMENT);

    rdata[2] = KeyResourceRecord::kKeyProtocolDnssec;
    rdata[3] = 14;
    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata), publicKey), CHIP_ERROR_INVALID_ARGUMENT);

    rdata[3] = kKeyAlgorithmEcdsaP256;
    rdata[4] = 0;
    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata), publicKey), CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(KeyResourceRecord::Parse(ByteSpan(rdata, sizeof(rdata) - 1), publicKey), CHIP_ERROR_INVALID_ARGUMENT);
}

} // namespace
