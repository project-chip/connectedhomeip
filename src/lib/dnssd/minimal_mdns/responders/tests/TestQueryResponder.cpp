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
#include <lib/dnssd/minimal_mdns/responders/QueryResponder.h>

#include <vector>

#include <lib/dnssd/minimal_mdns/records/Ptr.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

const QNamePart kDnsSdname[] = { "_services", "_dns-sd", "_udp", "local" };

const QNamePart kName1[] = { "some", "test" };
const QNamePart kName2[] = { "some", "other", "test" };

class EmptyResponder : public RecordResponder
{
public:
    EmptyResponder(const FullQName & qName) : RecordResponder(QType::NULLVALUE, qName) {}
    void AddAllResponses(const chip::Inet::IPPacketInfo *, ResponderDelegate *, const ResponseConfiguration &) override {}
};

class DnssdReplyAccumulator : public ResponderDelegate
{
public:
    DnssdReplyAccumulator(nlTestSuite * suite) : mSuite(suite) {}
    void AddResponse(const ResourceRecord & record) override
    {

        NL_TEST_ASSERT(mSuite, record.GetType() == QType::PTR);
        NL_TEST_ASSERT(mSuite, record.GetClass() == QClass::IN);
        NL_TEST_ASSERT(mSuite, record.GetName() == kDnsSdname);

        mCaptures.push_back(reinterpret_cast<const PtrResourceRecord &>(record).GetPtr());
    }

    std::vector<FullQName> & Captures() { return mCaptures; }

private:
    nlTestSuite * mSuite;
    std::vector<FullQName> mCaptures;
};

void CanIterateOverResponders(nlTestSuite * inSuite, void * inContext)
{
    QueryResponder<10> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);
    EmptyResponder empty3(kName2);

    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty1).SetReportInServiceListing(true).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty3).SetReportInServiceListing(true).IsValid());

    int idx = 0;
    QueryResponderRecordFilter noFilter;
    for (auto it = responder.begin(&noFilter); it != responder.end(); it++, idx++)
    {
        FullQName qName = it->responder->GetQName();
        NL_TEST_ASSERT(inSuite, (idx != 0) || (qName == kDnsSdname));
        NL_TEST_ASSERT(inSuite, (idx != 1) || (qName == kName1));
        NL_TEST_ASSERT(inSuite, (idx != 2) || (qName == kName2));
        NL_TEST_ASSERT(inSuite, (idx != 3) || (qName == kName2));
    }
    NL_TEST_ASSERT(inSuite, idx == 4);
}

void RespondsToDnsSdQueries(nlTestSuite * inSuite, void * inContext)
{
    QueryResponder<10> responder;
    QueryResponderRecordFilter noFilter;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);
    EmptyResponder empty3(kName1);
    EmptyResponder empty4(kName1);

    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty1).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty3).SetReportInServiceListing(true).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty4).IsValid());

    // It reports itself inside the iterator
    NL_TEST_ASSERT(inSuite, &(*responder.begin(&noFilter)->responder) == &responder);

    // It reponds dnssd PTR answers
    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::PTR);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kDnsSdname);

    DnssdReplyAccumulator accumulator(inSuite);
    responder.AddAllResponses(nullptr, &accumulator, ResponseConfiguration());

    NL_TEST_ASSERT(inSuite, accumulator.Captures().size() == 2);
    if (accumulator.Captures().size() == 2)
    {
        NL_TEST_ASSERT(inSuite, accumulator.Captures()[0] == kName2);
        NL_TEST_ASSERT(inSuite, accumulator.Captures()[1] == kName1);
    }
}

void LimitedStorage(nlTestSuite * inSuite, void * inContext)
{
    QueryResponder<3> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);

    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty1).SetReportInServiceListing(true).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());

    for (int i = 0; i < 100; i++)
    {
        EmptyResponder emptyX(kName1);
        NL_TEST_ASSERT(inSuite, !responder.AddResponder(&emptyX).SetReportInServiceListing(true).IsValid());
    }

    int idx = 0;
    QueryResponderRecordFilter noFilter;
    for (auto it = responder.begin(&noFilter); it != responder.end(); it++, idx++)
    {
        FullQName qName = it->responder->GetQName();
        NL_TEST_ASSERT(inSuite, (idx != 0) || (qName == kDnsSdname));
        NL_TEST_ASSERT(inSuite, (idx != 1) || (qName == kName1));
        NL_TEST_ASSERT(inSuite, (idx != 2) || (qName == kName2));
    }
    NL_TEST_ASSERT(inSuite, idx == 3);
}

void NonDiscoverableService(nlTestSuite * inSuite, void * inContext)
{
    QueryResponder<3> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);

    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty1).IsValid());
    NL_TEST_ASSERT(inSuite, responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());

    DnssdReplyAccumulator accumulator(inSuite);
    responder.AddAllResponses(nullptr, &accumulator, ResponseConfiguration());

    NL_TEST_ASSERT(inSuite, accumulator.Captures().size() == 1);
    if (accumulator.Captures().size() == 1)
    {
        NL_TEST_ASSERT(inSuite, accumulator.Captures()[0] == kName2);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("CanIterateOverResponders", CanIterateOverResponders), //
    NL_TEST_DEF("RespondsToDnsSdQueries", RespondsToDnsSdQueries),     //
    NL_TEST_DEF("LimitedStorage", LimitedStorage),                     //
    NL_TEST_DEF("NonDiscoverableService", NonDiscoverableService),     //
    NL_TEST_SENTINEL()                                                 //
};

} // namespace

int TestQueryResponder()
{
    nlTestSuite theSuite = { "QueryResponder", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestQueryResponder)
