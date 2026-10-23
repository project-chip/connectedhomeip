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
#include "boolean_state.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

// TODO: Ideally we should not depend on the codegen integration
// It would be best if we could use generic cluster API instead
#include <app/clusters/boolean-state-server/CodegenIntegration.h>

namespace example {
namespace Ui {
namespace Windows {

void BooleanState::UpdateState()
{
    auto booleanState = chip::app::Clusters::BooleanState::FindClusterOnEndpoint(mEndpointId);
    VerifyOrReturn(booleanState != nullptr);

    if (mTargetState.HasValue())
    {
        booleanState->SetStateValue(mTargetState.Value());
        mTargetState.ClearValue();
    }

    mState = booleanState->GetStateValue();
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
