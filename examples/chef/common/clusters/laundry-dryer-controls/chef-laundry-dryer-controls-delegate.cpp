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

#include "chef-laundry-dryer-controls-delegate.h"
#include <app/clusters/laundry-dryer-controls-server/laundry-dryer-controls-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryDryerControls {

CHIP_ERROR ChefDelegate::GetSupportedDrynessLevelAtIndex(size_t index, DrynessLevelEnum & supportedDryness)
{
    if (index >= ArraySize(mSupportedDrynessLevels))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    supportedDryness = mSupportedDrynessLevels[index];
    return CHIP_NO_ERROR;
}

void ChefDelegate::Register(EndpointId endpoint)
{
    LaundryDryerControlsServer::SetDefaultDelegate(endpoint, this);
}

} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip
