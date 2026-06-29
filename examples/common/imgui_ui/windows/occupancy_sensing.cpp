/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "occupancy_sensing.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

#include <app/clusters/occupancy-sensor-server/CodegenIntegration.h>
#include <optional>

namespace example {
namespace Ui {
namespace Windows {

using namespace chip::app::Clusters::OccupancySensing;
using chip::app::Clusters::OccupancySensingCluster;

void OccupancySensing::UpdateState()
{
    OccupancySensingCluster * cluster = FindClusterOnEndpoint(mEndpointId);
    VerifyOrReturn(cluster != nullptr);
    if (mTargetOccupied.has_value())
    {
        cluster->SetOccupancy(*mTargetOccupied);
        mTargetOccupied.reset();
    }

    mOccupied = cluster->IsOccupied();
}

void OccupancySensing::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    bool uiState = mOccupied;
    ImGui::Checkbox("Occupancy Value", &uiState);

    if (uiState != mOccupied)
    {
        // toggle value on the next 'UpdateState' call
        // Occupancy is just a single bit, update it as such
        mTargetOccupied = std::make_optional(uiState);
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
