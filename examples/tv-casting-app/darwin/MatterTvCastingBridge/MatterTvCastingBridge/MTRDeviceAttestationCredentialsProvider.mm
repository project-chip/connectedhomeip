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

#import "MTRDeviceAttestationCredentialsProvider.h"

#import "MTRDeviceAttestationCredentials.h"

#include "lib/support/logging/CHIPLogging.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#import <Foundation/Foundation.h>

namespace matter {
namespace casting {
    namespace support {

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::Initialize(id<MTRDataSource> dataSource)
        {
            VerifyOrReturnError(dataSource != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(mDataSource == nullptr, CHIP_ERROR_INCORRECT_STATE);

            mDac = [mDataSource
                castingAppDidReceiveRequestForDeviceAttestationCredentials:@"MTRDeviceAttestationCredentialsProvider.Initialize()"];

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::GetCertificationDeclaration(
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

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation)
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

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::GetDeviceAttestationCert(
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

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::GetProductAttestationIntermediateCert(
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

        CHIP_ERROR MTRDeviceAttestationCredentialsProvider::SignWithDeviceAttestationKey(
            const chip::ByteSpan & messageToSign, chip::MutableByteSpan & outSignatureBuffer)
        {
            VerifyOrReturnError(mDataSource != nullptr, CHIP_ERROR_INCORRECT_STATE);

            __block NSData * signedData = nil;
            NSData * csrData = [NSData dataWithBytes:messageToSign.data() length:messageToSign.size()];
            dispatch_sync(mDataSource.clientQueue, ^{
                signedData = [mDataSource castingApp:@"MTRDeviceAttestationCredentialsProvider.SignWithDeviceAttestationKey()"
                    didReceiveRequestToSignCertificateRequest:csrData];
            });

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
