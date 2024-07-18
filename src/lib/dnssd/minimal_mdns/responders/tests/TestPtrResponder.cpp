/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>

#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>

namespace {

using namespace chip;
using namespace chip::Encoding;
using namespace chip::Inet;
using namespace mdns::Minimal;

constexpr uint16_t kMdnsPort = 5353;

const QNamePart kNames[]       = { "some", "test", "local" };
const QNamePart kTargetNames[] = { "point", "to", "this" };

class PtrResponseAccumulator : public ResponderDelegate
{
public:
    PtrResponseAccumulator(const uint32_t expectedTtl) : mExpectedTtl(expectedTtl) {}
    void AddResponse(const ResourceRecord & record) override
    {

        EXPECT_EQ(record.GetType(), QType::PTR);
        EXPECT_EQ(record.GetClass(), QClass::IN);
        EXPECT_EQ(record.GetName(), kNames);

        if (record.GetType() == QType::PTR)
        {
            uint8_t headerBuffer[HeaderRef::kSizeBytes];
            uint8_t buffer[128];

            BigEndian::BufferWriter out(buffer, sizeof(buffer));
            RecordWriter writer(&out);

            HeaderRef hdr(headerBuffer);
            hdr.Clear();

            EXPECT_TRUE(record.Append(hdr, ResourceType::kAnswer, writer));

            ResourceData data;
            SerializedQNameIterator target;
            const uint8_t * start = buffer;

            EXPECT_TRUE(out.Fit());

            BytesRange validDataRange(buffer, buffer + out.Needed());

            EXPECT_TRUE(data.Parse(validDataRange, &start));
            EXPECT_EQ(start, (buffer + out.Needed()));
            EXPECT_EQ(data.GetName(), FullQName(kNames));
            EXPECT_EQ(data.GetType(), QType::PTR);
            EXPECT_EQ(data.GetTtlSeconds(), mExpectedTtl);

            EXPECT_TRUE(ParsePtrRecord(data.GetData(), validDataRange, &target));
            EXPECT_EQ(target, FullQName(kTargetNames));
        }
    }

private:
    const uint32_t mExpectedTtl;
};

TEST(TestPtrResponder, TestPtrResponse)
{
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("2607:f8b0:4005:804::200e", ipAddress));

    PtrResponder responder(kNames, kTargetNames);

    EXPECT_EQ(responder.GetQClass(), QClass::IN);
    EXPECT_EQ(responder.GetQType(), QType::PTR);
    EXPECT_EQ(responder.GetQName(), kNames);

    PtrResponseAccumulator acc(ResourceRecord::kDefaultTtl);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = InterfaceId::Null();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}

TEST(TestPtrResponder, TestPtrResponseOverrideTtl)
{
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("2607:f8b0:4005:804::200e", ipAddress));

    PtrResponder responder(kNames, kTargetNames);

    EXPECT_EQ(responder.GetQClass(), QClass::IN);
    EXPECT_EQ(responder.GetQType(), QType::PTR);
    EXPECT_EQ(responder.GetQName(), kNames);

    PtrResponseAccumulator acc(123);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = InterfaceId::Null();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration().SetTtlSecondsOverride(123));
}
} // namespace
