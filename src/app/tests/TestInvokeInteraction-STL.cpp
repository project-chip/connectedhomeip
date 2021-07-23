/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file implements unit tests for CHIP Interaction Model Command Interaction
 *
 */

#include <TestCluster-Gen.h>
#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVText.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <memory>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <stl/DemuxedInvokeInitiator.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

using namespace std::placeholders;

namespace chip {
static System::Layer gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static secure_channel::MessageCounterManager gMessageCounterManager;
static TransportMgr<Transport::UDP> gTransportManager;
static Transport::AdminId gAdminId = 0;

namespace app {

nlTestSuite * gpSuite                  = nullptr;
InvokeResponder * gServerInvoke        = nullptr;
Messaging::ExchangeContext * gClientEc = nullptr;

class TestServerCluster : public ClusterServer
{
public:
    TestServerCluster();
    CHIP_ERROR OnInvokeRequest(CommandParams & commandParams, InvokeResponder & invokeInteraction,
                               TLV::TLVReader * payload) override;

    void SetAsyncResp() { mDoAsyncResp = true; }

    CHIP_ERROR SendAsyncResp();

    bool mGotCommandA                         = false;
    bool mDoAsyncResp                         = false;
    InvokeResponder * mStashedInvokeResponder = nullptr;
    CommandParams mStashedCommandParams;
};

TestServerCluster::TestServerCluster() : ClusterServer(chip::app::Cluster::TestCluster::kClusterId) {}

CHIP_ERROR
TestServerCluster::SendAsyncResp()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandB::Type resp;

    resp.a   = 21;
    resp.b   = 49;
    resp.c.x = 19;
    resp.c.y = 233;

    for (size_t i = 0; i < 5; i++)
    {
        resp.d.insert(resp.d.begin() + (long) i, (uint8_t)(255 - i));
    }

    for (size_t i = 0; i < 5; i++)
    {
        resp.e.insert(resp.e.begin() + (long) i, chip::app::Cluster::TestCluster::StructA::Type());
        resp.e[i].x = (uint8_t)(255 - i);
        resp.e[i].y = (uint8_t)(255 - i);
    }

    mStashedCommandParams.CommandId = chip::app::Cluster::TestCluster::kCommandBId;

    err = mStashedInvokeResponder->AddResponse(mStashedCommandParams, &resp);
    NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

    mStashedInvokeResponder->DecrementHoldOffRef();
    return err;
}

CHIP_ERROR
TestServerCluster::OnInvokeRequest(CommandParams & commandParams, InvokeResponder & invokeInteraction, TLV::TLVReader * payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandA::Type req;

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandAId)
    {
        printf("Received CommandA\n");

        gServerInvoke = &invokeInteraction;

        NL_TEST_ASSERT(gpSuite, payload != nullptr);

        err = req.Decode(*payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, req.a == 10);
        NL_TEST_ASSERT(gpSuite, req.b == 20);
        NL_TEST_ASSERT(gpSuite, req.c.x == 13);
        NL_TEST_ASSERT(gpSuite, req.c.y == 99);

        for (size_t i = 0; i < req.d.size(); i++)
        {
            NL_TEST_ASSERT(gpSuite, req.d[i] == i);
        }

        //
        // Send response synchronously
        //

        if (!mDoAsyncResp)
        {
            chip::app::Cluster::TestCluster::CommandB::Type resp;

            resp.a   = 21;
            resp.b   = 49;
            resp.c.x = 19;
            resp.c.y = 233;

            for (size_t i = 0; i < 5; i++)
            {
                resp.d.insert(resp.d.begin() + (long) i, (uint8_t)(255 - i));
            }

            for (size_t i = 0; i < 5; i++)
            {
                resp.e.insert(resp.e.begin() + (long) i, chip::app::Cluster::TestCluster::StructA::Type());
                resp.e[i].x = (uint8_t)(255 - i);
                resp.e[i].y = (uint8_t)(255 - i);
            }

            commandParams.CommandId = chip::app::Cluster::TestCluster::kCommandBId;

            err = invokeInteraction.AddResponse(commandParams, &resp);
            NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);
        }
        else
        {
            invokeInteraction.IncrementHoldOffRef();
            mStashedInvokeResponder = &invokeInteraction;
            mStashedCommandParams   = commandParams;
        }

