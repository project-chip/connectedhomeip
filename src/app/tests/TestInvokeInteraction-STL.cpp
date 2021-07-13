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

#include <messaging/ExchangeDelegate.h>
#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVText.hpp>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <stl/DemuxedInvokeInitiator.h>
#include <nlunit-test.h>

#include <TestCluster-Gen.h>

using namespace std::placeholders;

namespace chip {
static System::Layer gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static secure_channel::MessageCounterManager gMessageCounterManager;
static TransportMgr<Transport::UDP> gTransportManager;
static Transport::AdminId gAdminId = 0;

namespace app {

nlTestSuite *gpSuite = nullptr;
InvokeResponder *gServerInvoke = nullptr;
Messaging::ExchangeContext *gClientEc = nullptr;

class TestServerCluster : public ClusterServer
{
public:
    TestServerCluster();
    CHIP_ERROR OnInvokeRequest(CommandParams &commandParams, InvokeResponder &invokeInteraction, TLV::TLVReader *payload) override;

    bool mGotCommandA = false;
};

TestServerCluster::TestServerCluster()
    : ClusterServer(chip::app::Cluster::TestCluster::kClusterId)
{
}

CHIP_ERROR 
TestServerCluster::OnInvokeRequest(CommandParams &commandParams, InvokeResponder &invokeInteraction, TLV::TLVReader *payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandA::Type req;

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandAId) {
        printf("Received CommandA\n");

        gServerInvoke = &invokeInteraction;

        NL_TEST_ASSERT(gpSuite, payload != nullptr); 

        err = req.Decode(*payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, req.a == 10);
        NL_TEST_ASSERT(gpSuite, req.b == 20);
        NL_TEST_ASSERT(gpSuite, req.c.x == 13);
        NL_TEST_ASSERT(gpSuite, req.c.y == 99);

        for (size_t i = 0; i < req.d.size(); i++) {
            NL_TEST_ASSERT(gpSuite, req.d[i] == i);
        }

        //
        // Send response synchronously
        //
        
        {
            chip::app::Cluster::TestCluster::CommandB::Type resp;

            resp.a = 21;
            resp.b = 49;
            resp.c.x = 19;
            resp.c.y = 233;
    
            for (size_t i = 0; i < 5; i++) {
                resp.d.insert(resp.d.begin() + (long)i, (uint8_t)(255 - i));
            }

            for (size_t i = 0; i < 5; i++) {
                resp.e.insert(resp.e.begin() + (long)i, chip::app::Cluster::TestCluster::StructA::Type());
                resp.e[i].x = (uint8_t)(255 - i);
                resp.e[i].y = (uint8_t)(255 - i);
            }

            commandParams.CommandId = chip::app::Cluster::TestCluster::kCommandBId;

            err = invokeInteraction.AddResponse(commandParams, &resp);
            NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);
        }

        mGotCommandA = true;
    }

    return CHIP_NO_ERROR;
}

class TestInvokeInteraction : public Messaging::ExchangeContextUnitTestDelegate
{
public:
    static void TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext);
    void InterceptMessage(System::PacketBufferHandle buf);
    int GetNumActiveInvokes();

    void OnCommandBResponse(DemuxedInvokeInitiator &invokeInteraction, CommandParams &commandParams, chip::app::Cluster::TestCluster::CommandB::Type *response);
    
protected:
    System::PacketBufferHandle mBuf;
    int mGotCommandB = 0;
};

using namespace chip::TLV;

void TestInvokeInteraction::InterceptMessage(System::PacketBufferHandle buf)
{
    mBuf = std::move(buf);
}

void
TestInvokeInteraction::OnCommandBResponse(DemuxedInvokeInitiator &invokeInteraction, CommandParams &commandParams, chip::app::Cluster::TestCluster::CommandB::Type *response)
{
    NL_TEST_ASSERT(gpSuite, response);

    printf("Received CommandB on EP%d\n", commandParams.EndpointId);

    NL_TEST_ASSERT(gpSuite, response->a == 21);
    NL_TEST_ASSERT(gpSuite, response->b == 49);
    NL_TEST_ASSERT(gpSuite, response->c.x == 19);
    NL_TEST_ASSERT(gpSuite, response->c.y == 233);

    for (size_t i = 0; i < response->d.size(); i++) {
        NL_TEST_ASSERT(gpSuite, response->d[i] == (uint8_t)(255 - i));
    }

    for (size_t i = 0; i < response->e.size(); i++) {
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

    InteractionModelEngine::GetInstance()->mInvokeResponders.ForEachActiveObject([&](InvokeResponder *apInteraction) {
        count++;
        return true;
    });

    return count;
}

void TestInvokeInteraction::TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    Messaging::ExchangeContext *pRxEc;
    PacketHeader packetHdr;
    PayloadHeader payloadHdr;
    TestServerCluster serverEp0;
    TestServerCluster serverEp1;
    std::unique_ptr<DemuxedInvokeInitiator> invokeInitiator;
    
    auto onDoneFunc = [apSuite, &invokeInitiator] (DemuxedInvokeInitiator &initiator) {
        printf("OnDone!\n");
        NL_TEST_ASSERT(apSuite, &initiator == invokeInitiator.get());
        (void)invokeInitiator.release();
    };

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

        req.a = 10;
        req.b = 20;
        req.c.x = 13;
        req.c.y = 99;
        req.d = {0, 1, 2, 3, 4};

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 0, true),  onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator->AddCommand<chip::app::Cluster::TestCluster::CommandB::Type>(&req, CommandParams(req, 1, true),  onSuccessFunc);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator->Send();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    gServerInvoke = nullptr;

    pRxEc = chip::gExchangeManager.NewContext({0, 0, 0}, NULL);
    NL_TEST_ASSERT(apSuite, pRxEc != nullptr);
    
    chip::app::InteractionModelEngine::GetInstance()->OnInvokeCommandRequest(pRxEc, packetHdr, payloadHdr, std::move(buf));
    NL_TEST_ASSERT(apSuite, gServerInvoke != nullptr);
    NL_TEST_ASSERT(apSuite, serverEp0.mGotCommandA);
    NL_TEST_ASSERT(apSuite, serverEp1.mGotCommandA);

    NL_TEST_ASSERT(apSuite, gTestInvoke.GetNumActiveInvokes() == 0);
    
    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(gTestInvoke.mBuf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    invokeInitiator->GetInitiator().OnMessageReceived(invokeInitiator->GetInitiator().GetExchange(), PacketHeader(), PayloadHeader(), std::move(buf));
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

    chip::gSystemLayer.Init(nullptr);

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins, &chip::gMessageCounterManager);
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
