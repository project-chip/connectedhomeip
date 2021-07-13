/**
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
 * @brief Routines for the Application Launcher plugin, the
 *server implementation of the Application Launcher cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/CommandHandler.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>
#include <string>

bool applicationBasicClusterChangeApplicationStatus(EmberAfApplicationBasicStatus status, chip::EndpointId endpoint);

bool emberAfApplicationBasicClusterChangeStatusCallback(chip::app::CommandHandler * commandObj, uint8_t newApplicationStatus)
{
    bool success = applicationBasicClusterChangeApplicationStatus(static_cast<EmberAfApplicationBasicStatus>(newApplicationStatus),
                                                                  emberAfCurrentEndpoint());
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
