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

const uint16_t kDefaultBufferSizeInBytes = 512;

SetupPayload GetDefaultPayload()
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

SetupPayload GetDefaultPayloadWithSerialNumber()
{
    SetupPayload payload = GetDefaultPayload();
    payload.serialNumber = "123456789QWDHANTYUIOP";

    return payload;
}

OptionalQRCodeInfo GetOptionalDefaultString()
{
    uint64_t tag;
    VendorTag(2, tag);

    OptionalQRCodeInfo info;
    info.tag  = tag;
    info.type = optionalQRCodeInfoTypeString;
    info.data = "myData";

    return info;
}

OptionalQRCodeInfo GetOptionalDefaultInt()
{
    uint64_t tag;
    VendorTag(3, tag);

    OptionalQRCodeInfo info;
    info.tag     = tag;
    info.type    = optionalQRCodeInfoTypeInt;
    info.integer = 12;

    return info;
}

SetupPayload GetDefaultPayloadWithOptionalDefaults()
{
    SetupPayload payload = GetDefaultPayloadWithSerialNumber();

    OptionalQRCodeInfo stringInfo = GetOptionalDefaultString();
    OptionalQRCodeInfo intInfo    = GetOptionalDefaultInt();

    payload.addOptionalData(stringInfo);
    payload.addOptionalData(intInfo);

    return payload;
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

void TestVendorTag(nlTestSuite * inSuite, void * inContext)
{
    uint64_t tag;
    NL_TEST_ASSERT(inSuite, VendorTag(128, tag) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, VendorTag(255, tag) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, VendorTag(127, tag) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, tag == ContextTag(127));
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

    err = payload.addOptionalData(stringInfo);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);

    err = payload.addOptionalData(intInfo);
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

void TestOptionalDataRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayloadWithOptionalDefaults();
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
    uint8_t optionalInfo[kTotalPayloadDataSizeInBytes];
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
    NL_TEST_DEF("Test Vendor Tag",                  TestVendorTag),
    NL_TEST_DEF("Test Simple Write",                TestSimpleWrite),
    NL_TEST_DEF("Test Simple Read",                 TestSimpleRead),
    NL_TEST_DEF("Test Optional Add Remove",         TestOptionalDataAddRemove),
    NL_TEST_DEF("Test Optional Write",              TestOptionalDataWrite),
    NL_TEST_DEF("Test Optional Write Serial",       TestOptionalDataWriteSerial),
    NL_TEST_DEF("Test Optional Write No Buffer",    TestOptionalDataWriteNoBuffer),
    NL_TEST_DEF("Test Optional Write Small Buffer", TestOptionalDataWriteSmallBuffer),
    NL_TEST_DEF("Test Optional Read",               TestOptionalDataRead),

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
