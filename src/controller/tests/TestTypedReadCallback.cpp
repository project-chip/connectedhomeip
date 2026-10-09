/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/data-model/DecodableList.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
#include <app/data-model/WrappedStructEncoder.h>
#include <controller/TypedReadCallback.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;

const ConcreteDataAttributePath kPath(1, 0xFFF1FC05, 1);

struct ContextRecordingValue
{
    static constexpr bool kIsFabricScoped = false;
    uint8_t value                         = 0;
    DataModel::DecodeContext context      = DataModel::DecodeContext::kUnspecified;

    CHIP_ERROR Decode(TLV::TLVReader & reader) { return reader.Get(value); }

    CHIP_ERROR DecodeWithContext(TLV::TLVReader & reader, DataModel::DecodeContext decodeContext)
    {
        context = decodeContext;
        return reader.Get(value);
    }
};

// This test value omits the fields required by UnitTesting::SimpleStruct.
struct EmptyStruct
{
    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        return DataModel::WrappedStructEncoder(writer, tag).Finalize();
    }
};

class TestTypedReadCallback : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_SUCCESS(Platform::MemoryInit()); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    template <typename T>
    void Encode(const T & value)
    {
        TLV::TLVWriter writer;
        writer.Init(mBuffer);
        ASSERT_SUCCESS(DataModel::Encode(writer, TLV::AnonymousTag(), value));
        ASSERT_SUCCESS(writer.Finalize());
        mReader.Init(mBuffer, writer.GetLengthWritten());
        ASSERT_SUCCESS(mReader.Next());
    }

    void Deliver(ReadClient::Callback & callback)
    {
        callback.OnAttributeData(kPath, &mReader, StatusIB(Protocols::InteractionModel::Status::Success));
    }

    uint8_t mBuffer[64];
    TLV::TLVReader mReader;
};

TEST_F(TestTypedReadCallback, ReadContextReachesAttributeValue)
{
    Encode(uint8_t{ 7 });
    ASSERT_FALSE(HasFailure());

    bool called = false;
    Controller::TypedReadAttributeCallback<ContextRecordingValue> callback(
        kPath.mClusterId, kPath.mAttributeId,
        [&](const ConcreteDataAttributePath &, const ContextRecordingValue & value) {
            called = true;
            EXPECT_EQ(value.value, 7);
            EXPECT_EQ(value.context, DataModel::DecodeContext::kRead);
        },
        [](const ConcreteDataAttributePath *, CHIP_ERROR) { ADD_FAILURE(); }, [](auto *) {});
    Deliver(callback);
    EXPECT_TRUE(called);
}

TEST_F(TestTypedReadCallback, ReadContextReachesListElements)
{
    const uint8_t values[] = { 7, 8 };
    Encode(DataModel::List<const uint8_t>(values));
    ASSERT_FALSE(HasFailure());

    bool called = false;
    Controller::TypedReadAttributeCallback<DataModel::DecodableList<ContextRecordingValue>> callback(
        kPath.mClusterId, kPath.mAttributeId,
        [&](const ConcreteDataAttributePath &, const DataModel::DecodableList<ContextRecordingValue> & list) {
            called        = true;
            auto iterator = list.begin();
            for (const auto value : values)
            {
                ASSERT_TRUE(iterator.Next());
                EXPECT_EQ(iterator.GetValue().value, value);
                EXPECT_EQ(iterator.GetValue().context, DataModel::DecodeContext::kRead);
            }
            EXPECT_FALSE(iterator.Next());
            EXPECT_SUCCESS(iterator.GetStatus());
        },
        [](const ConcreteDataAttributePath *, CHIP_ERROR) { ADD_FAILURE(); }, [](auto *) {});
    Deliver(callback);
    EXPECT_TRUE(called);
}

TEST_F(TestTypedReadCallback, DecodeErrorReachesErrorCallback)
{
    using DecodedStruct = Clusters::UnitTesting::Structs::SimpleStruct::DecodableType;
    Encode(EmptyStruct{});
    ASSERT_FALSE(HasFailure());

    bool called = false;
    Controller::TypedReadAttributeCallback<DecodedStruct> callback(
        kPath.mClusterId, kPath.mAttributeId, [](const ConcreteDataAttributePath &, const DecodedStruct &) { ADD_FAILURE(); },
        [&](const ConcreteDataAttributePath * path, CHIP_ERROR error) {
            called = true;
            ASSERT_NE(path, nullptr);
            EXPECT_EQ(*path, kPath);
            EXPECT_EQ(error, CHIP_ERROR_MISSING_TLV_ELEMENT);
        },
        [](auto *) {});
    Deliver(callback);
    EXPECT_TRUE(called);
}

TEST_F(TestTypedReadCallback, LazyListDecodeErrorReachesIteratorStatus)
{
    using DecodedStruct        = Clusters::UnitTesting::Structs::SimpleStruct::DecodableType;
    const EmptyStruct values[] = { {} };
    Encode(DataModel::List<const EmptyStruct>(values));
    ASSERT_FALSE(HasFailure());

    bool called = false;
    Controller::TypedReadAttributeCallback<DataModel::DecodableList<DecodedStruct>> callback(
        kPath.mClusterId, kPath.mAttributeId,
        [&](const ConcreteDataAttributePath &, const DataModel::DecodableList<DecodedStruct> & list) {
            called        = true;
            auto iterator = list.begin();
            EXPECT_FALSE(iterator.Next());
            EXPECT_EQ(iterator.GetStatus(), CHIP_ERROR_MISSING_TLV_ELEMENT);
        },
        [](const ConcreteDataAttributePath *, CHIP_ERROR) { ADD_FAILURE(); }, [](auto *) {});
    Deliver(callback);
    EXPECT_TRUE(called);
}

} // namespace
