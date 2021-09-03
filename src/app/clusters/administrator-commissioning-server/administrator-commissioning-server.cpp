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
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/SetupPayload.h>

using namespace chip;

// Specifications section 5.4.2.3. Announcement Duration
constexpr uint32_t kMaxCommissionioningTimeoutSeconds = 15 * 60;

bool emberAfAdministratorCommissioningClusterOpenCommissioningWindowCallback(EndpointId endpoint, app::CommandHandler * commandObj,
                                                                             uint16_t commissioningTimeout, ByteSpan pakeVerifier,
                                                                             uint16_t discriminator, uint32_t iterations,
                                                                             ByteSpan salt, uint16_t passcodeID)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    PASEVerifier verifier;
    const uint8_t * verifierData = pakeVerifier.data();

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    VerifyOrExit(!IsPairingWindowOpen(), status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(sizeof(verifier) == pakeVerifier.size(), status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(iterations >= kPBKDFMinimumIterations, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(iterations <= kPBKDFMaximumIterations, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(salt.size() >= kPBKDFMinimumSaltLen, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(salt.size() <= kPBKDFMaximumSaltLen, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, status = EMBER_ZCL_STATUS_FAILURE);

    memcpy(verifier.mW0, &verifierData[0], kSpake2p_WS_Length);
    memcpy(verifier.mL, &verifierData[kSpake2p_WS_Length], kSpake2p_WS_Length);

    VerifyOrExit(OpenEnhancedCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt, passcodeID) ==
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

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(EndpointId endpoint,
                                                                                  app::CommandHandler * commandObj,
                                                                                  uint16_t commissioningTimeout)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");
    VerifyOrExit(!IsPairingWindowOpen(), status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(commissioningTimeout <= kMaxCommissionioningTimeoutSeconds, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(OpenBasicCommissioningWindow(ResetFabrics::kNo, commissioningTimeout) == CHIP_NO_ERROR,
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

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    ChipLogProgress(Zcl, "Received command to close commissioning window");
    ClosePairingWindow();
    ChipLogProgress(Zcl, "Commissioning window is now closed");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
