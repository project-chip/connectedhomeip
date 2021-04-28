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
 *      This file implements unit tests for CHIP Interaction Model Engine
 *
 */

#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace {
static chip::System::Layer gSystemLayer;
static chip::SecureSessionMgr gSessionManager;
static chip::Messaging::ExchangeManager gExchangeManager;
static chip::secure_channel::MessageCounterManager gMessageCounterManager;
static chip::TransportMgr<chip::Transport::UDP> gTransportManager;
static const chip::Transport::AdminId gAdminId = 0;
} // namespace

namespace chip {
namespace app {
class TestInteractionModelEngine
{
public:
    static void TestClusterInfoPushRelease(nlTestSuite * apSuite, void * apContext);
    static int GetClusterInfoListLength(ClusterInfo * apClusterInfoList);
};

int TestInteractionModelEngine::GetClusterInfoListLength(ClusterInfo * apClusterInfoList)
{
    int length           = 0;
    ClusterInfo * runner = apClusterInfoList;
    while (runner != nullptr)
    {
        runner = runner->mpNext;
        length++;
    }
    return length;
}

void TestInteractionModelEngine::TestClusterInfoPushRelease(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = InteractionModelEngine::GetInstance()->Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ClusterInfo * clusterInfoList = nullptr;
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(2, 3, 4, 5, 6, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams3(3, 4, 5, 6, 7, AttributePathFlags::kFieldIdValid);

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, attributePathParams1);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfoList->mAttributePathParams.IsSamePath(attributePathParams1));
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 1);

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, attributePathParams2);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfoList->mAttributePathParams.IsSamePath(attributePathParams2));
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 2);

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, attributePathParams3);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfoList->mAttributePathParams.IsSamePath(attributePathParams3));
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 3);

    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(clusterInfoList);
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 0);
}
} // namespace app
} // namespace chip

namespace {
void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    gSystemLayer.Init(nullptr);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &gSystemLayer, &gTransportManager, &admins, &gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gExchangeManager.Init(&gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gMessageCounterManager.Init(&gExchangeManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("TestClusterInfoPushRelease", chip::app::TestInteractionModelEngine::TestClusterInfoPushRelease),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

int TestInteractionModelEngine()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestInteractionModelEngine",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInteractionModelEngine)
