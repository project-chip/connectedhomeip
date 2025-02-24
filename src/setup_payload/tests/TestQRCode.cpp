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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

using namespace chip;

namespace {

TEST(TestQRCode, TestRendezvousFlags)
{
    SetupPayload inPayload = GetDefaultPayload();

    // Not having a value in rendezvousInformation is not allowed for a QR code.
    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kNone);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kSoftAP);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kBLE);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kOnNetwork);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlag::kWiFiPAF);
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(
        RendezvousInformationFlags(RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(
        RendezvousInformationFlags(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(
        RendezvousInformationFlags(RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlags(
        RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlags(
        RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(RendezvousInformationFlags(
        RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.rendezvousInformation.SetValue(
        RendezvousInformationFlags(RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kBLE,
                                   RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork));
    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST(TestQRCode, TestCommissioningFlow)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.commissioningFlow = CommissioningFlow::kStandard;
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.commissioningFlow = CommissioningFlow::kUserActionRequired;
    EXPECT_TRUE(CheckWriteRead(inPayload));

    inPayload.commissioningFlow = CommissioningFlow::kCustom;
    EXPECT_TRUE(CheckWriteRead(inPayload));
}

TEST(TestQRCode, TestMaximumValues)
{
    SetupPayload inPayload = GetDefaultPayload();

    inPayload.version           = static_cast<uint8_t>((1 << kVersionFieldLengthInBits) - 1);
    inPayload.vendorID          = 0xFFFF;
    inPayload.productID         = 0xFFFF;
    inPayload.commissioningFlow = CommissioningFlow::kCustom;
    inPayload.rendezvousInformation.SetValue(
        RendezvousInformationFlags(RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kBLE,
                                   RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kOnNetwork));
    inPayload.discriminator.SetLongValue(static_cast<uint16_t>((1 << kPayloadDiscriminatorFieldLengthInBits) - 1));
    inPayload.setUpPINCode = static_cast<uint32_t>((1 << kSetupPINCodeFieldLengthInBits) - 1);

    EXPECT_TRUE(CheckWriteRead(inPayload, /* allowInvalidPayload */ true));
}

TEST(TestQRCode, TestPayloadByteArrayRep)
{
    SetupPayload payload = GetDefaultPayload();

    std::string expected = " 0000 000000000000000100000000000 000010000000 00000001 00 0000000000000001 0000000000001100 000";
    EXPECT_TRUE(CompareBinary(payload, expected));
}

TEST(TestQRCode, TestPayloadBase38Rep)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    std::string result;
    CHIP_ERROR err  = generator.payloadBase38Representation(result);
    bool didSucceed = err == CHIP_NO_ERROR;
    EXPECT_EQ(didSucceed, true);

    EXPECT_EQ(result, kDefaultPayloadQRCode);
}

TEST(TestQRCode, TestBase38)
{
    uint8_t input[3] = { 10, 10, 10 };
    char encodedBuf[64];
    MutableByteSpan inputSpan(input);
    MutableCharSpan encodedSpan(encodedBuf);

    // basic stuff
    base38Encode(inputSpan.SubSpan(0, 0), encodedSpan);
    EXPECT_EQ(strlen(encodedBuf), 0u);
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 1), encodedSpan);
    EXPECT_STREQ(encodedBuf, "A0");
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 2), encodedSpan);
    EXPECT_STREQ(encodedBuf, "OT10");
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "-N.B0");

    // test null termination of output buffer
    encodedSpan             = MutableCharSpan(encodedBuf);
    MutableCharSpan subSpan = encodedSpan.SubSpan(0, 2);
    EXPECT_EQ(base38Encode(inputSpan.SubSpan(0, 1), subSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
    // Force no nulls in output buffer
    memset(encodedSpan.data(), '?', encodedSpan.size());
    subSpan = encodedSpan.SubSpan(0, 3);
    base38Encode(inputSpan.SubSpan(0, 1), subSpan);
    size_t encodedLen = strnlen(encodedSpan.data(), MATTER_ARRAY_SIZE(encodedBuf));
    EXPECT_EQ(encodedLen, strlen("A0"));
    EXPECT_STREQ(encodedBuf, "A0");

    // passing empty parameters
    MutableCharSpan emptySpan;
    encodedSpan = MutableCharSpan(encodedBuf);
    EXPECT_EQ(base38Encode(inputSpan, emptySpan), CHIP_ERROR_BUFFER_TOO_SMALL);
    base38Encode(MutableByteSpan(), encodedSpan);
    EXPECT_STREQ(encodedBuf, "");
    EXPECT_EQ(base38Encode(MutableByteSpan(), emptySpan), CHIP_ERROR_BUFFER_TOO_SMALL);

    // test single odd byte corner conditions
    encodedSpan = MutableCharSpan(encodedBuf);
    input[2]    = 0;
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "OT100");
    input[2]    = 40;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "Y6V91");
    input[2]    = 41;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "KL0B1");
    input[2]    = 255;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "Q-M08");

    // verify chunks of 1,2 and 3 bytes result in fixed-length strings padded with '0'
    // for 1 byte we need always 2 characters
    input[0]    = 35;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 1), encodedSpan);
    EXPECT_STREQ(encodedBuf, "Z0");
    // for 2 bytes we need always 4 characters
    input[0]    = 255;
    input[1]    = 0;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 2), encodedSpan);
    EXPECT_STREQ(encodedBuf, "R600");
    // for 3 bytes we need always 5 characters
    input[0]    = 46;
    input[1]    = 0;
    input[2]    = 0;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "81000");

    // verify maximum available values for each chunk size to check selecting proper characters number
    // for 1 byte we need 2 characters
    input[0]    = 255;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 1), encodedSpan);
    EXPECT_STREQ(encodedBuf, "R6");
    // for 2 bytes we need 4 characters
    input[0]    = 255;
    input[1]    = 255;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan.SubSpan(0, 2), encodedSpan);
    EXPECT_STREQ(encodedBuf, "NE71");
    // for 3 bytes we need 5 characters
    input[0]    = 255;
    input[1]    = 255;
    input[2]    = 255;
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(inputSpan, encodedSpan);
    EXPECT_STREQ(encodedBuf, "PLS18");

    // fun with strings
    encodedSpan = MutableCharSpan(encodedBuf);
    base38Encode(ByteSpan((uint8_t *) "Hello World!", sizeof("Hello World!") - 1), encodedSpan);
    EXPECT_STREQ(encodedBuf, "KKHF3W2S013OPM3EJX11");

    std::vector<uint8_t> decoded = std::vector<uint8_t>();
    EXPECT_EQ(base38Decode("KKHF3W2S013OPM3EJX11", decoded), CHIP_NO_ERROR);

    std::string hello_world;
    for (uint8_t b : decoded)
    {
        hello_world += static_cast<char>(b);
    }
    EXPECT_EQ(hello_world, "Hello World!");

    // short input
    EXPECT_EQ(base38Decode("A0", decoded), CHIP_NO_ERROR);
    EXPECT_TRUE(decoded.size());
    EXPECT_EQ(decoded[0], 10u);

    // empty == empty
    EXPECT_EQ(base38Decode("", decoded), CHIP_NO_ERROR);
    EXPECT_TRUE(decoded.empty());

    // test invalid characters
    EXPECT_EQ(base38Decode("0\001", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("\0010", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("[0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("0[", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode(" 0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("!0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("\"0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("#0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("$0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("%0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("&0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("'0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("(0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode(")0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("*0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("+0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode(",0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode(";0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("<0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("=0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode(">0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(base38Decode("@0", decoded), CHIP_ERROR_INVALID_INTEGER_VALUE);

    // test strings that encode maximum values
    EXPECT_EQ(base38Decode("R6", decoded), CHIP_NO_ERROR); // this is 0xFF
    EXPECT_EQ(decoded, std::vector<uint8_t>({ 255 }));
    EXPECT_EQ(base38Decode("S6", decoded), CHIP_ERROR_INVALID_ARGUMENT); // trying to encode 0xFF + 1 in 2 chars
    EXPECT_EQ(base38Decode("S600", decoded), CHIP_NO_ERROR);             // this is 0xFF + 1, needs 4 chars
    EXPECT_EQ(decoded, std::vector<uint8_t>({ 0, 1 }));
    EXPECT_EQ(base38Decode("NE71", decoded), CHIP_NO_ERROR); // this is 0xFFFF
    EXPECT_EQ(decoded, std::vector<uint8_t>({ 255, 255 }));
    EXPECT_EQ(base38Decode("OE71", decoded), CHIP_ERROR_INVALID_ARGUMENT); // trying to encode 0xFFFF + 1 in 4 chars
    EXPECT_EQ(base38Decode("OE710", decoded), CHIP_NO_ERROR);              // this is 0xFFFF + 1, needs 5 chars
    EXPECT_EQ(decoded, std::vector<uint8_t>({ 0, 0, 1 }));
    EXPECT_EQ(base38Decode("PLS18", decoded), CHIP_NO_ERROR); // this is 0xFFFFFF
    EXPECT_EQ(decoded, std::vector<uint8_t>({ 255, 255, 255 }));
    EXPECT_EQ(base38Decode("QLS18", decoded), CHIP_ERROR_INVALID_ARGUMENT); // trying to encode 0xFFFFFF + 1
}

TEST(TestQRCode, TestBitsetLen)
{
    EXPECT_FALSE(kTotalPayloadDataSizeInBits % 8);
}

TEST(TestQRCode, TestSetupPayloadVerify)
{
    SetupPayload payload = GetDefaultPayload();
    EXPECT_EQ(payload.isValidQRCodePayload(), true);

    // test invalid commissioning flow
    SetupPayload test_payload      = payload;
    test_payload.commissioningFlow = CommissioningFlow::kCustom;
    EXPECT_TRUE(test_payload.isValidQRCodePayload());

    test_payload.commissioningFlow = static_cast<CommissioningFlow>(1 << kCommissioningFlowFieldLengthInBits);
    EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    // test invalid version
    test_payload         = payload;
    test_payload.version = 1 << kVersionFieldLengthInBits;
    EXPECT_EQ(test_payload.isValidQRCodePayload(), false);

    // test invalid rendezvousInformation
    test_payload = payload;
    RendezvousInformationFlags invalid =
        RendezvousInformationFlags(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kSoftAP,
                                   RendezvousInformationFlag::kOnNetwork, RendezvousInformationFlag::kWiFiPAF);
    invalid.SetRaw(static_cast<uint8_t>(invalid.Raw() + 1));
    test_payload.rendezvousInformation.SetValue(invalid);
    EXPECT_EQ(test_payload.isValidQRCodePayload(), false);
    // When validating in Consume mode, unknown rendezvous flags are OK.
    EXPECT_TRUE(test_payload.isValidQRCodePayload(PayloadContents::ValidationMode::kConsume));
    test_payload.rendezvousInformation.SetValue(RendezvousInformationFlags(0xff));
    EXPECT_TRUE(test_payload.isValidQRCodePayload(PayloadContents::ValidationMode::kConsume));
    // Rendezvous information is still required even in Consume mode.
    test_payload.rendezvousInformation.ClearValue();
    EXPECT_FALSE(test_payload.isValidQRCodePayload(PayloadContents::ValidationMode::kConsume));

    // test invalid setup PIN
    test_payload              = payload;
    test_payload.setUpPINCode = 1 << kSetupPINCodeFieldLengthInBits;
    EXPECT_EQ(test_payload.isValidQRCodePayload(), false);
}

TEST(TestQRCode, TestInvalidQRCodePayload_WrongCharacterSet)
{
    std::string invalidString = kDefaultPayloadQRCode;
    invalidString.back()      = ' '; // space is not contained in the base38 alphabet

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    EXPECT_EQ(didFail, true);
    EXPECT_EQ(payload.isValidQRCodePayload(), false);
}

TEST(TestQRCode, TestInvalidQRCodePayload_WrongLength)
{
    std::string invalidString = kDefaultPayloadQRCode;
    invalidString.pop_back();

    QRCodeSetupPayloadParser parser = QRCodeSetupPayloadParser(invalidString);
    SetupPayload payload;
    CHIP_ERROR err = parser.populatePayload(payload);
    bool didFail   = err != CHIP_NO_ERROR;
    EXPECT_EQ(didFail, true);
    EXPECT_EQ(payload.isValidQRCodePayload(), false);
}

TEST(TestQRCode, TestPayloadEquality)
{
    SetupPayload payload      = GetDefaultPayload();
    SetupPayload equalPayload = GetDefaultPayload();

    EXPECT_TRUE(payload == equalPayload);
}

TEST(TestQRCode, TestPayloadInEquality)
{
    SetupPayload payload = GetDefaultPayload();

    SetupPayload unequalPayload = GetDefaultPayload();
    unequalPayload.discriminator.SetLongValue(28);
    unequalPayload.setUpPINCode = 121233;

    EXPECT_FALSE(payload == unequalPayload);
}

TEST(TestQRCode, TestQRCodeToPayloadGeneration)
{
    SetupPayload payload = GetDefaultPayload();

    QRCodeSetupPayloadGenerator generator(payload);
    std::string base38Rep;
    CHIP_ERROR err  = generator.payloadBase38Representation(base38Rep);
    bool didSucceed = err == CHIP_NO_ERROR;
    EXPECT_EQ(didSucceed, true);

    SetupPayload resultingPayload;
    QRCodeSetupPayloadParser parser(base38Rep);

    err        = parser.populatePayload(resultingPayload);
    didSucceed = err == CHIP_NO_ERROR;
    EXPECT_EQ(didSucceed, true);
    EXPECT_EQ(resultingPayload.isValidQRCodePayload(), true);

    bool result = payload == resultingPayload;
    EXPECT_EQ(result, true);
}

TEST(TestQRCode, TestGenerateWithShortDiscriminatorInvalid)
{
    SetupPayload payload = GetDefaultPayload();
    EXPECT_TRUE(payload.isValidQRCodePayload());

    // A short discriminator isn't valid for a QR Code
    payload.discriminator.SetShortValue(1);
    EXPECT_FALSE(payload.isValidQRCodePayload());

    // QRCodeSetupPayloadGenerator should therefore return an error
    std::string base38Rep;
    QRCodeSetupPayloadGenerator generator(payload);
    EXPECT_EQ(generator.payloadBase38Representation(base38Rep), CHIP_ERROR_INVALID_ARGUMENT);

    // If we allow invalid payloads we should be able to encode
    generator.SetAllowInvalidPayload(true);
    EXPECT_EQ(generator.payloadBase38Representation(base38Rep), CHIP_NO_ERROR);
}

TEST(TestQRCode, TestGenerateWithoutRendezvousInformation)
{
    SetupPayload payload = GetDefaultPayload();
    EXPECT_TRUE(payload.isValidQRCodePayload());

    // Rendezvouz Information is required for a QR code
    payload.rendezvousInformation.ClearValue();
    EXPECT_FALSE(payload.isValidQRCodePayload());

    // QRCodeSetupPayloadGenerator should therefore return an error
    std::string base38Rep;
    QRCodeSetupPayloadGenerator generator(payload);
    EXPECT_EQ(generator.payloadBase38Representation(base38Rep), CHIP_ERROR_INVALID_ARGUMENT);

    // If we allow invalid payloads we should be able to encode
    generator.SetAllowInvalidPayload(true);
    EXPECT_EQ(generator.payloadBase38Representation(base38Rep), CHIP_NO_ERROR);
}

TEST(TestQRCode, TestExtractPayload)
{
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("MT:ABC")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("MT:")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("H:")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("ASMT:")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("Z%MT:ABC%")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("Z%MT:ABC")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%Z%MT:ABC")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%Z%MT:ABC%")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%Z%MT:ABC%DDD")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("MT:ABC%DDD")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("MT:ABC%")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%MT:")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%MT:%")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("A%")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("MT:%")), std::string(""));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("%MT:ABC")), std::string("ABC"));
    EXPECT_EQ(QRCodeSetupPayloadParser::ExtractPayload(std::string("ABC")), std::string(""));
}

} // namespace
