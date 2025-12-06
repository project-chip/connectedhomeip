/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/unit-localization-server/unit-localization-server.h>
#include <app/persistence/AttributePersistence.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/UnitLocalization/Metadata.h>
#include <platform/DeviceInfoProvider.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;
using namespace chip::app::Clusters::UnitLocalization::Attributes;

UnitLocalizationCluster::UnitLocalizationCluster(EndpointId endpointId, BitFlags<UnitLocalization::Feature> feature) :
    DefaultServerCluster({ endpointId, UnitLocalization::Id }), mFeatures{ feature }
{}

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

CHIP_ERROR UnitLocalizationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attrPersistence{ context.attributeStorage };

    auto defTempUnit = mTemperatureUnit;
    attrPersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, TemperatureUnit::Id),
                                          mTemperatureUnit, defTempUnit);
    return CHIP_NO_ERROR;
}

CHIP_ERROR UnitLocalizationCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry optionalAttributes[] = { TemperatureUnit::kMetadataEntry,
                                                                        SupportedTemperatureUnits::kMetadataEntry };

    AttributeListBuilder listBuilder(builder);

    OptionalAttributeSet<TemperatureUnit::Id, SupportedTemperatureUnits::Id> optionalAttributeSet;

    if (mFeatures.Has(UnitLocalization::Feature::kTemperatureUnit))
    {
        optionalAttributeSet.Set<TemperatureUnit::Id>();
        optionalAttributeSet.Set<SupportedTemperatureUnits::Id>();
    }

    return listBuilder.Append(kMandatoryMetadata, Span(optionalAttributes), optionalAttributeSet);
}

DataModel::ActionReturnStatus UnitLocalizationCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                 AttributeValueDecoder & decoder)
{

    if (request.path.mAttributeId == TemperatureUnit::Id)
    {
        AttributePersistence persistence{ mContext->attributeStorage };
        return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mTemperatureUnit);
    }

    return Protocols::InteractionModel::Status::UnsupportedWrite;
}

DataModel::ActionReturnStatus UnitLocalizationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus UnitLocalizationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case UnitLocalization::Attributes::TemperatureUnit::Id:
        return encoder.Encode(mTemperatureUnit);

    case UnitLocalization::Attributes::SupportedTemperatureUnits::Id:
        return encoder.Encode(mSupportedTemperatureUnits);

    case UnitLocalization::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case UnitLocalization::Attributes::ClusterRevision::Id:
        return encoder.Encode(UnitLocalization::kRevision);

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

UnitLocalizationClusterWithMigration::UnitLocalizationClusterWithMigration(EndpointId endpointId,
                                                                           BitFlags<UnitLocalization::Feature> feature) :
    UnitLocalizationCluster(endpointId, feature)
{}

CHIP_ERROR UnitLocalizationClusterWithMigration::Startup(ServerClusterContext & context)
{
    static constexpr AttributeId attributesToUpdate[] = { UnitLocalization::Attributes::TemperatureUnit::Id };
    ReturnErrorOnFailure(MigrateFromSafeAttributePersistenceProvider(mPath, Span(attributesToUpdate), context.storage));
    return UnitLocalizationCluster::Startup(context);
}
