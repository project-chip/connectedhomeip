/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceAttestationDelegate_Internal.h"
#import "MTRError_Internal.h"
#import "NSDataSpanConversion.h"

void MTRDeviceAttestationDelegateBridge::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
    chip::DeviceProxy * device, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
    chip::Credentials::AttestationVerificationResult attestationResult)
{
    dispatch_async(mQueue, ^{
        NSLog(@"MTRDeviceAttestationDelegateBridge::OnDeviceAttestationFailed completed with result: %hu", attestationResult);

        mResult = attestationResult;

        id<MTRDeviceAttestationDelegate> strongDelegate = mDeviceAttestationDelegate;
        if ([strongDelegate respondsToSelector:@selector(deviceAttestation:completedForDevice:attestationDeviceInfo:error:)]) {
            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSData * dacData = AsData(info.dacDerBuffer());
                NSData * paiData = AsData(info.paiDerBuffer());
                NSData * cdData = info.cdBuffer().HasValue() ? AsData(info.cdBuffer().Value()) : nil;
                MTRDeviceAttestationDeviceInfo * deviceInfo =
                    [[MTRDeviceAttestationDeviceInfo alloc] initWithDACCertificate:dacData
                                                                 dacPAICertificate:paiData
                                                            certificateDeclaration:cdData];
                NSError * error = (attestationResult == chip::Credentials::AttestationVerificationResult::kSuccess)
                    ? nil
                    : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                [strongDelegate deviceAttestation:mDeviceController
                               completedForDevice:device
                            attestationDeviceInfo:deviceInfo
                                            error:error];
            }
        } else if ((attestationResult != chip::Credentials::AttestationVerificationResult::kSuccess) &&
            [strongDelegate respondsToSelector:@selector(deviceAttestation:failedForDevice:error:)]) {

            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSError * error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                [strongDelegate deviceAttestation:mDeviceController failedForDevice:device error:error];
            }
        }
    });
}
