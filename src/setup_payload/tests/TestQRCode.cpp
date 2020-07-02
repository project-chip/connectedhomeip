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
#include "TestHelpers.h"

#include <iostream>
#include <nlbyteorder.h>
#include <nlunit-test.h>

using namespace chip;
using namespace std;

void TestRendezvousFlags(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.rendezvousInformation = RendezvousInformationFlags::kNone;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags::kWiFi;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags::kBLE;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags::kThread;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags::kEthernet;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags::kAllMask;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestPayloadByteArrayRep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    string expected = " 00000 000000000000000100000000000 000010000000 00000001 0 0000000000000001 0000000000001100 101";
    NL_TEST_ASSERT(inSuite, CompareBinary(payload, expected));
}

void TestPayloadBase41Rep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    CHIP_ERROR err  = generator.payloadBase41Representation(result);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    string expected = "CH:J20800G008008000";
    NL_TEST_ASSERT(inSuite, result == expected);
}

void TestBase41(nlTestSuite * inSuite, void * inContext)
{
    uint8_t input[] = { 10, 10, 10 };

    // basic stuff
    NL_TEST_ASSERT(inSuite, base41Encode(input, 0).compare("") == 0);
    NL_TEST_ASSERT(inSuite, base41Encode(input, 1).compare("A") == 0);
    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("SL1") == 0);
    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL1A") == 0);

    // test single odd byte corner conditions
    input[2] = 0;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL10") == 0);
    input[2] = 40;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL1.") == 0);
    input[2] = 41;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL101") == 0);
    input[2] = 255;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 3).compare("SL196") == 0);

    // testing optimized encoding
    // verify that we can't optimize a low value, need 3 chars
    input[0] = 255;
    input[1] = 0;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("960") == 0);
    // smallest optimized encoding, 256
    input[0] = 256 % 256;
    input[1] = 256 / 256;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("A6") == 0);
    // largest optimizated encoding value
    input[0] = ((kRadix * kRadix) - 1) % 256;
    input[1] = ((kRadix * kRadix) - 1) / 256;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("..") == 0);
    // can't optimize
    input[0] = ((kRadix * kRadix)) % 256;
    input[1] = ((kRadix * kRadix)) / 256;
    NL_TEST_ASSERT(inSuite, base41Encode(input, 2).compare("001") == 0);

    // fun with strings
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

    // single base41 means one byte of output
    NL_TEST_ASSERT(inSuite, base41Decode("A", decoded) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decoded.size() == 1);
    NL_TEST_ASSERT(inSuite, decoded[0] == 10);

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

    // odd byte(s) cases
    NL_TEST_ASSERT(inSuite, base41Decode("96", decoded) == CHIP_NO_ERROR); // this is 255
    NL_TEST_ASSERT(inSuite, decoded.size() == 1 && decoded[0] == 255);
    NL_TEST_ASSERT(inSuite, base41Decode("A6", decoded) == CHIP_NO_ERROR); // this is 256, needs 2 output bytes
    NL_TEST_ASSERT(inSuite, decoded.size() == 2 && decoded[0] + decoded[1] * 256 == 256);
    NL_TEST_ASSERT(inSuite, base41Decode("..", decoded) == CHIP_NO_ERROR); // this is (41*41)-1, or 1680, needs 2 output bytes
    NL_TEST_ASSERT(inSuite, decoded.size() == 2 && decoded[0] + decoded[1] * 256 == (kRadix * kRadix) - 1);
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
    test_payload.rendezvousInformation = static_cast<RendezvousInformationFlags>(1 << kRendezvousInfoFieldLengthInBits);
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid rendezvousInformation
    test_payload = payload;
    test_payload.rendezvousInformation =
        static_cast<RendezvousInformationFlags>(static_cast<uint16_t>(RendezvousInformationFlags::kAllMask) + 1);
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
    NL_TEST_DEF("Test Rendezvous Flags",                                            TestRendezvousFlags),
    NL_TEST_DEF("Test Base 41",                                                     TestBase41),
    NL_TEST_DEF("Test Bitset Length",                                               TestBitsetLen),
    NL_TEST_DEF("Test Payload Byte Array Representation",                           TestPayloadByteArrayRep),
    NL_TEST_DEF("Test Payload Base 41 Representation",                              TestPayloadBase41Rep),
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
