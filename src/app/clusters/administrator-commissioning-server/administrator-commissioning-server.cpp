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
#include <app/common/gen/af-structs.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

bool emberAfAdministratorCommissioningClusterOpenCommissioningWindowCallback(app::CommandHandler * commandObj,
                                                                             uint16_t CommissioningTimeout, ByteSpan PAKEVerifier,
                                                                             uint16_t Discriminator, uint32_t Iterations,
                                                                             ByteSpan Salt, uint16_t PasscodeID)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAdministratorCommissioningClusterOpenBasicCommissioningWindowCallback(app::CommandHandler * commandObj,
                                                                                  uint16_t CommissioningTimeout)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAdministratorCommissioningClusterRevokeCommissioningCallback(app::CommandHandler * commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
