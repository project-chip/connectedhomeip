/*
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
#include "LaundryWasherControlsCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/LaundryWasherControls/Metadata.h>

namespace chip::app::Clusters {

using namespace LaundryWasherControls;
using namespace LaundryWasherControls::Attributes;

DataModel::ActionReturnStatus LaundryWasherControlsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case SpinSpeeds::Id:
        return ReadSpinSpeeds(request.path, encoder);
    case SpinSpeedCurrent::Id:
        return encoder.Encode(mSpinSpeedCurrent);
    case NumberOfRinses::Id:
        return encoder.Encode(mNumberOfRinses);
    case SupportedRinses::Id:
        return ReadSupportedRinses(request.path, encoder);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus LaundryWasherControlsCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                           AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case SpinSpeedCurrent::Id: {
        DataModel::Nullable<uint8_t> spinSpeedCurrent;
        ReturnErrorOnFailure(decoder.Decode(spinSpeedCurrent));
        return SetSpinSpeedCurrent(spinSpeedCurrent);
    }
    case NumberOfRinses::Id: {
        NumberOfRinsesEnum numberOfRinses;
        ReturnErrorOnFailure(decoder.Decode(numberOfRinses));
        return SetNumberOfRinses(numberOfRinses);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR LaundryWasherControlsCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatures.Has(Feature::kSpin), SpinSpeedCurrent::kMetadataEntry },
        { mFeatures.Has(Feature::kSpin), SpinSpeeds::kMetadataEntry },
        { mFeatures.Has(Feature::kRinse), NumberOfRinses::kMetadataEntry },
        { mFeatures.Has(Feature::kRinse), SupportedRinses::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR LaundryWasherControlsCluster::SetSpinSpeedCurrent(DataModel::Nullable<uint8_t> spinSpeedCurrent)
{
    if (!mFeatures.Has(Feature::kSpin))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    ReturnErrorOnFailure(SpinSpeedIndexValidity(spinSpeedCurrent));

    if (SetAttributeValue(mSpinSpeedCurrent, spinSpeedCurrent, SpinSpeedCurrent::Id) && mDelegate != nullptr)
    {
        mDelegate->OnSpinSpeedCurrentChanged(spinSpeedCurrent);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryWasherControlsCluster::SetNumberOfRinses(NumberOfRinsesEnum numberOfRinses)
{
    if (!mFeatures.Has(Feature::kRinse))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    ReturnErrorOnFailure(NumberOfRinsesValidity(numberOfRinses));

    if (SetAttributeValue(mNumberOfRinses, numberOfRinses, NumberOfRinses::Id) && mDelegate != nullptr)
    {
        mDelegate->OnNumberOfRinsesChanged(numberOfRinses);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryWasherControlsCluster::SpinSpeedIndexValidity(DataModel::Nullable<uint8_t> spinSpeedCurrent)
{
    if (spinSpeedCurrent.IsNull())
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mDelegate != nullptr, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(spinSpeedCurrent.Value() < kMaxSpinSpeedsLength, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    char buffer[kMaxSpinSpeedLength];
    MutableCharSpan dummy(buffer);
    CHIP_ERROR err = mDelegate->GetSpinSpeedAtIndex(spinSpeedCurrent.Value(), dummy);
    VerifyOrReturnError(err != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    return err;
}

CHIP_ERROR LaundryWasherControlsCluster::NumberOfRinsesValidity(NumberOfRinsesEnum numberOfRinses)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_IM_GLOBAL_STATUS(InvalidInState));
    NumberOfRinsesEnum supportedRinse;
    for (uint8_t i = 0; i < kMaxSupportedRinsesLength && mDelegate->GetSupportedRinseAtIndex(i, supportedRinse) == CHIP_NO_ERROR;
         ++i)
    {
        if (numberOfRinses == supportedRinse)
        {
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_IM_GLOBAL_STATUS(InvalidInState);
}

CHIP_ERROR LaundryWasherControlsCluster::ReadSpinSpeeds(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; i < kMaxSpinSpeedsLength; i++)
        {
            char buffer[kMaxSpinSpeedLength];
            MutableCharSpan spinSpeed(buffer);
            auto err = mDelegate->GetSpinSpeedAtIndex(i, spinSpeed);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(spinSpeed));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR LaundryWasherControlsCluster::ReadSupportedRinses(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; i < kMaxSupportedRinsesLength; i++)
        {
            NumberOfRinsesEnum supportedRinse;
            auto err = mDelegate->GetSupportedRinseAtIndex(i, supportedRinse);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(supportedRinse));
        }
        return CHIP_NO_ERROR;
    });
}

} // namespace chip::app::Clusters
