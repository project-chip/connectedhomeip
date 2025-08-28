/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"

#include <app/DeviceProxy.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <lib/support/TypeTraits.h>
#include <platform/LockTracker.h>

void MTRDeviceAttestationDelegateBridge::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
    chip::DeviceProxy * device, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
    chip::Credentials::AttestationVerificationResult attestationResult)
{
    assertChipStackLockedByCurrentThread();

    // Capture any values passed by reference, so we are not relying on them
    // sticking around after we go async.
    NSData * dacData = AsData(info.dacDerBuffer());
    NSData * paiData = AsData(info.paiDerBuffer());
    NSData * cdData = info.cdBuffer().HasValue() ? AsData(info.cdBuffer().Value()) : nil;

    NSData * attestationChallenge;
    auto session = device->GetSecureSession();
    if (session.HasValue()) {
        attestationChallenge = AsData(session.Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge());
    } else {
        // Really should not happen.
        MTR_LOG_ERROR("OnDeviceAttestationCompleted for a device with no PASE session, making up fake attestation challenge");
        attestationChallenge = [NSData data];
    }

    // CommissioningParameters is always available; the DeviceCommissioner API is broken when it
    // pretends it might not be.  And at this point in commissioning, the various attestation
    // information received from the device is stored in those commissioning parameters; otherwise
    // we would not have been called at all.
    chip::Controller::CommissioningParameters commissioningParameters
        = deviceCommissioner->GetCommissioningParameters().Value();
    NSData * attestationNonce = AsData(commissioningParameters.GetAttestationNonce().Value());
    NSData * elementsTLV = AsData(commissioningParameters.GetAttestationElements().Value());
    NSData * elementsSignature = AsData(commissioningParameters.GetAttestationSignature().Value());

    NSNumber * basicInformationVendorID = @(info.BasicInformationVendorId());
    NSNumber * basicInformationProductID = @(info.BasicInformationProductId());

    void * deviceHandle = device;

    // TODO: Consider exposing the actual attestation verification result in MTRDeviceAttestationDeviceInfo; need to
    // figure out how best to do that.

    dispatch_async(mQueue, ^{
        // Hide things that are not passed to us by value, so we don't use them by accident.
        mtr_hide(deviceCommissioner);
        mtr_hide(device);
        mtr_hide(info);
        MTR_LOG("MTRDeviceAttestationDelegateBridge::OnDeviceAttestationCompleted with result: %hu (%s)",
            chip::to_underlying(attestationResult), chip::Credentials::GetAttestationResultDescription(attestationResult));

        mResult = attestationResult;

        id<MTRDeviceAttestationDelegate> strongDelegate = mDeviceAttestationDelegate;
        if ([strongDelegate respondsToSelector:@selector(deviceAttestationCompletedForController:opaqueDeviceHandle:attestationDeviceInfo:error:)]
            || [strongDelegate respondsToSelector:@selector(deviceAttestation:completedForDevice:attestationDeviceInfo:error:)]) {
            MTRDeviceController * strongController = mDeviceController;
            if (strongController) {
                MTRDeviceAttestationDeviceInfo * deviceInfo =
                    [[MTRDeviceAttestationDeviceInfo alloc] initWithAttestationChallenge:attestationChallenge
                                                                        attestationNonce:attestationNonce
                                                                             elementsTLV:elementsTLV
                                                                       elementsSignature:elementsSignature
                                                                          dacCertificate:dacData
                                                                       dacPAICertificate:paiData
                                                                certificationDeclaration:cdData
                                                                basicInformationVendorID:basicInformationVendorID
                                                               basicInformationProductID:basicInformationProductID];
                NSError * error = (attestationResult == chip::Credentials::AttestationVerificationResult::kSuccess)
                    ? nil
                    : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTEGRITY_CHECK_FAILED];
                if ([strongDelegate respondsToSelector:@selector(deviceAttestationCompletedForController:opaqueDeviceHandle:attestationDeviceInfo:error:)]) {
                    [strongDelegate deviceAttestationCompletedForController:mDeviceController
                                                         opaqueDeviceHandle:deviceHandle
                                                      attestationDeviceInfo:deviceInfo
                                                                      error:error];
                } else {
                    [strongDelegate deviceAttestation:mDeviceController
                                   completedForDevice:deviceHandle
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
                    [strongDelegate deviceAttestationFailedForController:mDeviceController opaqueDeviceHandle:deviceHandle error:error];
                } else {
                    [strongDelegate deviceAttestation:mDeviceController failedForDevice:deviceHandle error:error];
                }
            }
        }
    });
}
