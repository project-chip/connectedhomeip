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

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/laundry-dryer-controls-server/laundry-dryer-controls-delegate.h>
#include <app/util/config.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#if MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryDryerControls {
namespace Chef {

/**
 * @brief The ChefDelegate class implements the LaundryDryerControls::Delegate.
 */
class ChefDelegate : public Delegate
{
public:
    ChefDelegate()          = default;
    ~ChefDelegate() override = default;

    /**
     * @brief Get the supported dryness value at the given index in the list.
     * @param index The index of the supported dryness with 0 representing the first one.
     * @param supportedDryness The supported dryness at the given index
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the list of supported dryness.
     */
    CHIP_ERROR GetSupportedDrynessLevelAtIndex(size_t index, DrynessLevelEnum & supportedDryness) override;

    /**
     * @brief Register the delegate for the given endpoint.
     * @param endpoint The endpoint to register the delegate for.
     */
    void Register(EndpointId endpoint);

private:
    const DrynessLevelEnum mSupportedDrynessLevels[3] = { DrynessLevelEnum::kLow, DrynessLevelEnum::kNormal,
                                                          DrynessLevelEnum::kExtra };
};

} // namespace Chef
} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // #if MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT
