/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MCDeviceAttestationCredentialsProvider.h"

#import "MCDeviceAttestationCredentials.h"

#include "lib/support/logging/CHIPLogging.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#import <Foundation/Foundation.h>

namespace matter {
namespace casting {
    namespace support {

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::Initialize(id<MCDataSource> dataSource)
        {
            VerifyOrReturnError(dataSource != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(mDataSource == nullptr, CHIP_ERROR_INCORRECT_STATE);
            mDataSource = dataSource;
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetCertificationDeclaration(
            chip::MutableByteSpan & outCertificationDeclaration)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);
            MCDeviceAttestationCredentials * dac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MCDeviceAttestationCredentialsProvider.Initialize()"];

            if (dac.certificationDeclaration != nullptr && dac.certificationDeclaration.length > 0) {
                if (outCertificationDeclaration.size() >= dac.certificationDeclaration.length) {
                    memcpy(outCertificationDeclaration.data(), dac.certificationDeclaration.bytes,
                        dac.certificationDeclaration.length);
                    outCertificationDeclaration.reduce_size(dac.certificationDeclaration.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);
            MCDeviceAttestationCredentials * dac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MCDeviceAttestationCredentialsProvider.Initialize()"];

            if (dac.firmwareInformation != nullptr && dac.firmwareInformation.length > 0) {
                if (outFirmwareInformation.size() >= dac.firmwareInformation.length) {
                    memcpy(outFirmwareInformation.data(), dac.firmwareInformation.bytes, dac.firmwareInformation.length);
                    outFirmwareInformation.reduce_size(dac.firmwareInformation.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetDeviceAttestationCert(
            chip::MutableByteSpan & outDeviceAttestationCert)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);
            MCDeviceAttestationCredentials * dac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MCDeviceAttestationCredentialsProvider.Initialize()"];

            if (dac.deviceAttestationCert != nullptr && dac.deviceAttestationCert.length > 0) {
                if (outDeviceAttestationCert.size() >= dac.deviceAttestationCert.length) {
                    memcpy(outDeviceAttestationCert.data(), dac.deviceAttestationCert.bytes, dac.deviceAttestationCert.length);
                    outDeviceAttestationCert.reduce_size(dac.deviceAttestationCert.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetProductAttestationIntermediateCert(
            chip::MutableByteSpan & outProductAttestationIntermediateCert)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);
            MCDeviceAttestationCredentials * dac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MCDeviceAttestationCredentialsProvider.Initialize()"];

            if (dac.productAttestationIntermediateCert != nullptr && dac.productAttestationIntermediateCert.length > 0) {
                if (outProductAttestationIntermediateCert.size() >= dac.productAttestationIntermediateCert.length) {
                    memcpy(outProductAttestationIntermediateCert.data(), dac.productAttestationIntermediateCert.bytes,
                        dac.productAttestationIntermediateCert.length);
                    outProductAttestationIntermediateCert.reduce_size(dac.productAttestationIntermediateCert.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::SignWithDeviceAttestationKey(
            const chip::ByteSpan & messageToSign, chip::MutableByteSpan & outSignatureBuffer)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);

            NSData * csrData = [NSData dataWithBytes:messageToSign.data() length:messageToSign.size()];
            __block NSData * signedData = [NSData dataWithBytes:outSignatureBuffer.data() length:outSignatureBuffer.size()];
            __block MatterError * err = nil;
            dispatch_sync(mDataSource.clientQueue, ^{
                err = [mDataSource castingApp:@"MCDeviceAttestationCredentialsProvider.SignWithDeviceAttestationKey()"
                    didReceiveRequestToSignCertificateRequest:csrData
                                              outRawSignature:&signedData];
            });

            VerifyOrReturnValue(MATTER_NO_ERROR == err, CHIP_ERROR(chip::ChipError::SdkPart::kCore, err.code), ChipLogError(AppServer, "castingApp::SignCertificateRequest failed"));

            if (signedData != nil && outSignatureBuffer.size() >= signedData.length) {
                memcpy(outSignatureBuffer.data(), signedData.bytes, signedData.length);
                outSignatureBuffer.reduce_size(signedData.length);
            } else {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }

            return CHIP_NO_ERROR;
        }

    }; // namespace support
}; // namespace casting
}; // namespace matter
