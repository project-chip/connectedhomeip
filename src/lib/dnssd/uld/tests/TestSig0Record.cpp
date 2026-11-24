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
#include <lib/dnssd/uld/Sig0Record.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::Encoding::BigEndian;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

const QNamePart kSigner[] = { "host", "local" };

TEST(TestSig0Record, WritesWireLayoutWithZeroSignature)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[256];
    uint8_t signature[kP256RawSignatureSize] = {};

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    Sig0ResourceRecord record(kSigner, ByteSpan(signature), /*keyTag=*/0xABCD, /*inception=*/0x01020304,
                              /*expiration=*/0x05060708);

    EXPECT_TRUE(record.Append(header, ResourceType::kAdditional, writer));
    EXPECT_EQ(header.GetAdditionalCount(), 1);

    // NAME = root
    EXPECT_EQ(dataBuffer[0], 0);
    // TYPE SIG
    EXPECT_EQ(dataBuffer[1], 0);
    EXPECT_EQ(dataBuffer[2], static_cast<uint8_t>(QType::SIG));
    // CLASS ANY
    EXPECT_EQ(dataBuffer[3], 0);
    EXPECT_EQ(dataBuffer[4], static_cast<uint8_t>(QClass::ANY));
    // TTL 0
    EXPECT_EQ(dataBuffer[5], 0);
    EXPECT_EQ(dataBuffer[6], 0);
    EXPECT_EQ(dataBuffer[7], 0);
    EXPECT_EQ(dataBuffer[8], 0);

    const uint8_t * rdata = dataBuffer + 11;
    EXPECT_EQ(rdata[0], 0); // type covered
    EXPECT_EQ(rdata[1], 0);
    EXPECT_EQ(rdata[2], kKeyAlgorithmEcdsaP256);
    EXPECT_EQ(rdata[3], 0); // labels

    // expiration
    EXPECT_EQ(rdata[8], 0x05);
    EXPECT_EQ(rdata[9], 0x06);
    EXPECT_EQ(rdata[10], 0x07);
    EXPECT_EQ(rdata[11], 0x08);
    // inception
    EXPECT_EQ(rdata[12], 0x01);
    EXPECT_EQ(rdata[13], 0x02);
    EXPECT_EQ(rdata[14], 0x03);
    EXPECT_EQ(rdata[15], 0x04);
    // key tag
    EXPECT_EQ(rdata[16], 0xAB);
    EXPECT_EQ(rdata[17], 0xCD);

    // signer name host.local.
    EXPECT_EQ(rdata[18], 4);
    EXPECT_EQ(memcmp(rdata + 19, "host", 4), 0);
    EXPECT_EQ(rdata[23], 5);
    EXPECT_EQ(memcmp(rdata + 24, "local", 5), 0);
    EXPECT_EQ(rdata[29], 0);

    // 64-byte zero signature follows
    EXPECT_EQ(memcmp(rdata + 30, signature, sizeof(signature)), 0);
}

TEST(TestSig0Record, RejectsWrongSignatureLength)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];
    uint8_t shortSig[8] = {};

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    Sig0ResourceRecord record(kSigner, ByteSpan(shortSig));
    EXPECT_FALSE(record.Append(header, ResourceType::kAdditional, writer));
    EXPECT_EQ(header.GetAdditionalCount(), 0);
}

} // namespace
