/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/**
 *    @file
 *      Implementation of JNI bridge for Device Attestation
 *
 */
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/SafeInt.h>

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <jni.h>

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_DeviceAttestation_##METHOD_NAME

void ThrowException(JNIEnv * env, CHIP_ERROR err);

JNI_METHOD(jbyteArray, extractSkidFromPaaCert)
(JNIEnv * env, jclass clazz, jbyteArray paaCert)
{
    uint32_t allocatedCertLength = chip::Credentials::kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outJbytes = nullptr;
    chip::JniByteArray paaCertBytes(env, paaCert);

    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);
    {
        chip::MutableByteSpan outBytes(outBuf.Get(), allocatedCertLength);

        err = chip::Crypto::ExtractSKIDFromX509Cert(paaCertBytes.byteSpan(), outBytes);
        SuccessOrExit(err);

        VerifyOrExit(chip::CanCastTo<uint32_t>(outBytes.size()), err = CHIP_ERROR_INTERNAL);

        err =
            chip::JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), static_cast<jsize>(outBytes.size()), outJbytes);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to extract skid frome X509 cert. Err = %" CHIP_ERROR_FORMAT, err.Format());
        ThrowException(env, err);
    }

    return outJbytes;
}

JNI_METHOD(jbyteArray, extractAkidFromPaiCert)
(JNIEnv * env, jclass clazz, jbyteArray paiCert)
{
    uint32_t allocatedCertLength = chip::Credentials::kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outJbytes = nullptr;
    chip::JniByteArray paiCertBytes(env, paiCert);

    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);
    {
        chip::MutableByteSpan outBytes(outBuf.Get(), allocatedCertLength);

        err = chip::Crypto::ExtractAKIDFromX509Cert(paiCertBytes.byteSpan(), outBytes);
        SuccessOrExit(err);

        VerifyOrExit(chip::CanCastTo<uint32_t>(outBytes.size()), err = CHIP_ERROR_INTERNAL);

        err =
            chip::JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), static_cast<jsize>(outBytes.size()), outJbytes);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to extract akid frome X509 cert. Err = %" CHIP_ERROR_FORMAT, err.Format());
        ThrowException(env, err);
    }

    return outJbytes;
}

