/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      This file implements a unit test suite for CHIP Certification
 *      declaration classes and APIs.
 */

#include <inttypes.h>
#include <stddef.h>

#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::ASN1;
using namespace chip::Crypto;
using namespace chip::Credentials;

static constexpr uint8_t sTestCMS_SignerCert[] = {
    0x30, 0x82, 0x01, 0xb3, 0x30, 0x82, 0x01, 0x5a, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x45, 0xda, 0xf3, 0x9d, 0xe4, 0x7a,
    0xa0, 0x8f, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x2b, 0x31, 0x29, 0x30, 0x27, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0c, 0x20, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x44, 0x20,
    0x53, 0x69, 0x67, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30, 0x20, 0x17, 0x0d,
    0x32, 0x31, 0x30, 0x36, 0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34, 0x33, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32,
    0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x2b, 0x31, 0x29, 0x30, 0x27, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c,
    0x20, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x44, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x69,
    0x6e, 0x67, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x3c, 0x39, 0x89,
    0x22, 0x45, 0x2b, 0x55, 0xca, 0xf3, 0x89, 0xc2, 0x5b, 0xd1, 0xbc, 0xa4, 0x65, 0x69, 0x52, 0xcc, 0xb9, 0x0e, 0x88, 0x69, 0x24,
    0x9a, 0xd8, 0x47, 0x46, 0x53, 0x01, 0x4c, 0xbf, 0x95, 0xd6, 0x87, 0x96, 0x5e, 0x03, 0x6b, 0x52, 0x1c, 0x51, 0x03, 0x7e, 0x6b,
    0x8c, 0xed, 0xef, 0xca, 0x1e, 0xb4, 0x40, 0x46, 0x69, 0x4f, 0xa0, 0x88, 0x82, 0xee, 0xd6, 0x51, 0x9d, 0xec, 0xba, 0xa3, 0x66,
    0x30, 0x64, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xff, 0x02, 0x01,
    0x01, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d, 0x06, 0x03,
    0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a,
    0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x62, 0xfa,
    0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0a, 0x06,
    0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x2c, 0x54, 0x5c, 0xe4, 0xe4,
    0x57, 0xd8, 0xa6, 0xf0, 0xd9, 0xd9, 0xbb, 0xeb, 0xd6, 0xec, 0xe1, 0xdd, 0xfe, 0x7f, 0x8c, 0x6d, 0x9a, 0x6c, 0xf3, 0x75, 0x32,
    0x1f, 0xc6, 0xfa, 0xc7, 0x13, 0x84, 0x02, 0x20, 0x54, 0x07, 0x78, 0xe8, 0x74, 0x39, 0x72, 0x52, 0x7e, 0xed, 0xeb, 0xaf, 0x58,
    0x68, 0x62, 0x20, 0xb5, 0x40, 0x78, 0xf2, 0xcd, 0x4e, 0x62, 0xa7, 0x6a, 0xe7, 0xcb, 0xb9, 0x2f, 0xf5, 0x4c, 0x8b,
};

static constexpr uint8_t sTestCMS_SignerPublicKey[] = { 0x04, 0x3c, 0x39, 0x89, 0x22, 0x45, 0x2b, 0x55, 0xca, 0xf3, 0x89,
                                                        0xc2, 0x5b, 0xd1, 0xbc, 0xa4, 0x65, 0x69, 0x52, 0xcc, 0xb9, 0x0e,
                                                        0x88, 0x69, 0x24, 0x9a, 0xd8, 0x47, 0x46, 0x53, 0x01, 0x4c, 0xbf,
                                                        0x95, 0xd6, 0x87, 0x96, 0x5e, 0x03, 0x6b, 0x52, 0x1c, 0x51, 0x03,
                                                        0x7e, 0x6b, 0x8c, 0xed, 0xef, 0xca, 0x1e, 0xb4, 0x40, 0x46, 0x69,
                                                        0x4f, 0xa0, 0x88, 0x82, 0xee, 0xd6, 0x51, 0x9d, 0xec, 0xba };

