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

#include "CommissionerDeclarationHandler.h"

#include "CastingPlayer.h"
#include "Types.h"
#include "support/ChipDeviceEventHandler.h"
#include <app/server/Server.h>

namespace matter {
namespace casting {
namespace core {

CommissionerDeclarationHandler * CommissionerDeclarationHandler::sCommissionerDeclarationHandler_ = nullptr;

CommissionerDeclarationHandler * CommissionerDeclarationHandler::GetInstance()
{
    if (sCommissionerDeclarationHandler_ == nullptr)
    {
        sCommissionerDeclarationHandler_ = new CommissionerDeclarationHandler();
    }
    return sCommissionerDeclarationHandler_;
}

// TODO: In the following PRs. Implement setHandler() for CommissionerDeclaration messages and expose messages to higher layers for
// Linux, Android and iOS.
void CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(
    const chip::Transport::PeerAddress & source, chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)
{
    ChipLogProgress(AppServer,
                    "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(), calling CloseCommissioningWindow()");
    // Close the commissioning window. Since we recived a CommissionerDeclaration message from the Commissioner, we know that
    // commissioning via AccountLogin cluster failed. We will open a new commissioningWindow prior to sending the next
    // IdentificationDeclaration Message to the Commissioner.
    chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    support::ChipDeviceEventHandler::SetUdcStatus(false);

    // Flag to indicate when the CastingPlayer/Commissioner user has decided to exit the commissioning process.
    if (cd.GetCancelPasscode())
    {
        ChipLogProgress(AppServer,
                        "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(), Got CancelPasscode parameter, "
                        "CastingPlayer/Commissioner user has decided to exit the commissioning attempt. Connection aborted.");
        // Since the CastingPlayer/Commissioner user has decided to exit the commissioning process, we cancel the ongoing
        // connection attempt without notifying the CastingPlayer/Commissioner. Therefore the
        // shouldSendIdentificationDeclarationMessage flag in the internal StopConnecting() API call is set to false. The
        // CastingPlayer/Commissioner user and the Casting Client/Commissionee user are not necessarily the same user. For example,
        // in an enviroment with multiple CastingPlayer/Commissioner TVs, one user 1 might be controlling the Client/Commissionee
        // and user 2 might be controlling the CastingPlayer/Commissioner TV.
        CastingPlayer * targetCastingPlayer = CastingPlayer::GetTargetCastingPlayer();
        // Avoid crashing if we recieve this CommissionerDeclaration message when targetCastingPlayer is nullptr.
        if (targetCastingPlayer == nullptr)
        {
            ChipLogError(AppServer,
                         "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() targetCastingPlayer is nullptr");
        }
        else
        {
            CHIP_ERROR err = targetCastingPlayer->StopConnecting(false);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    AppServer,
                    "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() StopConnecting() failed due to error: "
                    "%" CHIP_ERROR_FORMAT,
                    err.Format());
            }
        }
    }

    if (mCmmissionerDeclarationCallback_)
    {
        mCmmissionerDeclarationCallback_(source, cd);
    }
    else
    {
        ChipLogError(AppServer,
                     "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() mCmmissionerDeclarationCallback_ not set");
    }
}

void CommissionerDeclarationHandler::SetCommissionerDeclarationCallback(
    matter::casting::core::CommissionerDeclarationCallback callback)
{
    ChipLogProgress(AppServer, "CommissionerDeclarationHandler::SetCommissionerDeclarationCallback()");
    if (callback != nullptr)
    {
        mCmmissionerDeclarationCallback_ = std::move(callback);
    }
}

} // namespace core
} // namespace casting
} // namespace matter
