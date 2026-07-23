/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/dnssd/srp/SrpDnsMessage.h>

#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

using namespace chip;
using namespace chip::Dnssd::Srp;

namespace {

TEST(TestSrpDnsMessage, HeaderRoundTrip)
{
    uint8_t buffer[64];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutHeader(0x1234, MakeFlags(kOpcodeUpdate), 1, 0, 2, 1), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    Header header;
    ASSERT_EQ(reader.ReadHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.id, 0x1234);
    EXPECT_EQ(header.Opcode(), kOpcodeUpdate);
    EXPECT_FALSE(header.IsResponse());
    EXPECT_EQ(header.qdcount, 1);
    EXPECT_EQ(header.nscount, 2);
    EXPECT_EQ(header.arcount, 1);
}

TEST(TestSrpDnsMessage, NameRoundTrip)
{
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutQuestion("myhost._matter._tcp.default.service.arpa", RecordType::kSrv, RecordClass::kIn), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    char name[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "myhost._matter._tcp.default.service.arpa");
    uint16_t type, klass;
    ASSERT_EQ(reader.ReadU16(type), CHIP_NO_ERROR);
    ASSERT_EQ(reader.ReadU16(klass), CHIP_NO_ERROR);
    EXPECT_EQ(type, static_cast<uint16_t>(RecordType::kSrv));
    EXPECT_EQ(klass, static_cast<uint16_t>(RecordClass::kIn));
}

TEST(TestSrpDnsMessage, RootNameRoundTrip)
{
    uint8_t buffer[16];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutName(""), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Length(), 1u); // single root octet

    DnsReader reader(buffer, writer.Length());
    char name[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "");
}

TEST(TestSrpDnsMessage, SrvRoundTrip)
{
    uint8_t buffer[256];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(
        writer.PutSrv("inst._matter._tcp.default.service.arpa", RecordClass::kIn, 3600, 1, 2, 5540, "myhost.default.service.arpa"),
        CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kSrv);
    EXPECT_EQ(header.ttl, 3600u);

    uint16_t priority, weight, port;
    char target[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadSrv(priority, weight, port, target, sizeof(target)), CHIP_NO_ERROR);
    EXPECT_EQ(priority, 1);
    EXPECT_EQ(weight, 2);
    EXPECT_EQ(port, 5540);
    EXPECT_STREQ(target, "myhost.default.service.arpa");
}

TEST(TestSrpDnsMessage, TxtRoundTrip)
{
    const uint8_t val1[] = { 'A', 'B' };
    Dnssd::TextEntry entries[2];
    entries[0].mKey      = "CM";
    entries[0].mData     = val1;
    entries[0].mDataSize = sizeof(val1);
    entries[1].mKey      = "T";
    entries[1].mData     = nullptr;
    entries[1].mDataSize = 0;

    uint8_t buffer[256];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutTxt("inst._matter._tcp.default.service.arpa", RecordClass::kIn, 3600, entries, 2), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kTxt);

    // RDATA: "CM=AB" (len 5), "T" (len 1)
    const uint8_t * rdata = reader.Data() + header.rdataOffset;
    ASSERT_EQ(rdata[0], 5);
    EXPECT_EQ(memcmp(rdata + 1, "CM=AB", 5), 0);
    EXPECT_EQ(rdata[6], 1);
    EXPECT_EQ(rdata[7], 'T');
}

TEST(TestSrpDnsMessage, AaaaRoundTrip)
{
    Inet::IPAddress address;
    ASSERT_TRUE(Inet::IPAddress::FromString("fe80::1234:5678", address));

    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutAaaa("myhost.default.service.arpa", RecordClass::kIn, 120, address), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kAaaa);
    EXPECT_EQ(header.rdlength, 16);

    Inet::IPAddress decoded;
    ASSERT_EQ(reader.ReadAaaa(decoded), CHIP_NO_ERROR);
    EXPECT_EQ(decoded, address);
}