static constexpr uint8_t sTestCMS_SignerSerializedKeypair[] = {
    0x04, 0x3c, 0x39, 0x89, 0x22, 0x45, 0x2b, 0x55, 0xca, 0xf3, 0x89, 0xc2, 0x5b, 0xd1, 0xbc, 0xa4, 0x65, 0x69, 0x52, 0xcc,
    0xb9, 0x0e, 0x88, 0x69, 0x24, 0x9a, 0xd8, 0x47, 0x46, 0x53, 0x01, 0x4c, 0xbf, 0x95, 0xd6, 0x87, 0x96, 0x5e, 0x03, 0x6b,
    0x52, 0x1c, 0x51, 0x03, 0x7e, 0x6b, 0x8c, 0xed, 0xef, 0xca, 0x1e, 0xb4, 0x40, 0x46, 0x69, 0x4f, 0xa0, 0x88, 0x82, 0xee,
    0xd6, 0x51, 0x9d, 0xec, 0xba, 0xae, 0xf3, 0x48, 0x41, 0x16, 0xe9, 0x48, 0x1e, 0xc5, 0x7b, 0xe0, 0x47, 0x2d, 0xf4, 0x1b,
    0xf4, 0x99, 0x06, 0x4e, 0x50, 0x24, 0xad, 0x86, 0x9e, 0xca, 0x5e, 0x88, 0x98, 0x02, 0xd4, 0x80, 0x75
};

// First set of test vectors for the following set of CD parameters:
// -> format_version = 1
// -> vendor_id = 0xFFF1
// -> product_id_array = [ 0x8000 ]
// -> device_type_id = 0x1234
// -> certificate_id = "ZIG20141ZB330001-24"
// -> security_level = 0
// -> security_information = 0
// -> version_number = 0x2694
// -> certification_type = 0
// -> dac_origin_vendor_id is not present
// -> dac_origin_product_id is not present
static constexpr CertificationElements sTestCMS_CertElements01 = { 1,    0xFFF1, { 0x8000 }, 1, 0x1234, "ZIG20141ZB330001-24",
                                                                   0,    0,      0x2694,     0, 0,      0,
                                                                   false };

static constexpr uint8_t sTestCMS_CDContent01[] = { 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05,
                                                    0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a,
                                                    0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33,
                                                    0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24,
                                                    0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18 };

static constexpr uint8_t sTestCMS_SignedMessage01[] = {
    0x30, 0x81, 0xe8, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xda, 0x30, 0x81, 0xd7,
    0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x45,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15, 0x24, 0x00, 0x01, 0x25,
    0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32,
    0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06,
    0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7c, 0x30, 0x7a, 0x02, 0x01, 0x03, 0x80, 0x14, 0x62, 0xfa,
    0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0b,
    0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x04, 0x03, 0x02, 0x04, 0x46, 0x30, 0x44, 0x02, 0x20, 0x43, 0xa6, 0x3f, 0x2b, 0x94, 0x3d, 0xf3, 0x3c, 0x38, 0xb3, 0xe0,
    0x2f, 0xca, 0xa7, 0x5f, 0xe3, 0x53, 0x2a, 0xeb, 0xbf, 0x5e, 0x63, 0xf5, 0xbb, 0xdb, 0xc0, 0xb1, 0xf0, 0x1d, 0x3c, 0x4f,
    0x60, 0x02, 0x20, 0x4c, 0x1a, 0xbf, 0x5f, 0x18, 0x07, 0xb8, 0x18, 0x94, 0xb1, 0x57, 0x6c, 0x47, 0xe4, 0x72, 0x4e, 0x4d,
    0x96, 0x6c, 0x61, 0x2e, 0xd3, 0xfa, 0x25, 0xc1, 0x18, 0xc3, 0xf2, 0xb3, 0xf9, 0x03, 0x69
};

