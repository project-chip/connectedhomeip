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

/**
 *    @file
 *      Unit tests for Example Device Attestation Credentials Accessor
 *
 */

#include <crypto/CHIPCryptoPAL.h>

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsAccess.h>
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

} // namespace

static void TestDACAccessorsExample_Accessors(nlTestSuite * inSuite, void * inContext)
{
    uint8_t der_cert_buf[kMaxDERCertLength];
    MutableByteSpan der_cert_span(der_cert_buf);

    // Make sure default accessor exists and is not implemented on at least one method
    DeviceAttestationCredentialsAccessor * defaultAccessor = GetDeviceAttestationCredentialsAccessor();
    NL_TEST_ASSERT(inSuite, defaultAccessor != nullptr);

    CHIP_ERROR err = defaultAccessor->GetDeviceAttestationCert(der_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NOT_IMPLEMENTED);

    // Replace default accessor with example accessor
    DeviceAttestationCredentialsAccessor * exampleDacAccessor = Examples::GetExampleDACAccessor();
    NL_TEST_ASSERT(inSuite, exampleDacAccessor != nullptr);
    NL_TEST_ASSERT(inSuite, defaultAccessor != exampleDacAccessor);

    SetDeviceAttestationCredentialsAccessor(exampleDacAccessor);
    defaultAccessor = GetDeviceAttestationCredentialsAccessor();
    NL_TEST_ASSERT(inSuite, defaultAccessor == exampleDacAccessor);

    // Make sure DAC is what we expect
    memset(der_cert_span.data(), 0, der_cert_span.size());
    err = exampleDacAccessor->GetDeviceAttestationCert(der_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Extract public key from DAC, prior to signature verification
    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(der_cert_span, dac_public_key);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dac_public_key.Length() == sizeof(kExpectedDacPublicKey));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey, sizeof(kExpectedDacPublicKey)));

    // TODO: Test all other accessors
}

static void TestDACAccessorsExample_Signature(nlTestSuite * inSuite, void * inContext)
{
    constexpr uint8_t kExampleDigest[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12,
                                           0x13, 0x14, 0x15, 0x16, 0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                                           0x26, 0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };

    DeviceAttestationCredentialsAccessor * exampleDacAccessor = Examples::GetExampleDACAccessor();
    NL_TEST_ASSERT(inSuite, exampleDacAccessor != nullptr);

    // Sign using the example attestation private key
    P256ECDSASignature da_signature;
    MutableByteSpan out_sig_span(da_signature.Bytes(), da_signature.Capacity());
    CHIP_ERROR err = exampleDacAccessor->SignWithDeviceAttestationKey(ByteSpan{ kExampleDigest }, out_sig_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    printf("err1: %s\n", err.AsString());

    NL_TEST_ASSERT(inSuite, out_sig_span.size() == kP256_ECDSA_Signature_Length_Raw);
    da_signature.SetLength(out_sig_span.size());

    // Get DAC from the accessor
    uint8_t dac_cert_buf[kMaxDERCertLength];
    MutableByteSpan dac_cert_span(dac_cert_buf);

    memset(dac_cert_span.data(), 0, dac_cert_span.size());
    err = exampleDacAccessor->GetDeviceAttestationCert(dac_cert_span);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Extract public key from DAC, prior to signature verification
    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(dac_cert_span, dac_public_key);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dac_public_key.Length() == sizeof(kExpectedDacPublicKey));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey, sizeof(kExpectedDacPublicKey)));

    // Verify round trip signature
    err = dac_public_key.ECDSA_validate_hash_signature(&kExampleDigest[0], sizeof(kExampleDigest), da_signature);
    printf("err1: %s\n", err.AsString());
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Example Device Attestation Credentials Accessors", TestDACAccessorsExample_Accessors),
    NL_TEST_DEF("Test Example Device Attestation Signature", TestDACAccessorsExample_Signature),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestDACAccessors()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Device Attestation Credentials Accessors",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDACAccessors);
