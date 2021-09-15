/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <credentials/examples/DeviceAttestationVerifierExample.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

namespace {

constexpr uint8_t kExpectedDacPublicKey[] = { 0x04, 0x7a, 0x84, 0x58, 0xaf, 0xbb, 0x9b, 0xcd, 0x15, 0xe1, 0x9a, 0xdc, 0xd2,
                                              0x66, 0xf6, 0x6c, 0x9c, 0x2f, 0x60, 0x7c, 0x74, 0x74, 0x7a, 0x35, 0xf8, 0x0f,
                                              0x37, 0xe1, 0x18, 0x13, 0x3f, 0x80, 0xf1, 0x76, 0x01, 0x13, 0x27, 0x8f, 0x91,
                                              0xf1, 0x5a, 0xa0, 0xf7, 0xf8, 0x79, 0x32, 0x09, 0x4f, 0xe6, 0x9f, 0xb7, 0x28,
                                              0x68, 0xa8, 0x1e, 0x26, 0x97, 0x9b, 0x36, 0x8b, 0x33, 0xb5, 0x54, 0x31, 0x03 };

constexpr uint8_t kExpectedPaiPublicKey[] = { 0x04, 0xca, 0x73, 0xce, 0x46, 0x41, 0xbf, 0x08, 0x3b, 0x4a, 0x33, 0x8d, 0xa0,
                                              0x43, 0x1a, 0x0a, 0x32, 0x30, 0x7f, 0x66, 0xd1, 0x60, 0x57, 0x4b, 0x66, 0x12,
                                              0x2f, 0x25, 0x06, 0xcf, 0x6a, 0xd3, 0x70, 0xe3, 0x7f, 0x65, 0xd6, 0x34, 0x7a,
                                              0xe7, 0x97, 0xa1, 0x97, 0x26, 0x50, 0x50, 0x97, 0x6d, 0x34, 0xac, 0x7b, 0x63,
                                              0x7b, 0x3b, 0xda, 0x0b, 0x5b, 0xd8, 0x43, 0xed, 0x8e, 0x5d, 0x5e, 0x9b, 0xf2 };

} // namespace

static void TestDACProvidersExample_Providers(nlTestSuite * inSuite, void * inContext)
{
    uint8_t der_cert_buf[kMaxDERCertLength];
    MutableByteSpan der_cert_span(der_cert_buf);

    // Make sure default provider exists and is not implemented on at least one method
    DeviceAttestationCredentialsProvider * default_provider = GetDeviceAttestationCredentialsProvider();
    NL_TEST_ASSERT(inSuite, default_provider != nullptr);

    CHIP_ERROR err = default_provider->GetDeviceAttestationCert(der_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_IMPLEMENTED);

    // Replace default provider with example provider
    DeviceAttestationCredentialsProvider * example_dac_provider = Examples::GetExampleDACProvider();
    NL_TEST_ASSERT(inSuite, example_dac_provider != nullptr);
    NL_TEST_ASSERT(inSuite, default_provider != example_dac_provider);

    SetDeviceAttestationCredentialsProvider(example_dac_provider);
    default_provider = GetDeviceAttestationCredentialsProvider();
    NL_TEST_ASSERT(inSuite, default_provider == example_dac_provider);

    // Make sure DAC is what we expect, by validating public key
    memset(der_cert_span.data(), 0, der_cert_span.size());
    err = example_dac_provider->GetDeviceAttestationCert(der_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(der_cert_span, dac_public_key);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dac_public_key.Length() == sizeof(kExpectedDacPublicKey));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey, sizeof(kExpectedDacPublicKey)));

    // Make sure PAI is what we expect, by validating public key
    der_cert_span = MutableByteSpan{ der_cert_span };
    memset(der_cert_span.data(), 0, der_cert_span.size());
    err = example_dac_provider->GetProductAttestationIntermediateCert(der_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    P256PublicKey pai_public_key;
    err = ExtractPubkeyFromX509Cert(der_cert_span, pai_public_key);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pai_public_key.Length() == sizeof(kExpectedPaiPublicKey));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(pai_public_key.ConstBytes(), kExpectedPaiPublicKey, sizeof(kExpectedPaiPublicKey)));

    // Check for CD presence
    uint8_t other_data_buf[256];
    MutableByteSpan other_data_span(other_data_buf);
    memset(other_data_span.data(), 0, other_data_span.size());

    err = example_dac_provider->GetCertificationDeclaration(other_data_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, other_data_span.size() > 0);
    NL_TEST_ASSERT(inSuite, other_data_span.data()[0] != 0);

    // Check for firmware information presence
    other_data_span = MutableByteSpan{ other_data_buf };
    memset(other_data_span.data(), 0, other_data_span.size());

    err = example_dac_provider->GetFirmwareInformation(other_data_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, other_data_span.size() == 0);
}

