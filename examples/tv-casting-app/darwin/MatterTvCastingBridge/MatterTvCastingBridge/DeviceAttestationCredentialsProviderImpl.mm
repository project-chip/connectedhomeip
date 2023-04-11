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
#import <Security/Security.h>

DeviceAttestationCredentialsProviderImpl::DeviceAttestationCredentialsProviderImpl(chip::MutableByteSpan * certificationDeclaration,
    chip::MutableByteSpan * firmwareInformation, chip::MutableByteSpan * deviceAttestationCert,
    chip::MutableByteSpan * productAttestationIntermediateCert, SecKeyRef deviceAttestationCertPrivateKeyRef)
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

    mDeviceAttestationCertPrivateKeyRef = deviceAttestationCertPrivateKeyRef;
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

    CHIP_ERROR result = CHIP_NO_ERROR;
    CFDataRef dataToSign = nil;
    CFDataRef asn1SignatureData = nil;
    uint8_t mAsn1SignatureBytes[256];
    chip::MutableByteSpan asn1SignatureByteSpan = chip::MutableByteSpan(mAsn1SignatureBytes, sizeof(mAsn1SignatureBytes));
    CFErrorRef error = nil;
    size_t signatureLen = 0;

    do {
        dataToSign = CFDataCreate(CFAllocatorGetDefault(), messageToSign.data(), messageToSign.size());
        if (nil == dataToSign) {
            ChipLogError(
                AppServer, "DeviceAttestationCredentialsProviderImpl::SignWithDeviceAttestationKey failed to create buffer");
            result = CHIP_ERROR_NO_MEMORY;
            break;
        }

        asn1SignatureData = SecKeyCreateSignature(
            mDeviceAttestationCertPrivateKeyRef, kSecKeyAlgorithmECDSASignatureMessageX962SHA256, dataToSign, &error);
        if (nil != error || nil == asn1SignatureData) {
            ChipLogError(AppServer,
                "DeviceAttestationCredentialsProviderImpl::SignWithDeviceAttestationKey failed to sign the message. error = %lu",
                CFErrorGetCode(error));
            result = CHIP_ERROR_INVALID_ARGUMENT;
            break;
        }

        signatureLen = CFDataGetLength(asn1SignatureData);

        CFDataGetBytes(asn1SignatureData, CFRangeMake(0, signatureLen), asn1SignatureByteSpan.data());
        asn1SignatureByteSpan.reduce_size(signatureLen);

        CHIP_ERROR conversionError = chip::Crypto::EcdsaAsn1SignatureToRaw(
            32, chip::ByteSpan(asn1SignatureByteSpan.data(), asn1SignatureByteSpan.size()), outSignatureBuffer);
        if (CHIP_NO_ERROR != conversionError) {
            ChipLogError(AppServer,
                "DeviceAttestationCredentialsProviderImpl::SignWithDeviceAttestationKey failed to convert to raw signature.");
            result = conversionError;
            break;
        }
    } while (0);

    if (dataToSign != nil) {
        CFRelease(dataToSign);
    }

    if (asn1SignatureData != nil) {
        CFRelease(asn1SignatureData);
    }

    return result;
}
