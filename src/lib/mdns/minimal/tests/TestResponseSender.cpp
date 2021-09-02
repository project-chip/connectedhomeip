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
#include <lib/mdns/minimal/ResponseSender.h>

#include <string>
#include <vector>

#include <lib/mdns/minimal/RecordData.h>
#include <lib/mdns/minimal/core/FlatAllocatedQName.h>
#include <lib/mdns/minimal/responders/Ptr.h>
#include <lib/mdns/minimal/responders/Srv.h>
#include <lib/mdns/minimal/responders/Txt.h>
#include <lib/mdns/minimal/tests/CheckOnlyServer.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace std;
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

    CommonTestElements(nlTestSuite * inSuite, const char * tag) :
        dnsSd(FlatAllocatedQName::Build(dnsSdServiceStorage, "_services", "_dns-sd", "_udp", "local")),
        service(FlatAllocatedQName::Build(serviceNameStorage, tag, "service")),
        instance(FlatAllocatedQName::Build(instanceNameStorage, tag, "instance")),
        host(FlatAllocatedQName::Build(hostNameStorage, tag, "host")),
        txt(FlatAllocatedQName::Build(txtStorage, tag, "L1=something", "L2=other")), server(inSuite)
    {
        queryResponder.Init();
        header.SetQueryCount(1);
    }
};

void SrvAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.srvResponder);

    // Build a query for our srv record
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    common.server.AddExpectedRecord(&common.srvRecord);
    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void SrvTxtAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We requested ANY on the host name, expect both back.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);
    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToServiceName(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportAdditional(common.instance);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the service name
    common.service.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We should get all because we request to report all instance names when teh PTR is sent.
    common.server.AddExpectedRecord(&common.ptrRecord);
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);

    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We shouldn't get back the PTR.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);

    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtSrvResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::SRV, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We didn't set the txt as an additional on the srv name so expect only srv.
    common.server.AddExpectedRecord(&common.srvRecord);

    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToServiceListing(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common.queryResponder) == CHIP_NO_ERROR);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the dns-sd services listing.
    common.dnsSd.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // Only one PTR in service listing.
    PtrResourceRecord serviceRecord = PtrResourceRecord(common.dnsSd, common.ptrRecord.GetName());
    common.server.AddExpectedRecord(&serviceRecord);

    responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void NoQueryResponder(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite, "test");
    ResponseSender responseSender(&common.server);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    common.dnsSd.Output(common.requestBufferWriter);
    responseSender.Respond(1, queryData, &common.packetInfo);
    NL_TEST_ASSERT(inSuite, !common.server.GetSendCalled());

    common.service.Output(common.requestBufferWriter);
    responseSender.Respond(1, queryData, &common.packetInfo);
    NL_TEST_ASSERT(inSuite, !common.server.GetSendCalled());

    common.instance.Output(common.requestBufferWriter);
    responseSender.Respond(1, queryData, &common.packetInfo);
    NL_TEST_ASSERT(inSuite, !common.server.GetSendCalled());
}

void AddManyQueryResponders(nlTestSuite * inSuite, void * inContext)
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
    QueryResponder<1> q8;

    // We should be able to re-add the same query responder as many times as we want.
    for (size_t i = 0; i < ResponseSender::kMaxQueryResponders + 1; ++i)
    {
        NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q1) == CHIP_NO_ERROR);
    }

    // There are 7 total
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q3) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q4) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q5) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q6) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q7) == CHIP_NO_ERROR);

    // Last one should return a no memory error (no space)
    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&q8) == CHIP_ERROR_NO_MEMORY);
}

void PtrSrvTxtMultipleRespondersToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common1(inSuite, "test1");
    CommonTestElements common2(inSuite, "test2");

    // Just use the server from common1.
    ResponseSender responseSender(&common1.server);

    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common1.queryResponder) == CHIP_NO_ERROR);
    common1.queryResponder.AddResponder(&common1.ptrResponder).SetReportInServiceListing(true);
    common1.queryResponder.AddResponder(&common1.srvResponder);
    common1.queryResponder.AddResponder(&common1.txtResponder);

    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common2.queryResponder) == CHIP_NO_ERROR);
    common2.queryResponder.AddResponder(&common2.ptrResponder).SetReportInServiceListing(true);
    common2.queryResponder.AddResponder(&common2.srvResponder);
    common2.queryResponder.AddResponder(&common2.txtResponder);

    // Build a query for the second instance.
    common2.instance.Output(common2.requestBufferWriter);
    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common2.requestNameStart, common2.requestBytesRange);

    // Should get back answers from second instance only.
    common1.server.AddExpectedRecord(&common2.srvRecord);
    common1.server.AddExpectedRecord(&common2.txtRecord);

    responseSender.Respond(1, queryData, &common1.packetInfo);

    NL_TEST_ASSERT(inSuite, common1.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common1.server.GetHeaderFound());
}

void PtrSrvTxtMultipleRespondersToServiceListing(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common1(inSuite, "test1");
    CommonTestElements common2(inSuite, "test2");

    // Just use the server from common1.
    ResponseSender responseSender(&common1.server);

    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common1.queryResponder) == CHIP_NO_ERROR);
    common1.queryResponder.AddResponder(&common1.ptrResponder).SetReportInServiceListing(true);
    common1.queryResponder.AddResponder(&common1.srvResponder);
    common1.queryResponder.AddResponder(&common1.txtResponder);

    NL_TEST_ASSERT(inSuite, responseSender.AddQueryResponder(&common2.queryResponder) == CHIP_NO_ERROR);
    common2.queryResponder.AddResponder(&common2.ptrResponder).SetReportInServiceListing(true);
    common2.queryResponder.AddResponder(&common2.srvResponder);
    common2.queryResponder.AddResponder(&common2.txtResponder);

    // Build a query for the instance
    common1.dnsSd.Output(common1.requestBufferWriter);
    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common1.requestNameStart, common1.requestBytesRange);

    // Should get service listing from both.
    PtrResourceRecord serviceRecord1 = PtrResourceRecord(common1.dnsSd, common1.ptrRecord.GetName());
    common1.server.AddExpectedRecord(&serviceRecord1);
    PtrResourceRecord serviceRecord2 = PtrResourceRecord(common2.dnsSd, common2.ptrRecord.GetName());
    common1.server.AddExpectedRecord(&serviceRecord2);

    responseSender.Respond(1, queryData, &common1.packetInfo);

    NL_TEST_ASSERT(inSuite, common1.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common1.server.GetHeaderFound());
}

const nlTest sTests[] = {
    NL_TEST_DEF("SrvAnyResponseToInstance", SrvAnyResponseToInstance),                                       //
    NL_TEST_DEF("SrvTxtAnyResponseToInstance", SrvTxtAnyResponseToInstance),                                 //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToServiceName", PtrSrvTxtAnyResponseToServiceName),                     //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToInstance", PtrSrvTxtAnyResponseToInstance),                           //
    NL_TEST_DEF("PtrSrvTxtSrvResponseToInstance", PtrSrvTxtSrvResponseToInstance),                           //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToServiceListing", PtrSrvTxtAnyResponseToServiceListing),               //
    NL_TEST_DEF("NoQueryResponder", NoQueryResponder),                                                       //
    NL_TEST_DEF("AddManyQueryResponders", AddManyQueryResponders),                                           //
    NL_TEST_DEF("PtrSrvTxtMultipleRespondersToInstance", PtrSrvTxtMultipleRespondersToInstance),             //
    NL_TEST_DEF("PtrSrvTxtMultipleRespondersToServiceListing", PtrSrvTxtMultipleRespondersToServiceListing), //

    NL_TEST_SENTINEL() //
};

} // namespace

int TestResponseSender(void)
{
    chip::Platform::MemoryInit();
    nlTestSuite theSuite = { "RecordData", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestResponseSender)
