/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>

using namespace chip;
using namespace chip::ASN1;
using namespace chip::TLV;

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
static uint8_t kTestVal_09_BitString_AsOctetString[] = { 0xE7, 0xC0 };
static uint8_t kTestVal_20_OctetString[]        = { 0x01, 0x03, 0x05, 0x07, 0x10, 0x30, 0x50, 0x70, 0x00 };
static const char kTestVal_21_PrintableString[] = "Sudden death in Venice";
static const char kTestVal_22_UTFString[]       = "Ond bra\xCC\x8A""d do\xCC\x88""d i Venedig";
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
        ASN1_ENCODE_STRING(kASN1UniversalTag_PrintableString, kTestVal_21_PrintableString,
                           static_cast<uint16_t>(strlen(kTestVal_21_PrintableString)));
        ASN1_ENCODE_STRING(kASN1UniversalTag_UTF8String, kTestVal_22_UTFString,
                           static_cast<uint16_t>(strlen(kTestVal_22_UTFString)));
        ASN1_START_OCTET_STRING_ENCAPSULATED
        {
            ASN1_START_SEQUENCE
            {
                ASN1_ENCODE_OBJECT_ID(kTestVal_23_OID);
                ASN1_START_BIT_STRING_ENCAPSULATED
                {
                    ASN1_ENCODE_INTEGER(kTestVal_24_Int);
                }
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

TEST(TestASN1, Encode)
{
    CHIP_ERROR err;
    static uint8_t buf[2048];
    ASN1Writer writer;
    size_t encodedLen;

    writer.Init(buf);

    err = EncodeASN1TestData(writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    encodedLen = writer.GetLengthWritten();
    EXPECT_EQ(encodedLen, sizeof(TestASN1_EncodedData));
    EXPECT_EQ(memcmp(buf, TestASN1_EncodedData, sizeof(TestASN1_EncodedData)), 0);

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

TEST(TestASN1, Decode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Reader reader;
    bool boolVal;
    uint32_t bitStringVal;
    int64_t intVal;
    OID oidVal;

    reader.Init(TestASN1_EncodedData);

    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_BOOLEAN(boolVal);
        EXPECT_EQ(boolVal, kTestVal_01_Bool);
        ASN1_PARSE_BOOLEAN(boolVal);
        EXPECT_EQ(boolVal, kTestVal_02_Bool);
        ASN1_PARSE_ENTER_SET {}
        ASN1_EXIT_SET;
        ASN1_PARSE_BIT_STRING(bitStringVal);
        EXPECT_EQ(bitStringVal, kTestVal_03_BitString);
        ASN1_PARSE_BIT_STRING(bitStringVal);
        EXPECT_EQ(bitStringVal, kTestVal_04_BitString);
        ASN1_PARSE_BIT_STRING(bitStringVal);
        EXPECT_EQ(bitStringVal, kTestVal_05_BitString);
        ASN1_PARSE_ENTER_SEQUENCE
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_ENTER_SEQUENCE
                {
                    ASN1_PARSE_BIT_STRING(bitStringVal);
                    EXPECT_EQ(bitStringVal, kTestVal_06_BitString);
                    ASN1_PARSE_BIT_STRING(bitStringVal);
                    EXPECT_EQ(bitStringVal, kTestVal_07_BitString);
                }
                ASN1_EXIT_SEQUENCE;
                ASN1_PARSE_BIT_STRING(bitStringVal);
                EXPECT_EQ(bitStringVal, kTestVal_08_BitString);
            }
            ASN1_EXIT_SEQUENCE;
            ASN1_PARSE_BIT_STRING(bitStringVal);
            EXPECT_EQ(bitStringVal, kTestVal_09_BitString);
        }
        ASN1_EXIT_SEQUENCE;
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_10_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_11_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_12_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_13_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_14_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_15_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_16_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_17_Int);
        ASN1_PARSE_INTEGER(intVal);
        EXPECT_EQ(intVal, kTestVal_18_Int);
        ASN1_PARSE_OBJECT_ID(oidVal);
        EXPECT_EQ(oidVal, kTestVal_19_OID);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        EXPECT_EQ(reader.GetValueLen(), sizeof(kTestVal_20_OctetString));
        EXPECT_EQ(memcmp(reader.GetValue(), kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString)), 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        EXPECT_EQ(reader.GetValueLen(), 1u);
        EXPECT_EQ(reader.GetValue()[0], kTestVal_20_OctetString[0]);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        EXPECT_EQ(reader.GetValueLen(), 0u);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_GeneralString);
        EXPECT_EQ(reader.GetValueLen(), 0u);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_PrintableString);
        EXPECT_EQ(reader.GetValueLen(), strlen(kTestVal_21_PrintableString));
        EXPECT_EQ(memcmp(reader.GetValue(), kTestVal_21_PrintableString, strlen(kTestVal_21_PrintableString)), 0);
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_UTF8String);
        EXPECT_EQ(reader.GetValueLen(), strlen(kTestVal_22_UTFString));
        EXPECT_EQ(memcmp(reader.GetValue(), kTestVal_22_UTFString, strlen(kTestVal_22_UTFString)), 0);
        ASN1_PARSE_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_OctetString)
        {
            ASN1_PARSE_ENTER_SEQUENCE
            {
                ASN1_PARSE_OBJECT_ID(oidVal);
                EXPECT_EQ(oidVal, kTestVal_23_OID);
                ASN1_PARSE_ENTER_ENCAPSULATED(kASN1TagClass_Universal, kASN1UniversalTag_BitString)
                {
                    ASN1_PARSE_INTEGER(intVal);
                    EXPECT_EQ(intVal, kTestVal_24_Int);
                }
                ASN1_EXIT_ENCAPSULATED;
            }
            ASN1_EXIT_SEQUENCE;
        }
        ASN1_EXIT_ENCAPSULATED;
    }
    ASN1_EXIT_SEQUENCE;

