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

    EXPECT_TRUE(builder.Ok());
    builder.Begin(0x1234);

    EXPECT_TRUE(builder.Ok());
    EXPECT_EQ(builder.Header().GetMessageId(), 0x1234);
    EXPECT_TRUE(builder.Header().GetFlags().IsQuery());
    EXPECT_EQ(builder.Header().GetFlags().GetOpcode(), Opcode::kUpdate);
    EXPECT_EQ(builder.PacketSize(), HeaderRef::kSizeBytes);
}

TEST(TestUpdateBuilder, MinimalUpdateWithZoneDeleteAddOptAndSig)
{
    uint8_t buffer[512];
    uint8_t rawKey[kP256RawPublicKeySize];
    uint8_t signature[kP256RawSignatureSize] = {};

    for (size_t i = 0; i < sizeof(rawKey); i++)
    {
        rawKey[i] = static_cast<uint8_t>(i + 1);
    }

    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(0xABCD);

    DeleteRrsetRecord delAny(kHost, QType::ANY);
    PtrResourceRecord ptr(kService, kInstance);
    SrvResourceRecord srv(kInstance, kHost, 5540);
    KeyResourceRecord key(kHost, ByteSpan(rawKey));
    OptLeaseRecord opt(/*udpPayloadSize=*/1232, /*leaseSeconds=*/7200, /*keyLeaseSeconds=*/86400);
    Sig0ResourceRecord sig(kHost, ByteSpan(signature));

    builder.AddZone(kZone).AddUpdate(delAny).AddUpdate(ptr).AddUpdate(srv).AddUpdate(key).AddAdditional(opt).AddAdditional(sig);

    EXPECT_TRUE(builder.Ok());
    EXPECT_EQ(builder.Header().GetQueryCount(), 1);      // ZONE
    EXPECT_EQ(builder.Header().GetAnswerCount(), 0);     // Prerequisite
    EXPECT_EQ(builder.Header().GetAuthorityCount(), 4);  // Update
    EXPECT_EQ(builder.Header().GetAdditionalCount(), 2); // OPT + SIG

    // Header flags: opcode UPDATE at offset 2.
    EXPECT_EQ(buffer[2], 0x28); // QR=0, OPCODE=5
    EXPECT_EQ(buffer[3], 0x00);
}

TEST(TestUpdateBuilder, RejectsUpdateBeforeZone)
{
    uint8_t buffer[256];
    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(1);

    // Query::Append requires no records yet; adding an update first is fine at the
    // ResourceRecord layer, but AddZone after records must fail.
    DeleteRrsetRecord delAny(kHost, QType::TXT);
    builder.AddUpdate(delAny);
    EXPECT_TRUE(builder.Ok());

    builder.AddZone(kZone);
    EXPECT_FALSE(builder.Ok());
}

TEST(TestUpdateBuilder, SectionOrderAnswerBeforeAuthority)
{
    uint8_t buffer[256];
    UpdateBuilder builder(buffer, sizeof(buffer));
    builder.Begin(1).AddZone(kZone);

    DeleteRrsetRecord delAny(kHost, QType::TXT);
    builder.AddUpdate(delAny);
    EXPECT_TRUE(builder.Ok());

    // Prerequisite (answer) after update (authority) is rejected by ResourceRecord::Append.
    DeleteRrsetRecord prereq(kHost, QType::TXT);
    builder.AddPrerequisite(prereq);
    EXPECT_FALSE(builder.Ok());
}

} // namespace
