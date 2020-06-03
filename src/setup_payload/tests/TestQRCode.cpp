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

/**
 *    @file
 *      This file implements a unit test suite for the Quick Response
 *      code functionality.
 *
 */

#include "TestQRCode.h"

#include <assert.h>
#include <bitset>
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

SetupPayload GetDefaultPayload()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    return payload;
}

string toBinaryRepresentation(string base41Result)
{
    // Remove the kQRCodePrefix
    base41Result.erase(0, strlen(kQRCodePrefix));

    // Decode the base41 encoded string
    vector<uint8_t> buffer;
    base41Decode(base41Result, buffer);

    // Convert it to binary
    string binaryResult;
    for (int i = buffer.size() - 1; i >= 0; i--)
    {
        binaryResult += bitset<8>(buffer[i]).to_string();
    }

    // Insert spaces after each block
    size_t pos = binaryResult.size();

    pos -= kVersionFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kVendorIDFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kProductIDFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kCustomFlowRequiredFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kRendezvousInfoFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kPayloadDiscriminatorFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kSetupPINCodeFieldLengthInBits;
    binaryResult.insert(pos, " ");

    pos -= kPaddingFieldLengthInBits;
    binaryResult.insert(pos, " ");

    return binaryResult;
}

void TestPayloadByteArrayRep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    CHIP_ERROR err  = generator.payloadBase41Representation(result);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    string expected = " 00000 000000000000000100000000000 000010000000 00000001 0 0000000000000001 0000000000001100 101";
    NL_TEST_ASSERT(inSuite, toBinaryRepresentation(result) == expected);
}

void TestPayloadBase41Rep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    CHIP_ERROR err  = generator.payloadBase41Representation(result);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    string expected = "CH:J20800G0080080000";
    NL_TEST_ASSERT(inSuite, result == expected);
}