exit:
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(TestASN1, NullWriter)
{
    CHIP_ERROR err;
    ASN1Writer writer;
    size_t encodedLen;

    writer.InitNullWriter();

    err = EncodeASN1TestData(writer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    encodedLen = writer.GetLengthWritten();
    EXPECT_EQ(encodedLen, 0u);

    // Methods that take a reader should still read from it,
    // even if the output is suppressed by the null writer.
    TLVReader emptyTlvReader;
    emptyTlvReader.Init(ByteSpan());
    err = writer.PutBitString(0, emptyTlvReader);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    emptyTlvReader.Init(ByteSpan());
    err = writer.PutOctetString(kASN1TagClass_ContextSpecific, 123, emptyTlvReader);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);
}

TEST(TestASN1, ASN1UniversalTime)
{
    struct ASN1TimeTestCase
    {
        ASN1UniversalTime asn1Time;
        const char * asn1TimeStr;
    };

    struct ASN1TimeErrorTestCase
    {
        const char * asn1TimeStr;
        CHIP_ERROR mExpectedResult;
    };

    // clang-format off
    static ASN1TimeTestCase sASN1TimeTestCases[] = {
        // ASN1 Universal Time           ASN1_TIME String
        // ====================================================
        {  { 2020, 10, 15, 14, 23, 43 },   "201015142343Z" },
        {  { 2020, 12,  1,  2, 34,  0 },   "201201023400Z" },
        {  { 1979,  1, 30, 12,  0,  0 },   "790130120000Z" },
        {  { 2079,  1, 30, 12,  0,  0 }, "20790130120000Z" },
        {  { 2049,  3, 31, 23, 59, 59 },   "490331235959Z" },
        {  { 1949,  3, 31, 23, 59, 59 }, "19490331235959Z" },
        {  { 1950,  3, 31, 23, 59, 59 },   "500331235959Z" },
    };
    // clang-format on

    // clang-format off
    static ASN1TimeErrorTestCase sASN1TimeErrorTestCases[] = {
        // ASN1_TIME String      Expected Result
        // =======================================================
        {    "201015142343z",    ASN1_ERROR_UNSUPPORTED_ENCODING },
        {    "20105142343Z",     ASN1_ERROR_UNSUPPORTED_ENCODING },
        {    "2010115142343Z",   ASN1_ERROR_UNSUPPORTED_ENCODING },
        {    "201014415142343Z", ASN1_ERROR_UNSUPPORTED_ENCODING },
        {    "201O15142343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201" "\xe9" "15142343Z", ASN1_ERROR_INVALID_ENCODING },
        {    "200015142343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201315142343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201000142343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201032142343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201015242343Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201015146043Z",    ASN1_ERROR_INVALID_ENCODING     },
        {    "201015142360Z",    ASN1_ERROR_INVALID_ENCODING     },
    };
    // clang-format on

    for (auto & testCase : sASN1TimeTestCases)
    {
        CharSpan testStr = CharSpan(testCase.asn1TimeStr, strlen(testCase.asn1TimeStr));
        ASN1UniversalTime result;

        EXPECT_EQ(result.ImportFrom_ASN1_TIME_string(testStr), CHIP_NO_ERROR);

        EXPECT_EQ(result.Year, testCase.asn1Time.Year);
        EXPECT_EQ(result.Month, testCase.asn1Time.Month);
        EXPECT_EQ(result.Day, testCase.asn1Time.Day);
        EXPECT_EQ(result.Hour, testCase.asn1Time.Hour);
        EXPECT_EQ(result.Minute, testCase.asn1Time.Minute);
        EXPECT_EQ(result.Second, testCase.asn1Time.Second);

        char buf[ASN1UniversalTime::kASN1TimeStringMaxLength];
        MutableCharSpan resultTimeStr(buf);
        EXPECT_EQ(result.ExportTo_ASN1_TIME_string(resultTimeStr), CHIP_NO_ERROR);
        EXPECT_TRUE(resultTimeStr.data_equal(testStr));
    }

    for (auto & testCase : sASN1TimeErrorTestCases)
    {
        CharSpan testStr = CharSpan(testCase.asn1TimeStr, strlen(testCase.asn1TimeStr));
        ASN1UniversalTime result;

        EXPECT_EQ(result.ImportFrom_ASN1_TIME_string(testStr), testCase.mExpectedResult);
    }
}

