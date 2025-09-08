/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::TestCerts;

struct TestDeviceAttestationVerifier : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Test that AttestationDeviceInfo correctly copies fields from AttestationInfo
TEST_F(TestDeviceAttestationVerifier, AttestationDeviceInfoCopiesFields)
{
    // Prepare minimal data for PAI, DAC and other required fields
    uint8_t paiData[3]   = { 0x01, 0x02, 0x03 };
    uint8_t dacData[5]   = { 0x10, 0x11, 0x12, 0x13, 0x14 };
    uint8_t nonceData[4] = { 0x21, 0x22, 0x23, 0x24 };

    ByteSpan paiSpan(paiData);
    ByteSpan dacSpan(dacData);
    ByteSpan nonceSpan(nonceData);

    // Prepare attestation elements TLV with a minimal CertificateDeclaration
    uint8_t cdData[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t tlvBuf[64];
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(nonceData));
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0)); // Timestamp
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());               // FirmwareInfo
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());               // VendorReserved
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ByteSpan attestationElementsSpan(tlvBuf, writer.GetLengthWritten());

    // Construct AttestationInfo
    DeviceAttestationVerifier::AttestationInfo attestationInfo(attestationElementsSpan,
                                                               nonceSpan, // attestationChallenge
                                                               nonceSpan, // attestationSignature
                                                               paiSpan, dacSpan,
                                                               nonceSpan, // certificationDeclarationBuffer
                                                               VendorId(0x1234),
                                                               0x5678 // productId
    );

    // Create AttestationDeviceInfo
    DeviceAttestationVerifier::AttestationDeviceInfo deviceInfo(attestationInfo);

    // Verify that vendorId and productId were copied correctly
    EXPECT_EQ(deviceInfo.BasicInformationVendorId(), VendorId(0x1234));
    EXPECT_EQ(deviceInfo.BasicInformationProductId(), 0x5678); // productId

    // Verify that the PAI, DAC, and CD buffers were copied correctly
    ByteSpan copiedPai = deviceInfo.paiDerBuffer();
    EXPECT_TRUE(copiedPai.data_equal(paiSpan));

    // Verify that the DAC buffer was copied correctly
    ByteSpan copiedDac = deviceInfo.dacDerBuffer();
    EXPECT_TRUE(copiedDac.data_equal(dacSpan));

    // Verify that the CD buffer was copied correctly
    ASSERT_TRUE(deviceInfo.cdBuffer().HasValue());
    ByteSpan copiedCd = deviceInfo.cdBuffer().Value();
    EXPECT_TRUE(copiedCd.data_equal(ByteSpan(cdData)));
}

// Test that setting the DeviceAttestationVerifier to nullptr does not change the current verifier
TEST_F(TestDeviceAttestationVerifier, SetDeviceAttestationVerifierNullptrArgument)
{
    DeviceAttestationVerifier * original = GetDeviceAttestationVerifier();
    SetDeviceAttestationVerifier(nullptr);
    EXPECT_EQ(GetDeviceAttestationVerifier(), original);
}

// Test that setting a revocation delegate works as expected
TEST_F(TestDeviceAttestationVerifier, SetRevocationDelegate)
{
    // Create a minimal AttestationTrustStore with a dummy certificate
    static uint8_t dummyCert[1]       = { 0x00 };
    static const ByteSpan certSpans[] = { ByteSpan(dummyCert) };
    ArrayAttestationTrustStore trustStore(certSpans, 1);
    DefaultDACVerifier verifier(&trustStore);

    // Check the initial state of the revocation delegate
    EXPECT_EQ(verifier.SetRevocationDelegate(nullptr), CHIP_NO_ERROR);
}
