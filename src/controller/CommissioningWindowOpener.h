/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/OperationalSessionSetup.h>
#include <app/data-model/NullObject.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowParams.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <setup_payload/SetupPayload.h>

namespace chip {
namespace Controller {

/**
 * A helper class to open a commissioning window given some parameters.
 */
class CommissioningWindowOpener
{
public:
    CommissioningWindowOpener(DeviceController * controller) :
        mController(controller), mDeviceConnected(&OnDeviceConnectedCallback, this),
        mDeviceConnectionFailure(&OnDeviceConnectionFailureCallback, this)
    {}

    enum class CommissioningWindowOption : uint8_t
    {
        kOriginalSetupCode = 0,
        kTokenWithRandomPIN,
        kTokenWithProvidedPIN,
    };

    /*
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   node id and ask it to re-enter commissioning mode with its original
     *   PASE verifier, discriminator, etc. The device will exit commissioning
     *   mode after a successful commissioning, or after the given `timeout`
     *   time.
     *
     * @param[in] deviceId The device Id.
     * @param[in] timeout  The commissioning mode should terminate after this much time.
     * @param[in] callback The callback to call once the commissioning window is
     *                     open or if an error occurs.
     */
    CHIP_ERROR OpenBasicCommissioningWindow(NodeId deviceId, System::Clock::Seconds16 timeout,
                                            Callback::Callback<OnOpenBasicCommissioningWindow> * callback);

    /**
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   node id and ask it to re-enter commissioning mode with a PASE verifier
     *   derived from the given information and the given discriminator. The
     *   device will exit commissioning mode after a successful commissioning,
     *   or after the given `timeout` time.
     *
     * @param[in] deviceId      The device Id.
     * @param[in] timeout       The commissioning mode should terminate after this much time.
     * @param[in] iteration     The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
     *                          PAKE passcode verifier to be used for this commissioning.
     * @param[in] discriminator The long discriminator for the DNS-SD advertisement.
     * @param[in] setupPIN      The setup PIN to use, or NullOptional to use a randomly-generated one.
     * @param[in] salt          The salt to use, or NullOptional to use a
     *                          randomly-generated one.  If provided, must be at
     *                          least kSpake2p_Min_PBKDF_Salt_Length bytes and
     *                          at most kSpake2p_Max_PBKDF_Salt_Length bytes in
     *                          length.
     * @param[in] callback      The function to be called on success or failure of opening of commissioning window.
     * @param[out] payload      The setup payload, not including the VID/PID bits,
     *                          even if those were asked for, that is generated
     *                          based on the passed-in information.  The payload
     *                          provided to the callback function, unlike this
     *                          out parameter, will include the VID/PID bits if
     *                          readVIDPIDAttributes is true.
     *
     * @param[in] readVIDPIDAttributes Should the API internally read VID and PID from the device while opening the
     *                                 commissioning window.  If this argument is `true`, the API will read VID and
     *                                 PID from the device and include them in the setup payload passed to the
     *                                 callback.
     */
    CHIP_ERROR OpenCommissioningWindow(NodeId deviceId, System::Clock::Seconds16 timeout, uint32_t iteration,
                                       uint16_t discriminator, Optional<uint32_t> setupPIN, Optional<ByteSpan> salt,
                                       Callback::Callback<OnOpenCommissioningWindow> * callback, SetupPayload & payload,
                                       bool readVIDPIDAttributes = false);

    /**
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   node id and ask it to re-enter commissioning mode with a PASE verifier
     *   derived from the given information and the given discriminator. The
     *   device will exit commissioning mode after a successful commissioning,
     *   or after the given `timeout` time.
     *
     * @param[in] params        The parameters required to open an enhanced commissioning window
     *                          with the provided or generated passcode.
     * @param[out] payload      The setup payload, not including the VID/PID bits,
     *                          even if those were asked for, that is generated
     *                          based on the passed-in information.  The payload
     *                          provided to the callback function, unlike this
     *                          out parameter, will include the VID/PID bits if
     *                          readVIDPIDAttributes is true.
     */
    CHIP_ERROR OpenCommissioningWindow(const CommissioningWindowPasscodeParams & params, SetupPayload & payload);

