/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "TestHelpers.h"

#include <pw_unit_test/framework.h>

#include <nlbyteorder.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ScopedBuffer.h>

using namespace chip;

namespace {

class TestQRCodeTLV : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestQRCodeTLV, TestOptionalDataAddRemove)
{
    SetupPayload payload = GetDefaultPayload();
    std::vector<OptionalQRCodeInfo> optionalData;
    CHIP_ERROR err;

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_TRUE(optionalData.empty());

    err = payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_TRUE(optionalData.size());

    err = payload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_EQ(optionalData.size(), 2u);

    err = payload.removeOptionalVendorData(kOptionalDefaultStringTag);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_EQ(optionalData.size(), 1u);

    payload.removeOptionalVendorData(kOptionalDefaultIntTag);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_TRUE(optionalData.empty());

    err = payload.removeOptionalVendorData(kOptionalDefaultStringTag);
    EXPECT_EQ(err, CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_TRUE(optionalData.empty());

    err = payload.removeOptionalVendorData(kOptionalDefaultIntTag);
    EXPECT_EQ(err, CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalVendorData();
    EXPECT_TRUE(optionalData.empty());
}

TEST_F(TestQRCodeTLV, TestSimpleWrite)
{
    SetupPayload inPayload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    CHIP_ERROR err = generator.payloadBase38Representation(result);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    std::string result2;
    err = generator.payloadBase38RepresentationWithAutoTLVBuffer(result2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(result, result2);
}

TEST_F(TestQRCodeTLV, TestSimpleRead)
{
    SetupPayload inPayload = GetDefaultPayload();
    SetupPayload outPayload;

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    CHIP_ERROR err = generator.payloadBase38Representation(result);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    err                             = parser.populatePayload(outPayload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_TRUE(inPayload == outPayload);
}

TEST_F(TestQRCodeTLV, TestOptionalTagValues)
{
    SetupPayload payload = GetDefaultPayload();
    CHIP_ERROR err;

    err = payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue); // Vendor specific tag
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(0x80, kOptionalDefaultStringValue); // Vendor specific tag
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(0x82, kOptionalDefaultStringValue); // Vendor specific tag
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(127, kOptionalDefaultStringValue); // Common tag
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    err = payload.addOptionalVendorData(0, kOptionalDefaultStringValue); // Common tag
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestQRCodeTLV, TestSerialNumberAddRemove)
{
    SetupPayload inPayload = GetDefaultPayload();

    std::string sn;
    EXPECT_EQ(inPayload.getSerialNumber(sn), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_EQ(inPayload.removeSerialNumber(), CHIP_ERROR_KEY_NOT_FOUND);

    EXPECT_EQ(inPayload.addSerialNumber(kSerialNumberDefaultStringValue), CHIP_NO_ERROR);
    EXPECT_EQ(inPayload.getSerialNumber(sn), CHIP_NO_ERROR);
    EXPECT_EQ(sn, kSerialNumberDefaultStringValue);

    EXPECT_EQ(inPayload.addSerialNumber(kSerialNumberDefaultUInt32Value), CHIP_NO_ERROR);
    EXPECT_EQ(inPayload.getSerialNumber(sn), CHIP_NO_ERROR);
    EXPECT_EQ(sn, std::to_string(kSerialNumberDefaultUInt32Value));

    EXPECT_EQ(inPayload.removeSerialNumber(), CHIP_NO_ERROR);
    EXPECT_EQ(inPayload.getSerialNumber(sn), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_EQ(inPayload.removeSerialNumber(), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestQRCodeTLV, TestOptionalDataWriteSerial)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    SetupPayload inPayload = GetDefaultPayload();
    err                    = inPayload.addSerialNumber("1");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    err = generator.payloadBase38Representation(result);
    EXPECT_NE(err, CHIP_NO_ERROR);

    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    err = generator.payloadBase38Representation(result, optionalInfo, sizeof(optionalInfo));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    std::string result2;
    err = generator.payloadBase38RepresentationWithAutoTLVBuffer(result2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(result, result2);
}

TEST_F(TestQRCodeTLV, TestOptionalDataWrite)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase38Representation(result, optionalInfo, sizeof(optionalInfo));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    std::string result2;
    err = generator.payloadBase38RepresentationWithAutoTLVBuffer(result2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(result, result2);
}

TEST_F(TestQRCodeTLV, TestOptionalDataReadSerial)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.addSerialNumber(kSerialNumberDefaultStringValue);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.addSerialNumber(kSerialNumberDefaultUInt32Value);
    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST_F(TestQRCodeTLV, TestOptionalDataReadVendorInt)
{
    SetupPayload inPayload = GetDefaultPayload();
    inPayload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);

    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST_F(TestQRCodeTLV, TestOptionalDataReadVendorString)
{
    SetupPayload inPayload = GetDefaultPayload();
    inPayload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);

    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST_F(TestQRCodeTLV, TestOptionalDataRead)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST_F(TestQRCodeTLV, TestOptionalDataWriteNoBuffer)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    CHIP_ERROR err = generator.payloadBase38Representation(result);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST_F(TestQRCodeTLV, TestOptionalDataWriteSmallBuffer)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    std::string result;
    uint8_t optionalInfo[kSmallBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase38Representation(result, optionalInfo, sizeof(optionalInfo));
    EXPECT_NE(err, CHIP_NO_ERROR);
}

} // namespace
