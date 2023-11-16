/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <cinttypes>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Span.h>

namespace chip {
namespace Controller {

/**
 * @brief
 * ICDRegistrationDelegate is an interface that provides a callback method for generating symmetric keys and reporting ICD
 * registration readiness.
 *
 * This delegate is used by the commissioner to generate symmetric keys and complete ICD registration with the ICD
 * device and to report when the device is ready for subscription.
 */
class ICDRegistrationDelegate
{
public:
    static constexpr size_t kKeySize = Crypto::kAES_CCM128_Key_Length;
    using ICDKey                     = FixedByteSpan<kKeySize>;

    /// The type of callback function that is called when symmetric key generation is completed.
    typedef void (*OnSymmetricKeyGenerationCompleted)(void * context, CHIP_ERROR error, NodeId checkInNodeId, uint64_t subjectId,
                                                      ICDKey key);

    virtual ~ICDRegistrationDelegate() = default;

    /**
     * @brief
     *   Called when the operational commissioning completed, the controller may register it with the ICD device.
     * The delegate can generate the symmetric key and call commissioner->OnSymmetricKeyGenerated to complete ICD
     * registration.
     *   This delegate supports Commissioner Self-Registration and External Controller Registration flow.
     *
     * @param[in] device       The node id of the device.
     * @param[in] onCompletion The callback object to be called when symmetric key generation is completed. The
     *                         `onCompletion` is expected to be valid through the key generation process. Implementations
     *                         should drop the reference of `onCompletion` after it is called.
     */
    virtual void GenerateSymmetricKey(NodeId device, Callback::Callback<OnSymmetricKeyGenerationCompleted> * onCompletion) = 0;

    /**
     * @bried
     *   Called when the registration flow for the ICD completes and the device is ready for accept subscriptions.
     * The callback is expected to trigger an async operation which subscription on the commissioner or external controllers.
     *
     * @param[in] device       The node id of the device.
     * @param[in] icdCounter   The ICD Counter received from the device.
     */
    virtual void OnRegistrationComplete(NodeId device, uint32_t icdCounter) = 0;
};

} // namespace Controller
} // namespace chip
