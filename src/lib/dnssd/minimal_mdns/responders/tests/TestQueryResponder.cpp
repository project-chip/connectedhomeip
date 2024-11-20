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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>

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
    void AddResponse(const ResourceRecord & record) override
    {

        EXPECT_EQ(record.GetType(), QType::PTR);
        EXPECT_EQ(record.GetClass(), QClass::IN);
        EXPECT_EQ(record.GetName(), kDnsSdname);

        mCaptures.push_back(reinterpret_cast<const PtrResourceRecord &>(record).GetPtr());
    }

    std::vector<FullQName> & Captures() { return mCaptures; }

private:
    std::vector<FullQName> mCaptures;
};

TEST(TestQueryResponder, CanIterateOverResponders)
{
    QueryResponder<10> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);
    EmptyResponder empty3(kName2);

    EXPECT_TRUE(responder.AddResponder(&empty1).SetReportInServiceListing(true).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty3).SetReportInServiceListing(true).IsValid());

    int idx = 0;
    QueryResponderRecordFilter noFilter;
    for (auto it = responder.begin(&noFilter); it != responder.end(); it++, idx++)
    {
        FullQName qName = it->responder->GetQName();
        EXPECT_TRUE((idx != 0) || (qName == kDnsSdname));
        EXPECT_TRUE((idx != 1) || (qName == kName1));
        EXPECT_TRUE((idx != 2) || (qName == kName2));
        EXPECT_TRUE((idx != 3) || (qName == kName2));
    }
    EXPECT_EQ(idx, 4);
}

TEST(TestQueryResponder, RespondsToDnsSdQueries)
{
    QueryResponder<10> responder;
    QueryResponderRecordFilter noFilter;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);
    EmptyResponder empty3(kName1);
    EmptyResponder empty4(kName1);

    EXPECT_TRUE(responder.AddResponder(&empty1).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty3).SetReportInServiceListing(true).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty4).IsValid());

    // It reports itself inside the iterator
    EXPECT_EQ(&(*responder.begin(&noFilter)->responder), &responder);

    // It reponds dnssd PTR answers
    EXPECT_EQ(responder.GetQClass(), QClass::IN);
    EXPECT_EQ(responder.GetQType(), QType::PTR);
    EXPECT_EQ(responder.GetQName(), kDnsSdname);

    DnssdReplyAccumulator accumulator;
    responder.AddAllResponses(nullptr, &accumulator, ResponseConfiguration());

    EXPECT_EQ(accumulator.Captures().size(), 2u);
    if (accumulator.Captures().size() == 2)
    {
        EXPECT_EQ(accumulator.Captures()[0], kName2);
        EXPECT_EQ(accumulator.Captures()[1], kName1);
    }
}

TEST(TestQueryResponder, LimitedStorage)
{
    QueryResponder<3> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);

    EXPECT_TRUE(responder.AddResponder(&empty1).SetReportInServiceListing(true).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());

    for (int i = 0; i < 100; i++)
    {
        EmptyResponder emptyX(kName1);
        EXPECT_FALSE(responder.AddResponder(&emptyX).SetReportInServiceListing(true).IsValid());
    }

    int idx = 0;
    QueryResponderRecordFilter noFilter;
    for (auto it = responder.begin(&noFilter); it != responder.end(); it++, idx++)
    {
        FullQName qName = it->responder->GetQName();
        EXPECT_TRUE((idx != 0) || (qName == kDnsSdname));
        EXPECT_TRUE((idx != 1) || (qName == kName1));
        EXPECT_TRUE((idx != 2) || (qName == kName2));
    }
    EXPECT_EQ(idx, 3);
}

TEST(TestQueryResponder, NonDiscoverableService)
{
    QueryResponder<3> responder;

    EmptyResponder empty1(kName1);
    EmptyResponder empty2(kName2);

    EXPECT_TRUE(responder.AddResponder(&empty1).IsValid());
    EXPECT_TRUE(responder.AddResponder(&empty2).SetReportInServiceListing(true).IsValid());

    DnssdReplyAccumulator accumulator;
    responder.AddAllResponses(nullptr, &accumulator, ResponseConfiguration());

    EXPECT_EQ(accumulator.Captures().size(), 1u);
    if (accumulator.Captures().size() == 1)
    {
        EXPECT_EQ(accumulator.Captures()[0], kName2);
    }
}
} // namespace
