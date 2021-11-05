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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <string>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaInput;

bool mediaInputClusterSelectInput(uint8_t input);
bool mediaInputClusterShowInputStatus();
bool mediaInputClusterHideInputStatus();
bool mediaInputClusterRenameInput(uint8_t input, std::string name);

static void storeCurrentInput(EndpointId endpoint, uint8_t currentInput)
{
    EmberAfStatus status = Attributes::CurrentMediaInput::Set(endpoint, currentInput);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

bool emberAfMediaInputClusterSelectInputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::SelectInput::DecodableType & commandData)
{
    auto & input = commandData.index;

    bool success         = mediaInputClusterSelectInput(input);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    if (success)
    {
        storeCurrentInput(emberAfCurrentEndpoint(), input);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterShowInputStatusCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::ShowInputStatus::DecodableType & commandData)
{
    bool success         = mediaInputClusterShowInputStatus();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterHideInputStatusCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::HideInputStatus::DecodableType & commandData)
{
    bool success         = mediaInputClusterHideInputStatus();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfMediaInputClusterRenameInputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RenameInput::DecodableType & commandData)
{
    auto & input = commandData.index;
    auto & name  = commandData.name;

    // TODO: char is not null terminated, verify this code once #7963 gets merged.
    std::string nameString(name.data(), name.size());
    bool success         = mediaInputClusterRenameInput(input, nameString);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterMediaInputPluginServerInitCallback() {}
