/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/dnssd/minimal_mdns/ResponseSender.h>

#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/core/FlatAllocatedQName.h>
#include <lib/dnssd/minimal_mdns/core/RecordWriter.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/dnssd/minimal_mdns/tests/CheckOnlyServer.h>
#include <lib/support/CHIPMem.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;
using namespace mdns::Minimal::test;

struct CommonTestElements
{
    uint8_t requestStorage[64];
    BytesRange requestBytesRange = BytesRange(requestStorage, requestStorage + sizeof(requestStorage));
    HeaderRef header             = HeaderRef(requestStorage);
    uint8_t * requestNameStart   = requestStorage + ConstHeaderRef::kSizeBytes;
    Encoding::BigEndian::BufferWriter requestBufferWriter =
        Encoding::BigEndian::BufferWriter(requestNameStart, sizeof(requestStorage) - HeaderRef::kSizeBytes);
    RecordWriter recordWriter;

    uint8_t dnsSdServiceStorage[64];
    uint8_t serviceNameStorage[64];
    uint8_t instanceNameStorage[64];
    uint8_t hostNameStorage[64];
    uint8_t txtStorage[64];
    FullQName dnsSd;
    FullQName service;
    FullQName instance;
    FullQName host;
    FullQName txt;

    static constexpr uint16_t kPort = 54;
    PtrResourceRecord ptrRecord     = PtrResourceRecord(service, instance);
    PtrResponder ptrResponder       = PtrResponder(service, instance);
    SrvResourceRecord srvRecord     = SrvResourceRecord(instance, host, kPort);
    SrvResponder srvResponder       = SrvResourceRecord(srvRecord);
    TxtResourceRecord txtRecord     = TxtResourceRecord(instance, txt);
    TxtResponder txtResponder       = TxtResponder(txtRecord);

    CheckOnlyServer server;
    QueryResponder<10> queryResponder;
    Inet::IPPacketInfo packetInfo;

    CommonTestElements(const char * tag) :
        recordWriter(&requestBufferWriter),
        dnsSd(FlatAllocatedQName::Build(dnsSdServiceStorage, "_services", "_dns-sd", "_udp", "local")),
        service(FlatAllocatedQName::Build(serviceNameStorage, tag, "service")),
        instance(FlatAllocatedQName::Build(instanceNameStorage, tag, "instance")),
        host(FlatAllocatedQName::Build(hostNameStorage, tag, "host")),
        txt(FlatAllocatedQName::Build(txtStorage, tag, "L1=something", "L2=other")), server()
    {
        queryResponder.Init();
        header.SetQueryCount(1);
    }
};

class TestResponseSender : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestResponseSender, SrvAnyResponseToInstance)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.srvResponder);

    // Build a query for our srv record
    common.recordWriter.WriteQName(common.instance);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    common.server.AddExpectedRecord(&common.srvRecord);
    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, SrvTxtAnyResponseToInstance)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.recordWriter.WriteQName(common.instance);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We requested ANY on the host name, expect both back.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);
    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, PtrSrvTxtAnyResponseToServiceName)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportAdditional(common.instance);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the service name
    common.recordWriter.WriteQName(common.service);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We should get all because we request to report all instance names when the PTR is sent.
    common.server.AddExpectedRecord(&common.ptrRecord);
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);

    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, PtrSrvTxtAnyResponseToInstance)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.recordWriter.WriteQName(common.instance);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We shouldn't get back the PTR.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);

    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, PtrSrvTxtSrvResponseToInstance)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance
    common.recordWriter.WriteQName(common.instance);

    QueryData queryData = QueryData(QType::SRV, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We didn't set the txt as an additional on the srv name so expect only srv.
    common.server.AddExpectedRecord(&common.srvRecord);

    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, PtrSrvTxtAnyResponseToServiceListing)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);
    EXPECT_EQ(responseSender.AddQueryResponder(&common.queryResponder), CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the dns-sd services listing.
    common.recordWriter.WriteQName(common.dnsSd);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // Only one PTR in service listing.
    PtrResourceRecord serviceRecord = PtrResourceRecord(common.dnsSd, common.ptrRecord.GetName());
    common.server.AddExpectedRecord(&serviceRecord);

    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common.server.GetSendCalled());
    EXPECT_TRUE(common.server.GetHeaderFound());
}

TEST_F(TestResponseSender, NoQueryResponder)
{
    CommonTestElements common("test");
    ResponseSender responseSender(&common.server);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    common.recordWriter.WriteQName(common.dnsSd);
    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());
    EXPECT_FALSE(common.server.GetSendCalled());

    common.recordWriter.WriteQName(common.service);
    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());
    EXPECT_FALSE(common.server.GetSendCalled());

    common.recordWriter.WriteQName(common.instance);
    responseSender.Respond(1, queryData, &common.packetInfo, ResponseConfiguration());
    EXPECT_FALSE(common.server.GetSendCalled());
}

