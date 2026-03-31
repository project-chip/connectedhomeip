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

/** Rate-limit subscription reporting per attribute: notify when endSystime advanced by at least kMinReportInterval
 *  (monotonic ms), or when null / missing timestamps / non-monotonic endSystime. */
bool ShouldNotifyEnergyMeasurementAttribute(const DataModel::Nullable<EnergyMeasurementStruct::Type> & previous,
                                            const DataModel::Nullable<EnergyMeasurementStruct::Type> & newValue)
{
    if (newValue.IsNull())
    {
        return true;
    }
    if (previous.IsNull() || !previous.Value().endSystime.HasValue())
    {
        return true;
    }
    if (!newValue.Value().endSystime.HasValue())
    {
        return true;
    }

    const uint64_t minIntervalMs = static_cast<uint64_t>(
        std::chrono::duration_cast<System::Clock::Milliseconds64>(ElectricalEnergyMeasurementCluster::kMinReportInterval).count());
    const uint64_t prevEnd = previous.Value().endSystime.Value();
    const uint64_t newEnd  = newValue.Value().endSystime.Value();
    return (newEnd < prevEnd) || (newEnd - prevEnd >= minIntervalMs);
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
    mDelegate(config.delegate), mTimerDelegate(config.timerDelegate), mReportInterval(config.reportInterval)
{
    // The report interval must be between kMinReportInterval and kMaxReportInterval, we cap it here.
    if (mReportInterval < kMinReportInterval)
    {
        mReportInterval = kMinReportInterval;
    }
    else if (mReportInterval > kMaxReportInterval)
    {
        mReportInterval = kMaxReportInterval;
    }

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

CHIP_ERROR ElectricalEnergyMeasurementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (mFeatureFlags.HasAny(Feature::kCumulativeEnergy, Feature::kPeriodicEnergy))
    {
        StartSnapshotTimer();
    }

    return CHIP_NO_ERROR;
}

void ElectricalEnergyMeasurementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    CancelSnapshotTimer();
    DefaultServerCluster::Shutdown(shutdownType);
}

void ElectricalEnergyMeasurementCluster::StartSnapshotTimer()
{
    CancelSnapshotTimer();
    ReturnAndLogOnFailure(mTimerDelegate.StartTimer(&mSnapshotTimer, mReportInterval), AppServer,
                          "EEM: Failed to start report timer");
}

void ElectricalEnergyMeasurementCluster::CancelSnapshotTimer()
{
    if (mTimerDelegate.IsTimerActive(&mSnapshotTimer))
    {
        mTimerDelegate.CancelTimer(&mSnapshotTimer);
    }
}

void ElectricalEnergyMeasurementCluster::SnapshotTimer::TimerFired()
{
    mCluster.DoGenerateSnapshots();
    mCluster.mLastReportTime = mCluster.mTimerDelegate.GetCurrentMonotonicTimestamp();
    mCluster.StartSnapshotTimer();
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

    // Get current timestamp
    uint32_t currentTimestamp;
    CHIP_ERROR err = System::Clock::GetClock_MatterEpochS(currentTimestamp);
    if (err == CHIP_NO_ERROR)
    {
        measurement.endTimestamp.SetValue(currentTimestamp);
    }
    else
    {
        System::Clock::Milliseconds64 sysTimeMs = mTimerDelegate.GetCurrentMonotonicTimestamp();
        measurement.endSystime.SetValue(static_cast<uint64_t>(sysTimeMs.count()));
    }

    return DataModel::MakeNullable(measurement);
}

void ElectricalEnergyMeasurementCluster::GenerateSnapshots()
{
    // Enforce min 1s between events
    System::Clock::Timestamp now = mTimerDelegate.GetCurrentMonotonicTimestamp();
    if (mLastReportTime != System::Clock::kZero && (now - mLastReportTime) < kMinReportInterval)
    {
        return;
    }

    DoGenerateSnapshots();
    StartSnapshotTimer();
}

