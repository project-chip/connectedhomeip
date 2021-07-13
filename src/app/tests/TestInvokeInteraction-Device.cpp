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

#include "TestCluster.h"
#include "messaging/ExchangeDelegate.h"
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

#include <nlunit-test.h>

#include <device/SchemaUtils.h>
#include <TestCluster-Gen.h>
#include <NetworkCommissioningCluster-Gen.h>

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
    CHIP_ERROR OnInvokeRequest(CommandParams &commandParams, InvokeResponder &invokeInteraction, TLV::TLVReader *payload) final;

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
    uint8_t d[5];

    req.d = chip::Span<uint8_t>{d};

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandAId) {
        printf("Received CommandA\n");

        gServerInvoke = &invokeInteraction;

        NL_TEST_ASSERT(gpSuite, payload != nullptr); 

        err = DecodeSchemaElement(req, *payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, req.a == 10);
        NL_TEST_ASSERT(gpSuite, req.b == 20);
        NL_TEST_ASSERT(gpSuite, req.c.x == 13);
        NL_TEST_ASSERT(gpSuite, req.c.y == 99);

        for (size_t i = 0; i < ArraySize(d); i++) {
            NL_TEST_ASSERT(gpSuite, d[i] == i);
        }

        //
        // Send response synchronously
        //
        
        {
            chip::app::Cluster::TestCluster::CommandB::Type resp;
            chip::app::Cluster::TestCluster::StructA::Type e[5];

            resp.a = 21;
            resp.b = 49;
            resp.c.x = 19;
            resp.c.y = 233;
            resp.d = chip::Span<uint8_t>{d};
            resp.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};
    
            for (size_t i = 0; i < ArraySize(d); i++) {
                d[i] = (uint8_t)(255 - i);
            }

            for (size_t i = 0; i < ArraySize(e); i++) {
                e[i].x = (uint8_t)(255 - i);
                e[i].y = (uint8_t)(255 - i);
            }

            commandParams.CommandId = chip::app::Cluster::TestCluster::kCommandBId;
            err = invokeInteraction.AddResponse(commandParams, [&](chip::TLV::TLVWriter &writer, uint64_t tag) {
                return EncodeSchemaElement(resp, writer, tag);
            });

            NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);
        }

        mGotCommandA = true;
    }

    return CHIP_NO_ERROR;
}

class TestInvokeInteraction : public Messaging::ExchangeContextUnitTestDelegate, public InvokeInitiator::ICommandHandler
{
public:
    static void TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext);
    void InterceptMessage(System::PacketBufferHandle buf);
    int GetNumActiveInvokes();

protected:
    void OnResponse(InvokeInitiator &invokeInteraction, CommandParams &commandParams, TLV::TLVReader *payload) final;
    void OnError(InvokeInitiator &invokeInteration, CommandParams *aPath, CHIP_ERROR error, StatusResponse *statusResponse) final {}
    void OnEnd(InvokeInitiator &invokeInteraction) final {}

    System::PacketBufferHandle mBuf;
    bool mGotCommandB = false;
};

using namespace chip::TLV;

void TestInvokeInteraction::InterceptMessage(System::PacketBufferHandle buf)
{
    mBuf = std::move(buf);
}

void
TestInvokeInteraction::OnResponse(InvokeInitiator &invokeInteraction, CommandParams &commandParams, TLV::TLVReader *payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandB::Type resp;
    uint8_t d[5];
    chip::app::Cluster::TestCluster::StructA::Type e[5];

    resp.d = chip::Span<uint8_t>{d};
    resp.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandBId) {
        printf("Received CommandB\n");

        // 
        // To prevent the stack from actually sending this message
        //
        //invokeInteraction.IncrementHoldoffRef();

        NL_TEST_ASSERT(gpSuite, payload != nullptr); 

        err = DecodeSchemaElement(resp, *payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, resp.a == 21);
        NL_TEST_ASSERT(gpSuite, resp.b == 49);
        NL_TEST_ASSERT(gpSuite, resp.c.x == 19);
        NL_TEST_ASSERT(gpSuite, resp.c.y == 233);

        for (size_t i = 0; i < ArraySize(d); i++) {
            NL_TEST_ASSERT(gpSuite, d[i] == (uint8_t)(255 - i));
        }

        for (size_t i = 0; i < ArraySize(e); i++) {
            NL_TEST_ASSERT(gpSuite, e[i].x == (uint8_t)(255 - i));
            NL_TEST_ASSERT(gpSuite, e[i].y == (uint8_t)(255 - i));
        }

        mGotCommandB = true;
    }

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
    InvokeInitiator invokeInitiator;

    serverEp0.SetEndpoint(0);
    serverEp1.SetEndpoint(1);
    
    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp0);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&serverEp1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeInitiator.Init(&chip::gExchangeManager, &gTestInvoke, 0, 0, NULL);

    {
        chip::app::Cluster::TestCluster::CommandA::Type req;
        uint8_t d[5];

        req.a = 10;
        req.b = 20;
        req.c.x = 13;
        req.c.y = 99;
        req.d = chip::Span<uint8_t>{d};

        for (size_t i = 0; i < ArraySize(d); i++) {
            d[i] = (uint8_t)i;
        }
    
        err = invokeInitiator.AddRequest(CommandParams(req, 0, true), [&req](auto &writer, auto tag) {
            return EncodeSchemaElement(req, writer, tag);
        });
        
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator.AddRequest(CommandParams(req, 1, true), [&req](auto &writer, auto tag) {
            return EncodeSchemaElement(req, writer, tag);
        });

        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = invokeInitiator.Send();
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

    invokeInitiator.OnMessageReceived(invokeInitiator.GetExchange(), PacketHeader(), PayloadHeader(), std::move(buf));
    NL_TEST_ASSERT(apSuite, gTestInvoke.mGotCommandB);
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
