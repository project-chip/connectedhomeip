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
#import "MTRError_Internal.h"

void MTRDeviceAttestationDelegateBridge::OnDeviceAttestationFailed(chip::Controller::DeviceCommissioner * deviceCommissioner,
    chip::DeviceProxy * device, chip::Credentials::AttestationVerificationResult attestationResult)
{
    dispatch_async(mQueue, ^{
        NSLog(@"MTRDeviceAttestationDelegateBridge::OnDeviceAttestationFailed failed with result: %hu", attestationResult);

        mResult = attestationResult;

        id<MTRDeviceAttestationDelegate> strongDelegate = mDeviceAttestationDelegate;
        if ([strongDelegate respondsToSelector:@selector(deviceAttestation:failedForDevice:error:)]) {

            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                NSError * error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                [strongDelegate deviceAttestation:mDeviceController failedForDevice:device error:error];
            }
        }
    });
}
