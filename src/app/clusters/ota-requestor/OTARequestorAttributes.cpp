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

#include <app/clusters/ota-requestor/OTARequestorStorage.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <app/data-model/Nullable.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

namespace chip {

using namespace app::Clusters::OtaSoftwareUpdateRequestor::Attributes;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Events;

constexpr ClusterId kClusterId = app::Clusters::OtaSoftwareUpdateRequestor::Id;

void OTARequestorAttributes::SetUpdateState(OTAUpdateStateEnum updateState, OTAChangeReasonEnum reason,
                                            app::DataModel::Nullable<uint32_t> targetSoftwareVersion)
{
    VerifyOrReturn(updateState != mUpdateState);

    OTAUpdateStateEnum previousState = mUpdateState;
    mUpdateState                     = updateState;
    if (mDataModelChangeListener)
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, UpdateState::Id });
    }
    if (mEventsGenerator)
    {
        StateTransition::Type event;
        event.previousState         = previousState;
        event.newState              = updateState;
        event.reason                = reason;
        event.targetSoftwareVersion = targetSoftwareVersion;
        mEventsGenerator->GenerateEvent(event, mEndpointId);
    }
}

CHIP_ERROR OTARequestorAttributes::SetUpdateStateProgress(app::DataModel::Nullable<uint8_t> updateStateProgress)
{
    VerifyOrReturnError(updateStateProgress.IsNull() || updateStateProgress.Value() <= 100, CHIP_ERROR_INVALID_ARGUMENT);

    if (mUpdateStateProgress.Update(updateStateProgress) && mDataModelChangeListener)
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, UpdateStateProgress::Id });
    }
    return CHIP_NO_ERROR;
}

void OTARequestorAttributes::SetUpdatePossible(bool updatePossible)
{
    VerifyOrReturn(updatePossible != mUpdatePossible);

    mUpdatePossible = updatePossible;
    if (mDataModelChangeListener)
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, UpdatePossible::Id });
    }
}

CHIP_ERROR OTARequestorAttributes::SetInteractionModelContext(EndpointId endpointId,
                                                              app::DataModel::ProviderChangeListener & dataModelChangeListener,
                                                              app::DataModel::EventsGenerator & eventsGenerator)
{
    VerifyOrReturnError(IsValidEndpointId(endpointId), CHIP_ERROR_INVALID_ARGUMENT);

    mDataModelChangeListener = &dataModelChangeListener;
    mEndpointId              = endpointId;
    mEventsGenerator         = &eventsGenerator;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::RemoveDefaultOtaProvider(FabricIndex fabricIndex)
{
    CHIP_ERROR error = mProviders.Delete(fabricIndex);

    // Ignore the error if no entry for the associated fabric index has been found.
    VerifyOrReturnError(error != CHIP_ERROR_NOT_FOUND, CHIP_NO_ERROR);
    ReturnErrorOnFailure(error);

    if (mDataModelChangeListener)
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, DefaultOTAProviders::Id });
    }
    if (mStorage)
    {
        ReturnErrorOnFailure(mStorage->StoreDefaultProviders(mProviders));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::AddDefaultOtaProvider(const ProviderLocationType & providerLocation)
{
    // Look for an entry with the same fabric index indicated
    auto iterator = mProviders.Begin();
    while (iterator.Next())
    {
        ProviderLocationType location = iterator.GetValue();
        if (location.GetFabricIndex() == providerLocation.GetFabricIndex())
        {
            ChipLogError(SoftwareUpdate, "Default OTA provider entry with fabric %d already exists", location.GetFabricIndex());
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    ReturnErrorOnFailure(mProviders.Add(providerLocation));

    if (mDataModelChangeListener)
    {
        mDataModelChangeListener->MarkDirty({ mEndpointId, kClusterId, DefaultOTAProviders::Id });
    }
    if (mStorage)
    {
        ReturnErrorOnFailure(mStorage->StoreDefaultProviders(mProviders));
    }
    return CHIP_NO_ERROR;
}

ProviderLocationList::Iterator OTARequestorAttributes::GetDefaultOtaProviderListIterator()
{
    return mProviders.Begin();
}

CHIP_ERROR OTARequestorAttributes::SetStorageAndLoadAttributes(OTARequestorStorage & storage)
{
    mStorage = &storage;
    if (mStorage->LoadCurrentUpdateState(mUpdateState) != CHIP_NO_ERROR)
    {
        mUpdateState = OTAUpdateStateEnum::kIdle;
    }
    return mStorage->LoadDefaultProviders(mProviders);
}

} // namespace chip