TEST(TestSrpDnsMessage, PtrRoundTrip)
{
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutPtr("_matter._tcp.default.service.arpa", RecordClass::kIn, 3600, "inst._matter._tcp.default.service.arpa"),
              CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kPtr);

    char target[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadPtr(target, sizeof(target)), CHIP_NO_ERROR);
    EXPECT_STREQ(target, "inst._matter._tcp.default.service.arpa");
}

TEST(TestSrpDnsMessage, DeleteRRsetForm)
{
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutDeleteRRset("inst._matter._tcp.default.service.arpa", RecordType::kSrv), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kSrv);
    EXPECT_EQ(header.recordClass, RecordClass::kAny);
    EXPECT_EQ(header.ttl, 0u);
    EXPECT_EQ(header.rdlength, 0);
}

TEST(TestSrpDnsMessage, CompressedNameDecode)
{
    // Manually craft: "a.bc" at offset 12, then a pointer to it at a later offset.
    uint8_t buffer[64];
    memset(buffer, 0, sizeof(buffer));
    size_t o          = 12;
    buffer[o++]       = 1;
    buffer[o++]       = 'a';
    buffer[o++]       = 2;
    buffer[o++]       = 'b';
    buffer[o++]       = 'c';
    buffer[o++]       = 0;
    size_t pointerPos = o;
    buffer[o++]       = 0xC0; // pointer high bits
    buffer[o++]       = 12;   // -> offset 12

    DnsReader reader(buffer, o);
    reader.Seek(pointerPos);
    char name[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "a.bc");
    // The reader must have advanced by exactly the 2 pointer bytes.
    EXPECT_EQ(reader.Offset(), pointerPos + 2);
}

TEST(TestSrpDnsMessage, WriterReportsBufferTooSmall)
{
    uint8_t buffer[4];
    DnsWriter writer(buffer, sizeof(buffer));
    EXPECT_NE(writer.PutHeader(1, 0, 0, 0, 0, 0), CHIP_NO_ERROR);
    EXPECT_FALSE(writer.Ok());
}

TEST(TestSrpDnsMessage, KeyRecordRoundTrip)
{
    // Temporarily use a fixed 64-byte public key;
    // TODO: Implement an SrpKeyPair helper in a later PR.
    uint8_t pubKey[kSrpPublicKeyRawSize];
    for (size_t i = 0; i < sizeof(pubKey); ++i)
    {
        pubKey[i] = static_cast<uint8_t>(i);
    }

    uint8_t buffer[256];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutKey("myhost.default.service.arpa", RecordClass::kIn, 3600, ByteSpan(pubKey)), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kKey);

    uint8_t decoded[kSrpPublicKeyRawSize];
    size_t decodedLen = 0;
    ASSERT_EQ(reader.ReadKey(header.rdlength, decoded, sizeof(decoded), decodedLen), CHIP_NO_ERROR);
    EXPECT_EQ(decodedLen, kSrpPublicKeyRawSize);
    EXPECT_EQ(memcmp(decoded, pubKey, kSrpPublicKeyRawSize), 0);
}

TEST(TestSrpDnsMessage, ResponseHeaderFlags)
{
    const uint16_t flags = MakeFlags(kOpcodeUpdate, true, ResponseCode::kYxDomain);
    uint8_t buffer[kDnsHeaderSize];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutHeader(0xABCD, flags, 0, 0, 0, 0), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    Header header;
    ASSERT_EQ(reader.ReadHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.id, 0xABCD);
    EXPECT_TRUE(header.IsResponse());
    EXPECT_EQ(header.Opcode(), kOpcodeUpdate);
    EXPECT_EQ(header.Rcode(), ResponseCode::kYxDomain);
}