JNI_METHOD(void, validateAttestationInfo)
(JNIEnv * env, jclass clazz, jint vendorId, jint productId, jbyteArray paaCert, jbyteArray paiCert, jbyteArray dacCert,
 jbyteArray attestationElements)
{
    chip::Credentials::AttestationVerificationResult attestationError = chip::Credentials::AttestationVerificationResult::kSuccess;
    CHIP_ERROR err                                                    = CHIP_NO_ERROR;

    VerifyOrExit(paaCert != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(paiCert != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(dacCert != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    {
        chip::JniByteArray paaCertBytes(env, paaCert);
        chip::JniByteArray paiCertBytes(env, paiCert);
        chip::JniByteArray dacCertBytes(env, dacCert);

        chip::Crypto::AttestationCertVidPid paaVidPid;
        chip::Crypto::AttestationCertVidPid paiVidPid;
        chip::Crypto::AttestationCertVidPid dacVidPid;

        uint8_t skidBuf[chip::Crypto::kAuthorityKeyIdentifierLength];
        chip::MutableByteSpan paaSKID(skidBuf);

        chip::Crypto::CertificateChainValidationResult chainValidationResult;

        err = chip::Crypto::VerifyAttestationCertificateFormat(paiCertBytes.byteSpan(), chip::Crypto::AttestationCertType::kPAI);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(Controller, "Verify PAI Attestation Cert format Error! : %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::Crypto::VerifyAttestationCertificateFormat(dacCertBytes.byteSpan(), chip::Crypto::AttestationCertType::kDAC);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(Controller, "Verify DAC Attestation Cert format Error! : %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::Crypto::ExtractVIDPIDFromX509Cert(paaCertBytes.byteSpan(), paaVidPid);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(Controller, "Extract VID, PID from PAA Error! : %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::Crypto::ExtractVIDPIDFromX509Cert(paiCertBytes.byteSpan(), paiVidPid);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(Controller, "Extract VID, PID from PAI Error! : %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::Crypto::ExtractVIDPIDFromX509Cert(dacCertBytes.byteSpan(), dacVidPid);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(Controller, "Extract VID, PID from DAC Error! : %" CHIP_ERROR_FORMAT, err.Format()));

        if (paaVidPid.mVendorId.HasValue())
        {
            VerifyOrExit(paaVidPid.mVendorId == paiVidPid.mVendorId,
                         attestationError = chip::Credentials::AttestationVerificationResult::kPaiVendorIdMismatch);
        }

        VerifyOrExit(!paaVidPid.mProductId.HasValue(),
                     attestationError = chip::Credentials::AttestationVerificationResult::kPaaFormatInvalid);

        err = chip::Crypto::ValidateCertificateChain(
            paaCertBytes.byteSpan().data(), paaCertBytes.byteSpan().size(), paiCertBytes.byteSpan().data(),
            paiCertBytes.byteSpan().size(), dacCertBytes.byteSpan().data(), dacCertBytes.byteSpan().size(), chainValidationResult);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     attestationError = static_cast<chip::Credentials::AttestationVerificationResult>(chainValidationResult));

        err = chip::Crypto::ExtractSKIDFromX509Cert(paaCertBytes.byteSpan(), paaSKID);
        VerifyOrExit(err == CHIP_NO_ERROR, attestationError = chip::Credentials::AttestationVerificationResult::kPaaFormatInvalid);
        VerifyOrExit(paaSKID.size() == chip::Crypto::kAuthorityKeyIdentifierLength,
                     attestationError = chip::Credentials::AttestationVerificationResult::kPaaFormatInvalid);
    }

    {
        VerifyOrExit(attestationElements != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        chip::JniByteArray attestationElementsBytes(env, attestationElements);

        chip::ByteSpan certificationDeclarationSpan;
        chip::ByteSpan attestationNonceSpan;
        uint32_t timestampDeconstructed;
        chip::ByteSpan firmwareInfoSpan;
        chip::Credentials::DeviceAttestationVendorReservedDeconstructor vendorReserved;
        chip::ByteSpan certificationDeclarationPayload;

        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        chip::Credentials::DeviceAttestationVerifier * dacVertifier = chip::Credentials::GetDefaultDACVerifier(testingRootStore);

        err = chip::Credentials::DeconstructAttestationElements(attestationElementsBytes.byteSpan(), certificationDeclarationSpan,
                                                                attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan,
                                                                vendorReserved);

        VerifyOrExit(err == CHIP_NO_ERROR,
                     attestationError = chip::Credentials::AttestationVerificationResult::kAttestationElementsMalformed);

        attestationError =
            dacVertifier->ValidateCertificationDeclarationSignature(certificationDeclarationSpan, certificationDeclarationPayload);
        VerifyOrExit(attestationError == chip::Credentials::AttestationVerificationResult::kSuccess, err = CHIP_ERROR_INTERNAL);
    }

exit:
    if (err == CHIP_NO_ERROR && attestationError != chip::Credentials::AttestationVerificationResult::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to validate Attestation Info. Err = %u, %" CHIP_ERROR_FORMAT,
                     static_cast<uint16_t>(attestationError), err.Format());
        ThrowException(env, err);
    }
}

void ThrowException(JNIEnv * env, CHIP_ERROR err)
{
    jclass controllerExceptionCls;
    CHIP_ERROR classRefErr = chip::JniReferences::GetInstance().GetLocalClassRef(
        env, "chip/devicecontroller/ChipDeviceControllerException", controllerExceptionCls);

    if (classRefErr != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to GetLocalClassRef: %" CHIP_ERROR_FORMAT, classRefErr.Format());
        return;
    }

    chip::JniReferences::GetInstance().ThrowError(env, controllerExceptionCls, err);
}
