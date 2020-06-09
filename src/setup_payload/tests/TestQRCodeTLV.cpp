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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <nlbyteorder.h>
#include <nlunit-test.h>

#include "Base41.cpp"
#include "QRCodeSetupPayloadGenerator.cpp"
#include "QRCodeSetupPayloadParser.cpp"
#include "SetupPayload.cpp"

using namespace chip;
using namespace std;

const uint16_t kSmallBufferSizeInBytes   = 1;
const uint16_t kDefaultBufferSizeInBytes = 512;

SetupPayload GetDefaultPayload(nlTestSuite * inSuite)
{
    SetupPayload payload;
    payload.version               = 1;
    payload.vendorID              = 2;
    payload.productID             = 3;
    payload.requiresCustomFlow    = 1;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 5;
    payload.setUpPINCode          = 13;

    return payload;
}

SetupPayload GetDefaultPayloadWithSerialNumber(nlTestSuite * inSuite)
{
    SetupPayload payload = GetDefaultPayload(inSuite);
    NL_TEST_ASSERT(inSuite, payload.addSerialNumber("123456789QWDHANTYUIOP") == CHIP_NO_ERROR);

    return payload;
}

OptionalQRCodeInfo GetOptionalDefaultString(nlTestSuite * inSuite)
{
    OptionalQRCodeInfo info;
    info.tag  = 2;
    info.type = optionalQRCodeInfoTypeString;
    info.data = "myData";

    return info;
}

CHIPQRCodeInfo GetCHIPOptionalDefaultString(nlTestSuite * inSuite)
{
    CHIPQRCodeInfo info;
    info.tag        = 2;
    info.type       = chipQRCodeInfoTypeString;
    info.stringData = "myData";

    return info;
}

OptionalQRCodeInfo GetOptionalDefaultInt(nlTestSuite * inSuite)
{
    OptionalQRCodeInfo info;
    info.tag     = 3;
    info.type    = optionalQRCodeInfoTypeInt;
    info.integer = 12;

    return info;
}

SetupPayload GetDefaultPayloadWithOptionalDefaults(nlTestSuite * inSuite)
{
    SetupPayload payload = GetDefaultPayloadWithSerialNumber(inSuite);

    payload.addVendorOptionalData(2, "myData");
    payload.addVendorOptionalData(3, 12);

    return payload;
}

void CompareSerialNumbers(nlTestSuite * inSuite, void * inContext, SetupPayload & inPayload, SetupPayload & outPayload)
{
    string inSerialNumber;
    string outSerialNumber;
    CHIP_ERROR inError  = inPayload.retrieveSerialNumber(inSerialNumber);
    CHIP_ERROR outError = outPayload.retrieveSerialNumber(outSerialNumber);
    if (inError == CHIP_ERROR_KEY_NOT_FOUND)
    {
        printf("\n inError\n");
    }
    if (outError == CHIP_ERROR_KEY_NOT_FOUND)
    {
        printf("\n outError\n");
    }
    if (inError == CHIP_ERROR_KEY_NOT_FOUND || outError == CHIP_ERROR_KEY_NOT_FOUND)
    {
        NL_TEST_ASSERT(inSuite, inError == outError);
        return;
    }
    NL_TEST_ASSERT(inSuite, inError == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, outError == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, inSerialNumber.size() > 0);
    NL_TEST_ASSERT(inSuite, outSerialNumber.size() > 0);

    NL_TEST_ASSERT(inSuite, inSerialNumber.compare(outSerialNumber) == 0);
}