// First set of test vectors for the following set of CD parameters:
// -> format_version = 1
// -> vendor_id = 0xFFF2
// -> product_id_array = [ 0x8001, 0x8002 ]
// -> device_type_id = 0x1234
// -> certificate_id = "ZIG20142ZB330002-24"
// -> security_level = 0
// -> security_information = 0
// -> version_number = 0x2694
// -> certification_type = 0
// -> dac_origin_vendor_id = 0xFFF1
// -> dac_origin_product_id = 0x8000
static constexpr CertificationElements sTestCMS_CertElements02 = {
    1, 0xFFF2, { 0x8001, 0x8002 }, 2, 0x1234, "ZIG20142ZB330002-24", 0, 0, 0x2694, 0, 0xFFF1, 0x8000, true
};

static constexpr uint8_t sTestCMS_CDContent02[] = { 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf2, 0xff, 0x36, 0x02, 0x05, 0x01, 0x80,
                                                    0x05, 0x02, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49,
                                                    0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30,
                                                    0x32, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94,
                                                    0x26, 0x24, 0x08, 0x00, 0x25, 0x09, 0xf1, 0xff, 0x25, 0x0a, 0x00, 0x80, 0x18 };

static constexpr uint8_t sTestCMS_SignedMessage02[] = {
    0x30, 0x81, 0xf5, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xe7, 0x30, 0x81, 0xe4, 0x02,
    0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x50, 0x06, 0x09,
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x43, 0x04, 0x41, 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf2, 0xff,
    0x36, 0x02, 0x05, 0x01, 0x80, 0x05, 0x02, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32, 0x30,
    0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x32, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25,
    0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x25, 0x09, 0xf1, 0xff, 0x25, 0x0a, 0x00, 0x80, 0x18, 0x31, 0x7e, 0x30, 0x7c, 0x02, 0x01,
    0x03, 0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3,
    0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x48, 0x30, 0x46, 0x02, 0x21, 0x00, 0x92, 0x62, 0x96, 0xf7, 0x57, 0x81, 0x58, 0xbe,
    0x7c, 0x45, 0x93, 0x88, 0x33, 0x6c, 0xa7, 0x38, 0x37, 0x66, 0xc9, 0xee, 0xdd, 0x98, 0x55, 0xcb, 0xda, 0x6f, 0x4c, 0xf6, 0xbd,
    0xf4, 0x32, 0x11, 0x02, 0x21, 0x00, 0xe0, 0xdb, 0xf4, 0xa2, 0xbc, 0xec, 0x4e, 0xa2, 0x74, 0xba, 0xf0, 0xde, 0xa2, 0x08, 0xb3,
    0x36, 0x5c, 0x6e, 0xd5, 0x44, 0x08, 0x6d, 0x10, 0x1a, 0xfd, 0xaf, 0x07, 0x9a, 0x2c, 0x23, 0xe0, 0xde
};

struct TestCase
{
    ByteSpan signerCert;
    P256PublicKeySpan signerPubkey;
    CertificationElements cdElements;
    ByteSpan cdContent;
    ByteSpan cdCMSSigned;
};

static constexpr TestCase sTestCases[] = {
    { ByteSpan(sTestCMS_SignerCert), P256PublicKeySpan(sTestCMS_SignerPublicKey), sTestCMS_CertElements01,
      ByteSpan(sTestCMS_CDContent01), ByteSpan(sTestCMS_SignedMessage01) },
    { ByteSpan(sTestCMS_SignerCert), P256PublicKeySpan(sTestCMS_SignerPublicKey), sTestCMS_CertElements02,
      ByteSpan(sTestCMS_CDContent02), ByteSpan(sTestCMS_SignedMessage02) },
};

static constexpr size_t sNumTestCases = ArraySize(sTestCases);

