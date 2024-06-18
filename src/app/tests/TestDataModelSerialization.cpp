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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

class TestDataModelSerialization : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
    void TearDown() override { System::PacketBufferHandle buf = mStore.Release(); }

    // Helper functions
    template <typename Encodable, typename Decodable>
    void NullablesOptionalsEncodeDecodeCheck(bool encodeNulls, bool encodeValues);

    template <typename Encodable, typename Decodable>
    void NullablesOptionalsEncodeDecodeCheck();

    void SetupBuf();
    void DumpBuf();
    void SetupReader();

    System::TLVPacketBufferBackingStore mStore;
    TLV::TLVWriter mWriter;
    TLV::TLVReader mReader;
};

using namespace TLV;

void TestDataModelSerialization::SetupBuf()
{
    System::PacketBufferHandle buf;

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
#if defined(ENABLE_TLV_PRINT_OUT) && ENABLE_TLV_PRINT_OUT
    TLV::Debug::Print(reader);
#endif
}

void TestDataModelSerialization::SetupReader()
{

    mReader.Init(mStore);
    EXPECT_EQ(mReader.Next(), CHIP_NO_ERROR);
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

TEST_F(TestDataModelSerialization, EncAndDecSimpleStruct)
{
    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
        EXPECT_TRUE(t.f.HasOnly(Clusters::UnitTesting::SimpleBitmap::kValueC));
    }
}

TEST_F(TestDataModelSerialization, EncAndDecSimpleStructNegativeEnum)

{

    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::Type t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        EXPECT_EQ(to_underlying(t.c), 4);
    }
}

TEST_F(TestDataModelSerialization, EncAndDecNestedStruct)
{

    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStruct::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c.a, 11);
        EXPECT_TRUE(t.c.b);
        EXPECT_EQ(t.c.c, Clusters::UnitTesting::SimpleEnum::kValueB);

        EXPECT_EQ(t.c.d.size(), 4u);

        for (uint32_t i = 0; i < t.c.d.size(); i++)
        {
            EXPECT_EQ(t.c.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.c.e, "chip"));
    }
}
TEST_F(TestDataModelSerialization, EncAndDecDecodableNestedStructList)
{

    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;
        int i;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c.a, 11);
        EXPECT_TRUE(t.c.b);
        EXPECT_EQ(t.c.c, Clusters::UnitTesting::SimpleEnum::kValueB);

        EXPECT_TRUE(StringMatches(t.c.e, "chip"));

        {
            i         = 0;
            auto iter = t.d.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item.a, static_cast<uint8_t>(i));
                EXPECT_TRUE(item.b);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }

        {
            i         = 0;
            auto iter = t.e.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item, static_cast<uint32_t>(i + 10000));
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
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
                    EXPECT_EQ(item.data()[j], j);
                }

                EXPECT_EQ(j, 4u);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }

        {
            i         = 0;
            auto iter = t.g.begin();

            while (iter.Next())
            {
                auto & item = iter.GetValue();
                EXPECT_EQ(item, i);
                i++;
            }

            EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
            EXPECT_EQ(i, 4);
        }
    }
}
TEST_F(TestDataModelSerialization, EncAndDecDecodableDoubleNestedStructList)
{

    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), t), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::DoubleNestedStructList::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

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

                EXPECT_EQ(nestedItem.a, (static_cast<uint8_t>(35) + j));
                j++;
            }

            EXPECT_EQ(j, 4u);
            i++;
        }

        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(i, 4u);
    }
}

TEST_F(TestDataModelSerialization, OptionalFields)
{

    SetupBuf();

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
            EXPECT_EQ(
                EncodeStruct(mWriter, TLV::AnonymousTag(),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e)),
                CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        SetupReader();

        // Set the value of a to a specific value, and ensure it is not over-written after decode.
        t.a = 150;

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        // Ensure that the decoder did not over-write the value set in the generated object
        EXPECT_EQ(t.a, 150);

        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
    }
}