void ElectricalEnergyMeasurementCluster::DoGenerateSnapshots()
{
    if (mFeatureFlags.Has(Feature::kCumulativeEnergy))
    {
        Nullable<EnergyMeasurementStruct> imported = DataModel::NullNullable;
        Nullable<EnergyMeasurementStruct> exported = DataModel::NullNullable;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            imported = BuildMeasurement(mDelegate.GetCumulativeEnergyImported(), mCumulativeImported, true);
        }
        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            exported = BuildMeasurement(mDelegate.GetCumulativeEnergyExported(), mCumulativeExported, true);
        }
        CumulativeEnergySnapshot(imported, exported);
    }

    if (mFeatureFlags.Has(Feature::kPeriodicEnergy))
    {
        Nullable<EnergyMeasurementStruct> imported = DataModel::NullNullable;
        Nullable<EnergyMeasurementStruct> exported = DataModel::NullNullable;

        if (mFeatureFlags.Has(Feature::kImportedEnergy))
        {
            imported = BuildMeasurement(mDelegate.GetPeriodicEnergyImported(), mPeriodicImported, false);
        }
        if (mFeatureFlags.Has(Feature::kExportedEnergy))
        {
            exported = BuildMeasurement(mDelegate.GetPeriodicEnergyExported(), mPeriodicExported, false);
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
    outValue = mCumulativeImported;
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
    outValue = mCumulativeExported;
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
    outValue = mPeriodicImported;
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
    outValue = mPeriodicExported;
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

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyImported(const Nullable<EnergyMeasurementStruct> & value)
{
    if (ValueChanged(mCumulativeImported, value))
    {
        const bool notify   = ShouldNotifyEnergyMeasurementAttribute(mCumulativeImported, value);
        mCumulativeImported = value;
        if (notify)
        {
            NotifyAttributeChanged(CumulativeEnergyImported::Id);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetCumulativeEnergyExported(const Nullable<EnergyMeasurementStruct> & value)
{
    if (ValueChanged(mCumulativeExported, value))
    {
        const bool notify   = ShouldNotifyEnergyMeasurementAttribute(mCumulativeExported, value);
        mCumulativeExported = value;
        if (notify)
        {
            NotifyAttributeChanged(CumulativeEnergyExported::Id);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyImported(const Nullable<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kImportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyImported requires kPeriodicEnergy and kImportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (ValueChanged(mPeriodicImported, value))
    {
        const bool notify = ShouldNotifyEnergyMeasurementAttribute(mPeriodicImported, value);
        mPeriodicImported = value;
        if (notify)
        {
            NotifyAttributeChanged(PeriodicEnergyImported::Id);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalEnergyMeasurementCluster::SetPeriodicEnergyExported(const Nullable<EnergyMeasurementStruct> & value)
{
    if (!mFeatureFlags.HasAll(ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                              ElectricalEnergyMeasurement::Feature::kExportedEnergy))
    {
        ChipLogError(Zcl,
                     "Electrical Energy Measurement: PeriodicEnergyExported requires kPeriodicEnergy and kExportedEnergy features");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (ValueChanged(mPeriodicExported, value))
    {
        const bool notify = ShouldNotifyEnergyMeasurementAttribute(mPeriodicExported, value);
        mPeriodicExported = value;
        if (notify)
        {
            NotifyAttributeChanged(PeriodicEnergyExported::Id);
        }
    }
    return CHIP_NO_ERROR;
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
        if (mCumulativeImported.IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mCumulativeImported.Value());

    case CumulativeEnergyExported::Id:
        if (mCumulativeExported.IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mCumulativeExported.Value());

    case PeriodicEnergyImported::Id:
        if (mPeriodicImported.IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mPeriodicImported.Value());

    case PeriodicEnergyExported::Id:
        if (mPeriodicExported.IsNull())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mPeriodicExported.Value());

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
        RETURN_SAFELY_IGNORED SetCumulativeEnergyImported(energyImported);
        event.energyImported.SetValue(energyImported.Value());
        hasData = true;
    }

    if (Features().Has(Feature::kExportedEnergy) && !energyExported.IsNull())
    {
        RETURN_SAFELY_IGNORED SetCumulativeEnergyExported(energyExported);
        event.energyExported.SetValue(energyExported.Value());
        hasData = true;
    }

    VerifyOrReturn(hasData && mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    mLastReportTime = mTimerDelegate.GetCurrentMonotonicTimestamp();
}

void ElectricalEnergyMeasurementCluster::PeriodicEnergySnapshot(const Nullable<EnergyMeasurementStruct> & energyImported,
                                                                const Nullable<EnergyMeasurementStruct> & energyExported)
{
    VerifyOrReturn(Features().Has(Feature::kPeriodicEnergy));
    Events::PeriodicEnergyMeasured::Type event;
    bool hasData = false;

    if (Features().Has(Feature::kImportedEnergy) && !energyImported.IsNull())
    {
        RETURN_SAFELY_IGNORED SetPeriodicEnergyImported(energyImported);
        event.energyImported.SetValue(energyImported.Value());
        hasData = true;
    }

    if (Features().Has(Feature::kExportedEnergy) && !energyExported.IsNull())
    {
        RETURN_SAFELY_IGNORED SetPeriodicEnergyExported(energyExported);
        event.energyExported.SetValue(energyExported.Value());
        hasData = true;
    }

    VerifyOrReturn(hasData && mContext != nullptr);
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    mLastReportTime = mTimerDelegate.GetCurrentMonotonicTimestamp();
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
