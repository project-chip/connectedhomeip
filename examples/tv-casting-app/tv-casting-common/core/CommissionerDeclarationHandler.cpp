/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

void CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(
    const chip::Transport::PeerAddress & source, chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)
{
    ChipLogProgress(AppServer, "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage()");

    CastingPlayer * targetCastingPlayer = CastingPlayer::GetTargetCastingPlayer();
    // FixedPasscodeCommissioningSelected is valid only when it comes from the current target CastingPlayer after the
    // Client/Commissionee requested Fixed Passcode Commissioning for this connection attempt.
    bool sourceMatchesTarget = false;
    if (targetCastingPlayer != nullptr)
    {
        for (size_t i = 0; i < targetCastingPlayer->mAttributes.numIPs; ++i)
        {
            if (targetCastingPlayer->mAttributes.ipAddresses[i] == source.GetIPAddress())
            {
                sourceMatchesTarget = true;
                break;
            }
        }
    }
    bool fixedPasscodeCommissioningSelected = cd.GetFixedPasscodeCommissioningSelected() && sourceMatchesTarget &&
        targetCastingPlayer->mIdOptions.mFixedPasscodeCommissioningSupported;
    bool fixedPasscodeAuthenticationFailed =
        cd.GetErrorCode() == chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration::CdError::kPaseAuthFailed &&
        targetCastingPlayer != nullptr && targetCastingPlayer->mFixedPasscodeCommissioningSelected;

    if (fixedPasscodeAuthenticationFailed && !sourceMatchesTarget)
    {
        ChipLogError(AppServer,
                     "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() ignoring PASE authentication failure "
                     "from a non-target Commissioner");
        return;
    }

    if (cd.GetFixedPasscodeCommissioningSelected() && !fixedPasscodeCommissioningSelected)
    {
        ChipLogError(AppServer,
                     "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() ignoring unsolicited "
                     "FixedPasscodeCommissioningSelected");
        cd.SetFixedPasscodeCommissioningSelected(false);
    }

    if (fixedPasscodeCommissioningSelected)
    {
        targetCastingPlayer->mFixedPasscodeCommissioningSelected = true;
    }

    // During a passcode flow that requires a second IdentificationDeclaration, the selected Passcode differs from the one used by
    // the current commissioning window. Close that window before the Client/Commissionee updates its
    // CommissionableDataProvider and opens a new commissioning window using the selected Passcode.
    if (cd.GetCommissionerPasscode() || fixedPasscodeCommissioningSelected)
    {
        ChipLogProgress(AppServer,
                        "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(), calling CloseCommissioningWindow()");
        // Close the commissioning window.
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        TEMPORARY_RETURN_IGNORED support::ChipDeviceEventHandler::SetUdcStatus(false);
    }

    // During the workflow for Detecting App Presence on Casting Player, we send UDC with NoPasscode=True.
    // The CD we get back will indicate NeedsPasscode=true and PasscodeDialogDisplayed=false.
    // In this scenario, we want to clear the UDC status to allow other UDC messages to be sent.
    if (cd.GetNeedsPasscode() && !cd.GetPasscodeDialogDisplayed())
    {
        ChipLogProgress(AppServer,
                        "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() "
                        "needs passcode - This is likely an App Check response");
        TEMPORARY_RETURN_IGNORED support::ChipDeviceEventHandler::SetUdcStatus(false);
    }

    // End the selected Fixed Passcode Commissioning attempt when the Commissioner reports a PASE authentication failure.
    if (fixedPasscodeAuthenticationFailed)
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        TEMPORARY_RETURN_IGNORED support::ChipDeviceEventHandler::SetUdcStatus(false);
        targetCastingPlayer->mFixedPasscodeCommissioningSelected = false;
    }

    // Flag to indicate when the CastingPlayer/Commissioner user has decided to exit the commissioning process.
    if (cd.GetCancelPasscode())
    {
        ChipLogProgress(AppServer,
                        "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(), Got CancelPasscode parameter, "
                        "CastingPlayer/Commissioner user has decided to exit the commissioning attempt. Connection aborted.");
        // Close the commissioning window.
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        TEMPORARY_RETURN_IGNORED support::ChipDeviceEventHandler::SetUdcStatus(false);
        // Since the CastingPlayer/Commissioner user has decided to exit the commissioning process, we cancel the ongoing
        // connection attempt without notifying the CastingPlayer/Commissioner. Therefore the
        // shouldSendIdentificationDeclarationMessage flag in the internal StopConnecting() API call is set to false. The
        // CastingPlayer/Commissioner user and the Casting Client/Commissionee user are not necessarily the same user. For example,
        // in an enviroment with multiple CastingPlayer/Commissioner TVs, one user 1 might be controlling the Client/Commissionee
        // and user 2 might be controlling the CastingPlayer/Commissioner TV.
        // Avoid crashing if we recieve this CommissionerDeclaration message when targetCastingPlayer is nullptr.
        if (targetCastingPlayer != nullptr)
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
        else
        {
            ChipLogError(AppServer,
                         "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() targetCastingPlayer is nullptr");
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
