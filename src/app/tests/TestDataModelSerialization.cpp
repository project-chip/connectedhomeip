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
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVData.hpp>
//#include <lib/core/CHIPTLVText.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/PrivateHeap.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemLayerImpl.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <cluster-objects.h>

namespace chip {
static System::LayerImpl gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static secure_channel::MessageCounterManager gMessageCounterManager;
static TransportMgr<Transport::UDP> gTransportManager;

namespace app {

class TestDataModelSerialization
{
public:
    static void TestDataModelSerialization_EncAndDecSimpleStruct(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecNestedStructList(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecIterableNestedStructList(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecIterableDoubleNestedStructList(nlTestSuite * apSuite, void * apContext);

    static void TestDataModelSerialization_OptionalFields(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_ExtraField(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_InvalidSimpleFieldTypes(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_InvalidListType(nlTestSuite * apSuite, void * apContext);

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

TestDataModelSerialization gTestDataModelSerialization;

void TestDataModelSerialization::SetupBuf()
{
    chip::System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::New(1024);
    mStore.Init(std::move(buf));

    mWriter.Init(mStore);
    mReader.Init(mStore);
}

void TestDataModelSerialization::DumpBuf()
{
    TLV::TLVReader reader;
    reader.Init(mStore);

    //
    // Enable this once the TLV pretty printer has been checked in.
    //
#if ENABLE_TLV_PRINT_OUT
    chip::TLV::Debug::Print(reader);
#endif
}

void TestDataModelSerialization::SetupReader()
{
    CHIP_ERROR err;

    mReader.Init(mStore);
    err = mReader.Next();

    NL_TEST_ASSERT(mpSuite, err == CHIP_NO_ERROR);
}

template <typename T>
struct TagValuePair
{
    uint64_t tag;
    T & value;
};

template <typename T>
TagValuePair<T> MakeTagValuePair(uint64_t tag, T & value)
{
    return TagValuePair<T>{ tag, value };
}

template <typename... ArgTypes>
CHIP_ERROR EncodeStruct(TLV::TLVWriter & writer, uint64_t tag, ArgTypes... Args)
{
    using expand_type = int[];
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, type));
    (void) (expand_type{ (DataModel::Encode(writer, Args.tag, Args.value), 0)... });
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

bool StringMatches(Span<const char> str1, const char * str2)
{
    if (str1.data() == nullptr || str2 == nullptr)
    {
        return false;
    }

    if (str1.size() != strlen(str2))
    {
        return false;
    }

    return (strncmp(str1.data(), str2, str1.size()) == 0);
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecSimpleStruct(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = clusters::TestCluster::SimpleEnum::VALUEA;
        t.d = buf;

        t.e = chip::Span<char>{ strbuf, strlen(strbuf) };

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == clusters::TestCluster::SimpleEnum::VALUEA);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.e, "chip"));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::NestedStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = clusters::TestCluster::SimpleEnum::VALUEB;
        t.c.d = buf;

        t.c.e = chip::Span<char>{ strbuf, strlen(strbuf) };

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::NestedStruct::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c.a == 11);
        NL_TEST_ASSERT(apSuite, t.c.b == true);
        NL_TEST_ASSERT(apSuite, t.c.c == clusters::TestCluster::SimpleEnum::VALUEB);

        for (uint32_t i = 0; i < t.c.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.c.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.c.e, "chip"));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecIterableNestedStructList(nlTestSuite * apSuite,
                                                                                              void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::NestedStructList::Type t;
        uint8_t buf[4]     = { 0, 1, 2, 3 };
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };
        char strbuf[10]    = "chip";
        clusters::TestCluster::SimpleStruct::Type structList[4];
        uint8_t i = 0;
        chip::ByteSpan spanList[4];

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = clusters::TestCluster::SimpleEnum::VALUEB;
        t.c.d = buf;
        t.e   = intBuf;

        for (auto & item : structList)
        {
            item.a = i;
            item.b = true;
            i++;
        }

        t.f = spanList;

        spanList[0] = buf;
        spanList[1] = buf;
        spanList[2] = buf;
        spanList[3] = buf;

        t.g = buf;

        t.c.e = chip::Span<char>{ strbuf, strlen(strbuf) };
        t.d   = structList;

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::IteratableNestedStructList::Type t;
        int i;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c.a == 11);
        NL_TEST_ASSERT(apSuite, t.c.b == true);
        NL_TEST_ASSERT(apSuite, t.c.c == clusters::TestCluster::SimpleEnum::VALUEB);
        NL_TEST_ASSERT(apSuite, StringMatches(t.c.e, "chip"));

        {
            i         = 0;
            auto iter = t.d.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                NL_TEST_ASSERT(apSuite, item.a == (uint8_t) i);
                NL_TEST_ASSERT(apSuite, item.b == true);
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetError() == CHIP_NO_ERROR);
        }

        {
            i         = 0;
            auto iter = t.e.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                NL_TEST_ASSERT(apSuite, item == (unsigned int) (i + 10000));
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetError() == CHIP_NO_ERROR);
        }

        {
            i         = 0;
            auto iter = t.f.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();

                for (unsigned int j = 0; j < item.size(); j++)
                {
                    NL_TEST_ASSERT(apSuite, item.data()[j] == j);
                }

                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetError() == CHIP_NO_ERROR);
        }

        {
            i         = 0;
            auto iter = t.g.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();
                NL_TEST_ASSERT(apSuite, item == i);
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetError() == CHIP_NO_ERROR);
        }
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecIterableDoubleNestedStructList(nlTestSuite * apSuite,
                                                                                                    void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::DoubleNestedStructList::Type t;
        clusters::TestCluster::NestedStructList::Type n[4];
        clusters::TestCluster::SimpleStruct::Type structList[4];
        uint8_t i;

        t.a = n;

        i = 0;
        for (auto & item : structList)
        {
            item.a = (uint8_t)(35 + i);
            i++;
        }

        for (auto & item : n)
        {
            item.d = structList;
        }

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::IteratableDoubleNestedStructList::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint8_t i = 0;

        auto iter = t.a.begin();
        while (iter.Next())
        {
            auto & item     = iter.GetValue();
            auto nestedIter = item.d.begin();

            i = 0;
            while (nestedIter.Next())
            {
                auto & nestedItem = nestedIter.GetValue();

                NL_TEST_ASSERT(apSuite, nestedItem.a == ((uint8_t) 35 + i));
                i++;
            }
        }

        NL_TEST_ASSERT(apSuite, iter.GetError() == CHIP_NO_ERROR);
    }
}

void TestDataModelSerialization::TestDataModelSerialization_OptionalFields(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = clusters::TestCluster::SimpleEnum::VALUEA;
        t.d = buf;

        t.e = chip::Span<char>{ strbuf, strlen(strbuf) };

        // Encode every field manually except a.
        {
            err = EncodeStruct(_this->mWriter, TLV::AnonymousTag,
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kBFieldId), t.b),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kCFieldId), t.c),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kDFieldId), t.d),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kEFieldId), t.e));
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;

        _this->SetupReader();

        // Set the value of a to a specific value, and ensure it is not over-written after decode.
        t.a = 150;

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        // Ensure that the decoder did not over-write the value set in the generated object
        NL_TEST_ASSERT(apSuite, t.a == 150);

        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == clusters::TestCluster::SimpleEnum::VALUEA);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.e, "chip"));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_ExtraField(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = clusters::TestCluster::SimpleEnum::VALUEA;
        t.d = buf;

        t.e = chip::Span<char>{ strbuf, strlen(strbuf) };

        // Encode every field + an extra field.
        {
            err = EncodeStruct(_this->mWriter, TLV::AnonymousTag,
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kAFieldId), t.a),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kBFieldId), t.b),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kCFieldId), t.c),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kDFieldId), t.d),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kEFieldId), t.e),
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kEFieldId + 1), t.a));
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::SimpleStruct::Type t;

        _this->SetupReader();

        // Ensure successful decode despite the extra field.
        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == clusters::TestCluster::SimpleEnum::VALUEA);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.e, "chip"));
    }
}

struct ByteSpann : public Span<uint8_t>
{
};

struct UintSpann : public Span<uint8_t>
{
};

void DoSomething(ByteSpann s) {}

void TestDataModelSerialization::TestDataModelSerialization_InvalidSimpleFieldTypes(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();
    ByteSpann s;

    DoSomething(s);

    //
    // Case #1: Swap out field a (an integer) with a boolean.
    //
    {
        //
        // Encode
        //
        {
            clusters::TestCluster::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = clusters::TestCluster::SimpleEnum::VALUEA;
            t.d = buf;

            t.e = chip::Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                err = EncodeStruct(_this->mWriter, TLV::AnonymousTag,
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kAFieldId), t.b),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kBFieldId), t.b),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kCFieldId), t.c),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kDFieldId), t.d),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kEFieldId), t.e));
                NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
            }

            err = _this->mWriter.Finalize();
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            _this->DumpBuf();
        }

        //
        // Decode
        //
        {
            clusters::TestCluster::SimpleStruct::Type t;

            _this->SetupReader();

            err = DataModel::Decode(_this->mReader, t);
            NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
        }
    }

    _this->SetupBuf();

    //
    // Case #2: Swap out an octet string with a UTF-8 string.
    //
    {
        //
        // Encode
        //
        {
            clusters::TestCluster::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = clusters::TestCluster::SimpleEnum::VALUEA;
            t.d = buf;

            t.e = chip::Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                err = EncodeStruct(_this->mWriter, TLV::AnonymousTag,
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kAFieldId), t.a),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kBFieldId), t.b),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kCFieldId), t.c),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kDFieldId), t.e),
                                   MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::SimpleStruct::kEFieldId), t.e));
                NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
            }

            err = _this->mWriter.Finalize();
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            _this->DumpBuf();
        }

        //
        // Decode
        //
        {
            clusters::TestCluster::SimpleStruct::Type t;

            _this->SetupReader();

            err = DataModel::Decode(_this->mReader, t);
            NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
        }
    }
}

void TestDataModelSerialization::TestDataModelSerialization_InvalidListType(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Encode
    //
    {
        clusters::TestCluster::NestedStructList::Type t;
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };

        t.e = intBuf;

        // Encode a list of integers for field d instead of a list of structs.
        {
            err = EncodeStruct(_this->mWriter, TLV::AnonymousTag,
                               MakeTagValuePair(TLV::ContextTag(clusters::TestCluster::NestedStructList::kDFieldId), t.e));
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        clusters::TestCluster::IteratableNestedStructList::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        auto iter     = t.d.begin();
        bool hadItems = false;

        while (iter.Next())
        {
            hadItems = true;
        }

        NL_TEST_ASSERT(apSuite, iter.GetError() != CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, !hadItems);
    }
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

    chip::gSystemLayer.Init();

    err = chip::gSessionManager.Init(&chip::gSystemLayer, &chip::gTransportManager, &fabrics, &chip::gMessageCounterManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecSimple", chip::app::TestDataModelSerialization::TestDataModelSerialization_EncAndDecSimpleStruct),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecNestedStruct", chip::app::TestDataModelSerialization::TestDataModelSerialization_EncAndDecNestedStruct),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecIterableNestedStructList", chip::app::TestDataModelSerialization::TestDataModelSerialization_EncAndDecIterableNestedStructList),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecIterableDoubleNestedStructList", chip::app::TestDataModelSerialization::TestDataModelSerialization_EncAndDecIterableDoubleNestedStructList),
    NL_TEST_DEF("TestDataModelSerialization_OptionalFields", chip::app::TestDataModelSerialization::TestDataModelSerialization_OptionalFields),
    NL_TEST_DEF("TestDataModelSerialization_ExtraField", chip::app::TestDataModelSerialization::TestDataModelSerialization_ExtraField),
    NL_TEST_DEF("TestDataModelSerialization_InvalidSimpleFieldTypes", chip::app::TestDataModelSerialization::TestDataModelSerialization_InvalidSimpleFieldTypes),
    NL_TEST_DEF("TestDataModelSerialization_InvalidListType", chip::app::TestDataModelSerialization::TestDataModelSerialization_InvalidListType),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestDataModelSerialization()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestDataModelSerialization",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, &chip::app::gTestDataModelSerialization);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDataModelSerialization)
