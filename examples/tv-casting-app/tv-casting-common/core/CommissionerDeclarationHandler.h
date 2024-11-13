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

#include "ConnectionCallbacks.h"
#include "Types.h"

namespace matter {
namespace casting {
namespace core {

/**
 * @brief React to the Commissioner's CommissionerDeclaration messages with this singleton. This is an implementation of the
 * CommissionerDeclarationHandler from connectedhomeip/src/protocols/user_directed_commissioning/UserDirectedCommissioning.h.
 */
class CommissionerDeclarationHandler : public chip::Protocols::UserDirectedCommissioning::CommissionerDeclarationHandler
{
public:
    CommissionerDeclarationHandler(const CommissionerDeclarationHandler &) = delete;
    void operator=(const CommissionerDeclarationHandler &)                 = delete;

    static CommissionerDeclarationHandler * GetInstance();

    /**
     * @brief Called when a Commissioner Declaration UDC message has been received.
     * @param[in] source The source of the Commissioner Declaration message.
     * @param[in] cd The Commissioner Declaration message.
     */
    void OnCommissionerDeclarationMessage(const chip::Transport::PeerAddress & source,
                                          chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd) override;

    /**
     * @brief OnCommissionerDeclarationMessage() will call the CommissionerDeclarationCallback set by this function.
     */
    void SetCommissionerDeclarationCallback(CommissionerDeclarationCallback callback);

    /**
     * @brief returns true if the CommissionerDeclarationHandler sigleton has a CommissionerDeclarationCallback set, false
     * otherwise.
     */
    bool HasCommissionerDeclarationCallback() { return static_cast<bool>(mCmmissionerDeclarationCallback_); };

private:
    static CommissionerDeclarationHandler * sCommissionerDeclarationHandler_;
    CommissionerDeclarationCallback mCmmissionerDeclarationCallback_;

    CommissionerDeclarationHandler() {}
    ~CommissionerDeclarationHandler() {}
};

} // namespace core
} // namespace casting
} // namespace matter
