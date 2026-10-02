/*
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

#include <device/types/thermostat/impl/LoggingThermostat.h>

#include <app/persistence/AttributePersistence.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {
namespace thermostat = Clusters::Thermostat;
using Protocols::InteractionModel::Status;

constexpr thermostat::PresetScenarioEnum kPresetScenarios[] = {
    thermostat::PresetScenarioEnum::kOccupied,
    thermostat::PresetScenarioEnum::kUnoccupied,
    thermostat::PresetScenarioEnum::kUserDefined,
};

bool IsSupportedMode(thermostat::SystemModeEnum mode, const BitFlags<thermostat::Feature> & features)
{
    if (mode == thermostat::SystemModeEnum::kOff)
    {
        return true;
    }
    if (mode == thermostat::SystemModeEnum::kHeat)
    {
        return features.Has(thermostat::Feature::kHeating);
    }
    if (mode == thermostat::SystemModeEnum::kCool)
    {
        return features.Has(thermostat::Feature::kCooling);
    }
    return false;
}

Thermostat::Context MakeLoggingContext(const LoggingThermostat::Context & context)
{
    Thermostat::Context loggingContext{
        .timerDelegate      = context.timerDelegate,
        .features           = context.features,
        .optionalAttributes = context.optionalAttributes,
    };
    loggingContext.optionalAttributes.ThermostatRunningState          = true;
    loggingContext.optionalAttributes.RemoteSensing                   = true;
    loggingContext.optionalAttributes.LocalTemperatureCalibration     = true;
    loggingContext.optionalAttributes.AbsMinHeatSetpointLimit         = true;
    loggingContext.optionalAttributes.AbsMaxHeatSetpointLimit         = true;
    loggingContext.optionalAttributes.AbsMinCoolSetpointLimit         = true;
    loggingContext.optionalAttributes.AbsMaxCoolSetpointLimit         = true;
    loggingContext.optionalAttributes.MinHeatSetpointLimit            = true;
    loggingContext.optionalAttributes.MaxHeatSetpointLimit            = true;
    loggingContext.optionalAttributes.MinCoolSetpointLimit            = true;
    loggingContext.optionalAttributes.MaxCoolSetpointLimit            = true;
    loggingContext.optionalAttributes.TemperatureSetpointHold         = true;
    loggingContext.optionalAttributes.TemperatureSetpointHoldDuration = true;
    loggingContext.optionalAttributes.CriticalFreezeProtection        = context.features.Has(thermostat::Feature::kHeating);
    loggingContext.optionalAttributes.CriticalOverheatProtection      = context.features.Has(thermostat::Feature::kCooling);
    return loggingContext;
}
} // namespace

LoggingThermostat::LoggingThermostat(const Context & context) :
    Thermostat(MakeLoggingContext(context), *this, *this, *this, *this, *this, *this, *this), mFabricTable(context.fabricTable),
    mGroupDataProvider(context.groupDataProvider)
{
    static_assert(MATTER_ARRAY_SIZE(kPresetScenarios) == kPresetCapacity);
    // Start with two built-in presets, leaving room for a user-defined preset.
    for (uint8_t index = 0; index < 2; ++index)
    {
        auto & preset          = mPresets[index];
        const uint8_t handle[] = { to_underlying(kPresetScenarios[index]) };
        VerifyOrDie(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))) == CHIP_NO_ERROR);
        preset.SetPresetScenario(kPresetScenarios[index]);
        preset.SetBuiltIn(DataModel::MakeNullable(true));
        if (Features().Has(thermostat::Feature::kHeating))
        {
            preset.SetHeatingSetpoint(MakeOptional(thermostat::kDefaultHeatingSetpoint));
        }
        if (Features().Has(thermostat::Feature::kCooling))
        {
            preset.SetCoolingSetpoint(MakeOptional(thermostat::kDefaultCoolingSetpoint));
        }
        ++mPresetCount;
    }
}

CHIP_ERROR LoggingThermostat::RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mGroupsCluster.Create(endpoint,
                          Clusters::GroupsCluster::Context{
                              .groupDataProvider   = mGroupDataProvider,
                              .identifyIntegration = &IdentifyCluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    Clusters::ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes.Set<Clusters::ThermostatUserInterfaceConfiguration::Attributes::ScheduleProgrammingVisibility::Id>();
    mUserInterfaceCluster.Create(endpoint, config);
    mUserInterfaceCluster.Cluster().SetDelegate(this);
    ReturnErrorOnFailure(provider.AddCluster(mUserInterfaceCluster.Registration()));

    return CHIP_NO_ERROR;
}

void LoggingThermostat::UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mUserInterfaceCluster.IsConstructed())
    {
        mUserInterfaceCluster.Cluster().SetDelegate(nullptr);
        LogErrorOnFailure(provider.RemoveCluster(&mUserInterfaceCluster.Cluster()));
        mUserInterfaceCluster.Destroy();
    }
    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }
}

CHIP_ERROR LoggingThermostat::Startup(ServerClusterContext & context)
{
    // Startup is called for the main delegate and both setpoint delegates.
    VerifyOrReturnError(mAttributeStorage == nullptr, CHIP_NO_ERROR);
    mAttributeStorage = &context.attributeStorage;
    AttributePersistence persistence(*mAttributeStorage);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::SystemMode::Id }, mSystemMode,
                                      thermostat::SystemModeEnum::kOff);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedHeatingSetpoint::Id },
                                      mHeatingSetpoint, thermostat::kDefaultHeatingSetpoint);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedCoolingSetpoint::Id },
                                      mCoolingSetpoint, thermostat::kDefaultCoolingSetpoint);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MinHeatSetpointLimit::Id },
                                      mMinHeatSetpointLimit, thermostat::kDefaultAbsMinHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MaxHeatSetpointLimit::Id },
                                      mMaxHeatSetpointLimit, thermostat::kDefaultAbsMaxHeatSetpointLimit);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MinCoolSetpointLimit::Id },
                                      mMinCoolSetpointLimit, thermostat::kDefaultAbsMinCoolSetpointLimit);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MaxCoolSetpointLimit::Id },
                                      mMaxCoolSetpointLimit, thermostat::kDefaultAbsMaxCoolSetpointLimit);
    persistence.LoadNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::TemperatureSetpointHold::Id },
                                      mSetpointHold, thermostat::TemperatureSetpointHoldEnum::kSetpointHoldOff);
    persistence.LoadNativeEndianValue<uint16_t>(
        { GetEndpointId(), thermostat::Id, thermostat::Attributes::TemperatureSetpointHoldDuration::Id }, mSetpointHoldDuration,
        DataModel::NullNullable);
    persistence.LoadNativeEndianValue<uint32_t>(
        { GetEndpointId(), thermostat::Id, thermostat::Attributes::SetpointHoldExpiryTimestamp::Id }, mSetpointHoldExpiryTimestamp,
        DataModel::NullNullable);

    if (!IsSupportedMode(mSystemMode, Features()))
    {
        mSystemMode = thermostat::SystemModeEnum::kOff;
    }

    if (mMinHeatSetpointLimit < thermostat::kDefaultAbsMinHeatSetpointLimit ||
        mMinHeatSetpointLimit > thermostat::kDefaultAbsMaxHeatSetpointLimit)
    {
        mMinHeatSetpointLimit = thermostat::kDefaultAbsMinHeatSetpointLimit;
    }
    if (mMaxHeatSetpointLimit < thermostat::kDefaultAbsMinHeatSetpointLimit ||
        mMaxHeatSetpointLimit > thermostat::kDefaultAbsMaxHeatSetpointLimit)
    {
        mMaxHeatSetpointLimit = thermostat::kDefaultAbsMaxHeatSetpointLimit;
    }
    if (mMinCoolSetpointLimit < thermostat::kDefaultAbsMinCoolSetpointLimit ||
        mMinCoolSetpointLimit > thermostat::kDefaultAbsMaxCoolSetpointLimit)
    {
        mMinCoolSetpointLimit = thermostat::kDefaultAbsMinCoolSetpointLimit;
    }
    if (mMaxCoolSetpointLimit < thermostat::kDefaultAbsMinCoolSetpointLimit ||
        mMaxCoolSetpointLimit > thermostat::kDefaultAbsMaxCoolSetpointLimit)
    {
        mMaxCoolSetpointLimit = thermostat::kDefaultAbsMaxCoolSetpointLimit;
    }

    if (mHeatingSetpoint < mMinHeatSetpointLimit || mHeatingSetpoint > mMaxHeatSetpointLimit)
    {
        mHeatingSetpoint = thermostat::kDefaultHeatingSetpoint;
    }
    if (mCoolingSetpoint < mMinCoolSetpointLimit || mCoolingSetpoint > mMaxCoolSetpointLimit)
    {
        mCoolingSetpoint = thermostat::kDefaultCoolingSetpoint;
    }

    UpdateSimulatedRunningState();
    return CHIP_NO_ERROR;
}

void LoggingThermostat::Shutdown(ClusterShutdownType type)
{
    mAttributeStorage = nullptr;
}

Status LoggingThermostat::GetCriticalFreezeProtection(bool & value) const
{
    value = false;
    return Status::Success;
}

Status LoggingThermostat::GetCriticalOverheatProtection(bool & value) const
{
    value = false;
    return Status::Success;
}

CHIP_ERROR LoggingThermostat::GetPresetTypeAtIndex(size_t index, thermostat::Structs::PresetTypeStruct::Type & value)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kPresetScenarios), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value.presetScenario     = kPresetScenarios[index];
    value.numberOfPresets    = 1;
    value.presetTypeFeatures = BitMask<thermostat::PresetTypeFeaturesBitmap>(thermostat::PresetTypeFeaturesBitmap::kSupportsNames);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetPresetAtIndex(size_t index, thermostat::PresetStructWithOwnedMembers & value)
{
    VerifyOrReturnError(index < mPresetCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = mPresets[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetPendingPresetAtIndex(size_t index, thermostat::PresetStructWithOwnedMembers & value)
{
    VerifyOrReturnError(index < mPendingPresetCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = mPendingPresets[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & value)
{
    if (mActivePresetHandle.IsNull())
    {
        value.SetNull();
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(!value.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t handle[] = { mActivePresetHandle.Value() };
    return CopySpanToMutableSpan(ByteSpan(handle), value.Value());
}

CHIP_ERROR LoggingThermostat::SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & value)
{
    if (value.IsNull())
    {
        mActivePresetHandle.SetNull();
    }
    else
    {
        VerifyOrReturnError(value.Value().size() == 1, CHIP_ERROR_INVALID_ARGUMENT);
        mActivePresetHandle.SetNonNull(value.Value()[0]);
    }
    return CHIP_NO_ERROR;
}

void LoggingThermostat::InitializePendingPresets()
{
    mPendingPresets     = mPresets;
    mPendingPresetCount = mPresetCount;
}

CHIP_ERROR LoggingThermostat::AppendToPendingPresetList(const thermostat::PresetStructWithOwnedMembers & value)
{
    VerifyOrReturnError(mPendingPresetCount < mPendingPresets.size(), CHIP_ERROR_NO_MEMORY);
    auto & pending = mPendingPresets[mPendingPresetCount];
    pending        = value;
    if (pending.GetPresetHandle().IsNull())
    {
        // Do not derive new handles from the scenario: an existing preset can change scenarios.
        // At most kPresetCapacity handles in each list are in use, so this search always has a free slot.
        for (uint8_t handle = 1; handle <= 2 * kPresetCapacity + 1; ++handle)
        {
            const ByteSpan candidate(&handle, 1);
            const auto containsHandle = [&](const auto & presets, uint8_t count) {
                for (uint8_t index = 0; index < count; ++index)
                {
                    const auto existing = presets[index].GetPresetHandle();
                    if (!existing.IsNull() && existing.Value().data_equal(candidate))
                    {
                        return true;
                    }
                }
                return false;
            };
            if (!containsHandle(mPresets, mPresetCount) && !containsHandle(mPendingPresets, mPendingPresetCount))
            {
                ReturnErrorOnFailure(pending.SetPresetHandle(DataModel::MakeNullable(candidate)));
                break;
            }
        }
    }
    ++mPendingPresetCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::CommitPendingPresets()
{
    mPresets     = mPendingPresets;
    mPresetCount = mPendingPresetCount;
    return CHIP_NO_ERROR;
}

std::optional<System::Clock::Milliseconds16> LoggingThermostat::GetMaxAtomicWriteTimeout(AttributeId attributeId)
{
    if (attributeId == thermostat::Attributes::Presets::Id)
    {
        return System::Clock::Milliseconds16(3000);
    }
    return std::nullopt;
}

Status LoggingThermostat::SetTemperatureSetpointHold(thermostat::TemperatureSetpointHoldEnum value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(mSetpointHold != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    ReturnValueOnFailure(AttributePersistence(*mAttributeStorage)
                             .StoreNativeEndianValue(
                                 { GetEndpointId(), thermostat::Id, thermostat::Attributes::TemperatureSetpointHold::Id }, value),
                         Status::Failure);
    mSetpointHold = value;
    changed       = true;
    return Status::Success;
}

Status LoggingThermostat::SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(mSetpointHoldDuration != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    ReturnValueOnFailure(
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue(
                { GetEndpointId(), thermostat::Id, thermostat::Attributes::TemperatureSetpointHoldDuration::Id }, value),
        Status::Failure);
    mSetpointHoldDuration = value;
    changed               = true;
    return Status::Success;
}

Status LoggingThermostat::SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(mSetpointHoldExpiryTimestamp != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    ReturnValueOnFailure(
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::SetpointHoldExpiryTimestamp::Id },
                                    value),
        Status::Failure);
    mSetpointHoldExpiryTimestamp = value;
    changed                      = true;
    return Status::Success;
}

void LoggingThermostat::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "Thermostat: Identify started");
}

void LoggingThermostat::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "Thermostat: Identify stopped");
}

Status LoggingThermostat::SetLocalTemperature(DataModel::Nullable<int16_t> value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value.IsNull() || value.Value() >= -27315, Status::ConstraintError);
    changed           = mLocalTemperature != value;
    mLocalTemperature = value;
    if (changed)
    {
        UpdateSimulatedRunningState();
    }
    return Status::Success;
}

Status LoggingThermostat::SetSystemMode(thermostat::SystemModeEnum value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(IsSupportedMode(value, Features()), Status::ConstraintError);
    VerifyOrReturnValue(mSystemMode != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::SystemMode::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting SystemMode failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mSystemMode = value;
    changed     = true;
    ChipLogProgress(AppServer, "Thermostat: SystemMode changed to %u", static_cast<unsigned>(value));
    UpdateSimulatedRunningState();
    return Status::Success;
}

thermostat::ControlSequenceOfOperationEnum LoggingThermostat::GetControlSequenceOfOperation() const
{
    if (Features().Has(thermostat::Feature::kHeating) && Features().Has(thermostat::Feature::kCooling))
    {
        return thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeating;
    }
    if (Features().Has(thermostat::Feature::kHeating))
    {
        return thermostat::ControlSequenceOfOperationEnum::kHeatingOnly;
    }
    if (Features().Has(thermostat::Feature::kCooling))
    {
        return thermostat::ControlSequenceOfOperationEnum::kCoolingOnly;
    }
    return thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeating;
}

Status LoggingThermostat::SetControlSequenceOfOperation(thermostat::ControlSequenceOfOperationEnum value, bool & changed)
{
    changed = false;
    return value == GetControlSequenceOfOperation() ? Status::Success : Status::ConstraintError;
}

Status LoggingThermostat::GetOccupiedHeatingSetpoint(int16_t & value) const
{
    value = mHeatingSetpoint;
    return Status::Success;
}

Status LoggingThermostat::SetOccupiedHeatingSetpoint(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= mMinHeatSetpointLimit && value <= mMaxHeatSetpointLimit, Status::ConstraintError);
    VerifyOrReturnValue(mHeatingSetpoint != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err = AttributePersistence(*mAttributeStorage)
                         .StoreNativeEndianValue(
                             { GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedHeatingSetpoint::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting heating setpoint failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mHeatingSetpoint = value;
    changed          = true;
    ChipLogProgress(AppServer, "Thermostat: heating setpoint changed to %d", value);
    UpdateSimulatedRunningState();
    return Status::Success;
}

Status LoggingThermostat::GetOccupiedCoolingSetpoint(int16_t & value) const
{
    value = mCoolingSetpoint;
    return Status::Success;
}

Status LoggingThermostat::SetOccupiedCoolingSetpoint(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= mMinCoolSetpointLimit && value <= mMaxCoolSetpointLimit, Status::ConstraintError);
    VerifyOrReturnValue(mCoolingSetpoint != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err = AttributePersistence(*mAttributeStorage)
                         .StoreNativeEndianValue(
                             { GetEndpointId(), thermostat::Id, thermostat::Attributes::OccupiedCoolingSetpoint::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting cooling setpoint failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mCoolingSetpoint = value;
    changed          = true;
    ChipLogProgress(AppServer, "Thermostat: cooling setpoint changed to %d", value);
    UpdateSimulatedRunningState();
    return Status::Success;
}

Status LoggingThermostat::GetAbsMinHeatSetpointLimit(int16_t & value) const
{
    value = thermostat::kDefaultAbsMinHeatSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::GetAbsMaxHeatSetpointLimit(int16_t & value) const
{
    value = thermostat::kDefaultAbsMaxHeatSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::GetMinHeatSetpointLimit(int16_t & value) const
{
    value = mMinHeatSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::SetMinHeatSetpointLimit(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinHeatSetpointLimit && value <= mMaxHeatSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mMinHeatSetpointLimit != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MinHeatSetpointLimit::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting min heating setpoint limit failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mMinHeatSetpointLimit = value;
    changed               = true;
    ChipLogProgress(AppServer, "Thermostat: min heating setpoint limit changed to %d", value);
    return Status::Success;
}

Status LoggingThermostat::GetMaxHeatSetpointLimit(int16_t & value) const
{
    value = mMaxHeatSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::SetMaxHeatSetpointLimit(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= mMinHeatSetpointLimit && value <= thermostat::kDefaultAbsMaxHeatSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mMaxHeatSetpointLimit != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MaxHeatSetpointLimit::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting max heating setpoint limit failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mMaxHeatSetpointLimit = value;
    changed               = true;
    ChipLogProgress(AppServer, "Thermostat: max heating setpoint limit changed to %d", value);
    return Status::Success;
}

Status LoggingThermostat::GetAbsMinCoolSetpointLimit(int16_t & value) const
{
    value = thermostat::kDefaultAbsMinCoolSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::GetAbsMaxCoolSetpointLimit(int16_t & value) const
{
    value = thermostat::kDefaultAbsMaxCoolSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::GetMinCoolSetpointLimit(int16_t & value) const
{
    value = mMinCoolSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::SetMinCoolSetpointLimit(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinCoolSetpointLimit && value <= mMaxCoolSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mMinCoolSetpointLimit != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MinCoolSetpointLimit::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting min cooling setpoint limit failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mMinCoolSetpointLimit = value;
    changed               = true;
    ChipLogProgress(AppServer, "Thermostat: min cooling setpoint limit changed to %d", value);
    return Status::Success;
}

Status LoggingThermostat::GetMaxCoolSetpointLimit(int16_t & value) const
{
    value = mMaxCoolSetpointLimit;
    return Status::Success;
}

Status LoggingThermostat::SetMaxCoolSetpointLimit(int16_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= mMinCoolSetpointLimit && value <= thermostat::kDefaultAbsMaxCoolSetpointLimit,
                        Status::ConstraintError);
    VerifyOrReturnValue(mMaxCoolSetpointLimit != value, Status::Success);
    VerifyOrReturnValue(mAttributeStorage != nullptr, Status::Failure);
    CHIP_ERROR err =
        AttributePersistence(*mAttributeStorage)
            .StoreNativeEndianValue({ GetEndpointId(), thermostat::Id, thermostat::Attributes::MaxCoolSetpointLimit::Id }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Thermostat: persisting max cooling setpoint limit failed: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    mMaxCoolSetpointLimit = value;
    changed               = true;
    ChipLogProgress(AppServer, "Thermostat: max cooling setpoint limit changed to %d", value);
    return Status::Success;
}

Status LoggingThermostat::GetMinDeadband(int16_t & value) const
{
    value = thermostat::kDefaultDeadBand;
    return Status::Success;
}

Status LoggingThermostat::GetRunningMode(thermostat::ThermostatRunningModeEnum & value) const
{
    value = mRunningMode;
    return Status::Success;
}

Status LoggingThermostat::SetRunningMode(thermostat::ThermostatRunningModeEnum value, bool & changed)
{
    changed = false;
    if (value == thermostat::ThermostatRunningModeEnum::kHeat && !Features().Has(thermostat::Feature::kHeating))
    {
        return Status::ConstraintError;
    }
    if (value == thermostat::ThermostatRunningModeEnum::kCool && !Features().Has(thermostat::Feature::kCooling))
    {
        return Status::ConstraintError;
    }
    VerifyOrReturnValue(value == thermostat::ThermostatRunningModeEnum::kOff ||
                            value == thermostat::ThermostatRunningModeEnum::kCool ||
                            value == thermostat::ThermostatRunningModeEnum::kHeat,
                        Status::InvalidValue);
    changed      = (mRunningMode != value);
    mRunningMode = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RunningMode set to %u", static_cast<unsigned>(value));
    }
    return Status::Success;
}

Status LoggingThermostat::GetRunningState(BitMask<thermostat::RelayStateBitmap> & value) const
{
    value = mRunningState;
    return Status::Success;
}

Status LoggingThermostat::SetRunningState(BitMask<thermostat::RelayStateBitmap> value, bool & changed)
{
    if (value.HasAny(thermostat::RelayStateBitmap::kHeat, thermostat::RelayStateBitmap::kHeatStage2) &&
        !Features().Has(thermostat::Feature::kHeating))
    {
        return Status::ConstraintError;
    }
    if (value.HasAny(thermostat::RelayStateBitmap::kCool, thermostat::RelayStateBitmap::kCoolStage2) &&
        !Features().Has(thermostat::Feature::kCooling))
    {
        return Status::ConstraintError;
    }
    changed       = (mRunningState != value);
    mRunningState = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RunningState set to 0x%04x", value.Raw());
    }
    return Status::Success;
}

int8_t LoggingThermostat::GetLocalTemperatureCalibration() const
{
    return mCalibration;
}

Status LoggingThermostat::SetLocalTemperatureCalibration(int8_t value, bool & changed)
{
    changed = false;
    VerifyOrReturnValue(value >= -30 && value <= 30, Status::ConstraintError);
    changed      = (mCalibration != value);
    mCalibration = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: LocalTemperatureCalibration set to %d", value);
    }
    return Status::Success;
}

Status LoggingThermostat::GetRemoteSensing(BitMask<thermostat::RemoteSensingBitmap> & value) const
{
    value = mRemoteSensing;
    return Status::Success;
}

Status LoggingThermostat::SetRemoteSensing(BitMask<thermostat::RemoteSensingBitmap> value, bool & changed)
{
    changed        = (mRemoteSensing != value);
    mRemoteSensing = value;
    if (changed)
    {
        ChipLogProgress(AppServer, "Thermostat: RemoteSensing set to 0x%02x", value.Raw());
    }
    return Status::Success;
}

void LoggingThermostat::UpdateSimulatedRunningState()
{
    thermostat::ThermostatRunningModeEnum newRunningMode = thermostat::ThermostatRunningModeEnum::kOff;
    BitMask<thermostat::RelayStateBitmap> newRunningState;

    if (!mLocalTemperature.IsNull())
    {
        int16_t currentTemp = mLocalTemperature.Value();
        if (Features().Has(thermostat::Feature::kHeating) && mSystemMode == thermostat::SystemModeEnum::kHeat &&
            currentTemp < mHeatingSetpoint)
        {
            newRunningMode = thermostat::ThermostatRunningModeEnum::kHeat;
            newRunningState.Set(thermostat::RelayStateBitmap::kHeat);
        }
        else if (Features().Has(thermostat::Feature::kCooling) && mSystemMode == thermostat::SystemModeEnum::kCool &&
                 currentTemp > mCoolingSetpoint)
        {
            newRunningMode = thermostat::ThermostatRunningModeEnum::kCool;
            newRunningState.Set(thermostat::RelayStateBitmap::kCool);
        }
    }

    if (HasThermostatCluster())
    {
        ThermostatCluster().SetRunningMode(newRunningMode);
        ThermostatCluster().SetRunningState(newRunningState);
    }
    else
    {
        mRunningMode  = newRunningMode;
        mRunningState = newRunningState;
    }
}

void LoggingThermostat::OnTemperatureDisplayModeChanged(
    Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value)
{
    ChipLogProgress(AppServer, "Thermostat: TemperatureDisplayMode changed to %u", static_cast<unsigned>(value));
}

void LoggingThermostat::OnKeypadLockoutChanged(Clusters::ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value)
{
    ChipLogProgress(AppServer, "Thermostat: KeypadLockout changed to %u", static_cast<unsigned>(value));
}

} // namespace chip::app
