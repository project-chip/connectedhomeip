/*
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

#include <app/clusters/soil-measurement-server/soil-measurement-logic.h>

#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/SoilMeasurement/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {

namespace {

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

} // namespace

SoilMeasurementLogic::SoilMeasurementLogic() : mSoilMoistureMeasurementLimits(kDefaultSoilMoistureMeasurementLimits)
{
    mAttrProvider = nullptr;
    mSoilMoistureMeasuredValue.SetNull();
}

void SoilMeasurementLogic::Startup(EndpointId endpointId, AttributePersistenceProvider * attrProvider)
{
    VerifyOrReturn(mAttrProvider == nullptr);
    VerifyOrReturn(attrProvider != nullptr);

    mAttrProvider = attrProvider;

    SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type measurementLimits;
    MutableByteSpan measurementLimitsBytes(reinterpret_cast<uint8_t *>(&measurementLimits), sizeof(measurementLimits));
    CHIP_ERROR error = mAttrProvider->ReadValue(
        { endpointId, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::Id },
        measurementLimitsBytes);

    if (error != CHIP_NO_ERROR)
    {
        measurementLimits = kDefaultSoilMoistureMeasurementLimits;
    }

    mSoilMoistureMeasurementLimits = measurementLimits;

    SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type measuredValue;
    MutableByteSpan measuredValueBytes(reinterpret_cast<uint8_t *>(&measuredValue), sizeof(measuredValue));
    error = mAttrProvider->ReadValue(
        { endpointId, SoilMeasurement::Id, SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id }, measuredValueBytes);

    if (error != CHIP_NO_ERROR)
    {
        measuredValue.SetNull();
    }

    mSoilMoistureMeasuredValue = measuredValue;
}

SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::TypeInfo::Type SoilMeasurementLogic::GetSoilMoistureMeasurementLimits()
{
    return mSoilMoistureMeasurementLimits;
}

SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type SoilMeasurementLogic::GetSoilMoistureMeasuredValue()
{
    return mSoilMoistureMeasuredValue;
}

CHIP_ERROR SoilMeasurementLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // Ensure capacity just in case
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(2 + DefaultServerCluster::GlobalAttributes().size()));
    // Mandatory attributes
    ReturnErrorOnFailure(builder.Append(SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::kMetadataEntry));
    ReturnErrorOnFailure(builder.Append(SoilMeasurement::Attributes::SoilMoistureMeasuredValue::kMetadataEntry));

    // Finally, the global attributes
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR
SoilMeasurementLogic::SetSoilMoistureMeasuredValue(
    EndpointId endpointId, const SoilMeasurement::Attributes::SoilMoistureMeasuredValue::TypeInfo::Type & soilMoistureMeasuredValue)
{
    if (mSoilMoistureMeasuredValue != soilMoistureMeasuredValue)
    {
        mSoilMoistureMeasuredValue = soilMoistureMeasuredValue;

        MatterReportingAttributeChangeCallback(endpointId, SoilMeasurement::Id,
                                               SoilMeasurement::Attributes::SoilMoistureMeasuredValue::Id);
    }

    return CHIP_NO_ERROR;
}

} // namespace Clusters
} // namespace app
} // namespace chip
