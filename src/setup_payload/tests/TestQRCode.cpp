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

#include "TestHelpers.h"

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <support/UnitTestRegistration.h>

using namespace chip;
using namespace std;

namespace {

void TestRendezvousFlags(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kNone);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kSoftAP);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kBLE);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation =
        RendezvousInformationFlags(RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation =
        RendezvousInformationFlags(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.rendezvousInformation = RendezvousInformationFlags(
        RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork);
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestCommissioningFlow(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.commissioningFlow = CommissioningFlow::kStandard;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.commissioningFlow = CommissioningFlow::kUserActionRequired;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));

    inPayload.commissioningFlow = CommissioningFlow::kCustom;
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestMaximumValues(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.version               = static_cast<uint8_t>((1 << kVersionFieldLengthInBits) - 1);
    inPayload.vendorID              = 0xFFFF;
    inPayload.productID             = 0xFFFF;
    inPayload.commissioningFlow     = CommissioningFlow::kCustom;
    inPayload.rendezvousInformation = RendezvousInformationFlags(
        RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork);
    inPayload.discriminator = static_cast<uint16_t>((1 << kPayloadDiscriminatorFieldLengthInBits) - 1);
    inPayload.setUpPINCode  = static_cast<uint32_t>((1 << kSetupPINCodeFieldLengthInBits) - 1);

    NL_TEST_ASSERT(inSuite, inPayload.isValidQRCodePayload());
    NL_TEST_ASSERT(inSuite, CheckWriteRead(inPayload));
}

void TestPayloadByteArrayRep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    string expected = " 0000 000000000000000100000000000 000010000000 00000001 00 0000000000000001 0000000000001100 101";
    NL_TEST_ASSERT(inSuite, CompareBinary(payload, expected));
}

void TestPayloadBase38Rep(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    CHIP_ERROR err  = generator.payloadBase38Representation(result);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    NL_TEST_ASSERT(inSuite, result == kDefaultPayloadQRCode);
}

