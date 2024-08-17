/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <stddef.h>

#include <app/InteractionModelEngine.h>
#include <lib/core/ScopedNodeId.h>
#include <messaging/ExchangeMgr.h>

/**
 * @brief StayActiveSender contains all the data and methods needed for active period extension of an ICD client.
 *
 * Lifetime of instance of StayActiveSender is entirely self managed.
 */
class StayActiveSender
{
private:
    // Ideally StayActiveSender would be a private constructor, unfortunately that is not possible as Platform::New
    // does not have access to private constructors. As a workaround we have defined this private struct that can
    // be forwarded by Platform::New that allows us to enforce that the only way StayActiveSender is constructed is
    // if SendStayActiveCommand is called.
    struct ConstructorOnlyInternallyCallable
    {
    };

public:
    using OnDoneCallbackType = std::function<void(uint32_t promisedActiveDurationMs)>;

    /**
     * @brief Attempts to send a StayActiveRequest command
     *
     * @param[in] stayActiveDurationMs StayActiveRequest command parameter.
     * @param[in] peerNode Peer node we sending StayActiveRequest command to
     * @param[in] engine Interaction Model Engine instance for sending command.
     * @param[in] onDone Upon this function returning success, it is expected that onDone will be called after we
     *            have successfully recieved a response
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    static CHIP_ERROR SendStayActiveCommand(uint32_t stayActiveDurationMs, const chip::ScopedNodeId & peerNode,
                                            chip::app::InteractionModelEngine * engine, OnDoneCallbackType onDone);

    // Ideally this would be a private constructor, unfortunately that is not possible as Platform::New does not
    // have access to private constructors. As a workaround we have defined a private struct that can be forwarded
    // by Platform::New that allows us to enforce that the only way this is constructed is if SendStayActiveCommand
    // is called.
    StayActiveSender(const ConstructorOnlyInternallyCallable & _, uint32_t stayActiveDurationMs,
                     const chip::ScopedNodeId & peerNode, chip::app::InteractionModelEngine * engine, OnDoneCallbackType onDone);

private:
    /**
     * @brief Sets up a CASE session with the peer to extend the client's active period with that peer.
     * Returns error if we did not even manage to kick off a CASE attempt.
     */
    CHIP_ERROR EstablishSessionToPeer();

    // CASE session callbacks
    /**
     *@brief Callback received on successfully establishing a CASE session in order to keep the 'lit icd device' active
     *
     * @param[in] context       - context of the client establishing the CASE session
     * @param[in] exchangeMgr   - exchange manager to use for the re-registration
     * @param[in] sessionHandle - session handle to use for the re-registration
     */
    static void HandleDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                      const chip::SessionHandle & sessionHandle);
    /**
     * @brief Callback received on failure to establish a CASE session
     *
     * @param[in] context - context of the client establishing the CASE session
     * @param[in] peerId  - Scoped Node ID of the peer node
     * @param[in] err     - failure reason
     */
    static void HandleDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err);

    /**
     * @brief Used to send a stayActive command to the peer
     *
     * @param[in] exchangeMgr   - exchange manager to use for the re-registration
     * @param[in] sessionHandle - session handle to use for the re-registration
     */
    CHIP_ERROR SendStayActiveCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    uint32_t mStayActiveDurationMs = 0;
    chip::ScopedNodeId mPeerNode;
    chip::app::InteractionModelEngine * mpImEngine = nullptr;
    OnDoneCallbackType mOnDone;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};