TEST_F(TestDataModelSerialization, ExtraField)
{
    SetupBuf();

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
            EXPECT_EQ(EncodeStruct(
                          mWriter, TLV::AnonymousTag(),
                          MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                          MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                          MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                          MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                          MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e),
                          MakeTagValuePair(
                              TLV::ContextTag(to_underlying(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE) + 1), t.a)),
                      CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

        SetupReader();

        // Ensure successful decode despite the extra field.
        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        EXPECT_EQ(t.a, 20);
        EXPECT_TRUE(t.b);
        EXPECT_EQ(t.c, Clusters::UnitTesting::SimpleEnum::kValueA);

        EXPECT_EQ(t.d.size(), 4u);

        for (uint32_t i = 0; i < t.d.size(); i++)
        {
            EXPECT_EQ(t.d.data()[i], i);
        }

        EXPECT_TRUE(StringMatches(t.e, "chip"));
    }
}

TEST_F(TestDataModelSerialization, InvalidSimpleFieldTypes)
{
    SetupBuf();
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
                EXPECT_EQ(
                    EncodeStruct(mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.d),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e)),
                    CHIP_NO_ERROR);
            }

            EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
            DumpBuf();
        }
        //
        // Decode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

            SetupReader();

            EXPECT_NE(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        }
    }

    SetupBuf();
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
                EXPECT_EQ(
                    EncodeStruct(mWriter, TLV::AnonymousTag(),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kA), t.a),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kB), t.b),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kC), t.c),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kD), t.e),
                                 MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::SimpleStruct::Fields::kE), t.e)),
                    CHIP_NO_ERROR);
            }

            EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

            DumpBuf();
        }

        //
        // Decode
        //
        {
            Clusters::UnitTesting::Structs::SimpleStruct::DecodableType t;

            SetupReader();

            EXPECT_NE(DataModel::Decode(mReader, t), CHIP_NO_ERROR);
        }
    }
}