TEST_F(TestResponseSender, AddManyQueryResponders)
{
    // TODO(cecille): Fix this test once #8000 gets resolved.
    ResponseSender responseSender(nullptr);
    QueryResponder<1> q1;
    QueryResponder<1> q2;
    QueryResponder<1> q3;
    QueryResponder<1> q4;
    QueryResponder<1> q5;
    QueryResponder<1> q6;
    QueryResponder<1> q7;

    // We should be able to re-add the same query responder as many times as we want.
    // and it shold only count as one
    constexpr size_t kAddLoopSize = 1000;
    for (size_t i = 0; i < kAddLoopSize; ++i)
    {
        EXPECT_EQ(responseSender.AddQueryResponder(&q1), CHIP_NO_ERROR);
    }

    // removing the only copy should clear out everything
    responseSender.RemoveQueryResponder(&q1);
    EXPECT_FALSE(responseSender.HasQueryResponders());

    // At least 7 should be supported:
    //   - 5 is the spec minimum
    //   - 2 for commissionable and commisioner responders
    EXPECT_EQ(responseSender.AddQueryResponder(&q1), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q2), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q3), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q4), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q5), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q6), CHIP_NO_ERROR);
    EXPECT_EQ(responseSender.AddQueryResponder(&q7), CHIP_NO_ERROR);
}

TEST_F(TestResponseSender, PtrSrvTxtMultipleRespondersToInstance)
{
    auto common1 = std::make_unique<CommonTestElements>("test1");
    auto common2 = std::make_unique<CommonTestElements>("test2");

    // Just use the server from common1.
    ResponseSender responseSender(&common1->server);

    EXPECT_EQ(responseSender.AddQueryResponder(&common1->queryResponder), CHIP_NO_ERROR);
    common1->queryResponder.AddResponder(&common1->ptrResponder).SetReportInServiceListing(true);
    common1->queryResponder.AddResponder(&common1->srvResponder);
    common1->queryResponder.AddResponder(&common1->txtResponder);

    EXPECT_EQ(responseSender.AddQueryResponder(&common2->queryResponder), CHIP_NO_ERROR);
    common2->queryResponder.AddResponder(&common2->ptrResponder).SetReportInServiceListing(true);
    common2->queryResponder.AddResponder(&common2->srvResponder);
    common2->queryResponder.AddResponder(&common2->txtResponder);

    // Build a query for the second instance.
    common2->recordWriter.WriteQName(common2->instance);
    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common2->requestNameStart, common2->requestBytesRange);

    // Should get back answers from second instance only.
    common1->server.AddExpectedRecord(&common2->srvRecord);
    common1->server.AddExpectedRecord(&common2->txtRecord);

    responseSender.Respond(1, queryData, &common1->packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common1->server.GetSendCalled());
    EXPECT_TRUE(common1->server.GetHeaderFound());
}

TEST_F(TestResponseSender, PtrSrvTxtMultipleRespondersToServiceListing)
{
    auto common1 = std::make_unique<CommonTestElements>("test1");
    auto common2 = std::make_unique<CommonTestElements>("test2");
    // Just use the server from common1.
    ResponseSender responseSender(&common1->server);

    EXPECT_EQ(responseSender.AddQueryResponder(&common1->queryResponder), CHIP_NO_ERROR);
    common1->queryResponder.AddResponder(&common1->ptrResponder).SetReportInServiceListing(true);
    common1->queryResponder.AddResponder(&common1->srvResponder);
    common1->queryResponder.AddResponder(&common1->txtResponder);

    EXPECT_EQ(responseSender.AddQueryResponder(&common2->queryResponder), CHIP_NO_ERROR);
    common2->queryResponder.AddResponder(&common2->ptrResponder).SetReportInServiceListing(true);
    common2->queryResponder.AddResponder(&common2->srvResponder);
    common2->queryResponder.AddResponder(&common2->txtResponder);

    // Build a query for the instance
    common1->recordWriter.WriteQName(common1->dnsSd);
    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common1->requestNameStart, common1->requestBytesRange);

    // Should get service listing from both.
    PtrResourceRecord serviceRecord1 = PtrResourceRecord(common1->dnsSd, common1->ptrRecord.GetName());
    common1->server.AddExpectedRecord(&serviceRecord1);
    PtrResourceRecord serviceRecord2 = PtrResourceRecord(common2->dnsSd, common2->ptrRecord.GetName());
    common1->server.AddExpectedRecord(&serviceRecord2);

    responseSender.Respond(1, queryData, &common1->packetInfo, ResponseConfiguration());

    EXPECT_TRUE(common1->server.GetSendCalled());

    EXPECT_TRUE(common1->server.GetHeaderFound());
}

} // namespace
