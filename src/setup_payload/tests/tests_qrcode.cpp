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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "SetupPayload.cpp"
#include "Base41.cpp"
#include "QRCodeSetupPayloadGenerator.cpp"
#include "QRCodeSetupPayloadParser.cpp"

using namespace chip;
using namespace std;

#define EXPECT_EQ(x, y)                                                                                                            \
    ((x) != (y)) ? cerr << endl << __FILE__ << ":" << __LINE__ << ":error EXPECT_EQ(" << x << ", " << y << ")\n", 1 : 0

int testPayloadByteArrayRep()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    QRCodeSetupPayloadGenerator generator(payload);
    string result   = generator.payloadBinaryRepresentation();
    string expected = "000000000000000000100000000000010000000000000000100000000000000000010000000000000011001010";

    return EXPECT_EQ(result, expected);
}

int testPayloadBase41Rep()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    QRCodeSetupPayloadGenerator generator(payload);
    string result   = generator.payloadBase41Representation();
    string expected = "CH:J20800G00HKJ000";

    return EXPECT_EQ(result, expected);
}

int testBase41()
{
    int surprises = 0;

    uint8_t input[] = { 10, 10, 10 };

    surprises += EXPECT_EQ(base41Encode(input, 0), "");

    surprises += EXPECT_EQ(base41Encode(input, 1), "A0");

    surprises += EXPECT_EQ(base41Encode(input, 2), "SL1");

    surprises += EXPECT_EQ(base41Encode(input, 3), "SL1A0");

    surprises += EXPECT_EQ(base41Encode((uint8_t *) "Hello World!", sizeof("Hello World!") - 1), "GHF.KGL+48-G5LGK35");

    vector<uint8_t> decoded = vector<uint8_t>();
    surprises += EXPECT_EQ(base41Decode("GHF.KGL+48-G5LGK35", decoded), CHIP_NO_ERROR);

    string hello_world;
    for (size_t _ = 0; _ < decoded.size(); _++)
    {
        hello_world += (char) decoded[_];
    }
    surprises += EXPECT_EQ(hello_world, "Hello World!");

    // short input
    surprises += EXPECT_EQ(base41Decode("A0", decoded), CHIP_NO_ERROR);
    surprises += EXPECT_EQ(decoded.size(), 2);

    // empty == empty
    surprises += EXPECT_EQ(base41Decode("", decoded), CHIP_NO_ERROR);
    surprises += EXPECT_EQ(decoded.size(), 0);
    // too short
    surprises += EXPECT_EQ(base41Decode("A", decoded), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // outside valid chars
    surprises += EXPECT_EQ(base41Decode("0\001", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("\0010", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("[0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("0[", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);

    // BOGUS chars
    surprises += EXPECT_EQ(base41Decode("!0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("\"0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("#0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("&0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("'0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("(0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode(")0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode(",0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode(";0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("<0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("=0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode(">0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    surprises += EXPECT_EQ(base41Decode("@0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);

    return surprises;
}

int testBitsetLen()
{
    return EXPECT_EQ(kTotalPayloadDataSizeInBits % 8, 0);
}

int testSetupPayloadVerify()
{
    int surprises = 0;
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;
    surprises += EXPECT_EQ(payload.isValidQRCodePayload(), true);

    // test invalid version
    SetupPayload test_payload = payload;
    test_payload.version      = 2 << kVersionFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    // test invalid rendezvousInformation
    test_payload                       = payload;
    test_payload.rendezvousInformation = 512;
    surprises += EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    // test invalid discriminator
    test_payload               = payload;
    test_payload.discriminator = 2 << kPayloadDiscriminatorFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    // test invalid stetup PIN
    test_payload              = payload;
    test_payload.setUpPINCode = 2 << kSetupPINCodeFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    return surprises;
}

int testInvalidQRCodePayload_WrongCharacterSet()
{
    int surprises        = 0;
    string invalidString = "adas12AA";

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    surprises += EXPECT_EQ(didFail, true);
    surprises += EXPECT_EQ(payload.isValidQRCodePayload(), false);
    return surprises;
}

int testInvalidQRCodePayload_WrongLength()
{
    int surprises                   = 0;
    string invalidString            = "AA12";
    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    surprises += EXPECT_EQ(didFail, true);
    surprises += EXPECT_EQ(payload.isValidQRCodePayload(), false);
    return surprises;
}

int testPayloadEquality()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    SetupPayload equalPayload;
    equalPayload.version               = 5;
    equalPayload.vendorID              = 12;
    equalPayload.productID             = 1;
    equalPayload.requiresCustomFlow    = 0;
    equalPayload.rendezvousInformation = 1;
    equalPayload.discriminator         = 128;
    equalPayload.setUpPINCode          = 2048;

    bool result = payload == equalPayload;
    return EXPECT_EQ(result, true);
}

int testPayloadInEquality()
{
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    SetupPayload unequalPayload;
    unequalPayload.version               = 5;
    unequalPayload.vendorID              = 12;
    unequalPayload.productID             = 1;
    unequalPayload.requiresCustomFlow    = 0;
    unequalPayload.rendezvousInformation = 1;
    unequalPayload.discriminator         = 28;
    unequalPayload.setUpPINCode          = 121233;

    bool result = payload == unequalPayload;
    return EXPECT_EQ(result, false);
}

int testQRCodeToPayloadGeneration()
{
    int surprises = 0;
    SetupPayload payload;
    payload.version               = 3;
    payload.vendorID              = 100;
    payload.productID             = 12;
    payload.requiresCustomFlow    = 1;
    payload.rendezvousInformation = 4;
    payload.discriminator         = 233;
    payload.setUpPINCode          = 5221133;

    QRCodeSetupPayloadGenerator generator(payload);
    string base41Rep = generator.payloadBase41Representation();

    SetupPayload resultingPayload;
    QRCodeSetupPayloadParser parser(base41Rep);

    CHIP_ERROR err  = parser.populatePayload(resultingPayload);
    bool didSucceed = err == CHIP_NO_ERROR;
    surprises += EXPECT_EQ(didSucceed, true);
    surprises += EXPECT_EQ(resultingPayload.isValidQRCodePayload(), true);

    bool result = payload == resultingPayload;
    surprises += EXPECT_EQ(result, true);
    return surprises;
}

int testExtractPayload()
{
    int surprises = 0;
    surprises += EXPECT_EQ(extractPayload(string("CH:ABC")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("CH:")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("H:")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("ASCH:")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("Z%CH:ABC%")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("Z%CH:ABC")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("%Z%CH:ABC")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("%Z%CH:ABC%")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("%Z%CH:ABC%DDD")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("CH:ABC%DDD")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("CH:ABC%")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("%CH:")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("%CH:%")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("A%")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("CH:%")), string(""));
    surprises += EXPECT_EQ(extractPayload(string("%CH:ABC")), string("ABC"));
    surprises += EXPECT_EQ(extractPayload(string("ABC")), string(""));

    return surprises;
}

int main(int argc, char ** argv)
{
    int result = testBitsetLen() + testPayloadByteArrayRep() + testPayloadBase41Rep() + testBase41() + testSetupPayloadVerify() +
        testPayloadEquality() + testPayloadInEquality() + testQRCodeToPayloadGeneration() +
        testInvalidQRCodePayload_WrongCharacterSet() + testInvalidQRCodePayload_WrongLength() + testExtractPayload();
    if (result == 0)
    {
        printf("\n** All QRCode tests pass **\n");
    }
    else
    {
        printf("\n**== QRCode tests FAILED ==**\n");
    }
    return result;
}
