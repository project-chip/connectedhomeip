/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "UnifyMPCContext.h"

// matter includes
#include <app/MessageDef/ReportDataMessage.h>
#include <app/ReadClient.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

// mpc includes
#include "mpc_cli_mock.h"
#include "mpc_command_sender.hpp"
#include "mpc_sessionprovider_mock.h"

using namespace unify::mpc::Test;
using namespace chip;
using namespace chip::app;

class TestContext : public UnifyMPCContext
{
    // public:
    //   static int Initialize(void * context)
    //   {

    //     Init();
    //   }

    //   static int Finalize(void * context)
    //   {
    //     Shutdown();
    //   }
};

void GenerateReportData(nlTestSuite * apSuite, void * apContext, System::PacketBufferHandle & aPayload, bool aNeedInvalidReport,
                        bool aSuppressResponse, bool aHasSubscriptionId = false)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    ReportDataMessage::Builder reportDataMessageBuilder;

    err = reportDataMessageBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    if (aHasSubscriptionId)
    {
        reportDataMessageBuilder.SubscriptionId(1);
        NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);
    }

    AttributeReportIBs::Builder & attributeReportIBsBuilder = reportDataMessageBuilder.CreateAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    AttributeReportIB::Builder & attributeReportIBBuilder = attributeReportIBsBuilder.CreateAttributeReport();
    NL_TEST_ASSERT(apSuite, attributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataIB::Builder & attributeDataIBBuilder = attributeReportIBBuilder.CreateAttributeData();
    NL_TEST_ASSERT(apSuite, attributeReportIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeDataIBBuilder.DataVersion(2);
    err = attributeDataIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder = attributeDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

    if (aNeedInvalidReport)
    {
        attributePathBuilder.Node(1).Endpoint(2).Cluster(3).ListIndex(5).EndOfAttributePathIB();
    }
    else
    {
        attributePathBuilder.Endpoint(1).Cluster(6).Attribute(0).EndOfAttributePathIB();
    }

    err = attributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = attributeDataIBBuilder.GetWriter();
        // chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        // err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::AttributeDataIB::Tag::kData)),
        //                               chip::TLV::kTLVType_Structure, dummyType);
        // NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(chip::to_underlying(chip::app::AttributeDataIB::Tag::kData)), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        // err = pWriter->EndContainer(dummyType);
        // NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    attributeDataIBBuilder.EndOfAttributeDataIB();
    NL_TEST_ASSERT(apSuite, attributeDataIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeReportIBBuilder.EndOfAttributeReportIB();
    NL_TEST_ASSERT(apSuite, attributeReportIBBuilder.GetError() == CHIP_NO_ERROR);

    attributeReportIBsBuilder.EndOfAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, attributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.MoreChunkedMessages(false);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.SuppressResponse(aSuppressResponse);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.EndOfReportDataMessage();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

class TestReadMockCallback : public ReadClient::Callback
{
public:
    void OnDone(ReadClient * apReadClient) override {}

    void OnError(CHIP_ERROR err) override { reported = false; }
    void OnReportBegin() override {}

    void OnReportEnd() override {}
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override
    {
        reported = false;
        if (aPath.mAttributeId == 0x00 && aPath.mClusterId == 0x06 && aPath.mEndpointId == 1)
        {
            Clusters::OnOff::Attributes::TypeInfo::DecodableType info;
            info.Decode(*apData, aPath);
            reported = info.onOff;
        }
    }

    bool reported = false;
};

namespace chip::app {
class TestReadInteraction
{
public:
    static void TestReadSender(nlTestSuite * inSuite, void * aContext);
    static void TestSubscribeSender(nlTestSuite * inSuite, void * aContext);
};
} // namespace chip::app

void TestReadInteraction::TestReadSender(nlTestSuite * inSuite, void * aContext)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    TestContext & ctx                  = *static_cast<TestContext *>(aContext);
    System::PacketBufferHandle payload = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    TestReadMockCallback mockCallbacks;

    AttributeReadRequest request(1, 1, 6, 0);

    request.SetCallbacks(&mockCallbacks);

    sl_status_t status = request.Send(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(inSuite, status == SL_STATUS_OK);
    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    GenerateReportData(inSuite, aContext, payload, false, true, false);
    err = request.client->ProcessReportData(std::move(payload));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mockCallbacks.reported == true);
}

void TestReadInteraction::TestSubscribeSender(nlTestSuite * inSuite, void * aContext)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    TestContext & ctx                  = *static_cast<TestContext *>(aContext);
    System::PacketBufferHandle payload = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    TestReadMockCallback mockCallbacks;
    SubscribeRequestParams params;
    params.minInterval      = 10;
    params.maxInterval      = 60;
    params.keepSubscription = 0;
    SubscribeRequest request(1, 1, 6, 0, params);

    request.SetCallbacks(&mockCallbacks);

    sl_status_t status = request.Send(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice());
    NL_TEST_ASSERT(inSuite, status == SL_STATUS_OK);
    // We don't actually want to deliver that message, because we want to
    // synthesize the read response.  But we don't want it hanging around
    // forever either.
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();

    GenerateReportData(inSuite, aContext, payload, false, true, true);
    err = request.client->ProcessReportData(std::move(payload));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mockCallbacks.reported == true);
}

