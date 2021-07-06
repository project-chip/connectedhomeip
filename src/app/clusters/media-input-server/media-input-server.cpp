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
 * @brief Routines for the Media Input plugin, the
 *server implementation of the Media Input cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/Command.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <string>

bool mediaInputClusterSelectInput(uint8_t input);
bool mediaInputClusterShowInputStatus();
bool mediaInputClusterHideInputStatus();
bool mediaInputClusterRenameInput(uint8_t input, std::string name);

static void storeCurrentInput(chip::EndpointId endpoint, uint8_t currentInput)
{
    EmberAfStatus status = emberAfWriteServerAttribute(
        endpoint, ZCL_MEDIA_INPUT_CLUSTER_ID, ZCL_MEDIA_INPUT_CURRENT_INPUT_ATTRIBUTE_ID, &currentInput, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

bool emberAfMediaInputClusterSelectInputCallback(chip::app::Command * command, uint8_t input)
{
    bool success         = mediaInputClusterSelectInput(input);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    if (success)
    {
        storeCurrentInput(emberAfCurrentEndpoint(), input);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterShowInputStatusCallback(chip::app::Command * command)
{
    bool success         = mediaInputClusterShowInputStatus();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterHideInputStatusCallback(chip::app::Command * command)
{
    bool success         = mediaInputClusterHideInputStatus();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterRenameInputCallback(chip::app::Command * command, uint8_t input, uint8_t * name)
{
    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string nameString(reinterpret_cast<char *>(name));
    bool success         = mediaInputClusterRenameInput(input, nameString);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