TEST(TestSrpDnsMessage, PatchHeaderCounts)
{
    uint8_t buffer[256];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutHeader(1, MakeFlags(kOpcodeUpdate), 0, 0, 0, 0), CHIP_NO_ERROR);
    ASSERT_EQ(writer.PutQuestion("default.service.arpa", RecordType::kSoa, RecordClass::kIn), CHIP_NO_ERROR);

    Inet::IPAddress address;
    ASSERT_TRUE(Inet::IPAddress::FromString("2001:db8::1", address));
    ASSERT_EQ(writer.PutAaaa("host.default.service.arpa", RecordClass::kIn, 120, address), CHIP_NO_ERROR);

    ASSERT_EQ(writer.PatchHeaderCounts(1, 0, 1, 0), CHIP_NO_ERROR);
    ASSERT_TRUE(writer.Ok());

    DnsReader reader(buffer, writer.Length());
    Header header;
    ASSERT_EQ(reader.ReadHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.qdcount, 1);
    EXPECT_EQ(header.ancount, 0);
    EXPECT_EQ(header.nscount, 1);
    EXPECT_EQ(header.arcount, 0);

    ASSERT_EQ(reader.SkipQuestion(), CHIP_NO_ERROR);
    DnsReader::RecordHeader rr;
    ASSERT_EQ(reader.ReadRecordHeader(rr), CHIP_NO_ERROR);
    EXPECT_EQ(rr.type, RecordType::kAaaa);
    EXPECT_STREQ(rr.name, "host.default.service.arpa");
}

TEST(TestSrpDnsMessage, DotRootAndTrailingDotNames)
{
    uint8_t buffer[64];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutName("."), CHIP_NO_ERROR);
    ASSERT_EQ(writer.PutName("host.default.service.arpa."), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    char name[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "");
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "host.default.service.arpa");
}

TEST(TestSrpDnsMessage, EmptyTxtRecord)
{
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutTxt("inst._matter._tcp.default.service.arpa", RecordClass::kIn, 3600, nullptr, 0), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kTxt);
    EXPECT_EQ(header.rdlength, 1);
    EXPECT_EQ(reader.Data()[header.rdataOffset], 0);
}

TEST(TestSrpDnsMessage, SkipQuestionAndRecordData)
{
    uint8_t buffer[256];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutHeader(2, MakeFlags(kOpcodeUpdate), 1, 0, 2, 0), CHIP_NO_ERROR);
    ASSERT_EQ(writer.PutQuestion("default.service.arpa", RecordType::kSoa, RecordClass::kIn), CHIP_NO_ERROR);
    ASSERT_EQ(writer.PutDeleteRRset("old._matter._tcp.default.service.arpa", RecordType::kSrv), CHIP_NO_ERROR);
    ASSERT_EQ(
        writer.PutSrv("new._matter._tcp.default.service.arpa", RecordClass::kIn, 3600, 0, 0, 5540, "host.default.service.arpa"),
        CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    Header header;
    ASSERT_EQ(reader.ReadHeader(header), CHIP_NO_ERROR);
    ASSERT_EQ(reader.SkipQuestion(), CHIP_NO_ERROR);

    DnsReader::RecordHeader deleteRr;
    ASSERT_EQ(reader.ReadRecordHeader(deleteRr), CHIP_NO_ERROR);
    EXPECT_EQ(deleteRr.recordClass, RecordClass::kAny);
    ASSERT_EQ(reader.SkipRecordData(deleteRr), CHIP_NO_ERROR);

    DnsReader::RecordHeader srvRr;
    ASSERT_EQ(reader.ReadRecordHeader(srvRr), CHIP_NO_ERROR);
    EXPECT_EQ(srvRr.type, RecordType::kSrv);
    uint16_t priority, weight, port;
    char target[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadSrv(priority, weight, port, target, sizeof(target)), CHIP_NO_ERROR);
    EXPECT_EQ(port, 5540);
    EXPECT_STREQ(target, "host.default.service.arpa");
}

TEST(TestSrpDnsMessage, PartialCompressedNameDecode)
{
    // Wire: "x.y" at offset 12, then "z" + pointer-to-12 => "z.x.y".
    uint8_t buffer[64];
    memset(buffer, 0, sizeof(buffer));
    size_t o    = 12;
    buffer[o++] = 1;
    buffer[o++] = 'x';
    buffer[o++] = 1;
    buffer[o++] = 'y';
    buffer[o++] = 0;

    size_t partialPos = o;
    buffer[o++]       = 1;
    buffer[o++]       = 'z';
    buffer[o++]       = 0xC0;
    buffer[o++]       = 12;

    DnsReader reader(buffer, o);
    reader.Seek(partialPos);
    char name[kMaxDottedNameSize];
    ASSERT_EQ(reader.ReadName(name, sizeof(name)), CHIP_NO_ERROR);
    EXPECT_STREQ(name, "z.x.y");
    EXPECT_EQ(reader.Offset(), partialPos + 4);
}

