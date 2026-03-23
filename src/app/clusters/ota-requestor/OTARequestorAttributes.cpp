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

#include <app/clusters/ota-requestor/OTARequestorAttributes.h>

#include <app/data-model/Nullable.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

namespace chip {

namespace Attributes = app::Clusters::OtaSoftwareUpdateRequestor::Attributes;

constexpr ClusterId kClusterId = app::Clusters::OtaSoftwareUpdateRequestor::Id;

CHIP_ERROR OTARequestorAttributes::SetUpdateState(OTAUpdateStateEnum updateState)
{
    VerifyOrReturnError(mDataModelChangeListener != nullptr, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(updateState != mUpdateState, CHIP_NO_ERROR);

    mUpdateState = updateState;
    mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, Attributes::UpdateState::Id });
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::SetUpdateStateProgress(app::DataModel::Nullable<uint8_t> updateStateProgress)
{
    VerifyOrReturnError(mDataModelChangeListener != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(updateStateProgress.IsNull() || updateStateProgress.Value() <= 100, CHIP_ERROR_INVALID_ARGUMENT);

    if (mUpdateStateProgress.Update(updateStateProgress))
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, Attributes::UpdateStateProgress::Id });
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::SetUpdatePossible(bool updatePossible)
{
    VerifyOrReturnError(mDataModelChangeListener != nullptr, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(updatePossible != mUpdatePossible, CHIP_NO_ERROR);

    mUpdatePossible = updatePossible;
    mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, Attributes::UpdatePossible::Id });
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::Init(app::DataModel::ProviderChangeListener & dataModelChangeListener, EndpointId endpointId)
{
    VerifyOrReturnError(mDataModelChangeListener == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidEndpointId(endpointId), CHIP_ERROR_INVALID_ARGUMENT);

    mDataModelChangeListener = &dataModelChangeListener;
    mEndpointId = endpointId;
    return CHIP_NO_ERROR;
}

} // namespace chip
