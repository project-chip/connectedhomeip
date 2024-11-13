/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "Types.h"

namespace matter {
namespace casting {
namespace core {

class CastingPlayer;

/**
 * @brief Called when the User Directed Commissioning (UDC) process succeeds or fails.
 * @param[in] err For success, called back with CHIP_NO_ERROR. For failure, called back with an error.
 * @param[in] castingPlayer For success, called back with a CastingPlayer *. For failure, called back with a nullptr.
 */
using ConnectCallback = std::function<void(CHIP_ERROR err, CastingPlayer * castingPlayer)>;

/**
 * @brief Called when a Commissioner Declaration UDC message has been received.
 * @param[in] source The source of the Commissioner Declaration message.
 * @param[in] cd The Commissioner Declaration message.
 */
using CommissionerDeclarationCallback = std::function<void(
    const chip::Transport::PeerAddress & source, const chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)>;

/**
 * @brief A container struct for User Directed Commissioning (UDC) callbacks.
 */
struct ConnectionCallbacks
{
    /**
     * (Required) The callback called when the connection process has ended, regardless of whether it was successful or not.
     */
    ConnectCallback mOnConnectionComplete = nullptr;
    /**
     * (Optional) The callback called when the Client/Commissionee receives a CommissionerDeclaration message from the
     * CastingPlayer/Commissioner. This callback is needed to support UDC features where a reply from the Commissioner is expected.
     * It provides information indicating the Commissioner’s pre-commissioning state.
     */
    CommissionerDeclarationCallback mCommissionerDeclarationCallback = nullptr;
};

} // namespace core
} // namespace casting
} // namespace matter
