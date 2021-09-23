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
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace {
static chip::System::LayerImpl gSystemLayer;
static chip::SessionManager gSessionManager;
static chip::Messaging::ExchangeManager gExchangeManager;
static chip::secure_channel::MessageCounterManager gMessageCounterManager;
static chip::TransportMgr<chip::Transport::UDP> gTransportManager;
} // namespace

namespace chip {
namespace app {
class TestInteractionModelEngine
{
public:
    static void TestClusterInfoPushRelease(nlTestSuite * apSuite, void * apContext);
    static void TestMergeOverlappedAttributePath(nlTestSuite * apSuite, void * apContext);
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
    ClusterInfo clusterInfo1;
    ClusterInfo clusterInfo2;
    ClusterInfo clusterInfo3;

    clusterInfo1.mEndpointId = 1;
    clusterInfo2.mEndpointId = 2;
    clusterInfo3.mEndpointId = 3;

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, clusterInfo1);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfo1.mEndpointId == clusterInfoList->mEndpointId);
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 1);

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, clusterInfo2);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfo2.mEndpointId == clusterInfoList->mEndpointId);
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 2);

    InteractionModelEngine::GetInstance()->PushFront(clusterInfoList, clusterInfo3);
    NL_TEST_ASSERT(apSuite, clusterInfoList != nullptr && clusterInfo3.mEndpointId == clusterInfoList->mEndpointId);
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 3);

    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(clusterInfoList);
    NL_TEST_ASSERT(apSuite, GetClusterInfoListLength(clusterInfoList) == 0);
}

void TestInteractionModelEngine::TestMergeOverlappedAttributePath(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = InteractionModelEngine::GetInstance()->Init(&gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ClusterInfo clusterInfoList[2];

    clusterInfoList[0].mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    clusterInfoList[0].mFieldId = 1;
    clusterInfoList[1].mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    clusterInfoList[1].mFieldId = 2;

    chip::app::ClusterInfo testClusterInfo;
    testClusterInfo.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    testClusterInfo.mFieldId = 3;

    NL_TEST_ASSERT(apSuite, !InteractionModelEngine::GetInstance()->MergeOverlappedAttributePath(clusterInfoList, testClusterInfo));
    testClusterInfo.mFieldId = 0xFFFFFFFF;
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->MergeOverlappedAttributePath(clusterInfoList, testClusterInfo));
    testClusterInfo.mFlags.Set(chip::app::ClusterInfo::Flags::kListIndexValid);
    testClusterInfo.mFieldId   = 1;
    testClusterInfo.mListIndex = 2;
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->MergeOverlappedAttributePath(clusterInfoList, testClusterInfo));
}
} // namespace app
} // namespace chip

namespace {
void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::FabricTable fabrics;

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    gSystemLayer.Init();

    err = gSessionManager.Init(&gSystemLayer, &gTransportManager, &fabrics, &gMessageCounterManager);
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
                NL_TEST_DEF("TestMergeOverlappedAttributePath", chip::app::TestInteractionModelEngine::TestMergeOverlappedAttributePath),
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

    gSystemLayer.Shutdown();

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInteractionModelEngine)