TEST(TestASN1, ObjectID)
{
    CHIP_ERROR err;
    static uint8_t buf[2048];
    ASN1Writer writer;
    ASN1Reader reader;
    size_t encodedLen;

    writer.Init(buf, sizeof(buf));

    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_OBJECT_ID(kOID_AttributeType_MatterNodeId);
        ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        ASN1_ENCODE_OBJECT_ID(kOID_EllipticCurve_prime256v1);
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_AuthorityKeyIdentifier);
        ASN1_ENCODE_OBJECT_ID(kOID_Extension_BasicConstraints);
        ASN1_ENCODE_OBJECT_ID(kOID_KeyPurpose_ServerAuth);
    }
    ASN1_END_SEQUENCE;

    encodedLen = writer.GetLengthWritten();
    EXPECT_GT(encodedLen, 0u);

    reader.Init(buf, encodedLen);

    // Parse and check OIDs as actual ASN1 encoded values.
    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_AttributeType_ChipNodeId));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_AttributeType_ChipNodeId, sizeof(sOID_AttributeType_ChipNodeId)), 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_SigAlgo_ECDSAWithSHA256));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_SigAlgo_ECDSAWithSHA256, sizeof(sOID_SigAlgo_ECDSAWithSHA256)), 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_EllipticCurve_prime256v1));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_EllipticCurve_prime256v1, sizeof(sOID_EllipticCurve_prime256v1)), 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_Extension_AuthorityKeyIdentifier));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_Extension_AuthorityKeyIdentifier, sizeof(sOID_Extension_AuthorityKeyIdentifier)),
                  0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_Extension_BasicConstraints));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_Extension_BasicConstraints, sizeof(sOID_Extension_BasicConstraints)), 0);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId);
        EXPECT_EQ(reader.GetValueLen(), sizeof(sOID_KeyPurpose_ServerAuth));
        EXPECT_EQ(memcmp(reader.GetValue(), sOID_KeyPurpose_ServerAuth, sizeof(sOID_KeyPurpose_ServerAuth)), 0);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(TestASN1, FromTLVReader)
{
    CHIP_ERROR err;
    static uint8_t tlvBuf[128];
    static uint8_t asn1Buf1[128];
    static uint8_t asn1Buf2[128];
    TLVWriter tlvWriter;
    TLVReader tlvReader;
    ASN1Writer writer;
    ASN1Reader reader;
    MutableByteSpan tlvEncodedData(tlvBuf);
    MutableByteSpan asn1EncodedData1(asn1Buf1);
    MutableByteSpan asn1EncodedData2(asn1Buf2);
    TLVType outerContainerType;

    // Construct TLV Encoded Structure.
    {
        tlvWriter.Init(tlvEncodedData);

        err = tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvWriter.PutBytes(TLV::ContextTag(1), kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvWriter.PutBytes(TLV::ContextTag(2), kTestVal_09_BitString_AsOctetString,
                                 sizeof(kTestVal_09_BitString_AsOctetString));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvWriter.PutString(TLV::ContextTag(3), kTestVal_21_PrintableString);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvWriter.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvWriter.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Construct first ASN1 SEQUESNCE using values.
    writer.Init(asn1EncodedData1);
    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_OCTET_STRING(kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString));

        ASN1_ENCODE_BIT_STRING(kTestVal_09_BitString);

        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_PrintableString, false,
                              reinterpret_cast<const uint8_t *>(kTestVal_21_PrintableString),
                              static_cast<uint16_t>(strlen(kTestVal_21_PrintableString)));
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    ASN1_END_SEQUENCE;
    asn1EncodedData1.reduce_size(writer.GetLengthWritten());

    // Construct second ASN1 SEQUENCE from TLVReader.
    tlvReader.Init(tlvEncodedData);
    writer.Init(asn1EncodedData2);
    ASN1_START_SEQUENCE
    {
        err = tlvReader.Next(kTLVType_Structure, AnonymousTag());
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvReader.EnterContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvReader.Next(kTLVType_ByteString, ContextTag(1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutOctetString(kASN1TagClass_Universal, kASN1UniversalTag_OctetString, tlvReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvReader.Next(kTLVType_ByteString, ContextTag(2));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutBitString(6, tlvReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvReader.Next(kTLVType_UTF8String, ContextTag(3));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_PrintableString, false, tlvReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tlvReader.ExitContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    ASN1_END_SEQUENCE;
    asn1EncodedData2.reduce_size(writer.GetLengthWritten());

    // Compare two ASN1 SEQUENCEs.
    EXPECT_TRUE(asn1EncodedData2.data_equal(asn1EncodedData1));

    // Initialize ASN1Reader and test data.
    reader.Init(asn1EncodedData2);
    ASN1_PARSE_ENTER_SEQUENCE
    {
        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_OctetString);
        ASSERT_NE(reader.GetValue(), nullptr);
        EXPECT_EQ(reader.GetValueLen(), sizeof(kTestVal_20_OctetString));
        EXPECT_EQ(memcmp(reader.GetValue(), kTestVal_20_OctetString, sizeof(kTestVal_20_OctetString)), 0);

        uint32_t val;
        ASN1_PARSE_BIT_STRING(val);
        EXPECT_EQ(val, kTestVal_09_BitString);

        ASN1_PARSE_ELEMENT(kASN1TagClass_Universal, kASN1UniversalTag_PrintableString);
        ASSERT_NE(reader.GetValue(), nullptr);
        EXPECT_EQ(reader.GetValueLen(), strlen(kTestVal_21_PrintableString));
        EXPECT_EQ(memcmp(reader.GetValue(), kTestVal_21_PrintableString, strlen(kTestVal_21_PrintableString)), 0);
    }
    ASN1_EXIT_SEQUENCE;

exit:
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// ---------------------------------------------------------------------------
// Regression coverage for the behavior change in this PR.
//
// The only externally observable contract change is in ASN1Reader::Next() and
// ASN1Reader::ExitContainer(): an inner element whose declared length overruns
// its parent container previously caused the reader to advance past
// mContainerEnd and report ASN1_END (which the ASN1_EXIT_* macros and
// DumpASN1-style loops treat as a clean end-of-stream, silently swallowing the
// malformed encoding). It now returns ASN1_ERROR_INVALID_ENCODING.
//
// The remaining production changes (the uint32 addition-overflow guards in
// Next()/ExitContainer()/GetConstructedType()/Get*(), the mContainerEnd >=
// mElemStart consistency guards, and the EnterContainer/ExitContainer
// peek-then-commit reordering) are defense-in-depth: DecodeHead already caps
// ValueLen at the remaining buffer, so mHeadLen + ValueLen cannot wrap a
// uint32 from any input that reaches these methods through the public API.
// Those branches are intentionally NOT exercised by fabricated private-state
// tests -- doing so would only prove the guard fires on states the parser
// cannot produce. They are exercised indirectly: every positive test below
// passes through the new guards on its way to a correct result.
//
// The GetBitString() signed-shift fix is a separate, real correctness fix and
// is proven by dedicated tests below (catchable under -fsanitize=shift).
// ---------------------------------------------------------------------------

// Positive round-trip: build a nested SEQUENCE-in-SEQUENCE, then assert that
// GetConstructedType reports valLen = HeadLen + ValueLen exactly equal to the
// byte span of the encoded element in the source buffer. This drives a valid
// element through the new GetConstructedType() overflow/consistency guards and
// confirms they do not reject well-formed input.
TEST(TestASN1, GetConstructedType_RoundTripValidNested)
{
    // Hand-rolled DER: SEQUENCE { SEQUENCE { INTEGER 0x42, INTEGER 0x7FFFFFFF } }.
    //   30 0B                          -- outer SEQUENCE, length 11 (entire inner SEQUENCE)
    //     30 09                        -- inner SEQUENCE, length 9 (two INTEGERs follow)
    //       02 01 42                   -- INTEGER 0x42
    //       02 04 7F FF FF FF          -- INTEGER 0x7FFFFFFF
    static const uint8_t kEncoded[] = { 0x30, 0x0B, 0x30, 0x09, 0x02, 0x01, 0x42, 0x02, 0x04, 0x7F, 0xFF, 0xFF, 0xFF };

    ASN1Reader reader;
    reader.Init(kEncoded, sizeof(kEncoded));

    // Position at outer SEQUENCE.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());

    // Capture pointer + length of the OUTER element via GetConstructedType.
    const uint8_t * outerVal = nullptr;
    uint32_t outerLen        = 0;
    EXPECT_EQ(reader.GetConstructedType(outerVal, outerLen), CHIP_NO_ERROR);
    EXPECT_EQ(outerVal, &kEncoded[0]);
    EXPECT_EQ(outerLen, static_cast<uint32_t>(sizeof(kEncoded)));

    // Descend, position on inner SEQUENCE, capture its span.
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());

    const uint8_t * innerVal = nullptr;
    uint32_t innerLen        = 0;
    EXPECT_EQ(reader.GetConstructedType(innerVal, innerLen), CHIP_NO_ERROR);
    ASSERT_NE(innerVal, nullptr);

    // Inner element must be wholly contained inside the outer element's reported
    // span, and its first byte must be the SEQUENCE tag (0x30).
    EXPECT_GE(innerVal, outerVal);
    EXPECT_LE(innerVal + innerLen, outerVal + outerLen);
    EXPECT_EQ(innerVal[0], 0x30u);

    // GetConstructedType on a primitive element must fail with INVALID_STATE.
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR); // INTEGER 0x42
    EXPECT_FALSE(reader.IsConstructed());
    const uint8_t * v = nullptr;
    uint32_t vLen     = 0;
    EXPECT_EQ(reader.GetConstructedType(v, vLen), ASN1_ERROR_INVALID_STATE);
}

// Edge case: a tag with a 0-byte length (ASN.1 NULL { 0x05, 0x00 }) should be
// readable, and Next() / EnterContainer / ExitContainer must all advance past it
// correctly without confusing the reader's position.
TEST(TestASN1, ASN1Reader_ZeroLengthElements)
{
    // SEQUENCE { NULL, NULL, INTEGER 0x2A }
    //   30 07     -- SEQUENCE, length 7
    //     05 00   -- NULL
    //     05 00   -- NULL
    //     02 01 2A -- INTEGER 42
    static const uint8_t kEncoded[] = { 0x30, 0x07, 0x05, 0x00, 0x05, 0x00, 0x02, 0x01, 0x2A };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Sequence);

    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);

    // First NULL: tag = 0x05, value length = 0.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_FALSE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Null);
    EXPECT_EQ(reader.GetValueLen(), 0u);

    // Second NULL: same tag, same zero length -- reader must not stall on a 0-length value.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Null);
    EXPECT_EQ(reader.GetValueLen(), 0u);

    // INTEGER 42 follows immediately after the two zero-length NULLs.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Integer);
    int64_t intVal = 0;
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 42);

    EXPECT_EQ(reader.Next(), ASN1_END);
    EXPECT_EQ(reader.ExitConstructedType(), CHIP_NO_ERROR);
}

