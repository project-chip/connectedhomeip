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

#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

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
    PtrResponseAccumulator(nlTestSuite * suite, const uint32_t expectedTtl) : mSuite(suite), mExpectedTtl(expectedTtl) {}
    void AddResponse(const ResourceRecord & record) override
    {

        NL_TEST_ASSERT(mSuite, record.GetType() == QType::PTR);
        NL_TEST_ASSERT(mSuite, record.GetClass() == QClass::IN);
        NL_TEST_ASSERT(mSuite, record.GetName() == kNames);

        if (record.GetType() == QType::PTR)
        {
            uint8_t headerBuffer[HeaderRef::kSizeBytes];
            uint8_t buffer[128];

            BigEndian::BufferWriter out(buffer, sizeof(buffer));
            RecordWriter writer(&out);

            HeaderRef hdr(headerBuffer);
            hdr.Clear();

            NL_TEST_ASSERT(mSuite, record.Append(hdr, ResourceType::kAnswer, writer));

            ResourceData data;
            SerializedQNameIterator target;
            const uint8_t * start = buffer;

            NL_TEST_ASSERT(mSuite, out.Fit());

            BytesRange validDataRange(buffer, buffer + out.Needed());

            NL_TEST_ASSERT(mSuite, data.Parse(validDataRange, &start));
            NL_TEST_ASSERT(mSuite, start == (buffer + out.Needed()));
            NL_TEST_ASSERT(mSuite, data.GetName() == FullQName(kNames));
            NL_TEST_ASSERT(mSuite, data.GetType() == QType::PTR);
            NL_TEST_ASSERT(mSuite, data.GetTtlSeconds() == mExpectedTtl);

            NL_TEST_ASSERT(mSuite, ParsePtrRecord(data.GetData(), validDataRange, &target));
            NL_TEST_ASSERT(mSuite, target == FullQName(kTargetNames));
        }
    }

private:
    nlTestSuite * mSuite;
    const uint32_t mExpectedTtl;
};

void TestPtrResponse(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("2607:f8b0:4005:804::200e", ipAddress));

    PtrResponder responder(kNames, kTargetNames);

    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::PTR);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kNames);

    PtrResponseAccumulator acc(inSuite, ResourceRecord::kDefaultTtl);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = InterfaceId::Null();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}

void TestPtrResponseOverrideTtl(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("2607:f8b0:4005:804::200e", ipAddress));

    PtrResponder responder(kNames, kTargetNames);

    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::PTR);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kNames);

    PtrResponseAccumulator acc(inSuite, 123);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = InterfaceId::Null();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration().SetTtlSecondsOverride(123));
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestPtrResponse", TestPtrResponse),                       //
    NL_TEST_DEF("TestPtrResponseOverrideTtl", TestPtrResponseOverrideTtl), //
    NL_TEST_SENTINEL()                                                     //
};

} // namespace

int TestPtr()
{
    nlTestSuite theSuite = { "IP", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPtr)