void ComparePayloads(nlTestSuite * inSuite, void * inContext, SetupPayload & inPayload, SetupPayload & outPayload)
{
    NL_TEST_ASSERT(inSuite, inPayload.version == outPayload.version);
    NL_TEST_ASSERT(inSuite, inPayload.vendorID == outPayload.vendorID);
    NL_TEST_ASSERT(inSuite, inPayload.productID == outPayload.productID);
    NL_TEST_ASSERT(inSuite, inPayload.requiresCustomFlow == outPayload.requiresCustomFlow);
    NL_TEST_ASSERT(inSuite, inPayload.rendezvousInformation == outPayload.rendezvousInformation);
    NL_TEST_ASSERT(inSuite, inPayload.discriminator == outPayload.discriminator);
    NL_TEST_ASSERT(inSuite, inPayload.setUpPINCode == outPayload.setUpPINCode);
    CompareSerialNumbers(inSuite, inContext, inPayload, outPayload);

    vector<OptionalQRCodeInfo> in  = inPayload.getAllVendorOptionalData();
    vector<OptionalQRCodeInfo> out = outPayload.getAllVendorOptionalData();

    NL_TEST_ASSERT(inSuite, in.size() == out.size());

    for (size_t i = 0; i < in.size(); i++)
    {
        NL_TEST_ASSERT(inSuite, in[i].type == out[i].type);
        NL_TEST_ASSERT(inSuite, in[i].tag == out[i].tag);
        NL_TEST_ASSERT(inSuite, in[i].data.compare(out[i].data) == 0);
    }
}