static void TestCD_EncodeDecode(nlTestSuite * inSuite, void * inContext)
{
    for (size_t i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];

        uint8_t encodedCertElemBuf[kCertificationElements_TLVEncodedMaxLength];
        MutableByteSpan encodedCDPayload(encodedCertElemBuf);

        NL_TEST_ASSERT(inSuite, EncodeCertificationElements(testCase.cdElements, encodedCDPayload) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, testCase.cdContent.data_equal(encodedCDPayload));

        CertificationElements decodedElements;
        NL_TEST_ASSERT(inSuite, DecodeCertificationElements(encodedCDPayload, decodedElements) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, decodedElements.FormatVersion == testCase.cdElements.FormatVersion);
        NL_TEST_ASSERT(inSuite, decodedElements.VendorId == testCase.cdElements.VendorId);
        NL_TEST_ASSERT(inSuite, decodedElements.ProductIdsCount == testCase.cdElements.ProductIdsCount);
        for (uint8_t j = 0; j < decodedElements.ProductIdsCount; j++)
        {
            NL_TEST_ASSERT(inSuite, decodedElements.ProductIds[j] == testCase.cdElements.ProductIds[j]);
        }
        NL_TEST_ASSERT(inSuite, decodedElements.DeviceTypeId == testCase.cdElements.DeviceTypeId);
        NL_TEST_ASSERT(inSuite,
                       memcmp(decodedElements.CertificateId, testCase.cdElements.CertificateId, kCertificateIdLength) == 0);
        NL_TEST_ASSERT(inSuite, decodedElements.SecurityLevel == testCase.cdElements.SecurityLevel);
        NL_TEST_ASSERT(inSuite, decodedElements.SecurityInformation == testCase.cdElements.SecurityInformation);
        NL_TEST_ASSERT(inSuite, decodedElements.VersionNumber == testCase.cdElements.VersionNumber);
        NL_TEST_ASSERT(inSuite, decodedElements.CertificationType == testCase.cdElements.CertificationType);
        NL_TEST_ASSERT(inSuite, decodedElements.DACOriginVIDandPIDPresent == testCase.cdElements.DACOriginVIDandPIDPresent);
        if (decodedElements.DACOriginVIDandPIDPresent)
        {
            NL_TEST_ASSERT(inSuite, decodedElements.DACOriginVendorId == testCase.cdElements.DACOriginVendorId);
            NL_TEST_ASSERT(inSuite, decodedElements.DACOriginProductId == testCase.cdElements.DACOriginProductId);
        }
        NL_TEST_ASSERT(inSuite, decodedElements.AuthorizedPAAListCount == testCase.cdElements.AuthorizedPAAListCount);
        for (uint8_t j = 0; j < decodedElements.AuthorizedPAAListCount; j++)
        {
            NL_TEST_ASSERT(
                inSuite,
                memcmp(decodedElements.AuthorizedPAAList[j], testCase.cdElements.AuthorizedPAAList[j], kKeyIdentifierLength) == 0);
        }
    }
}

