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
#include <lib/mdns/minimal/responders/Ptr.h>

#include <vector>

#include <lib/mdns/minimal/Parser.h>
#include <lib/mdns/minimal/RecordData.h>
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
    PtrResponseAccumulator(nlTestSuite * suite) : mSuite(suite) {}
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

            HeaderRef hdr(headerBuffer);
            hdr.Clear();

            NL_TEST_ASSERT(mSuite, record.Append(hdr, ResourceType::kAnswer, out));

            ResourceData data;
            SerializedQNameIterator target;
            const uint8_t * start = buffer;

            NL_TEST_ASSERT(mSuite, out.Fit());

            BytesRange validDataRange(buffer, buffer + out.Needed());

            NL_TEST_ASSERT(mSuite, data.Parse(validDataRange, &start));
            NL_TEST_ASSERT(mSuite, start == (buffer + out.Needed()));
            NL_TEST_ASSERT(mSuite, data.GetName() == FullQName(kNames));
            NL_TEST_ASSERT(mSuite, data.GetType() == QType::PTR);

            NL_TEST_ASSERT(mSuite, ParsePtrRecord(data.GetData(), validDataRange, &target));
            NL_TEST_ASSERT(mSuite, target == FullQName(kTargetNames));
        }
    }

private:
    nlTestSuite * mSuite;
};

#if INET_CONFIG_ENABLE_IPV4
void TestPtrResponse(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.20.30.40", ipAddress));

    PtrResponder responder(kNames, kTargetNames);

    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::PTR);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kNames);

    PtrResponseAccumulator acc(inSuite);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = INET_NULL_INTERFACEID;

    responder.AddAllResponses(&packetInfo, &acc);
}
#endif // INET_CONFIG_ENABLE_IPV4

const nlTest sTests[] = {
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("TestPtrResponse", TestPtrResponse), //
#endif                                               // INET_CONFIG_ENABLE_IPV4
    NL_TEST_SENTINEL()                               //
};

} // namespace

int TestPtr(void)
{
    nlTestSuite theSuite = { "IP", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPtr)