TEST_F(TestDataModelSerialization, InvalidListType)
{
    SetupBuf();

    //
    // Encode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::Type t;
        uint32_t intBuf[4] = { 10000, 10001, 10002, 10003 };

        t.e = intBuf;

        // Encode a list of integers for field d instead of a list of structs.
        {
            EXPECT_EQ(
                EncodeStruct(mWriter, TLV::AnonymousTag(),
                             MakeTagValuePair(TLV::ContextTag(Clusters::UnitTesting::Structs::NestedStructList::Fields::kD), t.e)),
                CHIP_NO_ERROR);
        }

        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
        DumpBuf();
    }

    //
    // Decode
    //
    {
        Clusters::UnitTesting::Structs::NestedStructList::DecodableType t;

        SetupReader();

        EXPECT_EQ(DataModel::Decode(mReader, t), CHIP_NO_ERROR);

        auto iter     = t.d.begin();
        bool hadItems = false;

        while (iter.Next())
        {
            hadItems = true;
        }

        EXPECT_NE(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_FALSE(hadItems);
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
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck(bool encodeNulls, bool encodeValues)
{

    SetupBuf();

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

        EXPECT_EQ(DataModel::Encode(mWriter, TLV::AnonymousTag(), encodable), CHIP_NO_ERROR);
        EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);
    }

    // Decode
    {
        SetupReader();

        Decodable decodable;
        EXPECT_EQ(DataModel::Decode(mReader, decodable), CHIP_NO_ERROR);

        if (encodeNulls)
        {
            EXPECT_TRUE(decodable.nullableInt.IsNull());
            EXPECT_FALSE(decodable.optionalInt.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalInt.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalInt.Value().IsNull());

            EXPECT_TRUE(decodable.nullableString.IsNull());
            EXPECT_FALSE(decodable.optionalString.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalString.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalString.Value().IsNull());

            EXPECT_TRUE(decodable.nullableStruct.IsNull());
            EXPECT_FALSE(decodable.optionalStruct.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalStruct.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalStruct.Value().IsNull());

            EXPECT_TRUE(decodable.nullableList.IsNull());
            EXPECT_FALSE(decodable.optionalList.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalList.HasValue());
            EXPECT_TRUE(decodable.nullableOptionalList.Value().IsNull());
        }
        else if (encodeValues)
        {
            static const char str[] = "abc";
            CharSpan strSpan        = CharSpan::fromCharString(str);

            EXPECT_FALSE(decodable.nullableInt.IsNull());
            EXPECT_EQ(decodable.nullableInt.Value(), 5);
            EXPECT_TRUE(decodable.optionalInt.HasValue());
            EXPECT_EQ(decodable.optionalInt.Value(), 6);
            EXPECT_TRUE(decodable.nullableOptionalInt.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalInt.Value().IsNull());
            EXPECT_EQ(decodable.nullableOptionalInt.Value().Value(), 7);

            EXPECT_FALSE(decodable.nullableString.IsNull());
            EXPECT_TRUE(decodable.nullableString.Value().data_equal(strSpan));
            EXPECT_TRUE(decodable.optionalString.HasValue());
            EXPECT_TRUE(decodable.optionalString.Value().data_equal(strSpan));
            EXPECT_TRUE(decodable.nullableOptionalString.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalString.Value().IsNull());
            EXPECT_TRUE(decodable.nullableOptionalString.Value().Value().data_equal(strSpan));

            EXPECT_FALSE(decodable.nullableStruct.IsNull());
            EXPECT_TRUE(SimpleStructsEqual(decodable.nullableStruct.Value(), myStruct));
            EXPECT_TRUE(decodable.optionalStruct.HasValue());
            EXPECT_TRUE(SimpleStructsEqual(decodable.optionalStruct.Value(), myStruct));
            EXPECT_TRUE(decodable.nullableOptionalStruct.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalStruct.Value().IsNull());
            EXPECT_TRUE(SimpleStructsEqual(decodable.nullableOptionalStruct.Value().Value(), myStruct));

            EXPECT_FALSE(decodable.nullableList.IsNull());
            EXPECT_TRUE(ListsEqual(decodable.nullableList.Value(), enumList));
            EXPECT_TRUE(decodable.optionalList.HasValue());
            EXPECT_TRUE(ListsEqual(decodable.optionalList.Value(), enumList));
            EXPECT_TRUE(decodable.nullableOptionalList.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalList.Value().IsNull());
            EXPECT_TRUE(ListsEqual(decodable.nullableOptionalList.Value().Value(), enumList));
        }
        else
        {
            EXPECT_TRUE(decodable.nullableInt.IsNull());
            EXPECT_FALSE(decodable.optionalInt.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalInt.HasValue());

            EXPECT_TRUE(decodable.nullableString.IsNull());
            EXPECT_FALSE(decodable.optionalString.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalString.HasValue());

            EXPECT_TRUE(decodable.nullableStruct.IsNull());
            EXPECT_FALSE(decodable.optionalStruct.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalStruct.HasValue());

            EXPECT_TRUE(decodable.nullableList.IsNull());
            EXPECT_FALSE(decodable.optionalList.HasValue());
            EXPECT_FALSE(decodable.nullableOptionalList.HasValue());
        }
    }
}

template <typename Encodable, typename Decodable>
void TestDataModelSerialization::NullablesOptionalsEncodeDecodeCheck()
{
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(false, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(true, false);
    NullablesOptionalsEncodeDecodeCheck<Encodable, Decodable>(false, true);
}

TEST_F(TestDataModelSerialization, NullablesOptionalsStruct)
{
    using EncType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::Type;
    using DecType = Clusters::UnitTesting::Structs::NullablesAndOptionalsStruct::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>();
}

TEST_F(TestDataModelSerialization, NullablesOptionalsCommand)
{
    using EncType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::Type;
    using DecType = Clusters::UnitTesting::Commands::TestComplexNullableOptionalRequest::DecodableType;
    NullablesOptionalsEncodeDecodeCheck<EncType, DecType>();
}

} // namespace
