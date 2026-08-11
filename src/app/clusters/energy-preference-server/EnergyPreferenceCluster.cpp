/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "EnergyPreferenceCluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/EnergyPreference/Metadata.h>
#include <clusters/EnergyPreference/Structs.h>

namespace chip::app::Clusters {

using namespace EnergyPreference;
using namespace EnergyPreference::Attributes;
using namespace EnergyPreference::Structs;

CHIP_ERROR EnergyPreferenceCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence persistence(context.attributeStorage);

    // These are indexes, so 0 is picked as fallback.
    constexpr uint8_t kCurrentenergyBalanceFallback = 0;
    constexpr uint8_t kCurrentLowPowerModeSensitivityFallback = 0;

    persistence.LoadNativeEndianValue({ GetEndpointId(), mPath.mClusterId, CurrentEnergyBalance::Id }, mCurrentEnergyBalance,
                                      kCurrentenergyBalanceFallback);
    persistence.LoadNativeEndianValue({ GetEndpointId(), mPath.mClusterId, CurrentLowPowerModeSensitivity::Id },
                                      mCurrentLowPowerModeSensitivity, kCurrentLowPowerModeSensitivityFallback);

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus EnergyPreferenceCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case EnergyBalances::Id:
        return ReadEnergyBalances(request.path, encoder);
    case EnergyPriorities::Id:
        return ReadEnergyPriorities(request.path, encoder);
    case LowPowerModeSensitivities::Id:
        return ReadLowPowerModeSensitivities(request.path, encoder);
    case CurrentEnergyBalance::Id:
        return encoder.Encode(mCurrentEnergyBalance);
    case CurrentLowPowerModeSensitivity::Id:
        return encoder.Encode(mCurrentLowPowerModeSensitivity);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus EnergyPreferenceCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case CurrentEnergyBalance::Id: {
        uint8_t currentEnergyBalance;
        ReturnErrorOnFailure(decoder.Decode(currentEnergyBalance));
        return SetCurrentEnergyBalance(currentEnergyBalance);
    }
    case CurrentLowPowerModeSensitivity::Id: {
        uint8_t currentLowPowerModeSensitivity;
        ReturnErrorOnFailure(decoder.Decode(currentLowPowerModeSensitivity));
        return SetCurrentLowPowerModeSensitivity(currentLowPowerModeSensitivity);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR EnergyPreferenceCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatures.Has(Feature::kEnergyBalance), EnergyBalances::kMetadataEntry },
        { mFeatures.Has(Feature::kEnergyBalance), EnergyPriorities::kMetadataEntry },
        { mFeatures.Has(Feature::kLowPowerModeSensitivity), LowPowerModeSensitivities::kMetadataEntry },
        { mFeatures.Has(Feature::kEnergyBalance), CurrentEnergyBalance::kMetadataEntry },
        { mFeatures.Has(Feature::kLowPowerModeSensitivity), CurrentLowPowerModeSensitivity::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR EnergyPreferenceCluster::SetCurrentUint8Attribute(uint8_t & attributeValue, uint8_t newValue, size_t correspondingArraySize, AttributeId attributeId, Feature featureGate, OnCurrentUint8AttributeChangedCallback onChangedCallback)
{
    if (!mFeatures.Has(featureGate))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (newValue >= correspondingArraySize)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (SetAttributeValue(attributeValue, newValue, attributeId))
    {
        (sDelegate->*onChangedCallback)(GetEndpointId(), newValue);
        if (mContext != nullptr)
        {
            AttributePersistence persistence(mContext->attributeStorage);
            ReturnErrorOnFailure(persistence.StoreNativeEndianValue({ GetEndpointId(), mPath.mClusterId, attributeId }, newValue));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyPreferenceCluster::ReadBalanceStructListAttribute(const ConcreteAttributePath & path, AttributeValueEncoder & encoder, BalanceStructAtIndexGetter getter)
{
    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    EndpointId endpoint = GetEndpointId();

    return encoder.EncodeList([endpoint, getter](const auto & enc) -> CHIP_ERROR {
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        do
        {
            Percent step;
            char buffer[kMaxBalanceStructLabelLength];
            Optional<MutableCharSpan> label{ MutableCharSpan(buffer) };
            if ((err = (sDelegate->*getter)(endpoint, index, step, label)) == CHIP_NO_ERROR)
            {
                BalanceStruct::Type balance = { step, Optional<CharSpan>(label) };
                ReturnErrorOnFailure(enc.Encode(balance));
                index++;
            }
        } while (err == CHIP_NO_ERROR);

        return err.NoErrorIf(CHIP_ERROR_NOT_FOUND);
    });
}

CHIP_ERROR EnergyPreferenceCluster::ReadEnergyPriorities(const ConcreteAttributePath & path, AttributeValueEncoder & encoder)
{
    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    EndpointId endpoint = GetEndpointId();

    return encoder.EncodeList([endpoint](const auto & enc) -> CHIP_ERROR {
        EnergyPriorityEnum priority;
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        while ((err = sDelegate->GetEnergyPriorityAtIndex(endpoint, index, priority)) == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(enc.Encode(priority));
            index++;
        }
        return err.NoErrorIf(CHIP_ERROR_NOT_FOUND);
    });
}

void EnergyPreferenceCluster::SetDelegate(Delegate * delegate)
{
    sDelegate = delegate;
}

Delegate * EnergyPreferenceCluster::GetDelegate()
{
    return sDelegate;
}

} // namespace chip::app::Clusters