        mGotCommandA = true;
    }

    return CHIP_NO_ERROR;
}

class TestInvokeInteraction : public Messaging::ExchangeContextUnitTestDelegate
{
public:
    static void TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext);
    static void TestInvokeInteractionAsyncResponder(nlTestSuite * apSuite, void * apContext);
    void InterceptMessage(System::PacketBufferHandle buf);
    int GetNumActiveInvokes();

    void OnCommandBResponse(DemuxedInvokeInitiator & invokeInteraction, CommandParams & commandParams,
                            chip::app::Cluster::TestCluster::CommandB::Type * response);

protected:
    System::PacketBufferHandle mBuf;
    int mGotCommandB = 0;
    bool mGotMessage = false;
};

using namespace chip::TLV;

void TestInvokeInteraction::InterceptMessage(System::PacketBufferHandle buf)
{
    mBuf        = std::move(buf);
    mGotMessage = true;
}

void TestInvokeInteraction::OnCommandBResponse(DemuxedInvokeInitiator & invokeInteraction, CommandParams & commandParams,
                                               chip::app::Cluster::TestCluster::CommandB::Type * response)
{
    NL_TEST_ASSERT(gpSuite, response);

    printf("Received CommandB on EP%d\n", commandParams.EndpointId);

    NL_TEST_ASSERT(gpSuite, response->a == 21);
    NL_TEST_ASSERT(gpSuite, response->b == 49);
    NL_TEST_ASSERT(gpSuite, response->c.x == 19);
    NL_TEST_ASSERT(gpSuite, response->c.y == 233);

    for (size_t i = 0; i < response->d.size(); i++)
    {
        NL_TEST_ASSERT(gpSuite, response->d[i] == (uint8_t)(255 - i));
    }

    for (size_t i = 0; i < response->e.size(); i++)
    {
        NL_TEST_ASSERT(gpSuite, response->e[i].x == (uint8_t)(255 - i));
        NL_TEST_ASSERT(gpSuite, response->e[i].y == (uint8_t)(255 - i));
    }

    mGotCommandB++;

    return;
}

TestInvokeInteraction gTestInvoke;

int TestInvokeInteraction::GetNumActiveInvokes()
{
    int count = 0;

    InteractionModelEngine::GetInstance()->mInvokeResponders.ForEachActiveObject([&](InvokeResponder * apInteraction) {
        count++;
        return true;
    });

    return count;
}

/*
 * @brief
 *
 * This test tries to achieve an 'end-to-end' test between a client and a server.
 * Specifically, it has a client sending an invoke action to a server containing two commands targeting
 * two different endpoints, with the server responding back syncronously with a response containing two response
 * payloads.
 *
 * To achieve 'end-to-endness' within a single stack, the message is created and intercepted before it is sent out,
 * and then 'looped back in' to the IM for it to make its way back up the stack to the application.
 */
