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

#include <device/SchemaTypes.h>
#include <device/SchemaUtils.h>
#include "core/CHIPTLVTags.h"
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
#include <support/PrivateHeap.h>
#include <nlunit-test.h>

#include <TestCluster-Gen.h>
#include <TestCluster2-Gen.h>

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
    static void TestSchemaUtilsEncAndDecSimplePrivateHeap(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecList(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecIteratableList(nlTestSuite * apSuite, void * apContext);
    static void TestSchemaUtilsEncAndDecListPrivateHeap(nlTestSuite * apSuite, void * apContext);

private:
    void SetupBuf();
    void DumpBuf();
    void SetupReader();

    chip::System::TLVPacketBufferBackingStore mStore;
    chip::TLV::TLVWriter mWriter;
    chip::TLV::TLVReader mReader;
    nlTestSuite *mpSuite;
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

void TestSchemaUtils::TestSchemaUtilsEncAndDecSimplePrivateHeap(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructA::Type sa;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    {
        uint8_t buf[4] = {0, 1, 2, 3};
        char strbuf[10] = "chip";

        _this->mpSuite = apSuite;
        _this->SetupBuf();

        sa.x = 20;
        sa.y = 30;
        sa.l = chip::ByteSpan{buf};

        // TODO: Bug in CHIPTLVWriter/Reader that don't quite deal with the null-terminator correctly.
        sa.m = chip::Span<char>(strbuf, strlen(strbuf));

        err = chip::app::EncodeSchemaElement(sa, _this->mWriter, TLV::AnonymousTag);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    sa = chip::app::Cluster::TestCluster::StructA::Type();

    {
        SchemaAllocator schemaAllocator;

        _this->SetupReader();

        err = chip::app::DecodeSchemaElement(sa, _this->mReader, &schemaAllocator);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, sa.x == 20);
        NL_TEST_ASSERT(apSuite, sa.y == 30);

        for (uint32_t i = 0; i < sa.l.size(); i++) {
            NL_TEST_ASSERT(apSuite, sa.l.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, strncmp(sa.m.data(), "chip", ArraySize("chip")) == 0);
    }
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecSimple(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructA::Type sa;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);
    uint8_t buf[4] = {0, 1, 2, 3};
    char strbuf[10] = "chip";

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    sa.x = 20;
    sa.y = 30;
    sa.l = chip::ByteSpan{buf};

    // TODO: Bug in CHIPTLVWriter/Reader that don't quite deal with the null-terminator correctly.
    sa.m = chip::Span<char>(strbuf, strlen(strbuf));
    
    err = chip::app::EncodeSchemaElement(sa, _this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    memset(&sa, 0, sizeof(sa));
    memset(buf, 0, ArraySize(buf));
    memset(strbuf, 0, ArraySize(strbuf));

    sa.l = chip::ByteSpan{buf};
    sa.m = chip::Span<char>{strbuf};

    _this->SetupReader();

    err = chip::app::DecodeSchemaElement(sa, _this->mReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, sa.x == 20);
    NL_TEST_ASSERT(apSuite, sa.y == 30);

    for (uint32_t i = 0; i < ArraySize(buf); i++) {
        NL_TEST_ASSERT(apSuite, buf[i] == i);
    }

    NL_TEST_ASSERT(apSuite, strncmp(strbuf, "chip", ArraySize("chip")) == 0);
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructB::Type sb;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    sb.x = 20;
    sb.y = 30;
    sb.z.x = 99;
    sb.z.y = 17;
    
    err = chip::app::EncodeSchemaElement(sb, _this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    memset(&sb, 0, sizeof(sb));

    _this->SetupReader();

    err = chip::app::DecodeSchemaElement(sb, _this->mReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, sb.x == 20);
    NL_TEST_ASSERT(apSuite, sb.y == 30);
    NL_TEST_ASSERT(apSuite, sb.z.x == 99);
    NL_TEST_ASSERT(apSuite, sb.z.y == 17);
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecListPrivateHeap(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructC::Type sc;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    {
        uint8_t d[5];
        chip::app::Cluster::TestCluster::StructA::Type e[5];

        _this->mpSuite = apSuite;
        _this->SetupBuf();

        for (size_t i = 0; i < ArraySize(d); i++) {
            d[i] = (uint8_t)i;
        }

        sc.a = 20;
        sc.b = 30;
        sc.c.x = 99;
        sc.c.y = 17;
        sc.d = chip::Span<uint8_t>{d};
        sc.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};

        for (size_t i = 0; i < ArraySize(e); i++) {
            e[i].x = (uint8_t)i;
            e[i].y = (uint8_t)i;
        }

        err = chip::app::EncodeSchemaElement(sc, _this->mWriter, TLV::AnonymousTag);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    sc = chip::app::Cluster::TestCluster::StructC::Type();

    {
        uint8_t privHeapBuf[1024];
        SchemaAllocator allocator(privHeapBuf, 1024);

        _this->SetupReader();

        err = chip::app::DecodeSchemaElement(sc, _this->mReader, &allocator);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, sc.a == 20);
        NL_TEST_ASSERT(apSuite, sc.b == 30);
        NL_TEST_ASSERT(apSuite, sc.c.x == 99);
        NL_TEST_ASSERT(apSuite, sc.c.y == 17);

        for (size_t i = 0; i < sc.d.size(); i++) {
            NL_TEST_ASSERT(apSuite, sc.d.data()[i] == i);
        }

        for (size_t i = 0; i < sc.e.size(); i++) {
            NL_TEST_ASSERT(apSuite, sc.e.data()[i].x == i);
            NL_TEST_ASSERT(apSuite, sc.e.data()[i].y == i);
        }
    }
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecList(nlTestSuite * apSuite, void * apContext)
{
    chip::app::Cluster::TestCluster::StructC::Type sc;
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);
    uint8_t d[5];
    chip::app::Cluster::TestCluster::StructA::Type e[5];

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    for (size_t i = 0; i < ArraySize(d); i++) {
        d[i] = (uint8_t)i;
    }

    sc.a = 20;
    sc.b = 30;
    sc.c.x = 99;
    sc.c.y = 17;
    sc.d = chip::Span<uint8_t>{d};
    sc.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};

    for (size_t i = 0; i < ArraySize(e); i++) {
        e[i].x = (uint8_t)i;
        e[i].y = (uint8_t)i;
    }
    
    err = chip::app::EncodeSchemaElement(sc, _this->mWriter, TLV::AnonymousTag);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    _this->DumpBuf();

    memset(&sc, 0, sizeof(sc));
    memset(d, 0, sizeof(d));
    memset(e, 0, sizeof(e));

    sc.d = chip::Span<uint8_t>{d};
    sc.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};

    _this->SetupReader();

    err = chip::app::DecodeSchemaElement(sc, _this->mReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, sc.a == 20);
    NL_TEST_ASSERT(apSuite, sc.b == 30);
    NL_TEST_ASSERT(apSuite, sc.c.x == 99);
    NL_TEST_ASSERT(apSuite, sc.c.y == 17);

    for (size_t i = 0; i < ArraySize(d); i++) {
        NL_TEST_ASSERT(apSuite, d[i] == i);
    }

    for (size_t i = 0; i < ArraySize(e); i++) {
        NL_TEST_ASSERT(apSuite, e[i].x == i);
        NL_TEST_ASSERT(apSuite, e[i].y == i);
    }
}

void TestSchemaUtils::TestSchemaUtilsEncAndDecIteratableList(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    chip::app::TestSchemaUtils *_this = static_cast<chip::app::TestSchemaUtils *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();
    
    {    
        chip::app::Cluster::TestCluster::StructC::Type sc;
        uint8_t d[5];
        chip::app::Cluster::TestCluster::StructA::Type e[5];


        for (size_t i = 0; i < ArraySize(d); i++) {
            d[i] = (uint8_t)i;
        }

        sc.a = 20;
        sc.b = 30;
        sc.c.x = 99;
        sc.c.y = 17;
        sc.d = chip::Span<uint8_t>{d};
        sc.e = chip::Span<chip::app::Cluster::TestCluster::StructA::Type>{e};

        for (size_t i = 0; i < ArraySize(e); i++) {
            e[i].x = (uint8_t)i;
            e[i].y = (uint8_t)i;
        }

        err = chip::app::EncodeSchemaElement(sc, _this->mWriter, TLV::AnonymousTag);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    _this->DumpBuf();
    _this->SetupReader();

    {
        chip::app::Cluster::TestCluster2::IteratableStructC::Type sc;
        int i;

        err = sc.Decode(_this->mReader);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, sc.a == 20);
        NL_TEST_ASSERT(apSuite, sc.b == 30);
        NL_TEST_ASSERT(apSuite, sc.c.x == 99);
        NL_TEST_ASSERT(apSuite, sc.c.y == 17);

        i = 0;
        for (auto iter = sc.d.begin(); !(iter == sc.d.end()); ++iter) {
            NL_TEST_ASSERT(apSuite, *iter == i);
            i++;
        }

        i = 0;
        for (auto iter = sc.e.begin(); !(iter == sc.e.end()); ++iter) {
            auto val = *iter;
            NL_TEST_ASSERT(apSuite, val.x == i);
            NL_TEST_ASSERT(apSuite, val.y == i);
            i++;
        }
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

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins, &chip::gMessageCounterManager);
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
    NL_TEST_DEF("TestSchemaUtilsEncAndDecSimpleWithPrivateHeap", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecSimplePrivateHeap),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecNestedStruct", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecNestedStruct),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecList", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecList),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecIteratableList", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecIteratableList),
    NL_TEST_DEF("TestSchemaUtilsEncAndDecListWithPrivateHeap", chip::app::TestSchemaUtils::TestSchemaUtilsEncAndDecListPrivateHeap),
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
