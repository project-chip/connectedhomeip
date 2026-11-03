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
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>

#include <protocols/interaction_model/StatusCode.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/ElectricalEnergyMeasurement/Attributes.h>
#include <clusters/ElectricalEnergyMeasurement/Enums.h>
#include <clusters/ElectricalEnergyMeasurement/Events.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

using namespace Attributes;
using namespace Structs;

namespace {

bool ValueChanged(const MeasurementAccuracyStruct::Type & previous, const MeasurementAccuracyStruct::Type & newValue)
{
    return (previous.measurementType != newValue.measurementType) || (previous.measured != newValue.measured) ||
        (previous.minMeasuredValue != newValue.minMeasuredValue) || (previous.maxMeasuredValue != newValue.maxMeasuredValue);
    // Note: accuracyRanges List is not compared as we do not expect it to change in operation
}

bool ValueChanged(const EnergyMeasurementStruct::Type & previous, const EnergyMeasurementStruct::Type & newValue)
{
    return (previous.energy != newValue.energy) || (previous.startTimestamp != newValue.startTimestamp) ||
        (previous.endTimestamp != newValue.endTimestamp) || (previous.startSystime != newValue.startSystime) ||
        (previous.endSystime != newValue.endSystime) || (previous.apparentEnergy != newValue.apparentEnergy) ||
        (previous.reactiveEnergy != newValue.reactiveEnergy);
}

bool ValueChanged(const DataModel::Nullable<EnergyMeasurementStruct::Type> & previous,
                  const DataModel::Nullable<EnergyMeasurementStruct::Type> & newValue)
{
    if (previous.IsNull() != newValue.IsNull())
        return true;
    if (previous.IsNull())
        return false;
    return ValueChanged(previous.Value(), newValue.Value());
}

bool ValueChanged(const DataModel::Nullable<CumulativeEnergyResetStruct::Type> & previous,
                  const DataModel::Nullable<CumulativeEnergyResetStruct::Type> & newValue)
{
    if (previous.IsNull() != newValue.IsNull())
        return true;
    if (previous.IsNull())
        return false;
    return (previous.Value().importedResetTimestamp != newValue.Value().importedResetTimestamp) ||
        (previous.Value().exportedResetTimestamp != newValue.Value().exportedResetTimestamp) ||
        (previous.Value().importedResetSystime != newValue.Value().importedResetSystime) ||
        (previous.Value().exportedResetSystime != newValue.Value().exportedResetSystime);
}

} // anonymous namespace

ElectricalEnergyMeasurementCluster::ElectricalEnergyMeasurementCluster(const Config & config) :
    DefaultServerCluster({ config.endpointId, ElectricalEnergyMeasurement::Id }), mFeatureFlags(config.featureFlags),
    mEnabledOptionalAttributes([&]() {
        OptionalAttributesSet attrs;
        attrs.Set<Attributes::CumulativeEnergyImported::Id>(
            config.featureFlags.HasAll(Feature::kCumulativeEnergy, Feature::kImportedEnergy));
        attrs.Set<Attributes::CumulativeEnergyExported::Id>(
            config.featureFlags.HasAll(Feature::kCumulativeEnergy, Feature::kExportedEnergy));
        attrs.Set<Attributes::PeriodicEnergyImported::Id>(
            config.featureFlags.HasAll(Feature::kPeriodicEnergy, Feature::kImportedEnergy));
        attrs.Set<Attributes::PeriodicEnergyExported::Id>(
            config.featureFlags.HasAll(Feature::kPeriodicEnergy, Feature::kExportedEnergy));
        attrs.Set<Attributes::CumulativeEnergyReset::Id>(config.optionalAttributes.IsSet(Attributes::CumulativeEnergyReset::Id) &&
                                                         config.featureFlags.Has(Feature::kCumulativeEnergy));
        return attrs;
    }()),
    mDelegate(config.delegate), mTimerDelegate(config.timerDelegate)
{
    mMeasurementAccuracy.measurementType  = config.accuracyStruct.measurementType;
    mMeasurementAccuracy.measured         = config.accuracyStruct.measured;
    mMeasurementAccuracy.minMeasuredValue = config.accuracyStruct.minMeasuredValue;
    mMeasurementAccuracy.maxMeasuredValue = config.accuracyStruct.maxMeasuredValue;

    using RangeType = MeasurementAccuracyRangeStruct::Type;
    ReadOnlyBufferBuilder<RangeType> rangesBuilder;
    VerifyOrDie(rangesBuilder.ReferenceExisting(config.accuracyStruct.accuracyRanges) == CHIP_NO_ERROR);
    mAccuracyRangesStorage              = rangesBuilder.TakeBuffer();
    mMeasurementAccuracy.accuracyRanges = mAccuracyRangesStorage;
}

void ElectricalEnergyMeasurementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct>
ElectricalEnergyMeasurementCluster::BuildMeasurement(DataModel::Nullable<int64_t> energy,
                                                     const DataModel::Nullable<EnergyMeasurementStruct> & previous,
                                                     bool isCumulative)
{
    if (energy.IsNull())
    {
        return DataModel::NullNullable;
    }

    EnergyMeasurementStruct measurement;
    measurement.energy = energy.Value();

    if (!isCumulative)
    {
        if (!previous.IsNull())
        {
            measurement.startTimestamp = previous.Value().endTimestamp;
            measurement.startSystime   = previous.Value().endSystime;
        }
    }

    // Matter epoch timestamp when available; monotonic endSystime is always set so ShouldNotifyEnergyMeasurementAttribute can
    // enforce kMinReportInterval even when real-time clock is available.
    uint32_t currentTimestamp = 0;
    CHIP_ERROR err            = System::Clock::GetClock_MatterEpochS(currentTimestamp);
    if (err == CHIP_NO_ERROR)
    {
        measurement.endTimestamp.SetValue(currentTimestamp);
    }
    System::Clock::Milliseconds64 sysTimeMs = mTimerDelegate.GetCurrentMonotonicTimestamp();
    measurement.endSystime.SetValue(static_cast<uint64_t>(sysTimeMs.count()));

    return DataModel::MakeNullable(measurement);
}

void ElectricalEnergyMeasurementCluster::GenerateSnapshots()
{
    if (mFeatureFlags.Has(Feature::kCumulativeEnergy))
    {
        Nullable<EnergyMeasurementStruct> imported = DataModel::NullNullable;
        Nullable<EnergyMeasurementStruct> exported = DataModel::NullNullable;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            imported = BuildMeasurement(mDelegate.GetCumulativeEnergyImported(), mCumulativeImported.value(), true);
        }
        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            exported = BuildMeasurement(mDelegate.GetCumulativeEnergyExported(), mCumulativeExported.value(), true);
        }
        CumulativeEnergySnapshot(imported, exported);
    }

    if (mFeatureFlags.Has(Feature::kPeriodicEnergy))
    {
        Nullable<EnergyMeasurementStruct> imported = DataModel::NullNullable;
        Nullable<EnergyMeasurementStruct> exported = DataModel::NullNullable;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            imported = BuildMeasurement(mDelegate.GetPeriodicEnergyImported(), mPeriodicImported.value(), false);
        }
        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            exported = BuildMeasurement(mDelegate.GetPeriodicEnergyExported(), mPeriodicExported.value(), false);
        }
        PeriodicEnergySnapshot(imported, exported);
    }
}

void ElectricalEnergyMeasurementCluster::GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const
{
    outValue = mMeasurementAccuracy;
}

