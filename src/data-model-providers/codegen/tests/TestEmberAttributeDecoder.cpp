/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/data-model/Encode.h>
#include <data-model-providers/codegen/EmberAttributeDecoder.h>
#include <functional>
#include <lib/support/tests/ExtraPwTestMacros.h>

// Empty implementation to satisfy linker for tests
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // Stub for tests
}

namespace chip {
namespace app {
namespace {

class MockServerCluster : public ServerClusterInterface
{
public:
    MockServerCluster(EndpointId endpoint, ClusterId cluster) : mPath(endpoint, cluster) {}

    CHIP_ERROR Startup(ServerClusterContext & context) override { return CHIP_NO_ERROR; }
    void Shutdown(ClusterShutdownType shutdownType) override {}

    Span<const ConcreteClusterPath> GetPaths() const override { return Span<const ConcreteClusterPath>(&mPath, 1); }

    DataVersion GetDataVersion(const ConcreteClusterPath & path) const override { return 0; }
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override { return {}; }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (request.path.mAttributeId == mAttributeId)
        {
            return encoder.Encode(mBoolValue);
        }
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override
    {
        return std::nullopt;
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    void SetBoolValue(bool value) { mBoolValue = value; }
    void SetAttributeId(AttributeId id) { mAttributeId = id; }

private:
    ConcreteClusterPath mPath;
    AttributeId mAttributeId = 1;
    bool mBoolValue          = false;
};

class FlexibleMockServerCluster : public ServerClusterInterface
{
public:
    FlexibleMockServerCluster(EndpointId endpoint, ClusterId cluster) : mPath(endpoint, cluster) {}

    CHIP_ERROR Startup(ServerClusterContext & context) override { return CHIP_NO_ERROR; }
    void Shutdown(ClusterShutdownType shutdownType) override {}

    Span<const ConcreteClusterPath> GetPaths() const override { return Span<const ConcreteClusterPath>(&mPath, 1); }

    DataVersion GetDataVersion(const ConcreteClusterPath & path) const override { return 0; }
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override { return {}; }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (mReadHandler)
        {
            return mReadHandler(encoder);
        }
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override
    {
        return std::nullopt;
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override
    {
        return CHIP_NO_ERROR;
    }

    void SetReadHandler(std::function<DataModel::ActionReturnStatus(AttributeValueEncoder &)> handler) { mReadHandler = handler; }

private:
    ConcreteClusterPath mPath;
    std::function<DataModel::ActionReturnStatus(AttributeValueEncoder &)> mReadHandler;
};

class EmberAttributeDecoderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mBuffer.Alloc(kBufferSize);
        ASSERT_NE(mBuffer.Get(), nullptr);
    }

    MutableByteSpan GetBuffer() { return MutableByteSpan(mBuffer.Get(), kBufferSize); }

    constexpr static size_t kBufferSize = 128;
    chip::Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

TEST_F(EmberAttributeDecoderTest, TestDecodeBool)
{
    MockServerCluster cluster(1, 2);
    cluster.SetAttributeId(3);
    cluster.SetBoolValue(true);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_BOOLEAN_ATTRIBUTE_TYPE, .emberSize = 1 };

    MutableByteSpan outBuffer = GetBuffer();

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(outBuffer.data()[0], 1);

    cluster.SetBoolValue(false);
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(outBuffer.data()[0], 0);
}

TEST_F(EmberAttributeDecoderTest, TestDecodeBoolNegative)
{
    MockServerCluster cluster(1, 2);
    cluster.SetAttributeId(3);
    cluster.SetBoolValue(true);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_BOOLEAN_ATTRIBUTE_TYPE, .emberSize = 1 };

    // Case 1: Buffer too small for TLV overhead
    {
        uint8_t buffer[2];
        MutableByteSpan outBuffer(buffer);

        CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    // Case 2: Buffer large enough to start TLV but not encode it
    {
        uint8_t buffer[10];
        MutableByteSpan outBuffer(buffer);

        CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(EmberAttributeDecoderTest, TestDecodeUnsignedInt)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_INT8U_ATTRIBUTE_TYPE, .emberSize = 1 };

    MutableByteSpan outBuffer = GetBuffer();

    // Test 8-bit unsigned int
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint8_t>(123)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(outBuffer.data()[0], 123);

    // Test 16-bit unsigned int
    params.emberType = ZCL_INT16U_ATTRIBUTE_TYPE;
    params.emberSize = 2;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint16_t>(0xABCD)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 2u);
    EXPECT_EQ(outBuffer.data()[0], 0xCD);
    EXPECT_EQ(outBuffer.data()[1], 0xAB);