static void TestCliSendRead(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    TestSessionProvider testSessPvdr(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice(), false);
    NodeId aliceNodeId  = ctx.GetAliceFabric()->GetNodeId();
    std::string command = "read_attribute ";     // subs minInterval,maxInterval,keepSubs,
    command.append(std::to_string(aliceNodeId)); // nodeId
    command.append(",1,6,0");                    //,epID,clustID,attrID

    NL_TEST_ASSERT(inSuite, (mpc_cli_init() == SL_STATUS_OK));
    NL_TEST_ASSERT(inSuite, (mpc_stdin_handle_command(command.c_str()) == SL_STATUS_OK));
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();
}

static void TestCliSendSubscribe(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    TestSessionProvider testSessPvdr(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice(), false);
    NodeId aliceNodeId  = ctx.GetAliceFabric()->GetNodeId();
    std::string command = "subscribe 10,60,0,";  // subs minInterval,maxInterval,keepSubs,
    command.append(std::to_string(aliceNodeId)); // nodeId
    command.append(",1,6,0");                    //,epID,clustID,attrID

    NL_TEST_ASSERT(inSuite, (mpc_cli_init() == SL_STATUS_OK));
    NL_TEST_ASSERT(inSuite, (mpc_stdin_handle_command(command.c_str()) == SL_STATUS_OK));
    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();
}

static void TestCliSendSubscribeSessionFail(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    TestSessionProvider testSessPvdr(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice(), true);
    NodeId aliceNodeId  = ctx.GetAliceFabric()->GetNodeId();
    std::string command = "subscribe 10,60,0,";  // subs minInterval,maxInterval,keepSubs,
    command.append(std::to_string(aliceNodeId)); // nodeId
    command.append(",1,6,0");                    //,epID,clustID,attrID

    NL_TEST_ASSERT(inSuite, (mpc_cli_init() == SL_STATUS_OK));
    NL_TEST_ASSERT(inSuite, (mpc_stdin_handle_command(command.c_str()) == SL_STATUS_OK));
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadSender", TestReadInteraction::TestReadSender),
    NL_TEST_DEF("TestSubscribeSender", TestReadInteraction::TestSubscribeSender),
    NL_TEST_DEF("TestCliSendRead", TestCliSendRead),
    NL_TEST_DEF("TestCliSendSubscribe", TestCliSendSubscribe),
    NL_TEST_DEF("TestCliSendSubscribeSessionFail", TestCliSendSubscribeSessionFail),
    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestCmdSenderInterface",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};

int TestCmdSenderInterface(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCmdSenderInterface)
