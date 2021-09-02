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
 * @brief Routines for the Audio Output plugin, the
 *server implementation of the Audio Output cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

bool audioOutputClusterSelectOutput(uint8_t index);
bool audioOutputClusterRenameOutput(uint8_t index, uint8_t * name);

bool emberAfAudioOutputClusterRenameOutputCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t index,
                                                   uint8_t * name)
{
    bool success         = audioOutputClusterRenameOutput(index, name);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAudioOutputClusterSelectOutputCallback(EndpointId endpoint, app::CommandHandler * command, uint8_t index)
{
    bool success         = audioOutputClusterSelectOutput(index);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
