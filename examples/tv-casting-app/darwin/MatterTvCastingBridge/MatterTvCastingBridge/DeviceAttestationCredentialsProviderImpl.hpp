/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/logging/CHIPLogging.h>

#include <Security/Security.h>

class DeviceAttestationCredentialsProviderImpl : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    DeviceAttestationCredentialsProviderImpl(chip::MutableByteSpan * certificationDeclaration,
                                             chip::MutableByteSpan * firmwareInformation,
                                             chip::MutableByteSpan * deviceAttestationCert,
                                             chip::MutableByteSpan * productAttestationIntermediateCert,
                                             SecKeyRef deviceAttestationCertPrivateKeyRef);

    CHIP_ERROR GetCertificationDeclaration(chip::MutableByteSpan & outCertificationDeclaration) override;
    CHIP_ERROR GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation) override;
    CHIP_ERROR GetDeviceAttestationCert(chip::MutableByteSpan & outDeviceAttestationCert) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(chip::MutableByteSpan & outProductAttestationIntermediateCert) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const chip::ByteSpan & messageToSign,
                                            chip::MutableByteSpan & outSignatureBuffer) override;

private:
    chip::MutableByteSpan mCertificationDeclaration;
    chip::MutableByteSpan mFirmwareInformation;
    chip::MutableByteSpan mDeviceAttestationCert;
    chip::MutableByteSpan mProductAttestationIntermediateCert;
    SecKeyRef mDeviceAttestationCertPrivateKeyRef;
};