void TestOptionalTagValues(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload(inSuite);
    CHIP_ERROR err;

    err = payload.addVendorOptionalData(2, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = payload.addVendorOptionalData(0, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = payload.addVendorOptionalData(128, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    err = payload.addVendorOptionalData(255, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    err = payload.addVendorOptionalData(127, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    CHIPQRCodeInfo info;
    info.stringData = "myData";
    info.tag        = 128;
    err             = payload.addCHIPOptionalData(info);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    info.tag = 127;
    err      = payload.addCHIPOptionalData(info);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    info.tag = 255;
    err      = payload.addCHIPOptionalData(info);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestAddRemoveSerialNumber(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload      = GetDefaultPayload(inSuite);
    string inSerialNumberString = "123456";
    string inSerialNumberUInt32;
    string outSerialNumber;

    NL_TEST_ASSERT(inSuite, inPayload.retrieveSerialNumber(outSerialNumber) == CHIP_ERROR_KEY_NOT_FOUND);

    NL_TEST_ASSERT(inSuite, inPayload.addSerialNumber(inSerialNumberString) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.retrieveSerialNumber(outSerialNumber) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inSerialNumberString.compare(outSerialNumber) == 0);

    inSerialNumberUInt32 = "0";
    NL_TEST_ASSERT(inSuite, inPayload.addSerialNumber(0) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.retrieveSerialNumber(outSerialNumber) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inSerialNumberUInt32.compare(outSerialNumber) == 0);

    inSerialNumberUInt32 = "34";
    NL_TEST_ASSERT(inSuite, inPayload.addSerialNumber(34) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.retrieveSerialNumber(outSerialNumber) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inSerialNumberUInt32.compare(outSerialNumber) == 0);

    NL_TEST_ASSERT(inSuite, inPayload.removeSerialNumber() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, inPayload.retrieveSerialNumber(outSerialNumber) == CHIP_ERROR_KEY_NOT_FOUND);
}

void TestOptionalDataAddRemove(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload(inSuite);
    vector<OptionalQRCodeInfo> optionalData;
    CHIP_ERROR err;

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.addVendorOptionalData(2, "myData");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    err = payload.addVendorOptionalData(3, 12);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);

    err = payload.removeVendorOptionalData(2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    payload.removeVendorOptionalData(3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeVendorOptionalData(2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeVendorOptionalData(3);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);
}

void TestOptionalTagValuesUsingInfoStruct(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload                = GetDefaultPayload(inSuite);
    OptionalQRCodeInfo vendorStringInfo = GetOptionalDefaultString(inSuite);
    CHIPQRCodeInfo chipStringInfo       = GetCHIPOptionalDefaultString(inSuite);
    CHIP_ERROR err;

    err = payload.addVendorOptionalData(vendorStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    vendorStringInfo.tag = 0;
    err                  = payload.addVendorOptionalData(vendorStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    vendorStringInfo.tag = 128;
    err                  = payload.addVendorOptionalData(vendorStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    vendorStringInfo.tag = 255;
    err                  = payload.addVendorOptionalData(vendorStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    vendorStringInfo.tag = 127;
    err                  = payload.addVendorOptionalData(vendorStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chipStringInfo.tag = 128;
    err                = payload.addCHIPOptionalData(chipStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chipStringInfo.tag = 127;
    err                = payload.addCHIPOptionalData(chipStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    chipStringInfo.tag = 255;
    err                = payload.addCHIPOptionalData(chipStringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestOptionalDataAddRemoveUsingInfoStruct(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload          = GetDefaultPayload(inSuite);
    OptionalQRCodeInfo stringInfo = GetOptionalDefaultString(inSuite);
    OptionalQRCodeInfo intInfo    = GetOptionalDefaultInt(inSuite);
    vector<OptionalQRCodeInfo> optionalData;
    CHIP_ERROR err;

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.addVendorOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    err = payload.addVendorOptionalData(intInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);

    err = payload.removeVendorOptionalData(stringInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    payload.removeVendorOptionalData(intInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeVendorOptionalData(stringInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    err = payload.removeVendorOptionalData(intInfo.tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_KEY_NOT_FOUND);

    optionalData = payload.getAllVendorOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);
}

void TestSimpleWrite(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload(inSuite);

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestSimpleRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload(inSuite);
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
    CHIP_ERROR err         = CHIP_NO_ERROR;
    SetupPayload inPayload = GetDefaultPayload(inSuite);
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

void TestOptionalDataWriteReadSerialNumberString(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    SetupPayload inPayload = GetDefaultPayload(inSuite);
    string inSerialNumber  = "B14D";
    err                    = inPayload.addSerialNumber("B14D");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    QRCodeSetupPayloadGenerator generator(inPayload);
    err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SetupPayload outPayload;
    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    err                             = parser.populatePayload(outPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    string outSerialNumber;
    err = outPayload.retrieveSerialNumber(outSerialNumber);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, outSerialNumber.compare(inSerialNumber) == 0);
}

void TestOptionalDataWriteReadSerialNumberUInt32(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    SetupPayload inPayload = GetDefaultPayload(inSuite);
    err                    = inPayload.addSerialNumber(14);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    QRCodeSetupPayloadGenerator generator(inPayload);
    err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SetupPayload outPayload;
    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);
    err                             = parser.populatePayload(outPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    string outSerialNumber;
    string expectedSerialNumber = "14";
    err                         = outPayload.retrieveSerialNumber(outSerialNumber);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, outSerialNumber.compare(expectedSerialNumber) == 0);
}

void TestOptionalDataWrite(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults(inSuite);

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    uint8_t optionalInfo[kDefaultBufferSizeInBytes];
    CHIP_ERROR err = generator.payloadBase41Representation(result, optionalInfo, sizeof(optionalInfo));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestOptionalDataRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults(inSuite);
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

void TestOptionalDataWriteNoBuffer(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults(inSuite);

    QRCodeSetupPayloadGenerator generator(inPayload);
    string result;
    CHIP_ERROR err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void TestOptionalDataWriteSmallBuffer(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults(inSuite);

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
static const nlTest sTests[] =
{
    NL_TEST_DEF("Test Simple Write",                                TestSimpleWrite),
    NL_TEST_DEF("Test Simple Read",                                 TestSimpleRead),
    NL_TEST_DEF("Test Optional Add Remove",                         TestOptionalDataAddRemove),
    NL_TEST_DEF("Test Optional Add Remove Using Info Struct",       TestOptionalDataAddRemoveUsingInfoStruct),
    NL_TEST_DEF("Test Optional Write",                              TestOptionalDataWrite),
    NL_TEST_DEF("Test Optional Write Serial",                       TestOptionalDataWriteSerial),
    NL_TEST_DEF("Test Optional Write No Buffer",                    TestOptionalDataWriteNoBuffer),
    NL_TEST_DEF("Test Optional Write Small Buffer",                 TestOptionalDataWriteSmallBuffer),
    NL_TEST_DEF("Test Optional Read",                               TestOptionalDataRead),
    NL_TEST_DEF("Test Optional Tag Values",                         TestOptionalTagValues),
    NL_TEST_DEF("Test Optional Tag Values Using Info Struct",       TestOptionalTagValuesUsingInfoStruct),
    NL_TEST_DEF("Test Add Remove Serial Number",                    TestAddRemoveSerialNumber),
    NL_TEST_DEF("Test Write Read Serial Number String",             TestOptionalDataWriteReadSerialNumberString),
    NL_TEST_DEF("Test Write Read Serial Number UInt32",             TestOptionalDataWriteReadSerialNumberUInt32),


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
