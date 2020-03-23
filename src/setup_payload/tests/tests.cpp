/*
 *
 *    <COPYRIGHT>
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
#include "SetupCodeUtils.cpp"
#include "SetupPayload.h"
#include "QRCodeSetupPayloadGenerator.cpp"

using namespace chip;
using namespace std;

#define EXPECT_EQ(x, y) ((x) != (y)) ? cerr << __FILE__ << ":" << __LINE__ << ":error EXPECT_EQ(" << x << ", " << y << ")\n", 1 : 0

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

int testPayloadBase45Rep()
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
    string result   = generator.payloadBase45Representation();
    string expected = "B20800G00G8G000";

    return EXPECT_EQ(result, expected);
}

int testBase45()
{
    int surprises = 0;

    uint8_t input[] = { 10, 10, 10 };

    surprises += EXPECT_EQ(base45Encode(input, 0), "");

    surprises += EXPECT_EQ(base45Encode(input, 1), "A0");

    surprises += EXPECT_EQ(base45Encode(input, 2), "5C1");

    surprises += EXPECT_EQ(base45Encode(input, 3), "5C1A0");

    surprises += EXPECT_EQ(base45Encode((uint8_t *) "Hello World!", sizeof("Hello World!") - 1), "8 C VDN44I3E.VD/94");

    vector<uint8_t> decoded = base45Decode("8 C VDN44I3E.VD/94");
    string hello_world;
    for (int _ = 0; _ < decoded.size(); _++)
    {
        hello_world += (char) decoded[_];
    }
    surprises += EXPECT_EQ(hello_world, "Hello World!");

    // empty == empty
    surprises += EXPECT_EQ(base45Decode("").size(), 0);
    // too short
    surprises += EXPECT_EQ(base45Decode("A").size(), 0);
    // outside valid chars
    surprises += EXPECT_EQ(base45Decode("0\x1").size(), 0);
    surprises += EXPECT_EQ(base45Decode("\x10").size(), 0);
    surprises += EXPECT_EQ(base45Decode("[0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("0[").size(), 0);

    // BOGUS chars
    surprises += EXPECT_EQ(base45Decode("!0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("\"0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("#0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("&0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("'0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("(0").size(), 0);
    surprises += EXPECT_EQ(base45Decode(")0").size(), 0);
    surprises += EXPECT_EQ(base45Decode(",0").size(), 0);
    surprises += EXPECT_EQ(base45Decode(";0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("<0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("=0").size(), 0);
    surprises += EXPECT_EQ(base45Decode(">0").size(), 0);
    surprises += EXPECT_EQ(base45Decode("@0").size(), 0);

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
    surprises += EXPECT_EQ(payload.isValid(), true);

    // test invalid version
    SetupPayload test_payload = payload;
    test_payload.version      = 2 << kVersionFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValid(), false);

    // test invalid rendezvousInformation
    test_payload                       = payload;
    test_payload.rendezvousInformation = 3;
    surprises += EXPECT_EQ(test_payload.isValid(), false);

    // test invalid discriminator
    test_payload               = payload;
    test_payload.discriminator = 2 << kPayloadDiscriminatorFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValid(), false);

    // test invalid stetup PIN
    test_payload              = payload;
    test_payload.setUpPINCode = 2 << kSetupPINCodeFieldLengthInBits;
    surprises += EXPECT_EQ(test_payload.isValid(), false);

    return surprises;
}

int main(int argc, char ** argv)
{
    printf("---Running Test--- tests from %s\n", __FILE__);
    return testBitsetLen() + testPayloadByteArrayRep() + testPayloadBase45Rep() + testBase45() + testSetupPayloadVerify();
}
