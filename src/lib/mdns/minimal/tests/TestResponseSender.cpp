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
#include <mdns/minimal/ResponseSender.h>

#include <string>
#include <vector>

#include <mdns/minimal/core/FlatAllocatedQName.h>
#include <mdns/minimal/responders/Ptr.h>
#include <mdns/minimal/responders/Srv.h>
#include <mdns/minimal/responders/Txt.h>

#include <support/CHIPMem.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace std;
using namespace chip;
using namespace mdns::Minimal;

class CheckOnlyServer : public ServerBase, public ParserDelegate
{
public:
    CheckOnlyServer(nlTestSuite * inSuite) : ServerBase(nullptr, 0), mInSuite(inSuite) {}
    ~CheckOnlyServer() {}

    void OnHeader(ConstHeaderRef & header) override
    {
        NL_TEST_ASSERT(mInSuite, header.GetFlags().IsResponse());
        NL_TEST_ASSERT(mInSuite, header.GetFlags().IsValidMdns());
        NL_TEST_ASSERT(mInSuite, header.GetAnswerCount() + header.GetAdditionalCount() == GetNumExpectedRecords());
        headerFound = true;
    }

    void OnResource(ResourceType type, const ResourceData & data) override
    {
        bool recordIsExpected = false;
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (expectedRecord[i] == nullptr)
            {
                continue;
            }
            // For now, types and names are sufficient for checking that the response sender is sending out the correct records.
            if (data.GetType() == expectedRecord[i]->GetType() && data.GetName() == expectedRecord[i]->GetName())
            {
                foundRecord[i]   = true;
                recordIsExpected = true;
                break;
            }
        }
        NL_TEST_ASSERT(mInSuite, recordIsExpected);
    }

    void OnQuery(const QueryData & data) override {}

    CHIP_ERROR
    DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
               chip::Inet::InterfaceId interface) override
    {
        ResetFoundRecords();
        ParsePacket(BytesRange(data->Start(), data->Start() + data->TotalLength()), this);
        TestGotAllExpectedPackets();
        sendCalled = true;
        return CHIP_NO_ERROR;
    }

    void AddExpectedRecord(ResourceRecord * record)
    {
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (expectedRecord[i] == nullptr)
            {
                expectedRecord[i] = record;
                return;
            }
        }
    }
    bool GetSendCalled() { return sendCalled; }
    bool GetHeaderFound() { return headerFound; }

private:
    nlTestSuite * mInSuite;
    static constexpr size_t kMaxExpectedRecords          = 10;
    ResourceRecord * expectedRecord[kMaxExpectedRecords] = {};
    bool foundRecord[kMaxExpectedRecords];
    bool headerFound = false;
    bool sendCalled  = false;
    void ResetFoundRecords()
    {
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (expectedRecord[i] == nullptr)
            {
                foundRecord[i] = true;
            }
        }
    }
    int GetNumExpectedRecords()
    {
        int num = 0;
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (expectedRecord[i] != nullptr)
            {
                ++num;
            }
        }
        return num;
    }
    void TestGotAllExpectedPackets()
    {
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            NL_TEST_ASSERT(mInSuite, foundRecord[i] == true);
        }
    }
};

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
    FullQName dnsSd    = FlatAllocatedQName::Build(dnsSdServiceStorage, "_services", "_dns-sd", "_udp", "local");
    FullQName service  = FlatAllocatedQName::Build(serviceNameStorage, "test", "service");
    FullQName instance = FlatAllocatedQName::Build(instanceNameStorage, "test", "instance");
    FullQName host     = FlatAllocatedQName::Build(hostNameStorage, "test", "host");
    FullQName txt      = FlatAllocatedQName::Build(txtStorage, "L1=something", "L2=other");

    static constexpr uint16_t kPort = 54;
    PtrResourceRecord ptrRecord     = PtrResourceRecord(service, instance);
    PtrResponder ptrResponder       = PtrResponder(service, instance);
    SrvResourceRecord srvRecord     = SrvResourceRecord(instance, host, kPort);
    SrvResponder srvResponder       = SrvResourceRecord(srvRecord);
    TxtResourceRecord txtRecord     = TxtResourceRecord(instance, txt);
    TxtResponder txtResponder       = TxtResponder(txtRecord);

    CheckOnlyServer server;
    QueryResponder<10> queryResponder;
    ResponseSender responseSender;
    Inet::IPPacketInfo packetInfo;

    CommonTestElements(nlTestSuite * inSuite) : server(inSuite), responseSender(&server, &queryResponder)
    {
        queryResponder.Init();
        header.SetQueryCount(1);
    }
};

void SrvAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
    common.queryResponder.AddResponder(&common.srvResponder);

    // Build a query for our srv record
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    common.server.AddExpectedRecord(&common.srvRecord);
    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void SrvTxtAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We requested ANY on the host name, expect both back.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);
    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToServiceName(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
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

    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
    common.queryResponder.AddResponder(&common.ptrResponder);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance name
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We shouldn't get back the PTR.
    common.server.AddExpectedRecord(&common.srvRecord);
    common.server.AddExpectedRecord(&common.txtRecord);

    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtSrvResponseToInstance(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the instance
    common.instance.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::SRV, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // We didn't set the txt as an additional on the srv name so expect only srv.
    common.server.AddExpectedRecord(&common.srvRecord);

    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

void PtrSrvTxtAnyResponseToServiceListing(nlTestSuite * inSuite, void * inContext)
{
    CommonTestElements common(inSuite);
    common.queryResponder.AddResponder(&common.ptrResponder).SetReportInServiceListing(true);
    common.queryResponder.AddResponder(&common.srvResponder);
    common.queryResponder.AddResponder(&common.txtResponder);

    // Build a query for the dns-sd services listing.
    common.dnsSd.Output(common.requestBufferWriter);

    QueryData queryData = QueryData(QType::ANY, QClass::IN, false, common.requestNameStart, common.requestBytesRange);

    // Only one PTR in service listing.
    PtrResourceRecord serviceRecord = PtrResourceRecord(common.dnsSd, common.ptrRecord.GetName());
    common.server.AddExpectedRecord(&serviceRecord);

    common.responseSender.Respond(1, queryData, &common.packetInfo);

    NL_TEST_ASSERT(inSuite, common.server.GetSendCalled());
    NL_TEST_ASSERT(inSuite, common.server.GetHeaderFound());
}

const nlTest sTests[] = {
    NL_TEST_DEF("SrvAnyResponseToInstance", SrvAnyResponseToInstance),                         //
    NL_TEST_DEF("SrvTxtAnyResponseToInstance", SrvTxtAnyResponseToInstance),                   //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToServiceName", PtrSrvTxtAnyResponseToServiceName),       //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToInstance", PtrSrvTxtAnyResponseToInstance),             //
    NL_TEST_DEF("PtrSrvTxtSrvResponseToInstance", PtrSrvTxtSrvResponseToInstance),             //
    NL_TEST_DEF("PtrSrvTxtAnyResponseToServiceListing", PtrSrvTxtAnyResponseToServiceListing), //
    NL_TEST_SENTINEL()                                                                         //
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
