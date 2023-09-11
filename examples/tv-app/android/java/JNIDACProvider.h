/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lib/support/logging/CHIPLogging.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <jni.h>

class JNIDACProvider : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    JNIDACProvider(jobject provider);
    CHIP_ERROR GetCertificationDeclaration(chip::MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(chip::MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(chip::MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(chip::MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const chip::ByteSpan & message_to_sign,
                                            chip::MutableByteSpan & out_signature_buffer) override;

private:
    CHIP_ERROR GetJavaByteByMethod(jmethodID method, chip::MutableByteSpan & out_buffer);
    CHIP_ERROR GetJavaByteByMethod(jmethodID method, const chip::ByteSpan & in_buffer, chip::MutableByteSpan & out_buffer);
    jobject mJNIDACProviderObject                          = nullptr;
    jmethodID mGetCertificationDeclarationMethod           = nullptr;
    jmethodID mGetFirmwareInformationMethod                = nullptr;
    jmethodID mGetDeviceAttestationCertMethod              = nullptr;
    jmethodID mGetProductAttestationIntermediateCertMethod = nullptr;
    jmethodID mSignWithDeviceAttestationKeyMethod          = nullptr;
};
