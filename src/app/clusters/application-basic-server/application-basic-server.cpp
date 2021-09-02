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

#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <string>

using namespace chip;

bool applicationBasicClusterChangeApplicationStatus(EmberAfApplicationBasicStatus status, EndpointId endpoint);

bool emberAfApplicationBasicClusterChangeStatusCallback(EndpointId endpoint, app::CommandHandler * commandObj,
                                                        uint8_t newApplicationStatus)
{
    bool success = applicationBasicClusterChangeApplicationStatus(static_cast<EmberAfApplicationBasicStatus>(newApplicationStatus),
                                                                  emberAfCurrentEndpoint());
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
