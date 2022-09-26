/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
        if ([strongDelegate respondsToSelector:@selector(deviceAttestationCompletedForController:
                                                                                          device:attestationDeviceInfo:error:)]) {
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
                [strongDelegate deviceAttestationCompletedForController:mDeviceController
                                                                 device:device
                                                  attestationDeviceInfo:deviceInfo
                                                                  error:error];
            }
        } else if ((attestationResult != chip::Credentials::AttestationVerificationResult::kSuccess) &&
            [strongDelegate respondsToSelector:@selector(deviceAttestationFailedForController:device:error:)]) {

            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSError * error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                [strongDelegate deviceAttestationFailedForController:mDeviceController device:device error:error];
            }
        }
    });
}