    // Test 32-bit unsigned int
    params.emberType = ZCL_INT32U_ATTRIBUTE_TYPE;
    params.emberSize = 4;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint32_t>(0x12345678)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 4u);
    EXPECT_EQ(outBuffer.data()[0], 0x78);
    EXPECT_EQ(outBuffer.data()[1], 0x56);
    EXPECT_EQ(outBuffer.data()[2], 0x34);
    EXPECT_EQ(outBuffer.data()[3], 0x12);

    // Test 64-bit unsigned int
    params.emberType = ZCL_INT64U_ATTRIBUTE_TYPE;
    params.emberSize = 8;
    cluster.SetReadHandler(
        [](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint64_t>(0xAABBCCDDEEFF1122)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 8u);
    EXPECT_EQ(outBuffer.data()[0], 0x22);
    EXPECT_EQ(outBuffer.data()[1], 0x11);
    EXPECT_EQ(outBuffer.data()[2], 0xFF);
    EXPECT_EQ(outBuffer.data()[3], 0xEE);
    EXPECT_EQ(outBuffer.data()[4], 0xDD);
    EXPECT_EQ(outBuffer.data()[5], 0xCC);
    EXPECT_EQ(outBuffer.data()[6], 0xBB);
    EXPECT_EQ(outBuffer.data()[7], 0xAA);
}

TEST_F(EmberAttributeDecoderTest, TestDecodeSignedInt)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_INT8S_ATTRIBUTE_TYPE, .emberSize = 1 };

    MutableByteSpan outBuffer = GetBuffer();

    // Test 8-bit signed int
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<int8_t>(-10)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(static_cast<int8_t>(outBuffer.data()[0]), -10);

    // Test 16-bit signed int
    params.emberType = ZCL_INT16S_ATTRIBUTE_TYPE;
    params.emberSize = 2;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<int16_t>(-1234)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 2u);
    int16_t decoded16;
    memcpy(&decoded16, outBuffer.data(), 2);
    EXPECT_EQ(decoded16, -1234);

    // Test 32-bit signed int
    params.emberType = ZCL_INT32S_ATTRIBUTE_TYPE;
    params.emberSize = 4;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<int32_t>(-12345678)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 4u);
    int32_t decoded32;
    memcpy(&decoded32, outBuffer.data(), 4);
    EXPECT_EQ(decoded32, -12345678);

    // Test 64-bit signed int
    params.emberType = ZCL_INT64S_ATTRIBUTE_TYPE;
    params.emberSize = 8;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<int64_t>(-12345678901234LL)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 8u);
    int64_t decoded64;
    memcpy(&decoded64, outBuffer.data(), 8);
    EXPECT_EQ(decoded64, -12345678901234LL);
}

TEST_F(EmberAttributeDecoderTest, TestDecodeString)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_CHAR_STRING_ATTRIBUTE_TYPE, .emberSize = 0 };

    MutableByteSpan outBuffer = GetBuffer();

    // Test short string
    const char * testStr = "hello";
    cluster.SetReadHandler(
        [testStr](AttributeValueEncoder & encoder) { return encoder.Encode(CharSpan::fromCharString(testStr)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 6u); // 1 byte length + 5 bytes data
    EXPECT_EQ(outBuffer.data()[0], 5);
    EXPECT_EQ(memcmp(outBuffer.data() + 1, testStr, 5), 0);

    // Test long string
    params.emberType = ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE;
    outBuffer        = GetBuffer();
    err              = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 7u); // 2 bytes length + 5 bytes data
    EXPECT_EQ(outBuffer.data()[0], 5);
    EXPECT_EQ(outBuffer.data()[1], 0);
    EXPECT_EQ(memcmp(outBuffer.data() + 2, testStr, 5), 0);

    // Test long string with potential overlap
    char longStr[91];
    memset(longStr, 'A', 90);
    longStr[90] = '\0';

    params.emberType = ZCL_CHAR_STRING_ATTRIBUTE_TYPE;
    params.emberSize = 0;
    cluster.SetReadHandler(
        [longStr](AttributeValueEncoder & encoder) { return encoder.Encode(CharSpan::fromCharString(longStr)); });

    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 91u); // 1 byte length + 90 bytes data
    EXPECT_EQ(outBuffer.data()[0], 90);
    EXPECT_EQ(memcmp(outBuffer.data() + 1, longStr, 90), 0);
}

