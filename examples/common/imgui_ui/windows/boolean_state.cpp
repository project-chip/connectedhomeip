/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "boolean_state.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

void BooleanState::UpdateState()
{
    if (mTargetState.HasValue())
    {
        // TODO: if src/app/clusters/boolean-state exists, we should use its
        //       mutation API.
        //
        // See  https://github.com/project-chip/connectedhomeip/issues/25225 for
        // the feature request asking for a BooleanState HAL.
        chip::app::Clusters::BooleanState::Attributes::StateValue::Set(mEndpointId, mTargetState.Value());
        mTargetState.ClearValue();
    }

    chip::app::Clusters::BooleanState::Attributes::StateValue::Get(mEndpointId, &mState);
}

void BooleanState::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    bool uiState = mState;
    ImGui::Checkbox("State Value", &uiState);

    if (uiState != mState)
    {
        // toggle value on the next 'UpdateState' call
        mTargetState.SetValue(uiState);
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
