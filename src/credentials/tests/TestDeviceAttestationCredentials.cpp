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
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/core/CHIPError.h>
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

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Example Device Attestation Credentials Providers", TestDACProvidersExample_Providers),
    NL_TEST_DEF("Test Example Device Attestation Signature", TestDACProvidersExample_Signature),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestDACProviders()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Device Attestation Credentials Providers",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDACProviders);