void TestBase38(nlTestSuite * inSuite, void * inContext)
{
    uint8_t input[] = { 10, 10, 10 };

    // basic stuff
    NL_TEST_ASSERT(inSuite, base38Encode(input, 0).empty());
    NL_TEST_ASSERT(inSuite, base38Encode(input, 1) == "A0");
    NL_TEST_ASSERT(inSuite, base38Encode(input, 2) == "OT10");
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "-N.B0");

    // test single odd byte corner conditions
    input[2] = 0;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "OT100");
    input[2] = 40;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "Y6V91");
    input[2] = 41;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "KL0B1");
    input[2] = 255;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "Q-M08");

    // testing optimized encoding
    // verify that we can't optimize a low value using less characters
    // for 1 byte we need always 2 characters
    input[0] = 35;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 1) == "Z0");
    // for 2 bytes we need always 4 characters
    input[0] = 255;
    input[1] = 0;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 2) == "R600");
    // for 3 bytes we need always 5 characters
    input[0] = 46;
    input[1] = 0;
    input[2] = 0;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "81000");

    // verify maximal available values for each chunk size to check selecting proper characters number
    // for 1 byte we need 2 characters
    input[0] = 255;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 1) == "R6");
    // for 2 bytes we need 4 characters
    input[0] = 255;
    input[1] = 255;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 2) == "NE71");
    // for 3 bytes we need 5 characters
    input[0] = 255;
    input[1] = 255;
    input[2] = 255;
    NL_TEST_ASSERT(inSuite, base38Encode(input, 3) == "PLS18");

    // fun with strings
    NL_TEST_ASSERT(inSuite, base38Encode((uint8_t *) "Hello World!", sizeof("Hello World!") - 1) == "KKHF3W2S013OPM3EJX11");

    vector<uint8_t> decoded = vector<uint8_t>();
    NL_TEST_ASSERT(inSuite, base38Decode("KKHF3W2S013OPM3EJX11", decoded) == CHIP_NO_ERROR);

    string hello_world;
    for (uint8_t b : decoded)
    {
        hello_world += static_cast<char>(b);
    }
    NL_TEST_ASSERT(inSuite, hello_world == "Hello World!");

    // short input
    NL_TEST_ASSERT(inSuite, base38Decode("A0", decoded) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decoded.size() == 1);
    NL_TEST_ASSERT(inSuite, decoded[0] == 10);

    // empty == empty
    NL_TEST_ASSERT(inSuite, base38Decode("", decoded) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decoded.empty());

    // outside valid chars
    NL_TEST_ASSERT(inSuite, base38Decode("0\001", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("\0010", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("[0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("0[", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);

    // BOGUS chars
    NL_TEST_ASSERT(inSuite, base38Decode(" 0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("!0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("\"0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("#0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("$0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("%0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("&0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("'0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("(0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode(")0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("*0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("+0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode(",0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode(";0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("<0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("=0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode(">0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, base38Decode("@0", decoded) == CHIP_ERROR_INVALID_INTEGER_VALUE);

    // odd byte(s) cases
    NL_TEST_ASSERT(inSuite, base38Decode("R6", decoded) == CHIP_NO_ERROR); // this is 255
    NL_TEST_ASSERT(inSuite, decoded.size() == 1 && decoded[0] == 255);
    NL_TEST_ASSERT(inSuite, base38Decode("S600", decoded) == CHIP_NO_ERROR); // this is 256, needs 2 output bytes
    NL_TEST_ASSERT(inSuite, decoded.size() == 2 && decoded[0] + decoded[1] * 256 == 256);
    NL_TEST_ASSERT(inSuite, base38Decode("..00", decoded) == CHIP_NO_ERROR); // this is (38*38)-1, or 1443, needs 2 output bytes
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

    // test invalid commissioning flow
    SetupPayload test_payload      = payload;
    test_payload.commissioningFlow = CommissioningFlow::kCustom;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload());

    test_payload.commissioningFlow = static_cast<CommissioningFlow>(1 << kCommissioningFlowFieldLengthInBits);
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid version
    test_payload         = payload;
    test_payload.version = 1 << kVersionFieldLengthInBits;
    NL_TEST_ASSERT(inSuite, test_payload.isValidQRCodePayload() == false);

    // test invalid rendezvousInformation
    test_payload                       = payload;
    RendezvousInformationFlags invalid = RendezvousInformationFlags(
        RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork);
    invalid.SetRaw(static_cast<uint8_t>(invalid.Raw() + 1));
    test_payload.rendezvousInformation = invalid;
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
    string invalidString = kDefaultPayloadQRCode;
    invalidString.back() = ' '; // space is not contained in the base38 alphabet

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didFail == true);
    NL_TEST_ASSERT(inSuite, payload.isValidQRCodePayload() == false);
}

void TestInvalidQRCodePayload_WrongLength(nlTestSuite * inSuite, void * inContext)
{
    string invalidString = kDefaultPayloadQRCode;
    invalidString.pop_back();

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

    NL_TEST_ASSERT(inSuite, payload == equalPayload);
}

void TestPayloadInEquality(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    SetupPayload unequalPayload  = GetDefaultPayload();
    unequalPayload.discriminator = 28;
    unequalPayload.setUpPINCode  = 121233;

    NL_TEST_ASSERT(inSuite, !(payload == unequalPayload));
}

void TestQRCodeToPayloadGeneration(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    string base38Rep;
    CHIP_ERROR err  = generator.payloadBase38Representation(base38Rep);
    bool didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);

    SetupPayload resultingPayload;
    QRCodeSetupPayloadParser parser(base38Rep);

    err        = parser.populatePayload(resultingPayload);
    didSucceed = err == CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite, didSucceed == true);
    NL_TEST_ASSERT(inSuite, resultingPayload.isValidQRCodePayload() == true);

    bool result = payload == resultingPayload;
    NL_TEST_ASSERT(inSuite, result == true);
}

void TestExtractPayload(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, extractPayload(string("MT:ABC")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("MT:")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("H:")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("ASMT:")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("Z%MT:ABC%")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("Z%MT:ABC")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%MT:ABC")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%MT:ABC%")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%Z%MT:ABC%DDD")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("MT:ABC%DDD")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("MT:ABC%")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%MT:")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%MT:%")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("A%")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("MT:%")) == string(""));
    NL_TEST_ASSERT(inSuite, extractPayload(string("%MT:ABC")) == string("ABC"));
    NL_TEST_ASSERT(inSuite, extractPayload(string("ABC")) == string(""));
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Rendezvous Flags",                                            TestRendezvousFlags),
    NL_TEST_DEF("Test Commissioning Flow",                                          TestCommissioningFlow),
    NL_TEST_DEF("Test Maximum Values",                                              TestMaximumValues),
    NL_TEST_DEF("Test Base 38",                                                     TestBase38),
    NL_TEST_DEF("Test Bitset Length",                                               TestBitsetLen),
    NL_TEST_DEF("Test Payload Byte Array Representation",                           TestPayloadByteArrayRep),
    NL_TEST_DEF("Test Payload Base 38 Representation",                              TestPayloadBase38Rep),
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

} // namespace

/**
 *  Main
 */
int TestQuickResponseCode()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-qrcode-general-tests",
        &sTests[0],
        nullptr,
        nullptr
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

CHIP_REGISTER_TEST_SUITE(TestQuickResponseCode);
