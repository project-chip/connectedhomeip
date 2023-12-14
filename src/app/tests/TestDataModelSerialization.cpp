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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

class TestDataModelSerialization
{
public:
    static void TestDataModelSerialization_EncAndDecSimpleStruct(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecSimpleStructNegativeEnum(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecNestedStruct(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecNestedStructList(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecDecodableNestedStructList(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_EncAndDecDecodableDoubleNestedStructList(nlTestSuite * apSuite, void * apContext);

    static void TestDataModelSerialization_OptionalFields(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_ExtraField(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_InvalidSimpleFieldTypes(nlTestSuite * apSuite, void * apContext);
    static void TestDataModelSerialization_InvalidListType(nlTestSuite * apSuite, void * apContext);

    static void NullablesOptionalsStruct(nlTestSuite * apSuite, void * apContext);
    static void NullablesOptionalsCommand(nlTestSuite * apSuite, void * apContext);

    void Shutdown();

protected:
    // Helper functions
    template <typename Encodable, typename Decodable>
    static void NullablesOptionalsEncodeDecodeCheck(nlTestSuite * apSuite, void * apContext, bool encodeNulls, bool encodeValues);

    template <typename Encodable, typename Decodable>
    static void NullablesOptionalsEncodeDecodeCheck(nlTestSuite * apSuite, void * apContext);

private:
    void SetupBuf();
    void DumpBuf();
    void SetupReader();

    System::TLVPacketBufferBackingStore mStore;
    TLV::TLVWriter mWriter;
    TLV::TLVReader mReader;
    nlTestSuite * mpSuite;
};

using namespace TLV;

TestDataModelSerialization gTestDataModelSerialization;

void TestDataModelSerialization::SetupBuf()
{
    System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::New(1024);
    mStore.Init(std::move(buf));

    mWriter.Init(mStore);
    mReader.Init(mStore);
}

void TestDataModelSerialization::Shutdown()
{
    System::PacketBufferHandle buf = mStore.Release();
}

void TestDataModelSerialization::DumpBuf()
{
    TLV::TLVReader reader;
    reader.Init(mStore);

    //
    // Enable this once the TLV pretty printer has been checked in.
    //
#if defined(ENABLE_TLV_PRINT_OUT) && ENABLE_TLV_PRINT_OUT
    TLV::Debug::Print(reader);
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
    TLV::Tag tag;
    T & value;
};

template <typename T>
TagValuePair<T> MakeTagValuePair(TLV::Tag tag, T & value)
{
    return TagValuePair<T>{ tag, value };
}

template <typename... ArgTypes>
CHIP_ERROR EncodeStruct(TLV::TLVWriter & writer, TLV::Tag tag, ArgTypes... Args)
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
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        t.f.Set(Clusters::UnitTesting::SimpleBitmap::kValueC);

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == Clusters::UnitTesting::SimpleEnum::kValueA);

        NL_TEST_ASSERT(apSuite, t.d.size() == 4);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.e, "chip"));
        NL_TEST_ASSERT(apSuite, t.f.HasOnly(Clusters::UnitTesting::SimpleBitmap::kValueC));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecSimpleStructNegativeEnum(nlTestSuite * apSuite,
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
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = static_cast<Clusters::UnitTesting::SimpleEnum>(10);
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        t.f.Set(Clusters::UnitTesting::SimpleBitmap::kValueC);

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, to_underlying(t.c) == 4);
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
        Clusters::UnitTesting::Structs::NestedStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = Clusters::UnitTesting::SimpleEnum::kValueB;
        t.c.d = buf;

        t.c.e = Span<char>{ strbuf, strlen(strbuf) };

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStruct::DecodableType t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c.a == 11);
        NL_TEST_ASSERT(apSuite, t.c.b == true);
        NL_TEST_ASSERT(apSuite, t.c.c == Clusters::UnitTesting::SimpleEnum::kValueB);

        NL_TEST_ASSERT(apSuite, t.c.d.size() == 4);

        for (uint32_t i = 0; i < t.c.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.c.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.c.e, "chip"));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecDecodableNestedStructList(nlTestSuite * apSuite,
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
        Clusters::UnitTesting::Structs::NestedStructList::Type t;
        uint8_t buf[4]     = { 0, 1, 2, 3 };
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };
        char strbuf[10]    = "chip";
        Clusters::UnitTesting::Structs::SimpleStruct::Type structList[4];
        uint8_t i = 0;
        ByteSpan spanList[4];

        t.a   = 20;
        t.b   = true;
        t.c.a = 11;
        t.c.b = true;
        t.c.c = Clusters::UnitTesting::SimpleEnum::kValueB;
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

        t.c.e = Span<char>{ strbuf, strlen(strbuf) };
        t.d   = structList;

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;
        int i;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c.a == 11);
        NL_TEST_ASSERT(apSuite, t.c.b == true);
        NL_TEST_ASSERT(apSuite, t.c.c == Clusters::UnitTesting::SimpleEnum::kValueB);

