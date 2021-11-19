/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Administrator Commissioning Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/SetupPayload.h>

using namespace chip;
using namespace chip::app::Clusters::AdministratorCommissioning;

// Specifications section 5.4.2.3. Announcement Duration
constexpr uint32_t kMaxCommissionioningTimeoutSeconds = 15 * 60;

bool emberAfAdministratorCommissioningClusterOpenCommissioningWindowCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::OpenCommissioningWindow::DecodableType & commandData)
{
    auto & commissioningTimeout = commandData.commissioningTimeout;
    auto & pakeVerifier         = commandData.PAKEVerifier;
    auto & discriminator        = commandData.discriminator;
    auto & iterations           = commandData.iterations;
    auto & salt                 = commandData.salt;
    auto & passcodeID           = commandData.passcodeID;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    PASEVerifier verifier;
    const uint8_t * verifierData = pakeVerifier.data();

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    VerifyOrExit(!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen(),
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(sizeof(verifier) == pakeVerifier.size(), status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(iterations >= kPBKDFMinimumIterations, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(iterations <= kPBKDFMaximumIterations, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() >= kPBKDFMinimumSaltLen, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(salt.size() <= kPBKDFMaximumSaltLen, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));

    memcpy(verifier.mW0, &verifierData[0], kSpake2p_WS_Length);
    memcpy(verifier.mL, &verifierData[kSpake2p_WS_Length], kSpake2p_WS_Length);

    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().OpenEnhancedCommissioningWindow(
                     commissioningTimeout, discriminator, verifier, iterations, salt, passcodeID) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    auto & commissioningTimeout = commandData.commissioningTimeout;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");
    VerifyOrExit(!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen(),
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_BUSY));
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    VerifyOrExit(Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(commissioningTimeout) ==
                     CHIP_NO_ERROR,
                 status.Emplace(StatusCode::EMBER_ZCL_STATUS_CODE_PAKE_PARAMETER_ERROR));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status.Value());
        commandObj->AddClusterSpecificFailure(commandPath, status.Value());
    }
    else
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RevokeCommissioning::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    if (!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogError(Zcl, "Commissioning window is currently not open");
        commandObj->AddClusterSpecificFailure(commandPath, StatusCode::EMBER_ZCL_STATUS_CODE_WINDOW_NOT_OPEN);
    }
    else
    {
        Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        ChipLogProgress(Zcl, "Commissioning window is now closed");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    return true;
}

void MatterAdministratorCommissioningPluginServerInitCallback() {}
