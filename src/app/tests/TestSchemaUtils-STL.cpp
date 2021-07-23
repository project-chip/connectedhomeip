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
#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVTags.h>
#include <core/CHIPTLVText.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <memory>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <TestCluster-Gen.h>

namespace chip {
static System::Layer gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static secure_channel::MessageCounterManager gMessageCounterManager;
static TransportMgr<Transport::UDP> gTransportManager;
static Transport::AdminId gAdminId = 0;

namespace app {

class TestSchemaUtils
{
public:
    static void TestSchemaUtilsEncAndDecSimple(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecList(nlTestSuite * apSuite, void * apContext);

private:
    void SetupBuf();
    void DumpBuf();
    void SetupReader();

    chip::System::TLVPacketBufferBackingStore mStore;
    chip::TLV::TLVWriter mWriter;
    chip::TLV::TLVReader mReader;
    nlTestSuite * mpSuite;
};

using namespace chip::TLV;

TestSchemaUtils gTestSchemaUtils;

void TestSchemaUtils::SetupBuf()
{
    chip::System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::New(1024);
    mStore.Init(std::move(buf));

    mWriter.Init(mStore);
    mReader.Init(mStore);
}

void TestSchemaUtils::DumpBuf()
{
    TLV::TLVReader reader;
    reader.Init(mStore);
    chip::TLV::Utilities::Print(reader);
}

void TestSchemaUtils::SetupReader()
{
    CHIP_ERROR err;

    mReader.Init(mStore);
    err = mReader.Next();

    NL_TEST_ASSERT(mpSuite, err == CHIP_NO_ERROR);
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecSimple(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructA::Type sa;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils * _this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    sa.x = 20;
    sa.y = 30;
    sa.l = { 0, 1, 2, 3 };
    sa.m = "chip";

    err = sa.Encode(_this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    sa = chip::app::Cluster::TestCluster::StructA::Type();

    _this->SetupReader();

    err = sa.Decode(_this->mReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, sa.x == 20);
    NL_TEST_ASSERT(apSuite, sa.y == 30);

    for (size_t i = 0; i < sa.l.size(); i++)
    {
        NL_TEST_ASSERT(apSuite, sa.l[i] == i);
    }

    NL_TEST_ASSERT(apSuite, !sa.m.compare("chip"));
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructB::Type sb;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils * _this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    sb.x   = 20;
    sb.y   = 30;
    sb.z.x = 99;
    sb.z.y = 17;

    err = sb.Encode(_this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    sb = chip::app::Cluster::TestCluster::StructB::Type();

    _this->SetupReader();

    err = sb.Decode(_this->mReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, sb.x == 20);
    NL_TEST_ASSERT(apSuite, sb.y == 30);
    NL_TEST_ASSERT(apSuite, sb.z.x == 99);
    NL_TEST_ASSERT(apSuite, sb.z.y == 17);
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecList(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructC::Type sc;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils * _this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    for (size_t i = 0; i < 5; i++)
    {
        sc.d.insert(sc.d.begin() + (long) i, (uint8_t) i);
    }

    sc.a   = 20;
    sc.b   = 30;
    sc.c.x = 99;
    sc.c.y = 17;

    for (size_t i = 0; i < 7; i++)
    {
        chip::app::Cluster::TestCluster::StructA::Type t;

        t.x = (uint8_t) i;
        t.y = (uint8_t) i;

        sc.e.insert(sc.e.begin() + (long) i, t);
    }

    err = sc.Encode(_this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    sc = chip::app::Cluster::TestCluster::StructC::Type();

    _this->SetupReader();

    err = sc.Decode(_this->mReader);

    NL_TEST_ASSERT(apSuite, sc.a == 20);
    NL_TEST_ASSERT(apSuite, sc.b == 30);
    NL_TEST_ASSERT(apSuite, sc.c.x == 99);
    NL_TEST_ASSERT(apSuite, sc.c.y == 17);

    for (size_t i = 0; i < 5; i++)
    {
        NL_TEST_ASSERT(apSuite, sc.d[i] == i);
    }

    for (size_t i = 0; i < 7; i++)
    {
        NL_TEST_ASSERT(apSuite, sc.e[i].x == i);
        NL_TEST_ASSERT(apSuite, sc.e[i].y == i);
    }
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
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestSchemaUtilsEncAndDecSimple", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecSimple),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecNestedStruct", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecNestedStruct),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecList", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecList),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestSchemaUtils()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestSchemaUtils",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, &chip::app::gTestSchemaUtils);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSchemaUtils)
