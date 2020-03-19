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

void testPayloadByteArrayRep()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    QRCodeSetupPayloadGenerator generator(payload);
    string result         = generator.payloadBinaryRepresentation();
    string expectedResult = "101000000000000110000000000000000010000000011000000000000000000000010000"
                            "00000000";

    assert(result.compare(expectedResult) == 0);
}

void testPayloadBase45Rep()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    QRCodeSetupPayloadGenerator generator(payload);
    string result         = generator.payloadBase45Representation();
    string expectedResult = "KABG8842Q000211";

    assert(result.compare(expectedResult) == 0);
}

void testBase45Encoding()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    uint16_t input        = 10;
    string result         = base45EncodedString(input, 3);
    string expectedResult = "00A";
    assert(result.compare(expectedResult) == 0);
}

void testBitsetLen()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    assert(kTotalPayloadDataSizeInBits % 8 == 0);
}

void testSetupPayloadVerify()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;

    payload.version               = 5;
    payload.vendorID              = 12;
    payload.productID             = 1;
    payload.requiresCustomFlow    = 0;
    payload.rendezvousInformation = 1;
    payload.discriminator         = 128;
    payload.setUpPINCode          = 2048;

    assert(payload.isValid());

    // test invalid version
    SetupPayload test_payload = payload;
    test_payload.version      = 2 << kVersionFieldLengthInBits;
    assert(!test_payload.isValid());

    // test invalid rendezvousInformation
    test_payload                       = payload;
    test_payload.rendezvousInformation = 3;
    assert(!test_payload.isValid());

    // test invalid discriminator
    test_payload               = payload;
    test_payload.discriminator = 2 << kPayloadDiscriminatorFieldLengthInBits;
    assert(!test_payload.isValid());

    // test invalid stetup PIN
    test_payload              = payload;
    test_payload.setUpPINCode = 2 << kSetupPINCodeFieldLengthInBits;
    assert(!test_payload.isValid());
}

int main(int argc, char ** argv)
{
    printf("---Running Test--- tests from %s\n", __FILE__);
    testBitsetLen();
    testPayloadByteArrayRep();
    testPayloadBase45Rep();
    testBase45Encoding();
    testSetupPayloadVerify();
}