// Build a deeply nested SEQUENCE-of-SEQUENCE structure (>5 levels), enter all
// the way down, then exit all the way back up, asserting CHIP_NO_ERROR at each
// step. Each ExitContainer passes through the new peek-then-commit unwind.
TEST(TestASN1, ASN1Reader_DeeplyNestedContainer)
{
    // Hand-rolled DER: 6 nested SEQUENCEs wrapping INTEGER 0x55.
    static const uint8_t kEncoded[] = {
        0x30, 0x0D, 0x30, 0x0B, 0x30, 0x09, 0x30, 0x07, 0x30, 0x05, 0x30, 0x03, 0x02, 0x01, 0x55,
    };

    ASN1Reader reader;
    reader.Init(kEncoded, sizeof(kEncoded));

    constexpr int kDepth = 6; // matches the 6 nested SEQUENCE wrappers above.
    for (int i = 0; i < kDepth; ++i)
    {
        EXPECT_EQ(reader.Next(), CHIP_NO_ERROR) << "Next() failed at depth " << i;
        EXPECT_TRUE(reader.IsConstructed()) << "Element at depth " << i << " should be constructed";
        EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR) << "EnterConstructedType failed at depth " << i;
    }

    // Innermost element is the INTEGER.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_FALSE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Integer);
    int64_t intVal = 0;
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 0x55);

    // Climb back out -- every ExitConstructedType must succeed.
    for (int i = 0; i < kDepth; ++i)
    {
        EXPECT_EQ(reader.Next(), ASN1_END) << "Next() at end of depth " << (kDepth - i);
        EXPECT_EQ(reader.ExitConstructedType(), CHIP_NO_ERROR) << "ExitConstructedType failed at depth " << (kDepth - i);
    }
}