TEST(TestEmberAttributeDecoder, TestDecodeOctetString)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_OCTET_STRING_ATTRIBUTE_TYPE, .emberSize = 0 };

    uint8_t buffer[128];
    MutableByteSpan outBuffer(buffer);

    // Test short octet string
    const uint8_t kOctetData[] = { 1, 2, 3, 4, 5 };
    cluster.SetReadHandler([&](AttributeValueEncoder & encoder) { return encoder.Encode(ByteSpan(kOctetData)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 6u); // 1 byte length + 5 bytes data
    EXPECT_EQ(buffer[0], 5);
    EXPECT_EQ(memcmp(buffer + 1, kOctetData, 5), 0);

    // Test long octet string
    params.emberType = ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE;
    outBuffer        = MutableByteSpan(buffer);
    err              = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 7u); // 2 bytes length + 5 bytes data
    EXPECT_EQ(buffer[0], 5);
    EXPECT_EQ(buffer[1], 0);
    EXPECT_EQ(memcmp(buffer + 2, kOctetData, 5), 0);
}

TEST(TestEmberAttributeDecoder, TestDecodeNullable)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{
        .path = path, .cluster = cluster, .emberType = ZCL_BOOLEAN_ATTRIBUTE_TYPE, .emberSize = 1, .isNullable = true
    };

    uint8_t buffer[128];
    MutableByteSpan outBuffer(buffer);

    // Test nullable boolean with value
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(true); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(buffer[0], 1);

    // Test nullable boolean with null
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.EncodeNull(); });
    outBuffer = MutableByteSpan(buffer);
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(buffer[0], 0xFF); // Null value for boolean in Ember

    // Test nullable integer with null
    params.emberType = ZCL_INT8U_ATTRIBUTE_TYPE;
    params.emberSize = 1;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.EncodeNull(); });
    outBuffer = MutableByteSpan(buffer);
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(buffer[0], 0xFF); // Null value for uint8 in Ember

    // Test nullable short string with null
    params.emberType = ZCL_CHAR_STRING_ATTRIBUTE_TYPE;
    params.emberSize = 0;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.EncodeNull(); });
    outBuffer = MutableByteSpan(buffer);
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(buffer[0], 0xFF); // Null value for short string in Ember

    // Test nullable long string with null
    params.emberType = ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.EncodeNull(); });
    outBuffer = MutableByteSpan(buffer);
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 2u);
    EXPECT_EQ(buffer[0], 0xFF);
    EXPECT_EQ(buffer[1], 0xFF); // Null value for long string in Ember
}

TEST(TestEmberAttributeDecoder, TestDecodeEnum)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_ENUM8_ATTRIBUTE_TYPE, .emberSize = 1 };

    uint8_t buffer[128];
    MutableByteSpan outBuffer(buffer);

    // Test 8-bit enum
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint8_t>(5)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 1u);
    EXPECT_EQ(buffer[0], 5);

    // Test 16-bit enum
    params.emberType = ZCL_ENUM16_ATTRIBUTE_TYPE;
    params.emberSize = 2;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<uint16_t>(0x1234)); });
    outBuffer = MutableByteSpan(buffer);
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 2u);
    int16_t decoded16;
    memcpy(&decoded16, buffer, 2);
    EXPECT_EQ(decoded16, 0x1234);
}

TEST_F(EmberAttributeDecoderTest, TestDecodeFloat)
{
    FlexibleMockServerCluster cluster(1, 2);

    ConcreteAttributePath path(1, 2, 3);
    AttributeDecoderParams params{ .path = path, .cluster = cluster, .emberType = ZCL_SINGLE_ATTRIBUTE_TYPE, .emberSize = 4 };

    MutableByteSpan outBuffer = GetBuffer();

    // Test float
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<float>(1.23f)); });

    CHIP_ERROR err = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 4u);
    float decoded;
    memcpy(&decoded, outBuffer.data(), 4);
    EXPECT_EQ(decoded, 1.23f);

    // Test double
    params.emberType = ZCL_DOUBLE_ATTRIBUTE_TYPE;
    params.emberSize = 8;
    cluster.SetReadHandler([](AttributeValueEncoder & encoder) { return encoder.Encode(static_cast<double>(1.23456789)); });
    outBuffer = GetBuffer();
    err       = DecodeAttributeToEmberBuffer(params, outBuffer);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(outBuffer.size(), 8u);
    double decodedDouble;
    memcpy(&decodedDouble, outBuffer.data(), 8);
    EXPECT_EQ(decodedDouble, 1.23456789);
}

} // namespace
} // namespace app
} // namespace chip
