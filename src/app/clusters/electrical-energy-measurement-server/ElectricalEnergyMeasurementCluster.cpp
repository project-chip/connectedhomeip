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

bool ValueChanged(const Optional<EnergyMeasurementStruct::Type> & previous,
                  const Optional<EnergyMeasurementStruct::Type> & newValue)
{
    if (previous.HasValue() != newValue.HasValue())
        return true;
    if (!previous.HasValue())
        return false;
    return ValueChanged(previous.Value(), newValue.Value());
}

bool ValueChanged(const Optional<CumulativeEnergyResetStruct::Type> & previous,
                  const Optional<CumulativeEnergyResetStruct::Type> & newValue)
{
    if (previous.HasValue() != newValue.HasValue())
        return true;
    if (!previous.HasValue())
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
        attrs.Set<Attributes::CumulativeEnergyReset::Id>(
            config.optionalAttributes.IsSet(Attributes::CumulativeEnergyReset::Id) &&
            config.featureFlags.Has(Feature::kCumulativeEnergy));
        return attrs;
    }()),
    mDelegate(config.delegate), mTimerDelegate(config.timerDelegate), mReportInterval(config.reportInterval)
{
    mMeasurementData.measurementAccuracy.measurementType  = config.accuracyStruct.measurementType;
    mMeasurementData.measurementAccuracy.measured         = config.accuracyStruct.measured;
    mMeasurementData.measurementAccuracy.minMeasuredValue = config.accuracyStruct.minMeasuredValue;
    mMeasurementData.measurementAccuracy.maxMeasuredValue = config.accuracyStruct.maxMeasuredValue;

    using RangeType = MeasurementAccuracyRangeStruct::Type;
    ReadOnlyBufferBuilder<RangeType> rangesBuilder;
    VerifyOrDie(rangesBuilder.ReferenceExisting(config.accuracyStruct.accuracyRanges) == CHIP_NO_ERROR);
    mAccuracyRangesStorage                              = rangesBuilder.TakeBuffer();
    mMeasurementData.measurementAccuracy.accuracyRanges = mAccuracyRangesStorage;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    StartReportTimer();
    return CHIP_NO_ERROR;
}

void ElectricalEnergyMeasurementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    CancelReportTimer();
    DefaultServerCluster::Shutdown(shutdownType);
}

void ElectricalEnergyMeasurementCluster::StartReportTimer()
{
    CancelReportTimer();
    ReturnAndLogOnFailure(mTimerDelegate.StartTimer(&mReportTimer, mReportInterval), AppServer,
                          "EEM: Failed to start report timer");
}

void ElectricalEnergyMeasurementCluster::CancelReportTimer()
{
    if (mTimerDelegate.IsTimerActive(&mReportTimer))
    {
        mTimerDelegate.CancelTimer(&mReportTimer);
    }
}

void ElectricalEnergyMeasurementCluster::ReportTimer::TimerFired()
{
    mCluster.DoGenerateReport();
    mCluster.StartReportTimer();
}

Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct>
ElectricalEnergyMeasurementCluster::BuildMeasurement(DataModel::Nullable<int64_t> energy,
                                                     const Optional<EnergyMeasurementStruct> & previous)
{
    if (energy.IsNull())
    {
        return Optional<EnergyMeasurementStruct>();
    }

    EnergyMeasurementStruct measurement;
    measurement.energy = energy.Value();

    // Carry forward previous endTimestamp/endSystime as new start
    measurement.startTimestamp.ClearValue();
    measurement.startSystime.ClearValue();
    if (previous.HasValue())
    {
        measurement.startTimestamp = previous.Value().endTimestamp;
        measurement.startSystime   = previous.Value().endSystime;
    }

    uint32_t epochTimestamp;
    CHIP_ERROR err = System::Clock::GetClock_MatterEpochS(epochTimestamp);
    if (err == CHIP_NO_ERROR)
    {
        measurement.endTimestamp.SetValue(epochTimestamp);
    }
    else
    {
        System::Clock::Milliseconds64 sysTimeMs = System::SystemClock().GetMonotonicMilliseconds64();
        measurement.endSystime.SetValue(static_cast<uint64_t>(sysTimeMs.count()));
    }

    return MakeOptional(measurement);
}