void TestInvokeInteraction::TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    Messaging::ExchangeContext * pRxEc;
    PacketHeader packetHdr;
    PayloadHeader payloadHdr;
    TestServerCluster serverEp0;
    TestServerCluster serverEp1;
    std::unique_ptr<DemuxedInvokeInitiator> invokeInitiator;

    auto onDoneFunc = [apSuite, &invokeInitiator](DemuxedInvokeInitiator & initiator) {
        printf("OnDone!\n");
        NL_TEST_ASSERT(apSuite, &initiator == invokeInitiator.get());
        (void) invokeInitiator.release();
    };

    // Re-init it.
    gTestInvoke = TestInvokeInteraction();

    serverEp0.SetEndpoint(0);
    serverEp1.SetEndpoint(1);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp0);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeInitiator = std::make_unique<DemuxedInvokeInitiator>(onDoneFunc);

    err = invokeInitiator->Init(&chip::gExchangeManager, 0, 0, NULL);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::app::Cluster::TestCluster::CommandA::Type req;
        auto onSuccessFunc = std::bind(&TestInvokeInteraction::OnCommandBResponse, &gTestInvoke, _1, _2, _3);

        req.a   = 10;
        req.b   = 20;
        req.c.x = 13;
        req.c.y = 99;
        req.d   = { 0, 1, 2, 3, 4 };

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 0, true),
                                                                                           onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 1, true),
                                                                                           onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        //
        // At this point, gTestInvoke::InterceptMessage() intercepts the actual packet buffer right upon
        // exit from the IM. Check that the IM did indeed try to send a message out.
        //
        err = invokeInitiator->Send();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, gTestInvoke.mGotMessage);
    }

    //
    // Print out the buffer contents by momentarily transferring
    // ownership of the buffer over to the reader, before transferring it back
    //
    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    //
    // Clear out this tracker variable that tracks if the server callback
    // did get invoked (at which point, this pointer points to the instance of InvokeResponder
    //
    gServerInvoke = nullptr;

    pRxEc = chip::gExchangeManager.NewContext({ 0, 0, 0 }, NULL);
    NL_TEST_ASSERT(apSuite, pRxEc != nullptr);

    //
    // Pump the previously created packet buffer back into the IM to mimic the receive pathway with the newly created EC from above.
    //
    chip::app::InteractionModelEngine::GetInstance()->OnInvokeCommandRequest(pRxEc, packetHdr, payloadHdr, std::move(buf));
    NL_TEST_ASSERT(apSuite, gServerInvoke != nullptr);
    NL_TEST_ASSERT(apSuite, serverEp0.mGotCommandA);
    NL_TEST_ASSERT(apSuite, serverEp1.mGotCommandA);

    //
    // Since there are no async activity in this test, the invoke should have been freed up and returned
    // to the pool.
    //
    NL_TEST_ASSERT(apSuite, gTestInvoke.GetNumActiveInvokes() == 0);

    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    //
    // Take the packetbuffer that was formed on the 'server side' that contains the InvokeResponse, and feed it back to the IM
    // again, but on the 'client side'.
    invokeInitiator->GetInitiator().OnMessageReceived(invokeInitiator->GetInitiator().GetExchange(), PacketHeader(),
                                                      PayloadHeader(), std::move(buf));

    //
    // Ensure that we got two calls to handle the command on both endpoints.
    //
    NL_TEST_ASSERT(apSuite, gTestInvoke.mGotCommandB == 2);
}

/*
 * @brief
 *
 * This test tries to achieve an 'end-to-end' test between a client and a server.
 * Specifically, it has a client sending an invoke action to a server containing two commands targeting
 * two different endpoints, with the server responding back with a response containing two response
 * payloads.
 *
 * The key distinction between this and the previous test is that one of the responses emitted by a cluster handler
 * is done asyncronously.
 */
