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
    uint8_t rawKey[kP256RawPublicKeySize];

    for (size_t i = 0; i < sizeof(rawKey); i++)
    {
        rawKey[i] = static_cast<uint8_t>(i);
    }

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    KeyResourceRecord record(kNames, ByteSpan(rawKey));
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
    EXPECT_EQ(memcmp(rdata + 4, rawKey, sizeof(rawKey)), 0);
}

TEST(TestKeyRecord, RejectsWrongKeyLength)
{
    uint8_t headerBuffer[HeaderRef::kSizeBytes];
    uint8_t dataBuffer[128];
    uint8_t shortKey[16] = {};

    HeaderRef header(headerBuffer);
    header.Clear();

    BufferWriter output(dataBuffer, sizeof(dataBuffer));
    RecordWriter writer(&output);

    KeyResourceRecord record(kNames, ByteSpan(shortKey));
    EXPECT_FALSE(record.Append(header, ResourceType::kAuthority, writer));
    EXPECT_EQ(header.GetAuthorityCount(), 0);
}

TEST(TestKeyRecord, ExtractRawP256PublicKeyStripsPrefix)
{
    uint8_t point[kP256RawPublicKeySize + 1];
    uint8_t raw[kP256RawPublicKeySize];

    point[0] = 0x04;
    for (size_t i = 0; i < kP256RawPublicKeySize; i++)
    {
        point[i + 1] = static_cast<uint8_t>(0xA0 + i);
    }

    MutableByteSpan out(raw);
    EXPECT_TRUE(KeyResourceRecord::ExtractRawP256PublicKey(ByteSpan(point), out));
    EXPECT_EQ(out.size(), kP256RawPublicKeySize);
    EXPECT_EQ(memcmp(raw, point + 1, kP256RawPublicKeySize), 0);

    point[0] = 0x02; // compressed — reject
    out      = MutableByteSpan(raw);
    EXPECT_FALSE(KeyResourceRecord::ExtractRawP256PublicKey(ByteSpan(point), out));
}

} // namespace
