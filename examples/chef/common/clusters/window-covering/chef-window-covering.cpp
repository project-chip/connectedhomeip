/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "chef-window-covering.h"
#include "app/clusters/window-covering-server/CodegenIntegration.h"
#include "app/clusters/window-covering-server/window-covering-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace ChefWindowCovering {

constexpr size_t kWindowCoveringDelegateTableSize = MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT;
static_assert(kWindowCoveringDelegateTableSize <= kEmberInvalidEndpointIndex, "WindowCovering Delegate table size error");

std::unique_ptr<WindowCovering::ChefDelegate> gDelegateTable[kWindowCoveringDelegateTableSize];

WindowCovering::ChefDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WindowCovering::Id, MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kWindowCoveringDelegateTableSize ? nullptr : gDelegateTable[ep].get());
}

void InitChefWindowCoveringCluster()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == kInvalidEndpointId)
        {
            continue;
        }

        // Check if endpoint has WindowCovering cluster enabled
        uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpointId, WindowCovering::Id,
                                                                MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
        if (epIndex >= kWindowCoveringDelegateTableSize)
            continue;

        // Skip if delegate was already initialized.
        if (gDelegateTable[epIndex])
            continue;

        gDelegateTable[epIndex] = std::make_unique<WindowCovering::ChefDelegate>();
        gDelegateTable[epIndex]->SetEndpoint(endpointId);
        WindowCovering::SetDefaultDelegate(endpointId, gDelegateTable[epIndex].get());
    }
}
} // namespace ChefWindowCovering

CHIP_ERROR WindowCovering::ChefDelegate::HandleMovement(WindowCoveringType type)
{
    app::DataModel::Nullable<Percent100ths> current;
    auto windowCoveringCluster = FindClusterOnEndpoint(mEndpoint);
    VerifyOrReturnValue(windowCoveringCluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (type == WindowCoveringType::Lift)
    {

        current = windowCoveringCluster->GetTargetPositionLiftPercent100ths();
        if (current.IsNull())
        {
            ChipLogError(DeviceLayer, "HandleMovement: Failed to get TargetPositionLiftPercent100ths");
            return CHIP_ERROR_READ_FAILED;
        }
        windowCoveringCluster->SetCurrentPositionLiftPercent100ths(current);

        return CHIP_NO_ERROR;
    }
    else if (type == WindowCoveringType::Tilt)
    {
        current = windowCoveringCluster->GetTargetPositionTiltPercent100ths();
        if (current.IsNull())
        {
            ChipLogError(DeviceLayer, "HandleMovement: Failed to get TargetPositionTiltPercent100ths");
            return CHIP_ERROR_READ_FAILED;
        }

        // Instant update. No transition for now.
        windowCoveringCluster->SetCurrentPositionTiltPercent100ths(current);

        return CHIP_NO_ERROR;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowCovering::ChefDelegate::HandleStopMotion()
{
    return CHIP_NO_ERROR;
}