void TestInvokeInteraction::TestInvokeInteractionAsyncResponder(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    Messaging::ExchangeContext * pRxEc;
    PacketHeader packetHdr;
    PayloadHeader payloadHdr;
    TestServerCluster serverEp0;
    TestServerCluster serverEp1;
    std::unique_ptr<DemuxedInvokeInitiator> invokeInitiator;

    auto onDoneFunc = [apSuite, &invokeInitiator](DemuxedInvokeInitiator & initiator) {
        NL_TEST_ASSERT(apSuite, &initiator == invokeInitiator.get());
        (void) invokeInitiator.release();
    };

    // Re-init it.
    gTestInvoke = TestInvokeInteraction();

    serverEp0.SetEndpoint(0);
    serverEp1.SetEndpoint(1);

    // Set the server EP1 test logic to not respond syncronously.
    serverEp1.SetAsyncResp();

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp0);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeInitiator = std::make_unique<DemuxedInvokeInitiator>(onDoneFunc);

    err = invokeInitiator->Init(&chip::gExchangeManager, 0, 0, NULL);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::app::Cluster::TestCluster::CommandA::Type req;
        auto onSuccessFunc = std::bind(&TestInvokeInteraction::OnCommandBResponse, &gTestInvoke, _1, _2, _3);

        req.a   = 10;
        req.b   = 20;
        req.c.x = 13;
        req.c.y = 99;
        req.d   = { 0, 1, 2, 3, 4 };

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 0, true),
                                                                                           onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 1, true),
                                                                                           onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        //
        // At this point, gTestInvoke::InterceptMessage() intercepts the actual packet buffer right upon
        // exit from the IM. Check that the IM did indeed try to send a message out.
        //
        err = invokeInitiator->Send();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, gTestInvoke.mGotMessage);
    }

    //
    // Print out the buffer contents by momentarily transferring
    // ownership of the buffer over to the reader, before transferring it back
    //
    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    //
    // Clear out this tracker variable that tracks if the server callback
    // did get invoked (at which point, this pointer points to the instance of InvokeResponder
    //
    gServerInvoke = nullptr;

    pRxEc = chip::gExchangeManager.NewContext({ 0, 0, 0 }, NULL);
    NL_TEST_ASSERT(apSuite, pRxEc != nullptr);

    //
    // Pump the previously created packet buffer back into the IM to mimic the receive pathway with the newly created EC from above.
    //
    chip::app::InteractionModelEngine::GetInstance()->OnInvokeCommandRequest(pRxEc, packetHdr, payloadHdr, std::move(buf));
    NL_TEST_ASSERT(apSuite, gServerInvoke != nullptr);
    NL_TEST_ASSERT(apSuite, serverEp0.mGotCommandA);
    NL_TEST_ASSERT(apSuite, serverEp1.mGotCommandA);

    // Make sure it's got the invoke responder stashed away since it's doing async.
    NL_TEST_ASSERT(apSuite, serverEp1.mStashedInvokeResponder != nullptr);

    // Make sure that we haven't detected the transmission of any buffers..
    NL_TEST_ASSERT(apSuite, gTestInvoke.mBuf.IsNull());

    // Make sure the invoke responder object has not been auto free'ed since there is still pending work on that object.
    NL_TEST_ASSERT(apSuite, gTestInvoke.GetNumActiveInvokes() == 1);

    err = serverEp1.SendAsyncResp();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // Now, make sure it's been freed.
    NL_TEST_ASSERT(apSuite, gTestInvoke.GetNumActiveInvokes() == 0);

    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    //
    // Take the packetbuffer that was formed on the 'server side' that contains the InvokeResponse, and feed it back to the IM
    // again, but on the 'client side'.
    invokeInitiator->GetInitiator().OnMessageReceived(invokeInitiator->GetInitiator().GetExchange(), PacketHeader(),
                                                      PayloadHeader(), std::move(buf));

    //
    // Ensure that we got two calls to handle the command on both endpoints.
    //
    NL_TEST_ASSERT(apSuite, gTestInvoke.mGotCommandB == 2);
}

} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(chip::gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    chip::app::gpSuite = apSuite;

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::gSystemLayer.Init();

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins,
                                     &chip::gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::Messaging::ExchangeContext::SetUnitTestDelegate(&chip::app::gTestInvoke);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestInvokeInteractionSimple", chip::app::TestInvokeInteraction::TestInvokeInteractionSimple),
    NL_TEST_DEF("TestInvokeInteractionAsyncResponder", chip::app::TestInvokeInteraction::TestInvokeInteractionAsyncResponder),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestInvokeInteraction()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "InvokeInteraction",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInvokeInteraction)
