/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "chef-supported-modes-manager.h"

#if MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

using namespace chip;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::app::Clusters::ModeSelect::Chef;

CHIP_ERROR ChefSupportedModesManager::AddModeOptionsProvider(EndpointId endpoint, ModeOptionsProvider optionsProvider)
{
    if (mModeOptionsPerEndpoint.count(endpoint) > 0)
    {
        return CHIP_ERROR_ENDPOINT_EXISTS;
    }
    mModeOptionsPerEndpoint[endpoint] = optionsProvider;
    return CHIP_NO_ERROR;
}

SupportedModesManager::ModeOptionsProvider ChefSupportedModesManager::getModeOptionsProvider(EndpointId endpointId) const
{
    auto it = mModeOptionsPerEndpoint.find(endpointId);
    if (it == mModeOptionsPerEndpoint.end())
    {
        return ModeOptionsProvider();
    }
    return it->second;
}

Protocols::InteractionModel::Status ChefSupportedModesManager::getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                                                   const ModeOptionStructType ** dataPtr) const
{
    SupportedModesManager::ModeOptionsProvider provider = getModeOptionsProvider(endpointId);
    if (provider.begin() == nullptr || provider.end() == nullptr)
    {
        return Protocols::InteractionModel::Status::NotFound;
    }

    for (const auto & option : provider)
    {
        if (option.mode == mode)
        {
            *dataPtr = &option;
            return Protocols::InteractionModel::Status::Success;
        }
    }
    return Protocols::InteractionModel::Status::NotFound;
}

#endif // MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT
