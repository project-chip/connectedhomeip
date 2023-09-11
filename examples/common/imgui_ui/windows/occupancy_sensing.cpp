/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "occupancy_sensing.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

using chip::app::Clusters::OccupancySensing::OccupancyBitmap;

void OccupancySensing::UpdateState()
{
    if (mTargetOccupancy.HasValue())
    {
        chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Set(mEndpointId, mTargetOccupancy.Value());
        mTargetOccupancy.ClearValue();
    }

    chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Get(mEndpointId, &mOccupancy);
}

void OccupancySensing::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    bool occupied = mOccupancy.Has(OccupancyBitmap::kOccupied);
    bool uiState  = occupied;
    ImGui::Checkbox("Occupancy Value", &uiState);

    if (uiState != occupied)
    {
        // toggle value on the next 'UpdateState' call
        // Occupancy is just a single bit, update it as such
        mTargetOccupancy.SetValue(uiState ? BitMask(OccupancyBitmap::kOccupied) : BitMask());
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