    /**
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   node id and ask it to re-enter commissioning mode with a PASE verifier
     *   derived from the given information and the given discriminator. The
     *   device will exit commissioning mode after a successful commissioning,
     *   or after the given `timeout` time.
     *
     * @param[in] params    The parameters required to open an enhanced commissioning window
     *                      with the provided PAKE passcode verifier.
     */
    CHIP_ERROR OpenCommissioningWindow(const CommissioningWindowVerifierParams & params);

private:
    enum class Step : uint8_t
    {
        // Ready to start opening a commissioning window.
        kAcceptCommissioningStart,
        // Need to read VID.
        kReadVID,
        // Need to read PID.
        kReadPID,
        // Need to open commissioning window.
        kOpenCommissioningWindow,
    };

    CHIP_ERROR OpenCommissioningWindowInternal(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnPIDReadResponse(void * context, uint16_t value);
    static void OnVIDReadResponse(void * context, VendorId value);
    static void OnVIDPIDReadFailureResponse(void * context, CHIP_ERROR error);
    static void OnOpenCommissioningWindowSuccess(void * context, const app::DataModel::NullObjectType &);
    static void OnOpenCommissioningWindowFailure(void * context, CHIP_ERROR error);
    static void OnDeviceConnectedCallback(void * context, Messaging::ExchangeManager & exchangeMgr,
                                          const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureCallback(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    DeviceController * const mController = nullptr;
    Step mNextStep                       = Step::kAcceptCommissioningStart;

    Callback::Callback<OnOpenCommissioningWindow> * mCommissioningWindowCallback                     = nullptr;
    Callback::Callback<OnOpenCommissioningWindowWithVerifier> * mCommissioningWindowVerifierCallback = nullptr;
    Callback::Callback<OnOpenBasicCommissioningWindow> * mBasicCommissioningWindowCallback           = nullptr;
    SetupPayload mSetupPayload;
    SetupDiscriminator mDiscriminator{};
    NodeId mNodeId               = kUndefinedNodeId;
    EndpointId mTargetEndpointId = kRootEndpointId; // Default endpoint for Administrator Commissioning Cluster
    System::Clock::Seconds16 mCommissioningWindowTimeout = System::Clock::kZero;
    CommissioningWindowOption mCommissioningWindowOption = CommissioningWindowOption::kOriginalSetupCode;
    Crypto::Spake2pVerifier mVerifier; // Used for non-basic commissioning.
    // Parameters needed for non-basic commissioning.
    uint32_t mPBKDFIterations = 0;
    uint8_t mPBKDFSaltBuffer[Crypto::kSpake2p_Max_PBKDF_Salt_Length];
    ByteSpan mPBKDFSalt;

    Callback::Callback<OnDeviceConnected> mDeviceConnected;
    Callback::Callback<OnDeviceConnectionFailure> mDeviceConnectionFailure;
};

/**
 * A helper class that can be used by consumers that don't care about the callback from the
 * open-commissioning-window process and just want automatic cleanup of the CommissioningWindowOpener when done
 * with it.
 */
class AutoCommissioningWindowOpener : private CommissioningWindowOpener
{
public:
    // Takes the same arguments as CommissioningWindowOpener::OpenBasicCommissioningWindow except without the
    // callback.
    static CHIP_ERROR OpenBasicCommissioningWindow(DeviceController * controller, NodeId deviceId,
                                                   System::Clock::Seconds16 timeout);
    // Takes the same arguments as CommissioningWindowOpener::OpenCommissioningWindow except without the
    // callback.
    static CHIP_ERROR OpenCommissioningWindow(DeviceController * controller, NodeId deviceId, System::Clock::Seconds16 timeout,
                                              uint32_t iteration, uint16_t discriminator, Optional<uint32_t> setupPIN,
                                              Optional<ByteSpan> salt, SetupPayload & payload, bool readVIDPIDAttributes = false);

private:
    AutoCommissioningWindowOpener(DeviceController * controller);

    static void OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload);
    static void OnOpenBasicCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status);

    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenBasicCommissioningWindow> mOnOpenBasicCommissioningWindowCallback;
};

} // Namespace Controller
} // namespace chip
