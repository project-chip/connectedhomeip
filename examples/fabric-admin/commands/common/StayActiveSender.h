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

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandSender.h>
#include <app/OperationalSessionSetup.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ExchangeContext.h>
#include <lib/support/CodeUtils.h>
#include <stddef.h>
#include <app/InteractionModelEngine.h>

namespace chip
{
namespace app
{
class InteractionModelEngine;
}
}

/**
 * @brief StayActiveSender contains all the data and methods needed for active period extension of an ICD client.
 */
class StayActiveSender
{
public:
    StayActiveSender(uint32_t stayActiveDuration, const chip::ScopedNodeId & peerNode,
                     chip::app::InteractionModelEngine * engine);

    /**
     * @brief Sets up a CASE session to the peer for extend a client active period with the peer.
     * Returns error if we did not even manage to kick off a CASE attempt.
     */
    CHIP_ERROR EstablishSessionToPeer();

private:
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

    uint32_t mStayActiveDuration = 0;
    chip::ScopedNodeId mPeerNode;
    chip::app::InteractionModelEngine * mpImEngine   = nullptr;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};