static void TestDACProvidersExample_Signature(nlTestSuite * inSuite, void * inContext)
{
    constexpr uint8_t kExampleDigest[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12,
                                           0x13, 0x14, 0x15, 0x16, 0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                                           0x26, 0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };

    DeviceAttestationCredentialsProvider * example_dac_provider = Examples::GetExampleDACProvider();
    NL_TEST_ASSERT(inSuite, example_dac_provider != nullptr);

    // Sign using the example attestation private key
    P256ECDSASignature da_signature;
    MutableByteSpan out_sig_span(da_signature.Bytes(), da_signature.Capacity());
    CHIP_ERROR err = example_dac_provider->SignWithDeviceAttestationKey(ByteSpan{ kExampleDigest }, out_sig_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, out_sig_span.size() == kP256_ECDSA_Signature_Length_Raw);
    da_signature.SetLength(out_sig_span.size());

    // Get DAC from the provider
    uint8_t dac_cert_buf[kMaxDERCertLength];
    MutableByteSpan dac_cert_span(dac_cert_buf);

    memset(dac_cert_span.data(), 0, dac_cert_span.size());
    err = example_dac_provider->GetDeviceAttestationCert(dac_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Extract public key from DAC, prior to signature verification
    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(dac_cert_span, dac_public_key);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dac_public_key.Length() == sizeof(kExpectedDacPublicKey));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey, sizeof(kExpectedDacPublicKey)));

    // Verify round trip signature
    err = dac_public_key.ECDSA_validate_hash_signature(&kExampleDigest[0], sizeof(kExampleDigest), da_signature);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

