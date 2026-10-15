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
#include <app/data-model-provider/Provider.h>
#include <app/data-model/Nullable.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <platform/LockTracker.h>

namespace chip {

using namespace app::Clusters::OtaSoftwareUpdateRequestor::Attributes;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Events;

OTARequestorAttributes::OTAUpdateStateEnum OTARequestorAttributes::GetUpdateState() const
{
    assertChipStackLockedByCurrentThread();
    return mUpdateState;
}

void OTARequestorAttributes::SetUpdateState(OTAUpdateStateEnum updateState, OTAChangeReasonEnum reason,
                                            app::DataModel::Nullable<uint32_t> targetSoftwareVersion)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(updateState != mUpdateState);

    OTAUpdateStateEnum previousState = mUpdateState;
    mUpdateState                     = updateState;
    if (mAttributeChangeListener)
    {
        mAttributeChangeListener->AttributeChanged(UpdateState::Id);
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

app::DataModel::Nullable<uint8_t> OTARequestorAttributes::GetUpdateStateProgress() const
{
    assertChipStackLockedByCurrentThread();
    return mUpdateStateProgress;
}

CHIP_ERROR OTARequestorAttributes::SetUpdateStateProgress(app::DataModel::Nullable<uint8_t> updateStateProgress)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(updateStateProgress.IsNull() || updateStateProgress.Value() <= 100, CHIP_ERROR_INVALID_ARGUMENT);

    if (mUpdateStateProgress.Update(updateStateProgress) && mAttributeChangeListener)
    {
        mAttributeChangeListener->AttributeChanged(UpdateStateProgress::Id);
    }
    return CHIP_NO_ERROR;
}

bool OTARequestorAttributes::GetUpdatePossible() const
{
    assertChipStackLockedByCurrentThread();
    return mUpdatePossible;
}

void OTARequestorAttributes::SetUpdatePossible(bool updatePossible)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(updatePossible != mUpdatePossible);

    mUpdatePossible = updatePossible;
    if (mAttributeChangeListener)
    {
        mAttributeChangeListener->AttributeChanged(UpdatePossible::Id);
    }
}

CHIP_ERROR OTARequestorAttributes::SetInteractionModelContext(EndpointId endpointId,
                                                              AttributeChangeListener & attributeChangeListener,
                                                              app::DataModel::EventsGenerator & eventsGenerator)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(IsValidEndpointId(endpointId), CHIP_ERROR_INVALID_ARGUMENT);

    mEndpointId              = endpointId;
    mAttributeChangeListener = &attributeChangeListener;
    mEventsGenerator         = &eventsGenerator;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::RemoveDefaultOtaProvider(FabricIndex fabricIndex)
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR error = mProviders.Delete(fabricIndex);
    // If no entry for the associated fabric index was found then the attribute hasn't changed and no further processing is needed.
    if (error == CHIP_ERROR_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(error);

    if (mAttributeChangeListener)
    {
        mAttributeChangeListener->AttributeChanged(DefaultOTAProviders::Id);
    }
    if (mStorage)
    {
        ReturnErrorOnFailure(mStorage->StoreDefaultProviders(mProviders));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorAttributes::AddDefaultOtaProvider(const ProviderLocationType & providerLocation)
{
    assertChipStackLockedByCurrentThread();

    // Look for an entry with the same fabric index indicated
    auto iterator = mProviders.Begin();
    while (iterator.Next())
    {
        ProviderLocationType location = iterator.GetValue();
        if (location.GetFabricIndex() == providerLocation.GetFabricIndex())
        {
            ChipLogError(SoftwareUpdate, "Default OTA provider entry with fabric index %d already exists",
                         location.GetFabricIndex());
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    ReturnErrorOnFailure(mProviders.Add(providerLocation));

    if (mAttributeChangeListener)
    {
        mAttributeChangeListener->AttributeChanged(DefaultOTAProviders::Id);
    }
    if (mStorage)
    {
        ReturnErrorOnFailure(mStorage->StoreDefaultProviders(mProviders));
    }
    return CHIP_NO_ERROR;
}

ProviderLocationList::Iterator OTARequestorAttributes::GetDefaultOtaProviderListIterator()
{
    assertChipStackLockedByCurrentThread();
    return mProviders.Begin();
}

CHIP_ERROR OTARequestorAttributes::SetStorageAndLoadAttributes(OTARequestorStorage & storage)
{
    assertChipStackLockedByCurrentThread();
    mStorage = &storage;
    if (mStorage->LoadCurrentUpdateState(mUpdateState) != CHIP_NO_ERROR)
    {
        mUpdateState = OTAUpdateStateEnum::kIdle;
    }
    return mStorage->LoadDefaultProviders(mProviders);
}

} // namespace chip
