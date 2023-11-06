/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/core/Optional.h>

namespace chip {

class DeviceProxy;

namespace Controller {
class DeviceCommissioner;
} // namespace Controller

namespace Credentials {

/// Callbacks for CHIP device attestation status
class DeviceAttestationDelegate
{
public:
    virtual ~DeviceAttestationDelegate() {}

    /**
     * @brief
     *   If valid, value to set for the fail-safe timer before the delegate's OnDeviceAttestationFailed is invoked.
     *
     * @return Optional value for the fail-safe timer in seconds.
     */
    virtual Optional<uint16_t> FailSafeExpiryTimeoutSecs() const = 0;

    /**
     * @brief
     *   This method is invoked when device attestation fails for a device that is being commissioned. The client
     *   handling the failure has the option to continue commissioning or fail the operation.
     *
     *   Optionally, when ShouldWaitAfterDeviceAttestation is overridden to return true, this method is also
     *   invoked when device attestation succeeds.
     *
     *   @param deviceCommissioner The commissioner object that is commissioning the device
     *   @param device The proxy represent the device being commissioned
     *   @param info The structure holding device info for additional verification by the application
     *   @param attestationResult The failure code for the device attestation validation operation
     */
    virtual void OnDeviceAttestationCompleted(Controller::DeviceCommissioner * deviceCommissioner, DeviceProxy * device,
                                              const DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                              AttestationVerificationResult attestationResult) = 0;

    /**
     * @brief
     *   Override this method to return whether the attestation delegate wants the commissioner to wait for a
     *   ContinueCommissioningAfterDeviceAttestation call in the case attestationResult is successful.
     */
    virtual bool ShouldWaitAfterDeviceAttestation() { return false; }
};

} // namespace Credentials
} // namespace chip
