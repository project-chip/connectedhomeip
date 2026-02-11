/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/unit-localization-server/UnitLocalizationCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/UnitLocalization/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;
using namespace chip::app::Clusters::UnitLocalization::Attributes;

CHIP_ERROR UnitLocalizationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint8_t storedTempUnit = 0;

    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(kRootEndpointId, UnitLocalization::Id, TemperatureUnit::Id), storedTempUnit);
    if (err == CHIP_NO_ERROR)
    {
        mTemperatureUnit = static_cast<TempUnitEnum>(storedTempUnit);
        ChipLogDetail(Zcl, "UnitLocalization ep0 Loaded TemperatureUnit: %u", storedTempUnit);
    }
    else
    {
        ChipLogDetail(Zcl, "UnitLocalization ep0 set default TemperatureUnit: %u", to_underlying(mTemperatureUnit));
    }

    return err;
}

CHIP_ERROR UnitLocalizationCluster::SetSupportedTemperatureUnits(DataModel::List<TempUnitEnum> & units)
{
    VerifyOrReturnError(units.size() >= kMinSupportedLocalizationUnits, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(units.size() <= kMaxSupportedLocalizationUnits, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    size_t i = 0;
    for (auto & item : units)
    {
        mUnitsBuffer[i++] = item;
    }
    mSupportedTemperatureUnits = DataModel::List<TempUnitEnum>(mUnitsBuffer, units.size());
    ChipLogProgress(Zcl, "Set number of supported temperature units to: %u",
                    static_cast<unsigned int>(mSupportedTemperatureUnits.size()));
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus UnitLocalizationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case TemperatureUnit::Id: {
        TempUnitEnum newTempUnit = TempUnitEnum::kCelsius;
        ReturnErrorOnFailure(decoder.Decode(newTempUnit));
        return SetTemperatureUnit(newTempUnit);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus UnitLocalizationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case TemperatureUnit::Id:
        return encoder.Encode(mTemperatureUnit);
    case SupportedTemperatureUnits::Id:
        return encoder.Encode(GetSupportedTemperatureUnits());
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR UnitLocalizationCluster::SetTemperatureUnit(TempUnitEnum newTempUnit)
{
    bool isValid       = false;
    const auto & units = GetSupportedTemperatureUnits();
    for (auto const & unit : units)
    {
        if (unit == newTempUnit)
        {
            isValid = true;
            break;
        }
    }
    VerifyOrReturnError(isValid, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    VerifyOrReturnError(SetAttributeValue(mTemperatureUnit, newTempUnit, TemperatureUnit::Id), CHIP_NO_ERROR);

    return GetSafeAttributePersistenceProvider()->WriteScalarValue(
        ConcreteAttributePath(kRootEndpointId, UnitLocalization::Id, TemperatureUnit::Id), to_underlying(mTemperatureUnit));
}

CHIP_ERROR UnitLocalizationCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = {
        { mFeatures.Has(Feature::kTemperatureUnit), Attributes::TemperatureUnit::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureUnit), Attributes::SupportedTemperatureUnits::kMetadataEntry }
    };

    return listBuilder.Append(Span(UnitLocalization::Attributes::kMandatoryMetadata), Span(optionalAttributeEntries));
}