TEST(TestSrpDnsMessage, RejectsForwardCompressionPointer)
{
    uint8_t buffer[16] = {};
    buffer[0]          = 0xC0;
    buffer[1]          = 4; // pointer to a later offset
    buffer[4]          = 1;
    buffer[5]          = 'a';
    buffer[6]          = 0;

    DnsReader reader(buffer, sizeof(buffer));
    char name[kMaxDottedNameSize];
    EXPECT_EQ(reader.ReadName(name, sizeof(name)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSrpDnsMessage, RejectsOversizedLabel)
{
    char label[80];
    memset(label, 'a', sizeof(label) - 1);
    label[sizeof(label) - 1] = '\0';

    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    EXPECT_EQ(writer.PutName(label), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSrpDnsMessage, RejectsNullName)
{
    uint8_t buffer[16];
    DnsWriter writer(buffer, sizeof(buffer));
    EXPECT_EQ(writer.PutName(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSrpDnsMessage, FinishRecordWithoutStartFails)
{
    uint8_t buffer[32];
    DnsWriter writer(buffer, sizeof(buffer));
    EXPECT_EQ(writer.FinishRecord(), CHIP_ERROR_INCORRECT_STATE);
}

TEST(TestSrpDnsMessage, NestedStartRecordFails)
{
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.StartRecord("host.default.service.arpa", RecordType::kAaaa, RecordClass::kIn, 120), CHIP_NO_ERROR);
    EXPECT_EQ(writer.StartRecord("other.default.service.arpa", RecordType::kAaaa, RecordClass::kIn, 120),
              CHIP_ERROR_INCORRECT_STATE);
}

TEST(TestSrpDnsMessage, TruncatedHeaderReadFails)
{
    uint8_t buffer[kDnsHeaderSize];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.PutHeader(1, 0, 0, 0, 0, 0), CHIP_NO_ERROR);

    DnsReader reader(buffer, kDnsHeaderSize - 1);
    Header header;
    EXPECT_EQ(reader.ReadHeader(header), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST(TestSrpDnsMessage, ReadKeyRejectsShortRdata)
{
    uint8_t buffer[8] = { 0x02, 0x00, 0x03, 0x0D }; // flags/protocol/algorithm only
    DnsReader reader(buffer, sizeof(buffer));
    uint8_t key[kSrpPublicKeyRawSize];
    size_t keyLen = 0;
    EXPECT_EQ(reader.ReadKey(3, key, sizeof(key), keyLen), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST(TestSrpDnsMessage, DeleteSpecificRrUsesClassNone)
{
    // RFC 2136 "delete an RR from an RRset": CLASS NONE, TTL 0, matching RDATA.
    uint8_t buffer[128];
    DnsWriter writer(buffer, sizeof(buffer));
    ASSERT_EQ(writer.StartRecord("host.default.service.arpa", RecordType::kAaaa, RecordClass::kNone, 0), CHIP_NO_ERROR);
    Inet::IPAddress address;
    ASSERT_TRUE(Inet::IPAddress::FromString("fe80::1", address));
    ASSERT_EQ(writer.PutBytes(reinterpret_cast<const uint8_t *>(address.Addr), 16), CHIP_NO_ERROR);
    ASSERT_EQ(writer.FinishRecord(), CHIP_NO_ERROR);

    DnsReader reader(buffer, writer.Length());
    DnsReader::RecordHeader header;
    ASSERT_EQ(reader.ReadRecordHeader(header), CHIP_NO_ERROR);
    EXPECT_EQ(header.type, RecordType::kAaaa);
    EXPECT_EQ(header.recordClass, RecordClass::kNone);
    EXPECT_EQ(header.ttl, 0u);
    EXPECT_EQ(header.rdlength, 16);
}

} // namespace
