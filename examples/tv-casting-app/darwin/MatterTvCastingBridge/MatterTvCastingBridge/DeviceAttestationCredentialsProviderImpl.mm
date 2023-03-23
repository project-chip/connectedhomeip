/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include "DeviceAttestationCredentialsProviderImpl.hpp"

#import <Foundation/Foundation.h>

DeviceAttestationCredentialsProviderImpl::DeviceAttestationCredentialsProviderImpl(chip::MutableByteSpan * certificationDeclaration,
    chip::MutableByteSpan * firmwareInformation, chip::MutableByteSpan * deviceAttestationCert,
    chip::MutableByteSpan * productAttestationIntermediateCert, chip::MutableByteSpan * deviceAttestationCertPrivateKey,
    chip::MutableByteSpan * deviceAttestationCertPublicKeyKey)
{
    if (certificationDeclaration != nullptr) {
        mCertificationDeclaration
            = chip::MutableByteSpan(new uint8_t[certificationDeclaration->size()], certificationDeclaration->size());
        memcpy(mCertificationDeclaration.data(), certificationDeclaration->data(), certificationDeclaration->size());
    }

    if (firmwareInformation != nullptr) {
        mFirmwareInformation = chip::MutableByteSpan(new uint8_t[firmwareInformation->size()], firmwareInformation->size());
        memcpy(mFirmwareInformation.data(), firmwareInformation->data(), firmwareInformation->size());
    }

    if (deviceAttestationCert != nullptr) {
        mDeviceAttestationCert = chip::MutableByteSpan(new uint8_t[deviceAttestationCert->size()], deviceAttestationCert->size());
        memcpy(mDeviceAttestationCert.data(), deviceAttestationCert->data(), deviceAttestationCert->size());
    }

    if (productAttestationIntermediateCert != nullptr) {
        mProductAttestationIntermediateCert = chip::MutableByteSpan(
            new uint8_t[productAttestationIntermediateCert->size()], productAttestationIntermediateCert->size());
        memcpy(mProductAttestationIntermediateCert.data(), productAttestationIntermediateCert->data(),
            productAttestationIntermediateCert->size());
    }

    if (deviceAttestationCertPrivateKey != nullptr) {
        mDeviceAttestationCertPrivateKey
            = chip::MutableByteSpan(new uint8_t[deviceAttestationCertPrivateKey->size()], deviceAttestationCertPrivateKey->size());
        memcpy(mDeviceAttestationCertPrivateKey.data(), deviceAttestationCertPrivateKey->data(),
            deviceAttestationCertPrivateKey->size());
    }

    if (deviceAttestationCertPublicKeyKey != nullptr) {
        mDeviceAttestationCertPublicKeyKey = chip::MutableByteSpan(
            new uint8_t[deviceAttestationCertPublicKeyKey->size()], deviceAttestationCertPublicKeyKey->size());
        memcpy(mDeviceAttestationCertPublicKeyKey.data(), deviceAttestationCertPublicKeyKey->data(),
            deviceAttestationCertPublicKeyKey->size());
    }
}

CHIP_ERROR DeviceAttestationCredentialsProviderImpl::GetCertificationDeclaration(
    chip::MutableByteSpan & outCertificationDeclaration)
{
    if (mCertificationDeclaration.size() > 0) {
        if (outCertificationDeclaration.size() >= mCertificationDeclaration.size()) {
            memcpy(outCertificationDeclaration.data(), mCertificationDeclaration.data(), mCertificationDeclaration.size());
            outCertificationDeclaration.reduce_size(mCertificationDeclaration.size());
        } else {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceAttestationCredentialsProviderImpl::GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation)
{
    if (mFirmwareInformation.size() > 0) {
        if (outFirmwareInformation.size() >= mFirmwareInformation.size()) {
            memcpy(outFirmwareInformation.data(), mFirmwareInformation.data(), mFirmwareInformation.size());
            outFirmwareInformation.reduce_size(mFirmwareInformation.size());
        } else {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceAttestationCredentialsProviderImpl::GetDeviceAttestationCert(chip::MutableByteSpan & outDeviceAttestationCert)
{
    if (mDeviceAttestationCert.size() > 0) {
        if (outDeviceAttestationCert.size() >= mDeviceAttestationCert.size()) {
            memcpy(outDeviceAttestationCert.data(), mDeviceAttestationCert.data(), mDeviceAttestationCert.size());
            outDeviceAttestationCert.reduce_size(mDeviceAttestationCert.size());
        } else {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceAttestationCredentialsProviderImpl::GetProductAttestationIntermediateCert(
    chip::MutableByteSpan & outProductAttestationIntermediateCert)
{
    if (mProductAttestationIntermediateCert.size() > 0) {
        if (outProductAttestationIntermediateCert.size() >= mProductAttestationIntermediateCert.size()) {
            memcpy(outProductAttestationIntermediateCert.data(), mProductAttestationIntermediateCert.data(),
                mProductAttestationIntermediateCert.size());
            outProductAttestationIntermediateCert.reduce_size(mProductAttestationIntermediateCert.size());
        } else {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceAttestationCredentialsProviderImpl::SignWithDeviceAttestationKey(
    const chip::ByteSpan & messageToSign, chip::MutableByteSpan & outSignatureBuffer)
{
    ChipLogProgress(AppServer, "DeviceAttestationCredentialsProviderImpl::SignWithDeviceAttestationKey called");
    chip::Crypto::P256ECDSASignature signature;
    chip::Crypto::P256Keypair keypair;

    VerifyOrReturnError(IsSpanUsable(outSignatureBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignatureBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    ReturnErrorOnFailure(LoadKeypairFromRaw(mDeviceAttestationCertPrivateKey, mDeviceAttestationCertPublicKeyKey, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(chip::ByteSpan { signature.ConstBytes(), signature.Length() }, outSignatureBuffer);
}