void TestBase41(nlTestSuite * inSuite, void * inContext)
{
    uint8_t input[] = { 10, 10, 10 };

    NL_TEST_ASSERT(inSuite, base41Encode(input, 0).compare("") == 0);

    NL_TEST_ASSERT(inSuite, base41Encode(input, 1).compare("A0") == 0);

    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("SL1") == 0);

    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL1A0") == 0);

    NL_TEST_ASSERT(inSuite,
                   base41Encode((uint8_t *) "Hello World!", sizeof("Hello World!") - 1).compare("GHF.KGL+48-G5LGK35") == 0);

    vector<uint8_t> decoded = vector<uint8_t>();
    NL_TEST_ASSERT(inSuite, base41Decode("GHF.KGL+48-G5LGK35", decoded) == CHIP_NO_ERROR);

    string hello_world;
    for (size_t _ = 0; _ < decoded.size(); _++)
    {
        hello_world += (char) decoded[_];
    }
    NL_TEST_ASSERT(inSuite, hello_world.compare("Hello World!") == 0);

    // short input
    NL_TEST_ASSERT(inSuite, base41Decode("A0", decoded) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decoded.size() == 1);

    // empty == empty
    NL_TEST_ASSERT(inSuite, base41Decode("", decoded) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decoded.size() == 0);
    // too short
    NL_TEST_ASSERT(inSuite, base41Decode("A", decoded) == CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // outside valid chars
    NL_TEST_ASSERT(inSuite, base41Decode("0\001", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("\0010", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("[0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("0[", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);

    // BOGUS chars
    NL_TEST_ASSERT(inSuite, base41Decode("!0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("\"0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("#0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("&0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("'0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("(0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode(")0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode(",0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode(";0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("<0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("=0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode(">0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base41Decode("@0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
}

void TestBitsetLen(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, kTotalPayloadDataSizeInBits % 8 == 0);
}

void TestSetupPayloadVerify(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();
    NL_TEST_ASSERT(inSuite, payload.isValidQRCodePayload() == true);

    // test invalid version
    SetupPayload test_payload = payload;
    test_payload.version      = 1 << kVersionFieldLengthInBits;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid rendezvousInformation
    test_payload                       = payload;
    test_payload.rendezvousInformation = 1 << kRendezvousInfoFieldLengthInBits;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid rendezvousInformation
    test_payload                       = payload;
    test_payload.rendezvousInformation = 1 << (kRendezvousInfoFieldLengthInBits - kRendezvousInfoReservedFieldLengthInBits);
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid discriminator
    test_payload               = payload;
    test_payload.discriminator = 1 << kPayloadDiscriminatorFieldLengthInBits;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid stetup PIN
    test_payload              = payload;
    test_payload.setUpPINCode = 1 << kSetupPINCodeFieldLengthInBits;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);
}

void TestInvalidQRCodePayload_WrongCharacterSet(nlTestSuite * inSuite, void * inContext)
{
    string invalidString = "adas12AA";

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didFail == true);
    NL_TEST_ASSERT(inSuite, payload.isValidQRCodePayload() == false);
}

void TestInvalidQRCodePayload_WrongLength(nlTestSuite * inSuite, void * inContext)
{
    string invalidString            = "AA12";
    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didFail == true);
    NL_TEST_ASSERT(inSuite, payload.isValidQRCodePayload() == false);
}

void TestPayloadEquality(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload      = GetDefaultPayload();
    SetupPayload equalPayload = GetDefaultPayload();

    bool result = payload == equalPayload;
    NL_TEST_ASSERT(inSuite, result == true);
}

void TestPayloadInEquality(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    SetupPayload unequalPayload  = GetDefaultPayload();
    unequalPayload.discriminator = 28;
    unequalPayload.setUpPINCode  = 121233;

    bool result = payload == unequalPayload;
    NL_TEST_ASSERT(inSuite, result == false);
}

void TestQRCodeToPayloadGeneration(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string base41Rep;
    CHIP_ERROR err  = generator.payloadBase41Representation(base41Rep);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    SetupPayload resultingPayload;
    QRCodeSetupPayloadParser parser(base41Rep);

    err        = parser.populatePayload(resultingPayload);
    didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);
    NL_TEST_ASSERT(inSuite, resultingPayload.isValidQRCodePayload() == true);

    bool result = payload == resultingPayload;
    NL_TEST_ASSERT(inSuite, result == true);
}

void TestExtractPayload(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, extractPayload(string("CH:ABC")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("CH:")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("H:")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("ASCH:")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("Z%CH:ABC%")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("Z%CH:ABC")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%CH:ABC")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%CH:ABC%")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%CH:ABC%DDD")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("CH:ABC%DDD")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("CH:ABC%")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%CH:")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%CH:%")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("A%")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("CH:%")).compare(string("")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("%CH:ABC")).compare(string("ABC")) == 0);
    NL_TEST_ASSERT(inSuite, extractPayload(string("ABC")).compare(string("")) == 0);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Test Bitset Length",                                               TestBitsetLen),
    NL_TEST_DEF("Test Payload Byte Array Representation",                           TestPayloadByteArrayRep),
    NL_TEST_DEF("Test Payload Base 41 Representation",                              TestPayloadBase41Rep),
    NL_TEST_DEF("Test Payload Base 41",                                             TestBase41),
    NL_TEST_DEF("Test Setup Payload Verify",                                        TestSetupPayloadVerify),
    NL_TEST_DEF("Test Payload Equality",                                            TestPayloadEquality),
    NL_TEST_DEF("Test Payload Inequality",                                          TestPayloadInEquality),
    NL_TEST_DEF("Test QRCode to Payload Generation",                                TestQRCodeToPayloadGeneration),
    NL_TEST_DEF("Test Invalid QR Code Payload - Wrong Character Set",               TestInvalidQRCodePayload_WrongCharacterSet),
    NL_TEST_DEF("Test Invalid QR Code Payload - Wrong  Length",                     TestInvalidQRCodePayload_WrongLength),
    NL_TEST_DEF("Test Extract Payload",                                             TestExtractPayload),

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
int TestQuickResponseCode(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-qrcode-general-tests",
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