void ElectricalEnergyMeasurementCluster::GenerateReport()
{
    // Enforce min 1s between reports
    System::Clock::Timestamp now = mTimerDelegate.GetCurrentMonotonicTimestamp();
    if (mLastReportTime != System::Clock::kZero && (now - mLastReportTime) < kMinReportInterval)
    {
        return;
    }

    DoGenerateReport();
    StartReportTimer();
}

void ElectricalEnergyMeasurementCluster::DoGenerateReport()
{
    mLastReportTime = mTimerDelegate.GetCurrentMonotonicTimestamp();

    // Generate cumulative energy report if feature is enabled
    if (mFeatureFlags.Has(Feature::kCumulativeEnergy))
    {
        Events::CumulativeEnergyMeasured::Type event;
        bool hasData = false;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            auto measurement = BuildMeasurement(mDelegate.GetCumulativeEnergyImported(), mMeasurementData.cumulativeImported);
            TEMPORARY_RETURN_IGNORED SetCumulativeEnergyImported(measurement);
            event.energyImported = measurement;
            hasData              = true;
        }

        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            auto measurement = BuildMeasurement(mDelegate.GetCumulativeEnergyExported(), mMeasurementData.cumulativeExported);
            TEMPORARY_RETURN_IGNORED SetCumulativeEnergyExported(measurement);
            event.energyExported = measurement;
            hasData              = true;
        }

        if (hasData && mContext != nullptr)
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        }
    }

    // Generate periodic energy report if feature is enabled
    if (mFeatureFlags.Has(Feature::kPeriodicEnergy))
    {
        Events::PeriodicEnergyMeasured::Type event;
        bool hasData = false;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            auto measurement = BuildMeasurement(mDelegate.GetPeriodicEnergyImported(), mMeasurementData.periodicImported);
            TEMPORARY_RETURN_IGNORED SetPeriodicEnergyImported(measurement);
            event.energyImported = measurement;
            hasData              = true;
        }

        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            auto measurement = BuildMeasurement(mDelegate.GetPeriodicEnergyExported(), mMeasurementData.periodicExported);
            TEMPORARY_RETURN_IGNORED SetPeriodicEnergyExported(measurement);
            event.energyExported = measurement;
            hasData              = true;
        }

        if (hasData && mContext != nullptr)
        {
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        }
    }
}

