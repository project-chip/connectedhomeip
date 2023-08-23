/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#include <dirent.h>
#include <stdio.h>
#include <string>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

static void OnAttestationInformationVerificationCallback(void * context, const DeviceAttestationVerifier::AttestationInfo & info,
                                                         AttestationVerificationResult result)
{
    AttestationVerificationResult * pResult = reinterpret_cast<AttestationVerificationResult *>(context);
    *pResult                                = result;
}

static void TestCommissionerDUTVectors(nlTestSuite * inSuite, void * inContext)
{
    DeviceAttestationVerifier * example_dac_verifier = GetDefaultDACVerifier(GetTestAttestationTrustStore());
    NL_TEST_ASSERT(inSuite, example_dac_verifier != nullptr);

    std::string dirPath("../../../../../credentials/development/commissioner_dut/");
    DIR * dir = opendir(dirPath.c_str());
    while (dir == nullptr && (dirPath.find("../") == 0))
    {
        dirPath = dirPath.substr(3);
        dir     = opendir(dirPath.c_str());
    }
    if (dir == nullptr)
    {
        ChipLogError(Crypto, "Couldn't open folder with Commissioner DUT Test Vectors.");
        return;
    }

    dirent * entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_name[0] == '.')
            continue;

        // Skip this test case because the code below will fail to use secp256k1 to sign attestation data.
        if (strstr(entry->d_name, "struct_dac_sig_curve_secp256k1"))
            continue;

        std::string jsonFilePath = dirPath + std::string(entry->d_name) + std::string("/test_case_vector.json");

        chip::Credentials::Examples::TestHarnessDACProvider dacProvider;
        dacProvider.Init(jsonFilePath.c_str());

        uint8_t attestationChallengeBuf[Crypto::kAES_CCM128_Key_Length];
        MutableByteSpan attestationChallengeSpan(attestationChallengeBuf);
        Crypto::DRBG_get_bytes(attestationChallengeBuf, sizeof(attestationChallengeBuf));

        Crypto::P256ECDSASignature signature;
        MutableByteSpan attestationSignatureSpan{ signature.Bytes(), signature.Capacity() };

        uint8_t certDeclBuf[Credentials::kMaxCMSSignedCDMessage];
        MutableByteSpan certDeclSpan(certDeclBuf);

        uint8_t dacCertBuf[kMaxDERCertLength];
        MutableByteSpan dacCertSpan(dacCertBuf);

        uint8_t paiCertBuf[kMaxDERCertLength];
        MutableByteSpan paiCertSpan(paiCertBuf);

        uint8_t attestationNonceBuf[kAttestationNonceLength];
        MutableByteSpan attestationNonceSpan(attestationNonceBuf);
        Crypto::DRBG_get_bytes(attestationNonceBuf, sizeof(attestationNonceBuf));

        VendorId vid = TestVendor1;
        uint16_t pid = strstr(entry->d_name, "_vidpid_fallback_encoding_") ? 0x00B1 : 0x8000;

        NL_TEST_ASSERT_SUCCESS(inSuite, dacProvider.GetCertificationDeclaration(certDeclSpan));
        NL_TEST_ASSERT_SUCCESS(inSuite, dacProvider.GetDeviceAttestationCert(dacCertSpan));
        NL_TEST_ASSERT_SUCCESS(inSuite, dacProvider.GetProductAttestationIntermediateCert(paiCertSpan));

        size_t attestationElementsLen =
            TLV::EstimateStructOverhead(certDeclSpan.size(), attestationNonceSpan.size(), sizeof(uint64_t) * 8);
        Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
        NL_TEST_ASSERT(inSuite, attestationElements.Alloc(attestationElementsLen + attestationChallengeSpan.size()));
        MutableByteSpan attestationElementsSpan(attestationElements.Get(), attestationElementsLen);

        // Construct attestation elements
        {
            uint32_t timestamp = 0;
            Credentials::DeviceAttestationVendorReservedConstructor emptyVendorReserved(nullptr, 0);
            const ByteSpan kEmptyFirmwareInfo;

            NL_TEST_ASSERT_SUCCESS(inSuite,
                                   Credentials::ConstructAttestationElements(certDeclSpan, attestationNonceSpan, timestamp,
                                                                             kEmptyFirmwareInfo, emptyVendorReserved,
                                                                             attestationElementsSpan));
        }

        // Generate attestation signature
        {
            // Append attestation challenge in the back of the reserved space for the signature
            memcpy(attestationElementsSpan.data() + attestationElementsSpan.size(), attestationChallengeSpan.data(),
                   attestationChallengeSpan.size());
            ByteSpan tbsSpan(attestationElementsSpan.data(), attestationElementsSpan.size() + attestationChallengeSpan.size());

            NL_TEST_ASSERT_SUCCESS(inSuite, dacProvider.SignWithDeviceAttestationKey(tbsSpan, attestationSignatureSpan));
            NL_TEST_ASSERT(inSuite, attestationSignatureSpan.size() == signature.Capacity());
        }

        AttestationVerificationResult attestationResult = AttestationVerificationResult::kNotImplemented;
        Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification>
            attestationInformationVerificationCallback(OnAttestationInformationVerificationCallback, &attestationResult);

        DeviceAttestationVerifier::AttestationInfo info(attestationElementsSpan, attestationChallengeSpan, attestationSignatureSpan,
                                                        paiCertSpan, dacCertSpan, attestationNonceSpan, vid, pid);

        example_dac_verifier->VerifyAttestationInformation(info, &attestationInformationVerificationCallback);

        bool isSuccessCase = dacProvider.IsSuccessCase();
        // The following test vectors are success conditions for an SDK commissioner for the following reasons:
        // struct_cd_device_type_id_mismatch - requires DCL access, which the SDK does not have and is not required
        // struct_cd_security_info_wrong - while devices are required to set this to 0, commissioners are required to ignore it
        //                                 (see 6.3.1)
        //                                 hence this is marked as a failure for devices, but should be a success case for
        //                                 commissioners
        // struct_cd_security_level_wrong - as with security info, commissioners are required to ignore this value (see 6.3.1)
        // struct_cd_version_number_wrong - this value is not meant to be interpreted by commissioners, so errors here should be
        //                                  ignored (6.3.1)
        // struct_cd_cert_id_mismatch - requires DCL access, which the SDK does not have and is not required.
        if (strstr(entry->d_name, "struct_cd_device_type_id_mismatch") || strstr(entry->d_name, "struct_cd_security_info_wrong") ||
            strstr(entry->d_name, "struct_cd_security_level_wrong") || strstr(entry->d_name, "struct_cd_version_number_wrong") ||
            strstr(entry->d_name, "struct_cd_cert_id_mismatch"))
        {
            isSuccessCase = true;
        }

        if (isSuccessCase)
        {
            NL_TEST_ASSERT(inSuite, attestationResult == AttestationVerificationResult::kSuccess);
        }
        else
        {
            NL_TEST_ASSERT(inSuite, attestationResult != AttestationVerificationResult::kSuccess);
        }
    }
    closedir(dir);
}

/**
 *  Set up the test suite.
 */
int TestCommissionerDUT_Setup(void * inContext)
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
int TestCommissionerDUT_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Device Attestation Credentials Vectors", TestCommissionerDUTVectors),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestCommissionerDUT()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Device Attestation Credentials Test Vectors",
        &sTests[0],
        TestCommissionerDUT_Setup,
        TestCommissionerDUT_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCommissionerDUT);
