/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file implements a process to effect a functional test for
 *      the CHIP Abstract Syntax Notifcation One (ASN1) encode and
 *      decode interfaces.
 *
 */
#include <nlunit-test.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/support/UnitTestRegistration.h>

using namespace chip;
using namespace chip::ASN1;

enum
{
    kTestVal_01_Bool      = false,
    kTestVal_02_Bool      = true,
    kTestVal_03_BitString = 0x0,
    kTestVal_04_BitString = 0x1,
    kTestVal_05_BitString = 0x3,
    kTestVal_06_BitString = 0x17,
    kTestVal_07_BitString = 0x37,
    kTestVal_08_BitString = 0x187,
    kTestVal_09_BitString = 0x3E7,
    kTestVal_10_Int       = 0,
    kTestVal_11_Int       = 1,
    kTestVal_12_Int       = -1,
    kTestVal_13_Int       = 0xFF00FF,
    kTestVal_14_Int       = -0xFF00FF,
    kTestVal_15_Int       = INT32_MAX,
    kTestVal_16_Int       = INT32_MIN,
    kTestVal_17_Int       = INT64_MAX,
    kTestVal_18_Int       = INT64_MIN,
    kTestVal_19_OID       = kOID_AttributeType_OrganizationName,
    kTestVal_23_OID       = kOID_AttributeType_CommonName,
    kTestVal_24_Int       = 42,
};

// clang-format off
static uint8_t kTestVal_20_OctetString[]        = { 0x01, 0x03, 0x05, 0x07, 0x10, 0x30, 0x50, 0x70, 0x00 };
static const char * kTestVal_21_PrintableString = "Sudden death in Venice";
static const char * kTestVal_22_UTFString       = "Ond bra\xCC\x8A""d do\xCC\x88""d i Venedig";
// clang-format on

// Manually copied from ASN1OID.h for testing.
static const uint8_t sOID_AttributeType_ChipNodeId[]         = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x01, 0x01 };
static const uint8_t sOID_SigAlgo_ECDSAWithSHA256[]          = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 };
static const uint8_t sOID_EllipticCurve_prime256v1[]         = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07 };
static const uint8_t sOID_Extension_AuthorityKeyIdentifier[] = { 0x55, 0x1D, 0x23 };
static const uint8_t sOID_Extension_BasicConstraints[]       = { 0x55, 0x1D, 0x13 };
static const uint8_t sOID_KeyPurpose_ServerAuth[]            = { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01 };

uint8_t TestASN1_EncodedData[] = {
    0x30, 0x81, 0xBA, 0x01, 0x01, 0x00, 0x01, 0x01, 0xFF, 0x31, 0x00, 0x03, 0x01, 0x00, 0x03, 0x02, 0x07, 0x80, 0x03, 0x02, 0x06,
    0xC0, 0x30, 0x16, 0x30, 0x0F, 0x30, 0x08, 0x03, 0x02, 0x03, 0xE8, 0x03, 0x02, 0x02, 0xEC, 0x03, 0x03, 0x07, 0xE1, 0x80, 0x03,
    0x03, 0x06, 0xE7, 0xC0, 0x02, 0x01, 0x00, 0x02, 0x01, 0x01, 0x02, 0x01, 0xFF, 0x02, 0x04, 0x00, 0xFF, 0x00, 0xFF, 0x02, 0x04,
    0xFF, 0x00, 0xFF, 0x01, 0x02, 0x04, 0x7F, 0xFF, 0xFF, 0xFF, 0x02, 0x04, 0x80, 0x00, 0x00, 0x00, 0x02, 0x08, 0x7F, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x04,
    0x09, 0x01, 0x03, 0x05, 0x07, 0x10, 0x30, 0x50, 0x70, 0x00, 0x04, 0x01, 0x01, 0x04, 0x00, 0x1B, 0x00, 0x13, 0x16, 0x53, 0x75,
    0x64, 0x64, 0x65, 0x6E, 0x20, 0x64, 0x65, 0x61, 0x74, 0x68, 0x20, 0x69, 0x6E, 0x20, 0x56, 0x65, 0x6E, 0x69, 0x63, 0x65, 0x0C,
    0x1A, 0x4F, 0x6E, 0x64, 0x20, 0x62, 0x72, 0x61, 0xCC, 0x8A, 0x64, 0x20, 0x64, 0x6F, 0xCC, 0x88, 0x64, 0x20, 0x69, 0x20, 0x56,
    0x65, 0x6E, 0x65, 0x64, 0x69, 0x67, 0x04, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x03, 0x04, 0x00, 0x02, 0x01, 0x2A
};

