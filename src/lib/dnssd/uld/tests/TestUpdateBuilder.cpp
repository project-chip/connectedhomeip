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
#include <lib/dnssd/uld/DeleteRecord.h>
#include <lib/dnssd/uld/KeyRecord.h>
#include <lib/dnssd/uld/OptRecord.h>
#include <lib/dnssd/uld/Sig0Record.h>
#include <lib/dnssd/uld/UpdateBuilder.h>
#include <lib/dnssd/wire/records/Ptr.h>
#include <lib/dnssd/wire/records/Srv.h>

#include <cstring>

namespace {

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Uld;

const QNamePart kZone[]     = { "default", "service", "arpa" };
const QNamePart kService[]  = { "foo", "_matterc", "_udp", "local" };
const QNamePart kInstance[] = { "inst", "_matterc", "_udp", "local" };
const QNamePart kHost[]     = { "host", "local" };

TEST(TestUpdateBuilder, BeginSetsOpcodeUpdate)
{
    uint8_t buffer[256];
    UpdateBuilder builder(buffer, sizeof(buffer));

    builder.Begin(0x1234);

    EXPECT_EQ(builder.Header().GetMessageId(), 0x1234);
    EXPECT_TRUE(builder.Header().GetFlags().IsQuery());
    EXPECT_EQ(builder.Header().GetFlags().GetOpcode(), Opcode::kUpdate);

    ByteSpan packet;
    EXPECT_EQ(builder.GetPacket(packet), CHIP_NO_ERROR);
    EXPECT_EQ(packet.size(), HeaderRef::kSizeBytes);
}

TEST(TestUpdateBuilder, MinimalUpdateWithZoneDeleteAddOptAndSig)
{
    uint8_t buffer[512];
    uint8_t signature[kP256RawSignatureSize] = {};

    Crypto::P256Keypair keypair;
    ASSERT_EQ(keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(0xABCD);

    DeleteRrsetRecord delAny(kHost, QType::ANY);
    PtrResourceRecord ptr(kService, kInstance);
    SrvResourceRecord srv(kInstance, kHost, 5540);
    KeyResourceRecord key(kHost, keypair.Pubkey());
    OptLeaseRecord opt(/*udpPayloadSize=*/1232, /*leaseSeconds=*/7200, /*keyLeaseSeconds=*/86400);
    Sig0ResourceRecord sig(kHost, ByteSpan(signature));

    builder.AddZone(kZone);
    builder.AddUpdate(delAny);
    builder.AddUpdate(ptr);
    builder.AddUpdate(srv);
    builder.AddUpdate(key);
    builder.AddAdditional(opt);
    builder.AddAdditional(sig);

    EXPECT_EQ(builder.Header().GetQueryCount(), 1);      // ZONE
    EXPECT_EQ(builder.Header().GetAnswerCount(), 0);     // Prerequisite
    EXPECT_EQ(builder.Header().GetAuthorityCount(), 4);  // Update
    EXPECT_EQ(builder.Header().GetAdditionalCount(), 2); // OPT + SIG

    // Header flags: opcode UPDATE at offset 2.
    EXPECT_EQ(buffer[2], 0x28); // QR=0, OPCODE=5
    EXPECT_EQ(buffer[3], 0x00);

    ByteSpan packet;
    EXPECT_EQ(builder.GetPacket(packet), CHIP_NO_ERROR);
    EXPECT_GT(packet.size(), HeaderRef::kSizeBytes);

    ByteSpan samePacket;
    EXPECT_EQ(builder.GetPacket(samePacket), CHIP_NO_ERROR);
    EXPECT_EQ(samePacket.data(), packet.data());
    EXPECT_EQ(samePacket.size(), packet.size());
}

TEST(TestUpdateBuilder, ReportsBufferTooSmall)
{
    uint8_t buffer[HeaderRef::kSizeBytes];
    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(1);

    builder.AddZone(kZone);

    ByteSpan packet;
    EXPECT_EQ(builder.GetPacket(packet), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST(TestUpdateBuilder, BeginClearsBufferOverflow)
{
    uint8_t buffer[64];
    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(1);
    builder.AddZone(kZone);

    DeleteRrsetRecord record(kHost, QType::TXT);
    builder.AddUpdate(record);
    builder.AddUpdate(record);

    ByteSpan packet;
    EXPECT_EQ(builder.GetPacket(packet), CHIP_ERROR_BUFFER_TOO_SMALL);

    builder.Begin(2);
    EXPECT_EQ(builder.GetPacket(packet), CHIP_NO_ERROR);
    EXPECT_EQ(packet.size(), HeaderRef::kSizeBytes);
    EXPECT_EQ(builder.Header().GetMessageId(), 2);
}

} // namespace