// When two elements abut and the second begins exactly at container_end, calling
// Next() after consuming the last in-range element must report ASN1_END rather
// than reading past the boundary (exercises the `mElemStart == mContainerEnd`
// early return; confirms the new bounds guard does NOT misclassify a clean end
// of container as INVALID_ENCODING).
TEST(TestASN1, ASN1Reader_AdjacentElementsAtBoundary)
{
    //   30 06          SEQUENCE, length 6
    //     02 01 01     INTEGER 1
    //     02 01 02     INTEGER 2
    //   02 01 03       INTEGER 3   <-- starts exactly at the byte after the SEQUENCE
    static const uint8_t kEncoded[] = { 0x30, 0x06, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03 };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Sequence);
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);

    int64_t intVal = 0;
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 1);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 2);

    // Next element would start exactly at mContainerEnd -- must be reported as
    // ASN1_END (clean end of container), NOT INVALID_ENCODING, and must NOT
    // surface the trailing INTEGER 3.
    EXPECT_EQ(reader.Next(), ASN1_END);

    // After ExitConstructedType we should see the trailing INTEGER 3.
    EXPECT_EQ(reader.ExitConstructedType(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Integer);
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 3);

    EXPECT_EQ(reader.Next(), ASN1_END);
}

// Zero-length elements positioned at container boundaries: a NULL at the very
// FIRST slot of a SEQUENCE, a NULL at the very LAST slot, and an entirely-empty
// SEQUENCE {}. Exercises the boundary positions (mElemStart at the start of the
// container's value bytes; header ending exactly at mContainerEnd; empty
// container) against the bounds guard.
TEST(TestASN1, ASN1Reader_ZeroLengthElementsAtContainerBoundaries)
{
    // SEQUENCE { NULL, INTEGER 7, NULL }
    static const uint8_t kEncoded[] = {
        0x30, 0x07, 0x05, 0x00, 0x02, 0x01, 0x07, 0x05, 0x00,
    };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Sequence);
    EXPECT_EQ(reader.GetValueLen(), 7u);
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);

    // FIRST slot: zero-length NULL at the very start of the container.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_FALSE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Null);
    EXPECT_EQ(reader.GetValueLen(), 0u);

    // Middle: INTEGER 7.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_FALSE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Integer);
    int64_t intVal = 0;
    EXPECT_EQ(reader.GetInteger(intVal), CHIP_NO_ERROR);
    EXPECT_EQ(intVal, 7);

    // LAST slot: zero-length NULL whose 2-byte header ends exactly at the
    // SEQUENCE's mContainerEnd.
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Null);
    EXPECT_EQ(reader.GetValueLen(), 0u);

    EXPECT_EQ(reader.Next(), ASN1_END);
    EXPECT_EQ(reader.ExitConstructedType(), CHIP_NO_ERROR);

    // Boundary case: SEQUENCE {} with no elements at all.
    static const uint8_t kEmptySeq[] = { 0x30, 0x00 };
    ASN1Reader emptyReader;
    emptyReader.Init(kEmptySeq);

    EXPECT_EQ(emptyReader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(emptyReader.IsConstructed());
    EXPECT_EQ(emptyReader.GetValueLen(), 0u);
    EXPECT_EQ(emptyReader.EnterConstructedType(), CHIP_NO_ERROR);
    EXPECT_EQ(emptyReader.Next(), ASN1_END);
    EXPECT_EQ(emptyReader.ExitConstructedType(), CHIP_NO_ERROR);
}

