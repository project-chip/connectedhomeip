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
#include <app/data-model/Nullable.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {

// Class that tracks the non-array attributes in the OTA Requestor cluster. Marks
// attributes dirty when they're changed.
class OTARequestorAttributes
{
public:
    using OTAUpdateStateEnum   = OTARequestorInterface::OTAUpdateStateEnum;
    using ProviderLocationType = OTARequestorInterface::ProviderLocationType;

    OTAUpdateStateEnum GetUpdateState() const { return mUpdateState; }
    void SetUpdateState(OTAUpdateStateEnum updateState);

    app::DataModel::Nullable<uint8_t> GetUpdateStateProgress() const { return mUpdateStateProgress; }
    // Returns an error if the new progress is out of range.
    CHIP_ERROR SetUpdateStateProgress(app::DataModel::Nullable<uint8_t> updateStateProgress);

    bool GetUpdatePossible() const { return mUpdatePossible; }
    void SetUpdatePossible(bool updatePossible);

    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex);
    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation);
    ProviderLocationList::Iterator GetDefaultOtaProviderListIterator();

    CHIP_ERROR SetChangeListener(EndpointId endpointId, app::DataModel::ProviderChangeListener & dataModelChangeListener);

private:
    ProviderLocationList mProviders;
    OTAUpdateStateEnum mUpdateState = OTAUpdateStateEnum::kUnknown;
    app::DataModel::Nullable<uint8_t> mUpdateStateProgress;
    bool mUpdatePossible = true;

    app::DataModel::ProviderChangeListener * mDataModelChangeListener = nullptr;
    EndpointId mEndpointId = kInvalidEndpointId;
};

} // namespace chip
