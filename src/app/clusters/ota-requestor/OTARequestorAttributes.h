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

/* This file contains the declaration for the OTARequestorAttributes class, which
 * stores most attributes in memory and notifies when an attribute changes. The
 * DefaultOTAProviders attribute is handled by ProviderLocationList instead.
 */

#pragma once

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/clusters/ota-requestor/OTARequestorStorage.h>
#include <app/data-model-provider/EventsGenerator.h>
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {

// Class that tracks the non-array attributes in the OTA Requestor cluster. Marks
// attributes dirty when they're changed. Updates the storage when the default OTA
// providers list changes. Generates events when the update state changes. All
// methods must be invoked with the Matter lock held.
class OTARequestorAttributes
{
public:
    using OTAChangeReasonEnum  = app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;
    using OTAUpdateStateEnum   = OTARequestorInterface::OTAUpdateStateEnum;
    using ProviderLocationType = OTARequestorInterface::ProviderLocationType;

    class AttributeChangeListener
    {
    public:
        virtual ~AttributeChangeListener()                     = default;
        virtual void AttributeChanged(AttributeId attributeId) = 0;
    };

    OTAUpdateStateEnum GetUpdateState() const;
    // If the events generator is set this will also generate a StateTransition event.
    void SetUpdateState(OTAUpdateStateEnum updateState, OTAChangeReasonEnum reason,
                        app::DataModel::Nullable<uint32_t> targetSoftwareVersion);

    app::DataModel::Nullable<uint8_t> GetUpdateStateProgress() const;
    // Returns an error if the new progress is out of range.
    CHIP_ERROR SetUpdateStateProgress(app::DataModel::Nullable<uint8_t> updateStateProgress);

    bool GetUpdatePossible() const;
    void SetUpdatePossible(bool updatePossible);

    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation);
    CHIP_ERROR RemoveDefaultOtaProvider(FabricIndex fabricIndex);
    // Retrieves an iterator to the list of default OTA providers. This iterator is no longer safe to use after a call to
    // AddDefaultOtaProvider or RemoveDefaultOtaProvider.
    ProviderLocationList::Iterator GetDefaultOtaProviderListIterator();

    CHIP_ERROR SetInteractionModelContext(EndpointId endpointId, AttributeChangeListener & attributeChangeListener,
                                          app::DataModel::EventsGenerator & eventsGenerator);
    // This loads the default OTA provider list and update state attributes from storage. This will not generate events.
    CHIP_ERROR SetStorageAndLoadAttributes(OTARequestorStorage & storage);

private:
    ProviderLocationList mProviders;
    OTAUpdateStateEnum mUpdateState = OTAUpdateStateEnum::kUnknown;
    app::DataModel::Nullable<uint8_t> mUpdateStateProgress;
    bool mUpdatePossible = true;

    AttributeChangeListener * mAttributeChangeListener = nullptr;
    EndpointId mEndpointId                             = kInvalidEndpointId;
    app::DataModel::EventsGenerator * mEventsGenerator = nullptr;

    OTARequestorStorage * mStorage = nullptr;
};

} // namespace chip
