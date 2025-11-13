/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/clusters/laundry-dryer-controls-server/laundry-dryer-controls-server.h>
#include <app/util/config.h>
#include <laundry-dryer-controls-delegate-impl.h>

using namespace chip;
using namespace chip::app::Clusters::LaundryDryerControls;

const DrynessLevelEnum LaundryDryerControlDelegate::supportedDrynessLevelOptions[] = { DrynessLevelEnum::kLow,
                                                                                       DrynessLevelEnum::kNormal,
                                                                                       DrynessLevelEnum::kMax };

LaundryDryerControlDelegate LaundryDryerControlDelegate::instance;

// TODO: Add EndpointId to the API so that different values per endpoint may be possible in some implementations.
CHIP_ERROR LaundryDryerControlDelegate::GetSupportedDrynessLevelAtIndex(size_t index, DrynessLevelEnum & supportedDrynessLevel)
{
    if (index >= MATTER_ARRAY_SIZE(supportedDrynessLevelOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    supportedDrynessLevel = supportedDrynessLevelOptions[index];
    return CHIP_NO_ERROR;
}