// Boundary regression for ExitContainer's "sum > ContainerEnd" check: when the
// saved context's ElemStart + HeadLen + ValueLen equals ContainerEnd EXACTLY,
// ExitContainer must succeed (not return an error). This pins the off-by-one the
// `<=` comparator must not regress to `<`.
TEST(TestASN1, ExitContainer_BoundarySumEqualsContainerEnd_Succeeds)
{
    // SEQUENCE { OCTET STRING (3 bytes "abc") }
    //   30 05            -- SEQUENCE, length 5
    //     04 03 61 62 63 -- OCTET STRING "abc"
    static const uint8_t kEncoded[] = { 0x30, 0x05, 0x04, 0x03, 0x61, 0x62, 0x63 };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_OctetString);
    EXPECT_EQ(reader.GetValueLen(), 3u);

    EXPECT_EQ(reader.Next(), ASN1_END);

    // The regression assertion: ExitContainer must NOT error on the exact-boundary path.
    EXPECT_EQ(reader.ExitConstructedType(), CHIP_NO_ERROR);
}

// A SEQUENCE whose declared length overshoots the buffer must be rejected.
// DecodeHead's `static_cast<uint32_t>(mBufEnd - p) >= ValueLen` bounds check
// surfaces this through Next() as ASN1_ERROR_VALUE_OVERFLOW before the parser
// ever enters the container. This is the existing master behavior; the test
// documents that the new EnterContainer reordering does not regress it.
TEST(TestASN1, EnterConstructedType_DeclaredLengthExceedsBuffer)
{
    //   30 05 02 01 2A  -- SEQUENCE length=5, but body is only 3 bytes.
    static const uint8_t kEncoded[] = { 0x30, 0x05, 0x02, 0x01, 0x2A };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), ASN1_ERROR_VALUE_OVERFLOW);
}

