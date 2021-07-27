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

#include <app/CommandHandler.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

bool emberAfAdministratorCommissioningClusterOpenCommissioningWindowCallback(app::CommandHandler * commandObj,
                                                                             uint16_t CommissioningTimeout, ByteSpan PAKEVerifier,
                                                                             uint16_t Discriminator, uint32_t Iterations,
                                                                             ByteSpan Salt, uint16_t PasscodeID)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    PASEVerifier verifier;
    ChipLogProgress(Zcl, "Received command to open commissioning window");
    VerifyOrExit(!IsPairingWindowOpen(), status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(sizeof(verifier) == PAKEVerifier.size(), status = EMBER_ZCL_STATUS_FAILURE);
    memcpy(&verifier[0][0], PAKEVerifier.data(), PAKEVerifier.size());
    VerifyOrExit(OpenPairingWindowUsingVerifier(CommissioningTimeout, Discriminator, verifier, Iterations, Salt, PasscodeID) ==
                     CHIP_NO_ERROR,
                 status = EMBER_ZCL_STATUS_FAILURE);
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(app::CommandHandler * commandObj,
                                                                                  uint16_t CommissioningTimeout)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");
    VerifyOrExit(!IsPairingWindowOpen(), status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(OpenDefaultPairingWindow(ResetFabrics::kNo, CommissioningTimeout) == CHIP_NO_ERROR,
                 status = EMBER_ZCL_STATUS_FAILURE);
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Status %d", status);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(app::CommandHandler * commandObj)
{
    ChipLogProgress(Zcl, "Received command to close commissioning window");
    ClosePairingWindow();
    ChipLogProgress(Zcl, "Commissioning window is now closed");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