static void TestDACVerifierExample_AttestationInfoVerification(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t attestationElementsTestVector[] = {
        0x15, 0x30, 0x01, 0x70, 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0, 0x58,
        0x1d, 0x15, 0x25, 0x01, 0xf1, 0xff, 0x25, 0x02, 0x01, 0x80, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05,
        0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x1f, 0x37, 0x2c, 0xaf, 0x6e, 0x78, 0x4d, 0x78,
        0x55, 0xd7, 0x81, 0x17, 0xeb, 0xef, 0x1c, 0x12, 0x98, 0x0f, 0xa6, 0xc2, 0x25, 0xc4, 0xad, 0x2e, 0x68, 0x44, 0x3d, 0x50,
        0x9e, 0xff, 0x20, 0xa7, 0x22, 0x7d, 0xe7, 0x0e, 0x80, 0xcb, 0x9b, 0x8b, 0xa0, 0xd4, 0x3f, 0x91, 0xba, 0xe1, 0xdd, 0xfb,
        0x3d, 0x59, 0xa7, 0x34, 0xb5, 0x37, 0xea, 0x41, 0x42, 0x0e, 0xb3, 0xe8, 0x6b, 0x3e, 0xbc, 0xbd, 0x30, 0x02, 0x20, 0xe0,
        0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d,
        0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x26, 0x03, 0xfd, 0xc6, 0x5b, 0x28, 0x30, 0x05, 0x17,
        0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76,
        0x65, 0x64, 0x31, 0x30, 0x07, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65,
        0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18
    };
    uint8_t attestationChallengeTestVector[] = { 0x7a, 0x49, 0x53, 0x05, 0xd0, 0x77, 0x79, 0xa4,
                                                 0x94, 0xdd, 0x39, 0xa0, 0x85, 0x1b, 0x66, 0x0d };
    uint8_t attestationSignatureTestVector[] = { 0x79, 0x82, 0x53, 0x5d, 0x24, 0xcf, 0xe1, 0x4a, 0x71, 0xab, 0x04, 0x24, 0xcf,
                                                 0x0b, 0xac, 0xf1, 0xe3, 0x45, 0x48, 0x7e, 0xd5, 0x0f, 0x1a, 0xc0, 0xbc, 0x25,
                                                 0x9e, 0xcc, 0xfb, 0x39, 0x08, 0x1e, 0x82, 0x0f, 0x43, 0x1c, 0x0d, 0x91, 0x49,
                                                 0x7a, 0xd1, 0xb5, 0x00, 0xdc, 0x46, 0x7d, 0x7b, 0xc9, 0xf8, 0x68, 0x58, 0x7f,
                                                 0xf8, 0x43, 0xee, 0x78, 0x15, 0xf4, 0x88, 0x98, 0x31, 0x30, 0xc6, 0x9d };
    uint8_t attestationNonceTestVector[]     = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d,
                                             0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a,
                                             0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };

    // Make sure default verifier exists and is not implemented on at least one method
    DeviceAttestationVerifier * default_verifier = GetDeviceAttestationVerifier();
    NL_TEST_ASSERT(inSuite, default_verifier != nullptr);

    AttestationVerificationResult attestation_result =
        default_verifier->VerifyAttestationInformation(ByteSpan(), ByteSpan(), ByteSpan(), ByteSpan(), ByteSpan(), ByteSpan());
    NL_TEST_ASSERT(inSuite, attestation_result == AttestationVerificationResult::kNotImplemented);

    // Replace default verifier with example verifier
    DeviceAttestationVerifier * example_dac_verifier = Examples::GetExampleDACVerifier();
    NL_TEST_ASSERT(inSuite, example_dac_verifier != nullptr);
    NL_TEST_ASSERT(inSuite, default_verifier != example_dac_verifier);

    SetDeviceAttestationVerifier(example_dac_verifier);
    default_verifier = GetDeviceAttestationVerifier();
    NL_TEST_ASSERT(inSuite, default_verifier == example_dac_verifier);

    DeviceAttestationCredentialsProvider * example_dac_provider = Examples::GetExampleDACProvider();
    NL_TEST_ASSERT(inSuite, example_dac_provider != nullptr);

    SetDeviceAttestationCredentialsProvider(example_dac_provider);
    DeviceAttestationCredentialsProvider * default_provider = GetDeviceAttestationCredentialsProvider();
    NL_TEST_ASSERT(inSuite, default_provider == example_dac_provider);

    uint8_t dac[kMaxDERCertLength];
    uint8_t pai[kMaxDERCertLength];
    MutableByteSpan dac_span(dac);
    MutableByteSpan pai_span(pai);

    err = default_provider->GetDeviceAttestationCert(dac_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = default_provider->GetProductAttestationIntermediateCert(pai_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    attestation_result = default_verifier->VerifyAttestationInformation(
        ByteSpan(attestationElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(attestationSignatureTestVector),
        pai_span, dac_span, ByteSpan(attestationNonceTestVector));
    NL_TEST_ASSERT(inSuite, attestation_result == AttestationVerificationResult::kSuccess);
}

/**
 *  Set up the test suite.
 */
int TestDeviceAttestation_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();

    if (error != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestDeviceAttestation_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Example Device Attestation Credentials Providers", TestDACProvidersExample_Providers),
    NL_TEST_DEF("Test Example Device Attestation Signature", TestDACProvidersExample_Signature),
    NL_TEST_DEF("Test Example Device Attestation Information Verification", TestDACVerifierExample_AttestationInfoVerification),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestDeviceAttestation()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Device Attestation Credentials",
        &sTests[0],
        TestDeviceAttestation_Setup,
        TestDeviceAttestation_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDeviceAttestation);