// Indefinite-length elements are not supported by ASN1Reader::Next() (DER only).
// The second Next() must return ASN1_ERROR_UNSUPPORTED_ENCODING and must NOT
// advance based on the indefinite-length state.
TEST(TestASN1, Next_IndefiniteLengthRejectedOnSecondCall)
{
    //   30 80 ... 00 00  -- SEQUENCE, indefinite length (BER, not DER)
    static const uint8_t kEncoded[] = { 0x30, 0x80, 0x02, 0x01, 0x2A, 0x00, 0x00 };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_TRUE(reader.IsIndefiniteLen());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_Sequence);

    // The IndefiniteLen guard at the top of Next() must fire before the
    // overflow/bounds checks, returning UNSUPPORTED_ENCODING.
    EXPECT_EQ(reader.Next(), ASN1_ERROR_UNSUPPORTED_ENCODING);
}

// ---------------------------------------------------------------------------
// Proving tests for the ASN1_END -> ASN1_ERROR_INVALID_ENCODING contract change.
//
// These build REAL DER blobs and FAIL on pre-fix master (where the reader bleeds
// past mContainerEnd and returns ASN1_END, which callers map to CHIP_NO_ERROR)
// and PASS post-fix (where Next() returns ASN1_ERROR_INVALID_ENCODING).
// ---------------------------------------------------------------------------

// DER blob where an inner element declares ValueLen larger than the parent
// SEQUENCE's remaining bytes WITHOUT wrapping uint32. Pre-fix Next() returned
// ASN1_END (silently swallowed as clean EOF); post-fix it returns
// ASN1_ERROR_INVALID_ENCODING.
TEST(TestASN1, Next_BoundsExceededReturnsErrorNotEnd)
{
    //   30 04                     -- outer SEQUENCE, ValueLen = 4
    //   04 0A 00 ... (10 bytes)   -- inner OCTET STRING, ValueLen = 10 > 2 spare
    // DecodeHead accepts the inner head because mBufEnd is far enough out; the
    // bounds check `2 + 10 <= 4 - 2` then fails inside Next().
    static const uint8_t kEncoded[] = {
        0x30, 0x04,                                     // outer SEQUENCE, ValueLen = 4
        0x04, 0x0A,                                     // inner OCTET STRING, ValueLen = 10
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8 fill bytes so DecodeHead's mBufEnd guard succeeds
        0x00, 0x00,                                     // 2 more fill bytes
    };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_TRUE(reader.IsConstructed());
    EXPECT_EQ(reader.GetValueLen(), 4u);
    EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);

    // First Next() decodes the inner OCTET STRING head (HeadLen=2, ValueLen=10).
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_FALSE(reader.IsConstructed());
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_OctetString);
    EXPECT_EQ(reader.GetValueLen(), 10u);

    // Second Next() now sees mHeadLen=2, ValueLen=10 and must reject the advance:
    //   2 + 10 = 12  >  (mContainerEnd - mElemStart) = 4 - 2 = 2
    // Pre-fix this bled past mContainerEnd and returned ASN1_END (which the
    // ASN1_EXIT_* macros map to CHIP_NO_ERROR, silently swallowing corruption);
    // post-fix it surfaces as the malformed-encoding error code.
    CHIP_ERROR err = reader.Next();
    EXPECT_EQ(err, ASN1_ERROR_INVALID_ENCODING) << "Next() must surface inner-overruns-parent as INVALID_ENCODING, not ASN1_END "
                                                << "(got " << err.Format() << ")";
    EXPECT_NE(err, ASN1_END);
    EXPECT_NE(err, ASN1_ERROR_LENGTH_OVERFLOW);
}