        NL_TEST_ASSERT(apSuite, StringMatches(t.c.e, "chip"));

        {
            i         = 0;
            auto iter = t.d.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                NL_TEST_ASSERT(apSuite, item.a == static_cast<uint8_t>(i));
                NL_TEST_ASSERT(apSuite, item.b == true);
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(apSuite, i == 4);
        }

        {
            i         = 0;
            auto iter = t.e.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                NL_TEST_ASSERT(apSuite, item == static_cast<uint32_t>(i + 10000));
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(apSuite, i == 4);
        }

        {
            i         = 0;
            auto iter = t.f.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();

                unsigned int j = 0;
                for (; j < item.size(); j++)
                {
                    NL_TEST_ASSERT(apSuite, item.data()[j] == j);
                }

                NL_TEST_ASSERT(apSuite, j == 4);
                i++;
            }

            NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(apSuite, i == 4);
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

            NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
            NL_TEST_ASSERT(apSuite, i == 4);
        }
    }
}

void TestDataModelSerialization::TestDataModelSerialization_EncAndDecDecodableDoubleNestedStructList(nlTestSuite * apSuite,
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
        Clusters::UnitTesting::Structs::DoubleNestedStructList::Type t;
        Clusters::UnitTesting::Structs::NestedStructList::Type n[4];
        Clusters::UnitTesting::Structs::SimpleStruct::Type structList[4];
        uint8_t i;

        t.a = n;

        i = 0;
        for (auto & item : structList)
        {
            item.a = static_cast<uint8_t>(35 + i);
            i++;
        }

        for (auto & item : n)
        {
            item.d = structList;
        }

        err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        _this->DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::DoubleNestedStructList::DecodableType t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        uint8_t i = 0;

        auto iter = t.a.begin();
        while (iter.Next())
        {
            auto & item     = iter.GetValue();
            auto nestedIter = item.d.begin();

            unsigned int j = 0;
            while (nestedIter.Next())
            {
                auto & nestedItem = nestedIter.GetValue();

                NL_TEST_ASSERT(apSuite, nestedItem.a == (static_cast<uint8_t>(35) + j));
                j++;
            }

            NL_TEST_ASSERT(apSuite, j == 4);
            i++;
        }

        NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, i == 4);
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
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        // Encode every field manually except a.
        {
            err = EncodeStruct(_this->mWriter, TLV::AnonymousTag(),
                               MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                               MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                               MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                               MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e));
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
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        _this->SetupReader();

        // Set the value of a to a specific value, and ensure it is not over-written after decode.
        t.a = 150;

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        // Ensure that the decoder did not over-write the value set in the generated object
        NL_TEST_ASSERT(apSuite, t.a == 150);

        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == Clusters::UnitTesting::SimpleEnum::kValueA);

        NL_TEST_ASSERT(apSuite, t.d.size() == 4);

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
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;
        uint8_t buf[4]  = { 0, 1, 2, 3 };
        char strbuf[10] = "chip";

        t.a = 20;
        t.b = true;
        t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
        t.d = buf;

        t.e = Span<char>{ strbuf, strlen(strbuf) };

        // Encode every field + an extra field.
        {
            err = EncodeStruct(
                _this->mWriter, TLV::AnonymousTag(),
                MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e),
                MakeTagValuePair(TLV::ContextTag(to_underlying(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE) + 1),
                                 t.a));
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
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        _this->SetupReader();

        // Ensure successful decode despite the extra field.
        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, t.a == 20);
        NL_TEST_ASSERT(apSuite, t.b == true);
        NL_TEST_ASSERT(apSuite, t.c == Clusters::UnitTesting::SimpleEnum::kValueA);

        NL_TEST_ASSERT(apSuite, t.d.size() == 4);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            NL_TEST_ASSERT(apSuite, t.d.data()[i] == i);
        }

        NL_TEST_ASSERT(apSuite, StringMatches(t.e, "chip"));
    }
}

void TestDataModelSerialization::TestDataModelSerialization_InvalidSimpleFieldTypes(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err;
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    //
    // Case #1: Swap out field a (an integer) with a boolean.
    //
    {
        //
        // Encode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
            t.d = buf;

            t.e = Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                err =
                    EncodeStruct(_this->mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e));
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
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

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
            Clusters::UnitTesting::Structs::SimpleStruct::Type t;
            uint8_t buf[4]  = { 0, 1, 2, 3 };
            char strbuf[10] = "chip";

            t.a = 20;
            t.b = true;
            t.c = Clusters::UnitTesting::SimpleEnum::kValueA;
            t.d = buf;

            t.e = Span<char>{ strbuf, strlen(strbuf) };

            // Encode every field manually except a.
            {
                err =
                    EncodeStruct(_this->mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.e),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e));
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
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

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
        Clusters::UnitTesting::Structs::NestedStructList::Type t;
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };

        t.e = intBuf;

        // Encode a list of integers for field d instead of a list of structs.
        {
            err =
                EncodeStruct(_this->mWriter, TLV::AnonymousTag(),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kD), t.e));
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
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;

        _this->SetupReader();

        err = DataModel::Decode(_this->mReader, t);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        auto iter     = t.d.begin();
        bool hadItems = false;

        while (iter.Next())
        {
            hadItems = true;
        }

        NL_TEST_ASSERT(apSuite, iter.GetStatus() != CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, !hadItems);
    }
}