static CHIP_ERROR EncodeASN1TestData(ASN1Writer & writer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_BOOLEAN(kTestVal_01_Bool);
        ASN1_ENCODE_BOOLEAN(kTestVal_02_Bool);
        ASN1_START_SET {}
        ASN1_END_SET;
        ASN1_ENCODE_BIT_STRING(kTestVal_03_BitString);
        ASN1_ENCODE_BIT_STRING(kTestVal_04_BitString);
        ASN1_ENCODE_BIT_STRING(kTestVal_05_BitString);
        ASN1_START_SEQUENCE
        {
            ASN1_START_SEQUENCE
            {
                ASN1_START_SEQUENCE
                {
                    ASN1_ENCODE_BIT_STRING(kTestVal_06_BitString);
                    ASN1_ENCODE_BIT_STRING(kTestVal_07_BitString);
                }
                ASN1_END_SEQUENCE;
                ASN1_ENCODE_BIT_STRING(kTestVal_08_BitString);
            }
            ASN1_END_SEQUENCE;
            ASN1_ENCODE_BIT_STRING(kTestVal_09_BitString);
        }
        ASN1_END_SEQUENCE;
        ASN1_ENCODE_INTEGER(kTestVal_10_Int);
        ASN1_ENCODE_INTEGER(kTestVal_11_Int);
        ASN1_ENCODE_INTEGER(kTestVal_12_Int);
        ASN1_ENCODE_INTEGER(kTestVal_13_Int);
        ASN1_ENCODE_INTEGER(kTestVal_14_Int);
        ASN1_ENCODE_INTEGER(kTestVal_15_Int);
        ASN1_ENCODE_INTEGER(kTestVal_16_Int);
        ASN1_ENCODE_INTEGER(kTestVal_17_Int);
        ASN1_ENCODE_INTEGER(kTestVal_18_Int);
        ASN1_ENCODE_OBJECT_ID(kTestVal_19_OID);
        ASN1_ENCODE_OCTET_STRING(kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString));
        ASN1_ENCODE_OCTET_STRING(kTestVal_20_OctetString, 1);
        ASN1_ENCODE_OCTET_STRING(kTestVal_20_OctetString, 0);
        ASN1_ENCODE_STRING(kASN1UniversalTag_GeneralString, "", 0);
        ASN1_ENCODE_STRING(kASN1UniversalTag_PrintableString, kTestVal_21_PrintableString, strlen(kTestVal_21_PrintableString));
        ASN1_ENCODE_STRING(kASN1UniversalTag_UTF8String, kTestVal_22_UTFString, strlen(kTestVal_22_UTFString));
        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                ASN1_ENCODE_OBJECT_ID(kTestVal_23_OID);
                ASN1_START_BIT_STRING_ENCAPSULATED { ASN1_ENCODE_INTEGER(kTestVal_24_Int); }
                ASN1_END_ENCAPSULATED;
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

