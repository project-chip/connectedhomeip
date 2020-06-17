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
#include "TestQRCodeTLV.h"
#include "TestHelpers.h"

#include <iostream>
#include <nlbyteorder.h>
#include <nlunit-test.h>

using namespace chip;
using namespace std;

void ComparePayloads(nlTestSuite * inSuite, void * inContext, SetupPayload & inPayload, SetupPayload & outPayload)
{
    NL_TEST_ASSERT(inSuite, inPayload.version == outPayload.version);
    NL_TEST_ASSERT(inSuite, inPayload.vendorID == outPayload.vendorID);
    NL_TEST_ASSERT(inSuite, inPayload.productID == outPayload.productID);
    NL_TEST_ASSERT(inSuite, inPayload.requiresCustomFlow == outPayload.requiresCustomFlow);
    NL_TEST_ASSERT(inSuite, inPayload.rendezvousInformation == outPayload.rendezvousInformation);
    NL_TEST_ASSERT(inSuite, inPayload.discriminator == outPayload.discriminator);
    NL_TEST_ASSERT(inSuite, inPayload.setUpPINCode == outPayload.setUpPINCode);
    NL_TEST_ASSERT(inSuite, inPayload.serialNumber.compare(outPayload.serialNumber) == 0);

    vector<OptionalQRCodeInfo> in  = inPayload.getAllOptionalData();
    vector<OptionalQRCodeInfo> out = outPayload.getAllOptionalData();

    NL_TEST_ASSERT(inSuite, in.size() == out.size());

    for (size_t i = 0; i < in.size(); i++)
    {
        NL_TEST_ASSERT(inSuite, in[i].type == out[i].type);
        NL_TEST_ASSERT(inSuite, in[i].tag == out[i].tag);
        NL_TEST_ASSERT(inSuite, in[i].data.compare(out[i].data) == 0);
    }
}
void CompareWriteRead(nlTestSuite * inSuite, void * inContext, SetupPayload & inPayload)
{
    SetupPayload outPayload;

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    err                             = parser.populatePayload(outPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ComparePayloads(inSuite, inContext, inPayload, outPayload);
}

void TestOptionalTagValues(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload          = GetDefaultPayload();
    OptionalQRCodeInfo stringInfo = GetOptionalDefaultString();
    CHIP_ERROR err;

    err = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    stringInfo.tag = 0;
    err            = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    stringInfo.tag = 128;
    err            = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    stringInfo.tag = 255;
    err            = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    stringInfo.tag = 127;
    err            = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    stringInfo.tag = 128;
    err            = payload.addCHIPOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    stringInfo.tag = 127;
    err            = payload.addCHIPOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    stringInfo.tag = 255;
    err            = payload.addCHIPOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestOptionalDataAddRemove(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload          = GetDefaultPayload();
    OptionalQRCodeInfo stringInfo = GetOptionalDefaultString();
    OptionalQRCodeInfo intInfo    = GetOptionalDefaultInt();
    vector<OptionalQRCodeInfo> optionalData;
    CHIP_ERROR err;

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    err = payload.addVendorOptionalData(intInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);

    err = payload.removeOptionalData(stringInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    payload.removeOptionalData(intInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeOptionalData(stringInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeOptionalData(intInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);
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

    ComparePayloads(inSuite, inContext, inPayload, outPayload);
}

void TestOptionalDataWriteSerial(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();
    inPayload.serialNumber = "1";

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
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
    inPayload.serialNumber = "1";

    CompareWriteRead(inSuite, inContext, inPayload);
}

void TestOptionalDataReadVendorInt(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload     = GetDefaultPayload();
    OptionalQRCodeInfo intInfo = GetOptionalDefaultInt();
    inPayload.addVendorOptionalData(intInfo);

    CompareWriteRead(inSuite, inContext, inPayload);
}

void TestOptionalDataReadVendorString(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload        = GetDefaultPayload();
    OptionalQRCodeInfo stringInfo = GetOptionalDefaultString();
    inPayload.addVendorOptionalData(stringInfo);

    CompareWriteRead(inSuite, inContext, inPayload);
}

void TestOptionalDataRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();

    CompareWriteRead(inSuite, inContext, inPayload);
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

void TestPayloadBinary(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 0));

    OptionalQRCodeInfo info = GetOptionalDefaultString();
    info.data               = "1";

    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 5));
    payload.removeOptionalData(info.tag);

    info         = GetOptionalDefaultInt();
    info.integer = 1;

    info.tag = 1;
    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 4));

    info.tag = 2;
    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 8));

    info.tag = 3;
    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 12));

    info.tag = 4;
    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 16));

    info.tag = 5;
    payload.addVendorOptionalData(info);
    NL_TEST_ASSERT(inSuite, CompareBinaryLength(payload, 19));
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Test Simple Write",                TestSimpleWrite),
    NL_TEST_DEF("Test Simple Read",                 TestSimpleRead),
    NL_TEST_DEF("Test Optional Add Remove",         TestOptionalDataAddRemove),
    NL_TEST_DEF("Test Optional Write",              TestOptionalDataWrite),
    NL_TEST_DEF("Test Optional Write Serial",       TestOptionalDataWriteSerial),
    NL_TEST_DEF("Test Optional Write No Buffer",    TestOptionalDataWriteNoBuffer),
    NL_TEST_DEF("Test Optional Write Small Buffer", TestOptionalDataWriteSmallBuffer),
    NL_TEST_DEF("Test Optional Read Serial",        TestOptionalDataReadSerial),
    NL_TEST_DEF("Test Optional Read Vendor String", TestOptionalDataReadVendorString),
    NL_TEST_DEF("Test Optional Read Vendor Int",    TestOptionalDataReadVendorInt),
    NL_TEST_DEF("Test Optional Read",               TestOptionalDataRead),
    NL_TEST_DEF("Test Optional Tag Values",         TestOptionalTagValues),
    NL_TEST_DEF("Test Payload Binary",              TestPayloadBinary),

    NL_TEST_SENTINEL()
};
// clang-format on

struct TestContext
{
    nlTestSuite * mSuite;
};

/**
 *  Main
 */
int TestQRCodeTLV(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-qrcode-optional-info-tests",
        &sTests[0],
        NULL,
        NULL
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