// DumpASN1-style top-level loop must surface a length-overrun child as a hard
// parse error (INVALID_ENCODING), not silently terminate via the
// ASN1_END -> CHIP_NO_ERROR mapping the ASN1_EXIT_* macros perform.
TEST(TestASN1, DumpLikeLoop_RejectsBoundsOverrun)
{
    // Same DER layout as Next_BoundsExceededReturnsErrorNotEnd.
    static const uint8_t kEncoded[] = {
        0x30, 0x04,                                     // outer SEQUENCE, ValueLen = 4
        0x04, 0x0A,                                     // inner OCTET STRING, ValueLen = 10
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8 fill bytes for DecodeHead's mBufEnd guard
        0x00, 0x00,                                     // 2 more fill bytes
    };

    ASN1Reader reader;
    reader.Init(kEncoded);

    CHIP_ERROR err   = CHIP_NO_ERROR;
    int nestLevel    = 0;
    bool sawOverflow = false;
    while (true)
    {
        err = reader.Next();
        if (err != CHIP_NO_ERROR)
        {
            // Pre-fix: err == ASN1_END here, the loop would break and the
            // caller would treat it as clean EOF (corruption swallowed).
            // Post-fix: err == ASN1_ERROR_INVALID_ENCODING, a hard error.
            sawOverflow = (err == ASN1_ERROR_INVALID_ENCODING);
            break;
        }
        if (reader.IsConstructed())
        {
            EXPECT_EQ(reader.EnterConstructedType(), CHIP_NO_ERROR);
            nestLevel++;
        }
    }

    EXPECT_TRUE(sawOverflow) << "DumpASN1-style loop must surface inner-overruns-parent as INVALID_ENCODING, "
                             << "not silently terminate via ASN1_END (got " << err.Format() << ")";
    EXPECT_NE(err, ASN1_END);
    EXPECT_NE(err, CHIP_NO_ERROR);
    EXPECT_EQ(nestLevel, 1) << "Should have descended into the outer SEQUENCE before the inner element tripped the guard";
}

// ---------------------------------------------------------------------------
// Proving tests for the GetBitString() signed-shift undefined-behavior fix.
//
// Pre-fix the inner expression was
//     static_cast<uint32_t>(ReverseBits(Value[i]) << shift)
// where the uint8_t result of ReverseBits is integer-promoted to (signed) int
// BEFORE the shift; a value with bit 7 set, shifted left by 24, sets the sign
// bit of an int -- undefined behavior, caught by -fsanitize=shift / UBSan.
//
// The fix moves the cast inside:
//     (static_cast<uint32_t>(ReverseBits(Value[i])) << shift)
// so the shift operates on a uint32_t. These tests use real DER BIT STRINGs and
// pin the exact bit pattern across every shift offset (8, 16, 24).
// ---------------------------------------------------------------------------

TEST(TestASN1, GetBitString_HighBitAtMaxShift_NoUndefinedBehavior)
{
    //   03 05 00 80 00 00 01  -- BIT STRING, 0 unused bits, 4 data bytes.
    //   ReverseBits(0x80)=0x01 at shift 0; ReverseBits(0x01)=0x80 at shift 24.
    //   Expected: 0x80 << 24 | 0x01 == 0x80000001 (the shift-24 high-bit UB case).
    static const uint8_t kBitStringHighBit[] = { 0x03, 0x05, 0x00, 0x80, 0x00, 0x00, 0x01 };

    ASN1Reader reader;
    reader.Init(kBitStringHighBit);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_BitString);

    uint32_t bits = 0;
    EXPECT_EQ(reader.GetBitString(bits), CHIP_NO_ERROR);
    EXPECT_EQ(bits, 0x80000001u);
}

TEST(TestASN1, GetBitString_AllBytesHighBit_ExactPattern)
{
    //   03 05 00 01 01 01 01  -- ReverseBits(0x01)=0x80, so each of the 4 data
    //   bytes contributes 0x80 at shifts 0, 8, 16, 24 => outVal = 0x80808080.
    static const uint8_t kEncoded[] = { 0x03, 0x05, 0x00, 0x01, 0x01, 0x01, 0x01 };

    ASN1Reader reader;
    reader.Init(kEncoded);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_BitString);

    uint32_t bits = 0;
    EXPECT_EQ(reader.GetBitString(bits), CHIP_NO_ERROR);
    EXPECT_EQ(bits, 0x80808080u);
}

// Edge case: ValueLen == 1 means only the "unused bits" byte is present and
// there are zero data bytes. The short-circuit branch must return 0 without
// reading past Value[0].
TEST(TestASN1, GetBitString_ZeroDataBytes_ReturnsZero)
{
    //   03 01 00  -- BIT STRING, length 1, 0 unused bits, no data.
    static const uint8_t kEmptyBitString[] = { 0x03, 0x01, 0x00 };

    ASN1Reader reader;
    reader.Init(kEmptyBitString);

    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetTag(), kASN1UniversalTag_BitString);
    EXPECT_EQ(reader.GetValueLen(), 1u);

    uint32_t bits = 0xDEADBEEF; // pre-populate to catch "never assigned"
    EXPECT_EQ(reader.GetBitString(bits), CHIP_NO_ERROR);
    EXPECT_EQ(bits, 0u);
}