namespace {
bool SimpleStructsEqual(const Clusters::UnitTesting::Structs::SimpleStruct::Type & s1,
                        const Clusters::UnitTesting::Structs::SimpleStruct::Type & s2)
{
    return s1.a == s2.a && s1.b == s2.b && s1.c == s2.c && s1.d.data_equal(s2.d) && s1.e.data_equal(s2.e) && s1.f == s2.f;
}

template <typename T>
bool ListsEqual(const DataModel::DecodableList<T> & list1, const DataModel::List<T> & list2)
{
    auto iter1 = list1.begin();
    auto iter2 = list2.begin();
    auto end2  = list2.end();
    while (iter1.Next())
    {
        if (iter2 == end2)
        {
            // list2 too small
            return false;
        }

        if (iter1.GetValue() != *iter2)
        {
            return false;
        }
        ++iter2;
    }
    if (iter1.GetStatus() != CHIP_NO_ERROR)
    {
        // Failed to decode
        return false;
    }
    if (iter2 != end2)
    {
        // list1 too small
        return false;
    }
    return true;
}

} // anonymous namespace

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck(nlTestSuite * apSuite, void * apContext, bool encodeNulls,
                                                                     bool encodeValues)
{
    auto * _this = static_cast<TestDataModelSerialization *>(apContext);

    _this->mpSuite = apSuite;
    _this->SetupBuf();

    static const char structStr[] = "something";
    const uint8_t structBytes[]   = { 1, 8, 17 };
    Clusters::UnitTesting::Structs::SimpleStruct::Type myStruct;
    myStruct.a = 17;
    myStruct.b = true;
    myStruct.c = Clusters::UnitTesting::SimpleEnum::kValueB;
    myStruct.d = ByteSpan(structBytes);
    myStruct.e = CharSpan::fromCharString(structStr);
    myStruct.f = Clusters::UnitTesting::SimpleBitmap(2);

    Clusters::UnitTesting::SimpleEnum enumListVals[] = { Clusters::UnitTesting::SimpleEnum::kValueA,
                                                         Clusters::UnitTesting::SimpleEnum::kValueC };
    DataModel::List<Clusters::UnitTesting::SimpleEnum> enumList(enumListVals);

    // Encode
    {
        // str needs to live until we call DataModel::Encode.
        static const char str[] = "abc";
        CharSpan strSpan        = CharSpan::fromCharString(str);
        Encodable encodable;
        if (encodeNulls)
        {
            encodable.nullableInt.SetNull();
            encodable.nullableOptionalInt.Emplace().SetNull();

            encodable.nullableString.SetNull();
            encodable.nullableOptionalString.Emplace().SetNull();

            encodable.nullableStruct.SetNull();
            encodable.nullableOptionalStruct.Emplace().SetNull();

            encodable.nullableList.SetNull();
            encodable.nullableOptionalList.Emplace().SetNull();
        }
        else if (encodeValues)
        {
            encodable.nullableInt.SetNonNull(static_cast<uint16_t>(5u));
            encodable.optionalInt.Emplace(static_cast<uint16_t>(6u));
            encodable.nullableOptionalInt.Emplace().SetNonNull() = 7;

            encodable.nullableString.SetNonNull(strSpan);
            encodable.optionalString.Emplace() = strSpan;
            encodable.nullableOptionalString.Emplace().SetNonNull(strSpan);

            encodable.nullableStruct.SetNonNull(myStruct);
            encodable.optionalStruct.Emplace(myStruct);
            encodable.nullableOptionalStruct.Emplace().SetNonNull(myStruct);

            encodable.nullableList.SetNonNull() = enumList;
            encodable.optionalList.Emplace(enumList);
            encodable.nullableOptionalList.Emplace().SetNonNull(enumList);
        }
        else
        {
            // Just encode the non-optionals, as null.
            encodable.nullableInt.SetNull();
            encodable.nullableString.SetNull();
            encodable.nullableStruct.SetNull();
            encodable.nullableList.SetNull();
        }

        CHIP_ERROR err = DataModel::Encode(_this->mWriter, TLV::AnonymousTag(), encodable);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        err = _this->mWriter.Finalize();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    // Decode
    {
        _this->SetupReader();

        Decodable decodable;
        CHIP_ERROR err = DataModel::Decode(_this->mReader, decodable);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        if (encodeNulls)
        {
            NL_TEST_ASSERT(apSuite, decodable.nullableInt.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalInt.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalInt.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalInt.Value().IsNull());

            NL_TEST_ASSERT(apSuite, decodable.nullableString.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalString.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalString.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalString.Value().IsNull());

            NL_TEST_ASSERT(apSuite, decodable.nullableStruct.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalStruct.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalStruct.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalStruct.Value().IsNull());

            NL_TEST_ASSERT(apSuite, decodable.nullableList.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalList.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalList.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalList.Value().IsNull());
        }
        else if (encodeValues)
        {
            static const char str[] = "abc";
            CharSpan strSpan        = CharSpan::fromCharString(str);

            NL_TEST_ASSERT(apSuite, !decodable.nullableInt.IsNull());
            NL_TEST_ASSERT(apSuite, decodable.nullableInt.Value() == 5);
            NL_TEST_ASSERT(apSuite, decodable.optionalInt.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.optionalInt.Value() == 6);
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalInt.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalInt.Value().IsNull());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalInt.Value().Value() == 7);

            NL_TEST_ASSERT(apSuite, !decodable.nullableString.IsNull());
            NL_TEST_ASSERT(apSuite, decodable.nullableString.Value().data_equal(strSpan));
            NL_TEST_ASSERT(apSuite, decodable.optionalString.HasValue());
            NL_TEST_ASSERT(apSuite, decodable.optionalString.Value().data_equal(strSpan));
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalString.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalString.Value().IsNull());
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalString.Value().Value().data_equal(strSpan));

            NL_TEST_ASSERT(apSuite, !decodable.nullableStruct.IsNull());
            NL_TEST_ASSERT(apSuite, SimpleStructsEqual(decodable.nullableStruct.Value(), myStruct));
            NL_TEST_ASSERT(apSuite, decodable.optionalStruct.HasValue());
            NL_TEST_ASSERT(apSuite, SimpleStructsEqual(decodable.optionalStruct.Value(), myStruct));
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalStruct.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalStruct.Value().IsNull());
            NL_TEST_ASSERT(apSuite, SimpleStructsEqual(decodable.nullableOptionalStruct.Value().Value(), myStruct));

            NL_TEST_ASSERT(apSuite, !decodable.nullableList.IsNull());
            NL_TEST_ASSERT(apSuite, ListsEqual(decodable.nullableList.Value(), enumList));
            NL_TEST_ASSERT(apSuite, decodable.optionalList.HasValue());
            NL_TEST_ASSERT(apSuite, ListsEqual(decodable.optionalList.Value(), enumList));
            NL_TEST_ASSERT(apSuite, decodable.nullableOptionalList.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalList.Value().IsNull());
            NL_TEST_ASSERT(apSuite, ListsEqual(decodable.nullableOptionalList.Value().Value(), enumList));
        }
        else
        {
            NL_TEST_ASSERT(apSuite, decodable.nullableInt.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalInt.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalInt.HasValue());

            NL_TEST_ASSERT(apSuite, decodable.nullableString.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalString.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalString.HasValue());

            NL_TEST_ASSERT(apSuite, decodable.nullableStruct.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalStruct.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalStruct.HasValue());

            NL_TEST_ASSERT(apSuite, decodable.nullableList.IsNull());
            NL_TEST_ASSERT(apSuite, !decodable.optionalList.HasValue());
            NL_TEST_ASSERT(apSuite, !decodable.nullableOptionalList.HasValue());
        }
    }
}

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck(nlTestSuite * apSuite, void * apContext)
{
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(apSuite, apContext, false, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(apSuite, apContext, true, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(apSuite, apContext, false, true);
}

void TestDataModelSerialization::NullablesOptionalsStruct(nlTestSuite * apSuite, void * apContext)
{
    using EncType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::Type;
    using DecType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>(apSuite, apContext);
}

void TestDataModelSerialization::NullablesOptionalsCommand(nlTestSuite * apSuite, void * apContext)
{
    using EncType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::Type;
    using DecType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>(apSuite, apContext);
}

int Initialize(void * apSuite)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    gTestDataModelSerialization.Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecSimple", TestDataModelSerialization::TestDataModelSerialization_EncAndDecSimpleStruct),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecSimpleStructNegativeEnum", TestDataModelSerialization::TestDataModelSerialization_EncAndDecSimpleStructNegativeEnum),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecNestedStruct", TestDataModelSerialization::TestDataModelSerialization_EncAndDecNestedStruct),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecDecodableNestedStructList",  TestDataModelSerialization::TestDataModelSerialization_EncAndDecDecodableNestedStructList),
    NL_TEST_DEF("TestDataModelSerialization_EncAndDecDecodableDoubleNestedStructList", TestDataModelSerialization::TestDataModelSerialization_EncAndDecDecodableDoubleNestedStructList),
    NL_TEST_DEF("TestDataModelSerialization_OptionalFields", TestDataModelSerialization::TestDataModelSerialization_OptionalFields),
    NL_TEST_DEF("TestDataModelSerialization_ExtraField",  TestDataModelSerialization::TestDataModelSerialization_ExtraField),
    NL_TEST_DEF("TestDataModelSerialization_InvalidSimpleFieldTypes", TestDataModelSerialization::TestDataModelSerialization_InvalidSimpleFieldTypes),
    NL_TEST_DEF("TestDataModelSerialization_InvalidListType", TestDataModelSerialization::TestDataModelSerialization_InvalidListType),
    NL_TEST_DEF("TestDataModelSerialization_NullablesOptionalsStruct", TestDataModelSerialization::NullablesOptionalsStruct),
    NL_TEST_DEF("TestDataModelSerialization_NullablesOptionalsCommand", TestDataModelSerialization::NullablesOptionalsCommand),
    NL_TEST_SENTINEL()
};
// clang-format on

nlTestSuite theSuite = { "TestDataModelSerialization", &sTests[0], Initialize, Finalize };

int DataModelSerializationTest()
{
    nlTestRunner(&theSuite, &gTestDataModelSerialization);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(DataModelSerializationTest)
