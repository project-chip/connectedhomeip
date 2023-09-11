/**
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceAttestationDelegate_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"

#include <lib/support/TypeTraits.h>

void MTRDeviceAttestationDelegateBridge::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
    chip::DeviceProxy * device, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
    chip::Credentials::AttestationVerificationResult attestationResult)
{
    dispatch_async(mQueue, ^{
        MTR_LOG_DEFAULT("MTRDeviceAttestationDelegateBridge::OnDeviceAttestationFailed completed with result: %hu",
            chip::to_underlying(attestationResult));

        mResult = attestationResult;

        id<MTRDeviceAttestationDelegate> strongDelegate = mDeviceAttestationDelegate;
        if ([strongDelegate respondsToSelector:@selector(deviceAttestationCompletedForController:
                                                                              opaqueDeviceHandle:attestationDeviceInfo:error:)]
            || [strongDelegate respondsToSelector:@selector(deviceAttestation:completedForDevice:attestationDeviceInfo:error:)]) {
            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSData * dacData = AsData(info.dacDerBuffer());
                NSData * paiData = AsData(info.paiDerBuffer());
                NSData * cdData = info.cdBuffer().HasValue() ? AsData(info.cdBuffer().Value()) : nil;
                MTRDeviceAttestationDeviceInfo * deviceInfo =
                    [[MTRDeviceAttestationDeviceInfo alloc] initWithDACCertificate:dacData
                                                                 dacPAICertificate:paiData
                                                            certificateDeclaration:cdData
                                                          basicInformationVendorID:@(info.BasicInformationVendorId())
                                                         basicInformationProductID:@(info.BasicInformationProductId())];
                NSError * error = (attestationResult == chip::Credentials::AttestationVerificationResult::kSuccess)
                    ? nil
                    : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                if ([strongDelegate respondsToSelector:@selector
                                    (deviceAttestationCompletedForController:opaqueDeviceHandle:attestationDeviceInfo:error:)]) {
                    [strongDelegate deviceAttestationCompletedForController:mDeviceController
                                                         opaqueDeviceHandle:device
                                                      attestationDeviceInfo:deviceInfo
                                                                      error:error];
                } else {
                    [strongDelegate deviceAttestation:mDeviceController
                                   completedForDevice:device
                                attestationDeviceInfo:deviceInfo
                                                error:error];
                }
            }
        } else if ((attestationResult != chip::Credentials::AttestationVerificationResult::kSuccess)
            && ([strongDelegate respondsToSelector:@selector(deviceAttestationFailedForController:opaqueDeviceHandle:error:)] ||
                [strongDelegate respondsToSelector:@selector(deviceAttestation:failedForDevice:error:)])) {

            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSError * error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                if ([strongDelegate respondsToSelector:@selector(deviceAttestationFailedForController:opaqueDeviceHandle:error:)]) {
                    [strongDelegate deviceAttestationFailedForController:mDeviceController opaqueDeviceHandle:device error:error];
                } else {
                    [strongDelegate deviceAttestation:mDeviceController failedForDevice:device error:error];
                }
            }
        }
    });
}