static void TestASN1_Encode(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    uint8_t buf[2048];
    ASN1Writer writer;
    uint16_t encodedLen;

    writer.Init(buf, sizeof(buf));

    err = EncodeASN1TestData(writer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    encodedLen = writer.GetLengthWritten();
    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(TestASN1_EncodedData));
    NL_TEST_ASSERT(inSuite, memcmp(buf, TestASN1_EncodedData, sizeof(TestASN1_EncodedData)) == 0);

#define DUMP_HEX 0
#if DUMP_HEX
    for (uint16_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif
}

static void TestASN1_Decode(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Reader reader;
    bool boolVal;
    uint32_t bitStringVal;
    int64_t intVal;
    OID oidVal;

    reader.Init(TestASN1_EncodedData, sizeof(TestASN1_EncodedData));

    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_BOOLEAN(boolVal);
        NL_TEST_ASSERT(inSuite, boolVal == kTestVal_01_Bool);
        ASN1_PARSE_BOOLEAN(boolVal);
        NL_TEST_ASSERT(inSuite, boolVal == kTestVal_02_Bool);
        ASN1_PARSE_ENTER_SET {}
        ASN1_EXIT_SET;
        ASN1_PARSE_BIT_STRING(bitStringVal);
        NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_03_BitString);
        ASN1_PARSE_BIT_STRING(bitStringVal);
        NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_04_BitString);
        ASN1_PARSE_BIT_STRING(bitStringVal);
        NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_05_BitString);
        ASN1_PARSE_ENTER_SEQUENCE
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    ASN1_PARSE_BIT_STRING(bitStringVal);
                    NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_06_BitString);
                    ASN1_PARSE_BIT_STRING(bitStringVal);
                    NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_07_BitString);
                }
                ASN1_EXIT_SEQUENCE;
                ASN1_PARSE_BIT_STRING(bitStringVal);
                NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_08_BitString);
            }
            ASN1_EXIT_SEQUENCE;
            ASN1_PARSE_BIT_STRING(bitStringVal);
            NL_TEST_ASSERT(inSuite, bitStringVal == kTestVal_09_BitString);
        }
        ASN1_EXIT_SEQUENCE;
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_10_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_11_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_12_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_13_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_14_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_15_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_16_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_17_Int);
        ASN1_PARSE_INTEGER(intVal);
        NL_TEST_ASSERT(inSuite, intVal == kTestVal_18_Int);
        ASN1_PARSE_OBJECT_ID(oidVal);
        NL_TEST_ASSERT(inSuite, oidVal == kTestVal_19_OID);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(kTestVal_20_OctetString));
        NL_TEST_ASSERT(inSuite, memcmp(reader.GetValue(), kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString)) == 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == 1);
        NL_TEST_ASSERT(inSuite, reader.GetValue()[0] == kTestVal_20_OctetString[0]);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_GeneralString);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_PrintableString);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == strlen(kTestVal_21_PrintableString));
        NL_TEST_ASSERT(inSuite, memcmp(reader.GetValue(), kTestVal_21_PrintableString, strlen(kTestVal_21_PrintableString)) == 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_UTF8String);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == strlen(kTestVal_22_UTFString));
        NL_TEST_ASSERT(inSuite, memcmp(reader.GetValue(), kTestVal_22_UTFString, strlen(kTestVal_22_UTFString)) == 0);
        ASN1_PARSE_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_OctetString)
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_OBJECT_ID(oidVal);
                NL_TEST_ASSERT(inSuite, oidVal == kTestVal_23_OID);
                ASN1_PARSE_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_BitString)
                {
                    ASN1_PARSE_INTEGER(intVal);
                    NL_TEST_ASSERT(inSuite, intVal == kTestVal_24_Int);
                }
                ASN1_EXIT_ENCAPSULATED;
            }
            ASN1_EXIT_SEQUENCE;
        }
        ASN1_EXIT_ENCAPSULATED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestASN1_NullWriter(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ASN1Writer writer;
    uint16_t encodedLen;

    writer.InitNullWriter();

    err = EncodeASN1TestData(writer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    encodedLen = writer.GetLengthWritten();
    NL_TEST_ASSERT(inSuite, encodedLen == 0);
}

static void TestASN1_ObjectID(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    uint8_t buf[2048];
    ASN1Writer writer;
    ASN1Reader reader;
    uint16_t encodedLen;

    writer.Init(buf, sizeof(buf));

    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_OBJECT_ID(kOID_AttributeType_ChipNodeId);
        ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        ASN1_ENCODE_OBJECT_ID(kOID_EllipticCurve_prime256v1);
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_AuthorityKeyIdentifier);
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_BasicConstraints);
        ASN1_ENCODE_OBJECT_ID(kOID_KeyPurpose_ServerAuth);
    }
    ASN1_END_SEQUENCE;

    encodedLen = writer.GetLengthWritten();
    NL_TEST_ASSERT(inSuite, encodedLen > 0);

    reader.Init(buf, encodedLen);

    // Parse and check OIDs as actual ASN1 encoded values.
    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_AttributeType_ChipNodeId));
        NL_TEST_ASSERT(inSuite,
                       memcmp(reader.GetValue(), sOID_AttributeType_ChipNodeId, sizeof(sOID_AttributeType_ChipNodeId)) == 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_SigAlgo_ECDSAWithSHA256));
        NL_TEST_ASSERT(inSuite, memcmp(reader.GetValue(), sOID_SigAlgo_ECDSAWithSHA256, sizeof(sOID_SigAlgo_ECDSAWithSHA256)) == 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_EllipticCurve_prime256v1));
        NL_TEST_ASSERT(inSuite,
                       memcmp(reader.GetValue(), sOID_EllipticCurve_prime256v1, sizeof(sOID_EllipticCurve_prime256v1)) == 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_Extension_AuthorityKeyIdentifier));
        NL_TEST_ASSERT(
            inSuite,
            memcmp(reader.GetValue(), sOID_Extension_AuthorityKeyIdentifier, sizeof(sOID_Extension_AuthorityKeyIdentifier)) == 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_Extension_BasicConstraints));
        NL_TEST_ASSERT(inSuite,
                       memcmp(reader.GetValue(), sOID_Extension_BasicConstraints, sizeof(sOID_Extension_BasicConstraints)) == 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        NL_TEST_ASSERT(inSuite, reader.GetValueLen() == sizeof(sOID_KeyPurpose_ServerAuth));
        NL_TEST_ASSERT(inSuite, memcmp(reader.GetValue(), sOID_KeyPurpose_ServerAuth, sizeof(sOID_KeyPurpose_ServerAuth)) == 0);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Test ASN1 encoding macros", TestASN1_Encode),
    NL_TEST_DEF("Test ASN1 decoding macros", TestASN1_Decode),
    NL_TEST_DEF("Test ASN1 NULL writer", TestASN1_NullWriter),
    NL_TEST_DEF("Test ASN1 Object IDs", TestASN1_ObjectID),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestASN1(void)
{
    nlTestSuite theSuite = { "Support-ASN1", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestASN1);