void ElectricalEnergyMeasurementCluster::GetMeasurementAccuracy(MeasurementAccuracyStruct & outValue) const
{
    outValue = mMeasurementData.measurementAccuracy;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyImported requires kCumulativeEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeImported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyExported requires kCumulativeEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeExported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyImported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.periodicImported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetPeriodicEnergyExported(Optional<EnergyMeasurementStruct> & outValue) const
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.periodicExported;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::GetCumulativeEnergyReset(Optional<CumulativeEnergyResetStruct> & outValue) const
{
    if (!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    outValue = mMeasurementData.cumulativeReset;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetMeasurementAccuracy(const MeasurementAccuracyStruct & value)
{
    if (ValueChanged(mMeasurementData.measurementAccuracy, value))
    {
        mMeasurementData.measurementAccuracy.measurementType  = value.measurementType;
        mMeasurementData.measurementAccuracy.measured         = value.measured;
        mMeasurementData.measurementAccuracy.minMeasuredValue = value.minMeasuredValue;
        mMeasurementData.measurementAccuracy.maxMeasuredValue = value.maxMeasuredValue;
        NotifyAttributeChanged(Accuracy::Id);
    }

    // Always update ranges if they are present since ValueChanged intentionally skips comparing them
    if (!value.accuracyRanges.empty())
    {
        ReadOnlyBufferBuilder<MeasurementAccuracyRangeStruct::Type> rangesBuilder;
        ReturnErrorOnFailure(rangesBuilder.AppendElements(value.accuracyRanges));
        mAccuracyRangesStorage                              = rangesBuilder.TakeBuffer();
        mMeasurementData.measurementAccuracy.accuracyRanges = mAccuracyRangesStorage;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyImported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(Feature::kCumulativeEnergy, Feature::kImportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyImported requires kCumulativeEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (ValueChanged(mMeasurementData.cumulativeImported, value))
    {
        mMeasurementData.cumulativeImported = value;
        NotifyAttributeChanged(CumulativeEnergyImported::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyExported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kCumulativeEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(
            Zcl, "Electrical Energy Measurement: CumulativeEnergyExported requires kCumulativeEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (ValueChanged(mMeasurementData.cumulativeExported, value))
    {
        mMeasurementData.cumulativeExported = value;
        NotifyAttributeChanged(CumulativeEnergyExported::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyImported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (ValueChanged(mMeasurementData.periodicImported, value))
    {
        mMeasurementData.periodicImported = value;
        NotifyAttributeChanged(PeriodicEnergyImported::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyExported(const Optional<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (ValueChanged(mMeasurementData.periodicExported, value))
    {
        mMeasurementData.periodicExported = value;
        NotifyAttributeChanged(PeriodicEnergyExported::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyReset(const Optional<CumulativeEnergyResetStruct> & value)
{
    if (!mEnabledOptionalAttributes.IsSet(CumulativeEnergyReset::Id))
    {
        ChipLogError(Zcl, "Electrical Energy Measurement: CumulativeEnergyReset requires kCumulativeEnergy feature");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (ValueChanged(mMeasurementData.cumulativeReset, value))
    {
        mMeasurementData.cumulativeReset = value;
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
        return encoder.Encode(mMeasurementData.measurementAccuracy);

    case CumulativeEnergyImported::Id:
        if (!mMeasurementData.cumulativeImported.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mMeasurementData.cumulativeImported.Value());

    case CumulativeEnergyExported::Id:
        if (!mMeasurementData.cumulativeExported.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mMeasurementData.cumulativeExported.Value());

    case PeriodicEnergyImported::Id:
        if (!mMeasurementData.periodicImported.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mMeasurementData.periodicImported.Value());

    case PeriodicEnergyExported::Id:
        if (!mMeasurementData.periodicExported.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mMeasurementData.periodicExported.Value());

    case CumulativeEnergyReset::Id:
        if (!mMeasurementData.cumulativeReset.HasValue())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mMeasurementData.cumulativeReset.Value());

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

void ElectricalEnergyMeasurementCluster::CumulativeEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                                                  const Optional<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturn(Features().Has(Feature::kCumulativeEnergy));
    Events::CumulativeEnergyMeasured::Type event;

    if (Features().Has(Feature::kImportedEnergy))
    {
        TEMPORARY_RETURN_IGNORED SetCumulativeEnergyImported(energyImported);
        event.energyImported = energyImported;
    }

    if (Features().Has(Feature::kExportedEnergy))
    {
        TEMPORARY_RETURN_IGNORED SetCumulativeEnergyExported(energyExported);
        event.energyExported = energyExported;
    }

    VerifyOrReturn(mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ElectricalEnergyMeasurementCluster::PeriodicEnergySnapshot(const Optional<EnergyMeasurementStruct> & energyImported,
                                                                const Optional<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturn(Features().Has(Feature::kPeriodicEnergy));
    Events::PeriodicEnergyMeasured::Type event;

    if (Features().Has(Feature::kImportedEnergy))
    {
        TEMPORARY_RETURN_IGNORED SetPeriodicEnergyImported(energyImported);
        event.energyImported = energyImported;
    }

    if (Features().Has(Feature::kExportedEnergy))
    {
        TEMPORARY_RETURN_IGNORED SetPeriodicEnergyExported(energyExported);
        event.energyExported = energyExported;
    }

    VerifyOrReturn(mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