static void TestCD_EncodeDecode_Errors(nlTestSuite * inSuite, void * inContext)
{
    uint8_t encodedCertElemBuf[kCertificationElements_TLVEncodedMaxLength];
    MutableByteSpan encodedCDPayload(encodedCertElemBuf);
    NL_TEST_ASSERT(inSuite, EncodeCertificationElements(sTestCMS_CertElements01, encodedCDPayload) == CHIP_NO_ERROR);

    // Test Encode Error: CHIP_ERROR_BUFFER_TOO_SMALL
    // Provide a smaller buffer as an input.
    encodedCDPayload.reduce_size(encodedCDPayload.size() - 4);
    NL_TEST_ASSERT(inSuite, EncodeCertificationElements(sTestCMS_CertElements01, encodedCDPayload) == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test Decode Error: CHIP_ERROR_INVALID_INTEGER_VALUE
    // Manually modified sTestCMS_CDContent01[]: updated VendorId element to 4-octet
    // (0x25, 0x01, 0xf1, 0xff, --> 0x26, 0x01, 0xf1, 0xff, 0xff, 0xff,)
    static constexpr uint8_t sTestCMS_CDContent01_Err01[] = {
        0x15, 0x24, 0x00, 0x01, 0x26, 0x01, 0xf1, 0xff, 0xff, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x18, 0x25, 0x03, 0x34,
        0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30,
        0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18
    };

    CertificationElements certElementsOut;
    NL_TEST_ASSERT(inSuite,
                   DecodeCertificationElements(ByteSpan(sTestCMS_CDContent01_Err01), certElementsOut) ==
                       CHIP_ERROR_INVALID_INTEGER_VALUE);

    // Test Decode Error: CHIP_ERROR_UNEXPECTED_TLV_ELEMENT
    // Manually modified sTestCMS_CDContent01[]: switched ProductIds and DeviceTypeId elements tag (0x02 <--> 0x03)
    static constexpr uint8_t sTestCMS_CDContent01_Err02[] = { 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x03, 0x05,
                                                              0x00, 0x80, 0x18, 0x25, 0x02, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a,
                                                              0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33,
                                                              0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24,
                                                              0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18 };
    NL_TEST_ASSERT(inSuite,
                   DecodeCertificationElements(ByteSpan(sTestCMS_CDContent01_Err02), certElementsOut) ==
                       CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
}

static void TestCD_CMSSignAndVerify(nlTestSuite * inSuite, void * inContext)
{
    ByteSpan cdContentIn(sTestCMS_CDContent01);
    ByteSpan cdContentOut;
    uint8_t signerKeyIdBuf[Crypto::kSubjectKeyIdentifierLength];
    MutableByteSpan signerKeyId(signerKeyIdBuf);
    uint8_t signedMessageBuf[kMaxCMSSignedCDMessage];
    MutableByteSpan signedMessage(signedMessageBuf);

    NL_TEST_ASSERT(inSuite, ExtractSKIDFromX509Cert(ByteSpan(sTestCMS_SignerCert), signerKeyId) == CHIP_NO_ERROR);

    // Test with random key
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CMS_Sign(cdContentIn, signerKeyId, keypair, signedMessage) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CMS_Verify(signedMessage, keypair.Pubkey(), cdContentOut) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, cdContentIn.data_equal(cdContentOut));

    // Test with known key
    P256Keypair keypair2;
    P256SerializedKeypair serializedKeypair;
    memcpy(serializedKeypair, sTestCMS_SignerSerializedKeypair, sizeof(sTestCMS_SignerSerializedKeypair));
    serializedKeypair.SetLength(sizeof(sTestCMS_SignerSerializedKeypair));
    cdContentIn   = ByteSpan(sTestCMS_CDContent02);
    signedMessage = MutableByteSpan(signedMessageBuf);
    NL_TEST_ASSERT(inSuite, keypair2.Deserialize(serializedKeypair) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CMS_Sign(cdContentIn, signerKeyId, keypair2, signedMessage) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, CMS_Verify(signedMessage, keypair2.Pubkey(), cdContentOut) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, cdContentIn.data_equal(cdContentOut));
}

static void TestCD_CMSVerifyAndExtract(nlTestSuite * inSuite, void * inContext)
{
    for (size_t i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];

        // Verify using signer P256PublicKey
        ByteSpan cdContentOut;
        NL_TEST_ASSERT(inSuite,
                       CMS_Verify(testCase.cdCMSSigned, P256PublicKey(testCase.signerPubkey), cdContentOut) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, testCase.cdContent.data_equal(cdContentOut));

        // Verify using signer X509 Certificate
        cdContentOut = ByteSpan();
        NL_TEST_ASSERT(inSuite, CMS_Verify(testCase.cdCMSSigned, testCase.signerCert, cdContentOut) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, testCase.cdContent.data_equal(cdContentOut));

        // Test CMS_ExtractCDContent()
        cdContentOut = ByteSpan();
        NL_TEST_ASSERT(inSuite, CMS_ExtractCDContent(testCase.cdCMSSigned, cdContentOut) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, testCase.cdContent.data_equal(cdContentOut));

        // Test CMS_ExtractKeyId()
        uint8_t signerKeyIdBuf[Crypto::kSubjectKeyIdentifierLength];
        MutableByteSpan signerKeyId(signerKeyIdBuf);
        NL_TEST_ASSERT(inSuite, ExtractSKIDFromX509Cert(testCase.signerCert, signerKeyId) == CHIP_NO_ERROR);

        ByteSpan signerKeyIdOut;
        NL_TEST_ASSERT(inSuite, CMS_ExtractKeyId(testCase.cdCMSSigned, signerKeyIdOut) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, signerKeyId.data_equal(signerKeyIdOut));
    }
}

