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
    uint8_t currentEnergyBalance           = 0;
    uint8_t currentLowPowerModeSensitivity = 0;

    persistence.LoadNativeEndianValue({ GetEndpointId(), mPath.mClusterId, CurrentEnergyBalance::Id }, currentEnergyBalance,
                                      currentEnergyBalance);
    persistence.LoadNativeEndianValue({ GetEndpointId(), mPath.mClusterId, CurrentLowPowerModeSensitivity::Id },
                                      currentLowPowerModeSensitivity, currentLowPowerModeSensitivity);

    if (mFeatures.Has(Feature::kEnergyBalance))
    {
        ReturnErrorOnFailure(SetCurrentEnergyBalance(currentEnergyBalance));
    }

    if (mFeatures.Has(Feature::kLowPowerModeSensitivity))
    {
        ReturnErrorOnFailure(SetCurrentLowPowerModeSensitivity(currentLowPowerModeSensitivity));
    }
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

CHIP_ERROR EnergyPreferenceCluster::SetCurrentEnergyBalance(uint8_t currentEnergyBalance)
{
    if (!mFeatures.Has(Feature::kEnergyBalance))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (currentEnergyBalance >= sDelegate->GetNumEnergyBalances(GetEndpointId()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (SetAttributeValue(mCurrentEnergyBalance, currentEnergyBalance, CurrentEnergyBalance::Id))
    {
        sDelegate->OnCurrentEnergyBalanceChanged(GetEndpointId(), currentEnergyBalance);
        if (mContext != nullptr)
        {
            AttributePersistence persistence(mContext->attributeStorage);
            ReturnErrorOnFailure(persistence.StoreNativeEndianValue({ GetEndpointId(), mPath.mClusterId, CurrentEnergyBalance::Id },
                                                                    currentEnergyBalance));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyPreferenceCluster::SetCurrentLowPowerModeSensitivity(uint8_t currentLowPowerModeSensitivity)
{
    if (!mFeatures.Has(Feature::kLowPowerModeSensitivity))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (currentLowPowerModeSensitivity >= sDelegate->GetNumLowPowerModeSensitivities(GetEndpointId()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (SetAttributeValue(mCurrentLowPowerModeSensitivity, currentLowPowerModeSensitivity, CurrentLowPowerModeSensitivity::Id))
    {
        sDelegate->OnCurrentLowPowerModeSensitivityChanged(GetEndpointId(), currentLowPowerModeSensitivity);
        if (mContext != nullptr)
        {
            AttributePersistence persistence(mContext->attributeStorage);
            ReturnErrorOnFailure(persistence.StoreNativeEndianValue(
                { GetEndpointId(), mPath.mClusterId, CurrentLowPowerModeSensitivity::Id }, currentLowPowerModeSensitivity));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyPreferenceCluster::ReadEnergyBalances(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    EndpointId endpoint = GetEndpointId();

    return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        do
        {
            Percent step;
            char buffer[64];
            Optional<MutableCharSpan> label{ MutableCharSpan(buffer) };
            if ((err = sDelegate->GetEnergyBalanceAtIndex(endpoint, index, step, label)) == CHIP_NO_ERROR)
            {
                BalanceStruct::Type balance = { step, Optional<CharSpan>(label) };
                ReturnErrorOnFailure(encoder.Encode(balance));
                index++;
            }
        } while (err == CHIP_NO_ERROR);

        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
        return err;
    });
}

CHIP_ERROR EnergyPreferenceCluster::ReadEnergyPriorities(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    EndpointId endpoint = GetEndpointId();

    return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
        EnergyPriorityEnum priority;
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        while ((err = sDelegate->GetEnergyPriorityAtIndex(endpoint, index, priority)) == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(encoder.Encode(priority));
            index++;
        }
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
        return err;
    });
}

CHIP_ERROR EnergyPreferenceCluster::ReadLowPowerModeSensitivities(const ConcreteAttributePath & aPath,
                                                                  AttributeValueEncoder & aEncoder)
{
    VerifyOrReturnError(sDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    EndpointId endpoint = GetEndpointId();

    return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        do
        {
            Percent step;
            char buffer[64];
            Optional<MutableCharSpan> label{ MutableCharSpan(buffer) };
            if ((err = sDelegate->GetLowPowerModeSensitivityAtIndex(endpoint, index, step, label)) == CHIP_NO_ERROR)
            {
                BalanceStruct::Type balance = { step, Optional<CharSpan>(label) };
                ReturnErrorOnFailure(encoder.Encode(balance));
                index++;
            }
        } while (err == CHIP_NO_ERROR);
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
        return err;
    });
}

void EnergyPreferenceCluster::SetDelegate(Delegate * aDelegate)
{
    sDelegate = aDelegate;
}

Delegate * EnergyPreferenceCluster::GetDelegate()
{
    return sDelegate;
}

} // namespace chip::app::Clusters