CHIP_ERROR
ElectricalEnergyMeasurementCluster::GetCumulativeEnergyImported(DataModel::Nullable<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyImported requires kCumulativeEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mCumulativeImported.value();
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ElectricalEnergyMeasurementCluster::GetCumulativeEnergyExported(Nullable<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyExported requires kCumulativeEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mCumulativeExported.value();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyImported(Nullable<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mPeriodicImported.value();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyExported(Nullable<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mPeriodicExported.value();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyReset(Nullable<CumulativeEnergyResetStruct> & outValue) const
{
    if (!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mCumulativeReset;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetMeasurementAccuracy(const MeasurementAccuracyStruct & value)
{
    if (ValueChanged(mMeasurementAccuracy, value))
    {
        mMeasurementAccuracy.measurementType  = value.measurementType;
        mMeasurementAccuracy.measured         = value.measured;
        mMeasurementAccuracy.minMeasuredValue = value.minMeasuredValue;
        mMeasurementAccuracy.maxMeasuredValue = value.maxMeasuredValue;
        NotifyAttributeChanged(Accuracy::Id);
    }

    // Always update ranges if they are present since ValueChanged intentionally skips comparing them
    if (!value.accuracyRanges.empty())
    {
        ReadOnlyBufferBuilder<MeasurementAccuracyRangeStruct::Type> rangesBuilder;
        ReturnErrorOnFailure(rangesBuilder.AppendElements(value.accuracyRanges));
        mAccuracyRangesStorage              = rangesBuilder.TakeBuffer();
        mMeasurementAccuracy.accuracyRanges = mAccuracyRangesStorage;
    }

    return CHIP_NO_ERROR;
}

AttributeDirtyState ElectricalEnergyMeasurementCluster::SetCumulativeEnergyImported(const Nullable<EnergyMeasurementStruct> & value)
{
    AttributeDirtyState dirtyState = AttributeDirtyState::kNoReportNeeded;
    if (ValueChanged(mCumulativeImported.value(), value))
    {
        dirtyState = mCumulativeImported.SetValue(value, mTimerDelegate.GetCurrentMonotonicTimestamp());
        if (dirtyState == AttributeDirtyState::kMustReport)
        {
            NotifyAttributeChanged(CumulativeEnergyImported::Id);
        }
    }
    return dirtyState;
}

AttributeDirtyState ElectricalEnergyMeasurementCluster::SetCumulativeEnergyExported(const Nullable<EnergyMeasurementStruct> & value)
{
    AttributeDirtyState dirtyState = AttributeDirtyState::kNoReportNeeded;
    if (ValueChanged(mCumulativeExported.value(), value))
    {
        dirtyState = mCumulativeExported.SetValue(value, mTimerDelegate.GetCurrentMonotonicTimestamp());
        if (dirtyState == AttributeDirtyState::kMustReport)
        {
            NotifyAttributeChanged(CumulativeEnergyExported::Id);
        }
    }
    return dirtyState;
}

AttributeDirtyState ElectricalEnergyMeasurementCluster::SetPeriodicEnergyImported(const Nullable<EnergyMeasurementStruct> & value)
{
    AttributeDirtyState dirtyState = AttributeDirtyState::kNoReportNeeded;
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return AttributeDirtyState::kNoReportNeeded;
    }
    if (ValueChanged(mPeriodicImported.value(), value))
    {
        dirtyState = mPeriodicImported.SetValue(value, mTimerDelegate.GetCurrentMonotonicTimestamp());
        if (dirtyState == AttributeDirtyState::kMustReport)
        {
            NotifyAttributeChanged(PeriodicEnergyImported::Id);
        }
    }
    return dirtyState;
}

AttributeDirtyState ElectricalEnergyMeasurementCluster::SetPeriodicEnergyExported(const Nullable<EnergyMeasurementStruct> & value)
{
    AttributeDirtyState dirtyState = AttributeDirtyState::kNoReportNeeded;
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return AttributeDirtyState::kNoReportNeeded;
    }

    if (ValueChanged(mPeriodicExported.value(), value))
    {
        dirtyState = mPeriodicExported.SetValue(value, mTimerDelegate.GetCurrentMonotonicTimestamp());
        if (dirtyState == AttributeDirtyState::kMustReport)
        {
            NotifyAttributeChanged(PeriodicEnergyExported::Id);
        }
    }
    return dirtyState;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyReset(const Nullable<CumulativeEnergyResetStruct> & value)
{
    if (!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (ValueChanged(mCumulativeReset, value))
    {
        mCumulativeReset = value;
        NotifyAttributeChanged(CumulativeEnergyReset::Id);
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ElectricalEnergyMeasurementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case ClusterRevision::Id:
        return encoder.Encode(kRevision);

    case Accuracy::Id:
        return encoder.Encode(mMeasurementAccuracy);

    case CumulativeEnergyImported::Id:
        if (mCumulativeImported.value().IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mCumulativeImported.value().Value());

    case CumulativeEnergyExported::Id:
        if (mCumulativeExported.value().IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mCumulativeExported.value().Value());

    case PeriodicEnergyImported::Id:
        if (mPeriodicImported.value().IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mPeriodicImported.value().Value());

    case PeriodicEnergyExported::Id:
        if (mPeriodicExported.value().IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mPeriodicExported.value().Value());

    case CumulativeEnergyReset::Id:
        if (mCumulativeReset.IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mCumulativeReset.Value());

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::Attributes(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        CumulativeEnergyImported::kMetadataEntry, //
        CumulativeEnergyExported::kMetadataEntry, //
        PeriodicEnergyImported::kMetadataEntry,   //
        PeriodicEnergyExported::kMetadataEntry,   //
        CumulativeEnergyReset::kMetadataEntry,    //
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes);
}

void ElectricalEnergyMeasurementCluster::CumulativeEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                                                  const Nullable<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturn(Features().Has(Feature::kCumulativeEnergy));
    Events::CumulativeEnergyMeasured::Type event;
    bool hasData = false;

    if (Features().Has(Feature::kImportedEnergy) && !energyImported.IsNull())
    {
        if (SetCumulativeEnergyImported(energyImported) == AttributeDirtyState::kMustReport)
        {
            event.energyImported.SetValue(energyImported.Value());
            hasData = true;
        }
    }

    if (Features().Has(Feature::kExportedEnergy) && !energyExported.IsNull())
    {
        if (SetCumulativeEnergyExported(energyExported) == AttributeDirtyState::kMustReport)
        {
            event.energyExported.SetValue(energyExported.Value());
            hasData = true;
        }
    }

    VerifyOrReturn(hasData && mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ElectricalEnergyMeasurementCluster::PeriodicEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                                                const Nullable<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturn(Features().Has(Feature::kPeriodicEnergy));
    Events::PeriodicEnergyMeasured::Type event;
    bool hasData = false;

    if (Features().Has(Feature::kImportedEnergy) && !energyImported.IsNull())
    {
        if (SetPeriodicEnergyImported(energyImported) == AttributeDirtyState::kMustReport)
        {
            event.energyImported.SetValue(energyImported.Value());
            hasData = true;
        }
    }

    if (Features().Has(Feature::kExportedEnergy) && !energyExported.IsNull())
    {
        if (SetPeriodicEnergyExported(energyExported) == AttributeDirtyState::kMustReport)
        {
            event.energyExported.SetValue(energyExported.Value());
            hasData = true;
        }
    }

    VerifyOrReturn(hasData && mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
