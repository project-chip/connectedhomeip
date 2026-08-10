/**
 *
 *    Copyright (c) 2023 - 2026 Project CHIP Authors
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

#include "LaundryDryerControlsCluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/LaundryDryerControls/Ids.h>
#include <clusters/LaundryDryerControls/Metadata.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryDryerControls;
using namespace chip::app::Clusters::LaundryDryerControls::Attributes;

namespace chip {
namespace app {
namespace Clusters {

LaundryDryerControlsCluster::LaundryDryerControlsCluster(EndpointId endpointId, LaundryDryerControls::Delegate & delegate) :
    DefaultServerCluster({ endpointId, LaundryDryerControls::Id }), mDelegate(&delegate)
{}

LaundryDryerControlsCluster::LaundryDryerControlsCluster(EndpointId endpointId) :
    DefaultServerCluster({ endpointId, LaundryDryerControls::Id }), mDelegate(nullptr)
{}

LaundryDryerControlsCluster::~LaundryDryerControlsCluster() = default;

void LaundryDryerControlsCluster::SetDelegate(LaundryDryerControls::Delegate & delegate)
{
    mDelegate = &delegate;
}

Protocols::InteractionModel::Status
LaundryDryerControlsCluster::SetSelectedDrynessLevel(DataModel::Nullable<DrynessLevelEnum> drynessLevel)
{
    using Status = Protocols::InteractionModel::Status;

    VerifyOrReturnValue(mContext != nullptr, Status::InvalidInState);

    // A null value clears the selection and is always valid. A non-null value must be one of the levels
    // advertised by SupportedDrynessLevels. This mirrors the validation the legacy
    // MatterLaundryDryerControlsClusterServerPreAttributeChangedCallback performed against the delegate.
    if (!drynessLevel.IsNull())
    {
        VerifyOrReturnValue(IsSupportedDrynessLevel(drynessLevel.Value()), Status::ConstraintError);
    }

    // Update + notify only when the value actually changes.
    const bool changed = SetAttributeValue(mSelectedDrynessLevel, drynessLevel, SelectedDrynessLevel::Id);

    if (changed)
    {
        AttributePersistence attrPersistence{ mContext->attributeStorage };
        CHIP_ERROR err = attrPersistence.StoreNativeEndianValue<DrynessLevelEnum>(
            ConcreteAttributePath(mPath.mEndpointId, LaundryDryerControls::Id, SelectedDrynessLevel::Id), mSelectedDrynessLevel);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "LaundryDryerControls: Failed to persist SelectedDrynessLevel: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    return Status::Success;
}

DataModel::Nullable<DrynessLevelEnum> LaundryDryerControlsCluster::GetSelectedDrynessLevel() const
{
    return mSelectedDrynessLevel;
}

CHIP_ERROR LaundryDryerControlsCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryDryerControlsCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    // This cluster only has Mandatory attributes
    return listBuilder.Append(Span(LaundryDryerControls::Attributes::kMandatoryMetadata), {});
}

DataModel::ActionReturnStatus LaundryDryerControlsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(LaundryDryerControls::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case SupportedDrynessLevels::Id:
        return encoder.EncodeList(
            [this](const auto & subEncoder) -> CHIP_ERROR { return this->EncodeSupportedDrynessLevels(subEncoder); });
    case SelectedDrynessLevel::Id:
        return encoder.Encode(mSelectedDrynessLevel);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus LaundryDryerControlsCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case SelectedDrynessLevel::Id: {
        DataModel::Nullable<DrynessLevelEnum> newValue;
        ReturnErrorOnFailure(decoder.Decode(newValue));
        return DataModel::ActionReturnStatus(SetSelectedDrynessLevel(newValue));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

// Gets the supported dryness levels one by one from the delegate and encodes them into a list.
CHIP_ERROR LaundryDryerControlsCluster::EncodeSupportedDrynessLevels(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    for (size_t i = 0; true; i++)
    {
        DrynessLevelEnum supportedDrynessLevel;
        auto err = mDelegate->GetSupportedDrynessLevelAtIndex(i, supportedDrynessLevel);

        // Return once we've run off the end of the supported dryness level list on the delegate.
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(encoder.Encode(supportedDrynessLevel));
    }
}

// Checks whether the given dryness level is one of the levels advertised by the delegate.
bool LaundryDryerControlsCluster::IsSupportedDrynessLevel(DrynessLevelEnum drynessLevel)
{
    VerifyOrReturnValue(mDelegate != nullptr, false);

    for (size_t i = 0; true; i++)
    {
        DrynessLevelEnum supportedDrynessLevel;
        auto err = mDelegate->GetSupportedDrynessLevelAtIndex(i, supportedDrynessLevel);

        // Either we ran off the end of the list (CHIP_ERROR_PROVIDER_LIST_EXHAUSTED) or the delegate
        // could not provide the list; in both cases the value is not a supported option.
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }
        if (supportedDrynessLevel == drynessLevel)
        {
            return true;
        }
    }
}

void LaundryDryerControlsCluster::LoadPersistentAttributes()
{
    AttributePersistence attrPersistence{ mContext->attributeStorage };

    // Load SelectedDrynessLevel
    const DataModel::Nullable<DrynessLevelEnum> defaultDrynessLevel = mSelectedDrynessLevel;
    if (!attrPersistence.LoadNativeEndianValue<DrynessLevelEnum>(
            ConcreteAttributePath(mPath.mEndpointId, LaundryDryerControls::Id, SelectedDrynessLevel::Id), mSelectedDrynessLevel,
            defaultDrynessLevel))
    {
        // otherwise keep the default
        ChipLogDetail(Zcl, "LaundryDryerControls: Unable to load the SelectedDrynessLevel attribute from the KVS. Using default.");
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