static void TestCD_CertificationElementsDecoder(nlTestSuite * inSuite, void * inContext)
{
    for (size_t i = 0; i < sNumTestCases; i++)
    {
        const TestCase & testCase = sTestCases[i];

        uint8_t encodedCertElemBuf[kCertificationElements_TLVEncodedMaxLength];
        MutableByteSpan encodedCDPayload(encodedCertElemBuf);

        NL_TEST_ASSERT(inSuite, EncodeCertificationElements(testCase.cdElements, encodedCDPayload) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, testCase.cdContent.data_equal(encodedCDPayload));

        CertificationElementsWithoutPIDs certificationDeclarationContent;
        CertificationElementsDecoder certificationElementsDecoder;
        NL_TEST_ASSERT(inSuite, DecodeCertificationElements(encodedCDPayload, certificationDeclarationContent) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.formatVersion == testCase.cdElements.FormatVersion);
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.vendorId == testCase.cdElements.VendorId);
        for (uint8_t j = 0; j < testCase.cdElements.ProductIdsCount; j++)
        {
            NL_TEST_ASSERT(inSuite,
                           certificationElementsDecoder.IsProductIdIn(encodedCDPayload, testCase.cdElements.ProductIds[j]));
            // now test for an unexistent ProductId
            NL_TEST_ASSERT(inSuite, certificationElementsDecoder.IsProductIdIn(encodedCDPayload, 0x9000) == false);
        }
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.deviceTypeId == testCase.cdElements.DeviceTypeId);
        NL_TEST_ASSERT(
            inSuite,
            memcmp(certificationDeclarationContent.certificateId, testCase.cdElements.CertificateId, kCertificateIdLength) == 0);
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.securityLevel == testCase.cdElements.SecurityLevel);
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.securityInformation == testCase.cdElements.SecurityInformation);
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.versionNumber == testCase.cdElements.VersionNumber);
        NL_TEST_ASSERT(inSuite, certificationDeclarationContent.certificationType == testCase.cdElements.CertificationType);
        NL_TEST_ASSERT(inSuite,
                       certificationDeclarationContent.dacOriginVIDandPIDPresent == testCase.cdElements.DACOriginVIDandPIDPresent);
        if (certificationDeclarationContent.dacOriginVIDandPIDPresent)
        {
            NL_TEST_ASSERT(inSuite, certificationDeclarationContent.dacOriginVendorId == testCase.cdElements.DACOriginVendorId);
            NL_TEST_ASSERT(inSuite, certificationDeclarationContent.dacOriginProductId == testCase.cdElements.DACOriginProductId);
        }
        if (testCase.cdElements.AuthorizedPAAListCount > 0)
        {
            NL_TEST_ASSERT(inSuite, certificationDeclarationContent.authorizedPAAListPresent);
        }
        else
        {
            NL_TEST_ASSERT(inSuite, !certificationDeclarationContent.authorizedPAAListPresent);
        }
    }
}

