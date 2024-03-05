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
            mDac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MCDeviceAttestationCredentialsProvider.Initialize()"];

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetCertificationDeclaration(
            chip::MutableByteSpan & outCertificationDeclaration)
        {
            VerifyOrReturnError(mDac != nullptr, CHIP_ERROR_INCORRECT_STATE);

            if (mDac.certificationDeclaration != nullptr && mDac.certificationDeclaration.length > 0) {
                if (outCertificationDeclaration.size() >= mDac.certificationDeclaration.length) {
                    memcpy(outCertificationDeclaration.data(), mDac.certificationDeclaration.bytes,
                        mDac.certificationDeclaration.length);
                    outCertificationDeclaration.reduce_size(mDac.certificationDeclaration.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation)
        {
            VerifyOrReturnError(mDac != nullptr, CHIP_ERROR_INCORRECT_STATE);

            if (mDac.firmwareInformation != nullptr && mDac.firmwareInformation.length > 0) {
                if (outFirmwareInformation.size() >= mDac.firmwareInformation.length) {
                    memcpy(outFirmwareInformation.data(), mDac.firmwareInformation.bytes, mDac.firmwareInformation.length);
                    outFirmwareInformation.reduce_size(mDac.firmwareInformation.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetDeviceAttestationCert(
            chip::MutableByteSpan & outDeviceAttestationCert)
        {
            VerifyOrReturnError(mDac != nullptr, CHIP_ERROR_INCORRECT_STATE);

            if (mDac.deviceAttestationCert != nullptr && mDac.deviceAttestationCert.length > 0) {
                if (outDeviceAttestationCert.size() >= mDac.deviceAttestationCert.length) {
                    memcpy(outDeviceAttestationCert.data(), mDac.deviceAttestationCert.bytes, mDac.deviceAttestationCert.length);
                    outDeviceAttestationCert.reduce_size(mDac.deviceAttestationCert.length);
                } else {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            }
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceAttestationCredentialsProvider::GetProductAttestationIntermediateCert(
            chip::MutableByteSpan & outProductAttestationIntermediateCert)
        {
            VerifyOrReturnError(mDac != nullptr, CHIP_ERROR_INCORRECT_STATE);

            if (mDac.productAttestationIntermediateCert != nullptr && mDac.productAttestationIntermediateCert.length > 0) {
                if (outProductAttestationIntermediateCert.size() >= mDac.productAttestationIntermediateCert.length) {
                    memcpy(outProductAttestationIntermediateCert.data(), mDac.productAttestationIntermediateCert.bytes,
                        mDac.productAttestationIntermediateCert.length);
                    outProductAttestationIntermediateCert.reduce_size(mDac.productAttestationIntermediateCert.length);
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
