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

#include "SetupPayload.cpp"
#include "Base41.cpp"
#include "QRCodeSetupPayloadGenerator.cpp"
#include "QRCodeSetupPayloadParser.cpp"

using namespace chip;
using namespace std;

void TestOptionalDataPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    payload.version               = 1;
    payload.vendorID              = 2;
    payload.productID             = 3;
    payload.requiresCustomFlow    = 1;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 5;
    payload.setUpPINCode          = 13;
    payload.serialNumber          = "123456789QWDHANTYUIOP";

    vector<OptionalQRCodeInfo> optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 0);

    OptionalQRCodeInfo stringInfo;
    uint64_t tag;
    CHIP_ERROR err = VendorTag(2, tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    stringInfo.tag  = tag;
    stringInfo.type = optionalQRCodeInfoTypeString;
    stringInfo.data = "myData";
    payload.addOptionalData(stringInfo);

    OptionalQRCodeInfo intInfo;
    err = VendorTag(3, tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    intInfo.tag     = tag;
    intInfo.type    = optionalQRCodeInfoTypeInt;
    intInfo.integer = 12;
    payload.addOptionalData(intInfo);

    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);

    for (OptionalQRCodeInfo info : optionalData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            NL_TEST_ASSERT(inSuite, stringInfo.data.compare(info.data) == 0);
            NL_TEST_ASSERT(inSuite, stringInfo.tag == info.tag);
        }
        else if (info.type == optionalQRCodeInfoTypeInt)
        {
            NL_TEST_ASSERT(inSuite, intInfo.integer == info.integer);
            NL_TEST_ASSERT(inSuite, intInfo.tag == info.tag);
        }
    }

    payload.removeOptionalData(stringInfo.tag);
    optionalData = payload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, optionalData.size() == 1);
    OptionalQRCodeInfo info = optionalData.front();
    NL_TEST_ASSERT(inSuite, intInfo.integer == info.integer);
    NL_TEST_ASSERT(inSuite, intInfo.tag == info.tag);
}

void CheckSimpleWriteRead(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    payload.version               = 1;
    payload.vendorID              = 1;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 1;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 1;
    payload.setUpPINCode          = 1;
    payload.serialNumber          = "123456789QWERTYUIOP";

    OptionalQRCodeInfo stringInfo;
    stringInfo.type = optionalQRCodeInfoTypeString;
    stringInfo.data = "info1";
    uint64_t tag;
    CHIP_ERROR err = VendorTag(2, tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    stringInfo.tag = tag;
    payload.addOptionalData(stringInfo);

    OptionalQRCodeInfo intInfo;
    intInfo.type    = optionalQRCodeInfoTypeInt;
    intInfo.integer = 9;
    err             = VendorTag(4, tag);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    intInfo.tag = tag;
    payload.addOptionalData(intInfo);

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    err = generator.payloadBase41Representation(result);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(result);

    SetupPayload outPayload;
    err                                     = parser.populatePayload(outPayload);
    vector<OptionalQRCodeInfo> optionalData = outPayload.getAllOptionalData();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, optionalData.size() == 2);
    NL_TEST_ASSERT(inSuite, payload.serialNumber.compare(outPayload.serialNumber) == 0);
    for (OptionalQRCodeInfo info : optionalData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            NL_TEST_ASSERT(inSuite, stringInfo.data.compare(info.data) == 0);
            NL_TEST_ASSERT(inSuite, stringInfo.tag == info.tag);
        }
        else if (info.type == optionalQRCodeInfoTypeInt)
        {
            NL_TEST_ASSERT(inSuite, intInfo.integer == info.integer);
            NL_TEST_ASSERT(inSuite, intInfo.tag == info.tag);
        }
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

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Optional Data Remove Add Test",       TestOptionalDataPayload),
    NL_TEST_DEF("Simple Write Read Test",              CheckSimpleWriteRead),
    NL_TEST_DEF("Test Vendor Tag",                     TestVendorTag),

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