static void TestCD_EncodeDecode_Random(nlTestSuite * inSuite, void * inContext)
{
    CertificationElements randomElements = { .FormatVersion             = 0x6F,
                                             .VendorId                  = 0x88EA,
                                             .ProductIds                = { 0 },
                                             .ProductIdsCount           = kMaxProductIdsCount,
                                             .DeviceTypeId              = 0x1234,
                                             .CertificateId             = "ZIG20141ZB330001-24",
                                             .SecurityLevel             = 10,
                                             .SecurityInformation       = 0xFA,
                                             .VersionNumber             = 0x28CA,
                                             .CertificationType         = 1,
                                             .DACOriginVendorId         = 0x01DC,
                                             .DACOriginProductId        = 0x10EE,
                                             .DACOriginVIDandPIDPresent = false,
                                             .AuthorizedPAAList         = { { 0 } },
                                             .AuthorizedPAAListCount    = kMaxAuthorizedPAAListCount };

    uint16_t pid = 0x8000;
    for (uint8_t j = 0; j < randomElements.ProductIdsCount; j++, pid++)
    {
        randomElements.ProductIds[j] = pid;
    }
    uint8_t kid[kKeyIdentifierLength] = { 0xF4, 0x44, 0xCA, 0xBB, 0xC5, 0x01, 0x65, 0x77, 0xAA, 0x8B,
                                          0x44, 0xFF, 0xB9, 0x0F, 0xCC, 0xA1, 0x40, 0xFE, 0x66, 0x20 };
    for (uint8_t j = 0; j < randomElements.AuthorizedPAAListCount; j++)
    {
        kid[(j % kKeyIdentifierLength)] ^= 0xFF;
        memcpy(randomElements.AuthorizedPAAList[j], kid, kKeyIdentifierLength);
    }

    uint8_t encodedCertElemBuf[kCertificationElements_TLVEncodedMaxLength];
    MutableByteSpan encodedCDPayload(encodedCertElemBuf);
    CertificationElements decodedElements;

    NL_TEST_ASSERT(inSuite, EncodeCertificationElements(randomElements, encodedCDPayload) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, DecodeCertificationElements(encodedCDPayload, decodedElements) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, decodedElements.FormatVersion == randomElements.FormatVersion);
    NL_TEST_ASSERT(inSuite, decodedElements.VendorId == randomElements.VendorId);
    NL_TEST_ASSERT(inSuite, decodedElements.ProductIdsCount == randomElements.ProductIdsCount);
    for (uint8_t j = 0; j < decodedElements.ProductIdsCount; j++)
    {
        NL_TEST_ASSERT(inSuite, decodedElements.ProductIds[j] == randomElements.ProductIds[j]);
    }
    NL_TEST_ASSERT(inSuite, decodedElements.DeviceTypeId == randomElements.DeviceTypeId);
    NL_TEST_ASSERT(inSuite, memcmp(decodedElements.CertificateId, randomElements.CertificateId, kCertificateIdLength) == 0);
    NL_TEST_ASSERT(inSuite, decodedElements.SecurityLevel == randomElements.SecurityLevel);
    NL_TEST_ASSERT(inSuite, decodedElements.SecurityInformation == randomElements.SecurityInformation);
    NL_TEST_ASSERT(inSuite, decodedElements.VersionNumber == randomElements.VersionNumber);
    NL_TEST_ASSERT(inSuite, decodedElements.CertificationType == randomElements.CertificationType);
    NL_TEST_ASSERT(inSuite, decodedElements.DACOriginVIDandPIDPresent == randomElements.DACOriginVIDandPIDPresent);
    if (decodedElements.DACOriginVIDandPIDPresent)
    {
        NL_TEST_ASSERT(inSuite, decodedElements.DACOriginVendorId == randomElements.DACOriginVendorId);
        NL_TEST_ASSERT(inSuite, decodedElements.DACOriginProductId == randomElements.DACOriginProductId);
    }
    NL_TEST_ASSERT(inSuite, decodedElements.AuthorizedPAAListCount == randomElements.AuthorizedPAAListCount);
    for (uint8_t j = 0; j < decodedElements.AuthorizedPAAListCount; j++)
    {
        NL_TEST_ASSERT(
            inSuite, memcmp(decodedElements.AuthorizedPAAList[j], randomElements.AuthorizedPAAList[j], kKeyIdentifierLength) == 0);
    }

    CertificationElementsWithoutPIDs decodedElements2;
    CertificationElementsDecoder cdElementsDecoder;
    NL_TEST_ASSERT(inSuite, DecodeCertificationElements(encodedCDPayload, decodedElements2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, decodedElements2.formatVersion == randomElements.FormatVersion);
    NL_TEST_ASSERT(inSuite, decodedElements2.vendorId == randomElements.VendorId);
    for (uint8_t j = 0; j < randomElements.ProductIdsCount; j++)
    {
        NL_TEST_ASSERT(inSuite, cdElementsDecoder.IsProductIdIn(encodedCDPayload, randomElements.ProductIds[j]));
        // now test for an unexistent ProductId
        NL_TEST_ASSERT(inSuite, cdElementsDecoder.IsProductIdIn(encodedCDPayload, pid++) == false);
    }
    NL_TEST_ASSERT(inSuite, decodedElements2.deviceTypeId == randomElements.DeviceTypeId);
    NL_TEST_ASSERT(inSuite, memcmp(decodedElements2.certificateId, randomElements.CertificateId, kCertificateIdLength) == 0);
    NL_TEST_ASSERT(inSuite, decodedElements2.securityLevel == randomElements.SecurityLevel);
    NL_TEST_ASSERT(inSuite, decodedElements2.securityInformation == randomElements.SecurityInformation);
    NL_TEST_ASSERT(inSuite, decodedElements2.versionNumber == randomElements.VersionNumber);
    NL_TEST_ASSERT(inSuite, decodedElements2.certificationType == randomElements.CertificationType);
    NL_TEST_ASSERT(inSuite, decodedElements2.dacOriginVIDandPIDPresent == randomElements.DACOriginVIDandPIDPresent);
    if (decodedElements2.dacOriginVIDandPIDPresent)
    {
        NL_TEST_ASSERT(inSuite, decodedElements2.dacOriginVendorId == randomElements.DACOriginVendorId);
        NL_TEST_ASSERT(inSuite, decodedElements2.dacOriginProductId == randomElements.DACOriginProductId);
    }
    NL_TEST_ASSERT(inSuite, decodedElements2.authorizedPAAListPresent);
    for (uint8_t j = 0; j < randomElements.AuthorizedPAAListCount; j++)
    {
        NL_TEST_ASSERT(inSuite,
                       cdElementsDecoder.HasAuthorizedPAA(encodedCDPayload, ByteSpan(randomElements.AuthorizedPAAList[j])));
        // now test for an unexistent PAA
        kid[(kKeyIdentifierLength - 1 - j) % kKeyIdentifierLength] ^= 0x5A;
        NL_TEST_ASSERT(inSuite, cdElementsDecoder.HasAuthorizedPAA(encodedCDPayload, ByteSpan(kid)) == false);
    }
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestCD_EncodeDecode),
                                 NL_TEST_DEF_FN(TestCD_EncodeDecode_Errors),
                                 NL_TEST_DEF_FN(TestCD_CMSSignAndVerify),
                                 NL_TEST_DEF_FN(TestCD_CMSVerifyAndExtract),
                                 NL_TEST_DEF_FN(TestCD_CertificationElementsDecoder),
                                 NL_TEST_DEF_FN(TestCD_EncodeDecode_Random),
                                 NL_TEST_SENTINEL() };

int TestCertificationDeclaration(void)
{
    nlTestSuite theSuite = { "CHIP Certification Declaration tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCertificationDeclaration);
