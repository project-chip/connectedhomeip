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

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <support/ScopedBuffer.h>
#include <support/UnitTestRegistration.h>

using namespace chip;
using namespace std;

namespace {

void TestOptionalDataAddRemove(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();
    vector<OptionalQRCodeInfo> optionalData;
    CHIP_ERROR err;

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.empty());

    err = payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    err = payload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);

    err = payload.removeOptionalVendorData(kOptionalDefaultStringTag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    payload.removeOptionalVendorData(kOptionalDefaultIntTag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.empty());

    err = payload.removeOptionalVendorData(kOptionalDefaultStringTag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.empty());

    err = payload.removeOptionalVendorData(kOptionalDefaultIntTag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalVendorData();
    NL_TEST_ASSERT(inSuite, optionalData.empty());
}

void TestSimpleWrite(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestSimpleRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();
    SetupPayload outPayload;

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    err                             = parser.populatePayload(outPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, inPayload == outPayload);
}

void TestOptionalTagValues(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();
    CHIP_ERROR err;

    err = payload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(0, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(127, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = payload.addOptionalVendorData(128, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    err = payload.addOptionalVendorData(255, kOptionalDefaultStringValue);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
}

void TestSerialNumberAddRemove(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    string sn;
    NL_TEST_ASSERT(inSuite, inPayload.getSerialNumber(sn) == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, inPayload.removeSerialNumber() == CHIP_ERROR_KEY_NOT_FOUND);

    NL_TEST_ASSERT(inSuite, inPayload.addSerialNumber(kSerialNumberDefaultStringValue) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.getSerialNumber(sn) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, sn == kSerialNumberDefaultStringValue);

    NL_TEST_ASSERT(inSuite, inPayload.addSerialNumber(kSerialNumberDefaultUInt32Value) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.getSerialNumber(sn) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, sn == to_string(kSerialNumberDefaultUInt32Value));

    NL_TEST_ASSERT(inSuite, inPayload.removeSerialNumber() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.getSerialNumber(sn) == CHIP_ERROR_KEY_NOT_FOUND);
    NL_TEST_ASSERT(inSuite, inPayload.removeSerialNumber() == CHIP_ERROR_KEY_NOT_FOUND);
}

void TestOptionalDataWriteSerial(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    SetupPayload inPayload = GetDefaultPayload();
    err                    = inPayload.addSerialNumber("1");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestOptionalDataWrite(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestOptionalDataReadSerial(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.addSerialNumber(kSerialNumberDefaultStringValue);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.addSerialNumber(kSerialNumberDefaultUInt32Value);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestOptionalDataReadVendorInt(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();
    inPayload.addOptionalVendorData(kOptionalDefaultIntTag, kOptionalDefaultIntValue);

    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestOptionalDataReadVendorString(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();
    inPayload.addOptionalVendorData(kOptionalDefaultStringTag, kOptionalDefaultStringValue);

    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestOptionalDataRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestOptionalDataWriteNoBuffer(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void TestOptionalDataWriteSmallBuffer(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    uint8_t optionalInfo[kSmallBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Simple Write",                TestSimpleWrite),
    NL_TEST_DEF("Test Simple Read",                 TestSimpleRead),
    NL_TEST_DEF("Test Optional Add Remove",         TestOptionalDataAddRemove),
    NL_TEST_DEF("Test Serial Number Add Remove",    TestSerialNumberAddRemove),
    NL_TEST_DEF("Test Optional Write",              TestOptionalDataWrite),
    NL_TEST_DEF("Test Optional Write Serial",       TestOptionalDataWriteSerial),
    NL_TEST_DEF("Test Optional Write No Buffer",    TestOptionalDataWriteNoBuffer),
    NL_TEST_DEF("Test Optional Write Small Buffer", TestOptionalDataWriteSmallBuffer),
    NL_TEST_DEF("Test Optional Read Serial",        TestOptionalDataReadSerial),
    NL_TEST_DEF("Test Optional Read Vendor String", TestOptionalDataReadVendorString),
    NL_TEST_DEF("Test Optional Read Vendor Int",    TestOptionalDataReadVendorInt),
    NL_TEST_DEF("Test Optional Read",               TestOptionalDataRead),
    NL_TEST_DEF("Test Optional Tag Values",         TestOptionalTagValues),

    NL_TEST_SENTINEL()
};
// clang-format on

struct TestContext
{
    nlTestSuite * mSuite;
};

/**
 *  Set up the test suite.
 */
int TestQRCodeTLV_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestQRCodeTLV_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestQRCodeTLV()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-qrcode-optional-info-tests",
        &sTests[0],
        TestQRCodeTLV_Setup,
        TestQRCodeTLV_Teardown
    };
    // clang-format on
    TestContext context;

    context.mSuite = &theSuite;

    // Generate machine-readable, comma-separated value (CSV) output.
    nl_test_set_output_style(OUTPUT_CSV);

    // Run test suit against one context
    nlTestRunner(&theSuite, &context);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestQRCodeTLV);
