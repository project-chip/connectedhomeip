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

constexpr thermostat::SystemModeEnum kScheduleModes[] = {
    thermostat::SystemModeEnum::kHeat,
    thermostat::SystemModeEnum::kCool,
};

bool IsSupportedMode(thermostat::SystemModeEnum mode)
{
    if (mode == thermostat::SystemModeEnum::kOff || mode == thermostat::SystemModeEnum::kAuto ||
        mode == thermostat::SystemModeEnum::kHeat || mode == thermostat::SystemModeEnum::kCool)
    {
        return true;
    }
    return false;
}

FullThermostat::Context MakeLoggingContext(const LoggingThermostat::Context & context)
{
    FullThermostat::Context loggingContext{
        .timerDelegate      = context.timerDelegate,
        .optionalAttributes = context.optionalAttributes,
    };
    loggingContext.optionalAttributes.ThermostatRunningMode           = true;
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
    loggingContext.optionalAttributes.SetpointHoldExpiryTimestamp     = true;
    loggingContext.optionalAttributes.CriticalFreezeProtection        = true;
    loggingContext.optionalAttributes.CriticalOverheatProtection      = true;
    return loggingContext;
}
} // namespace

LoggingThermostat::LoggingThermostat(const Context & context) :
    FullThermostat(MakeLoggingContext(context), *this, *this, *this, *this, *this, *this, *this, *this),
    mFabricTable(context.fabricTable), mGroupDataProvider(context.groupDataProvider)
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
        preset.SetHeatingSetpoint(MakeOptional(thermostat::kDefaultHeatingSetpoint));
        preset.SetCoolingSetpoint(MakeOptional(thermostat::kDefaultCoolingSetpoint));
        ++mPresetCount;
    }

    // Start with a single built-in heating schedule with morning and evening transitions.
    auto & schedule        = mSchedules[0];
    const uint8_t handle[] = { to_underlying(thermostat::SystemModeEnum::kHeat) };
    VerifyOrDie(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle))) == CHIP_NO_ERROR);
    schedule.SetSystemMode(thermostat::SystemModeEnum::kHeat);
    VerifyOrDie(schedule.SetName(NullOptional) == CHIP_NO_ERROR);
    VerifyOrDie(schedule.SetPresetHandle(NullOptional) == CHIP_NO_ERROR);

    const BitMask<thermostat::ScheduleDayOfWeekBitmap> everyDay(0x7F);
    thermostat::Structs::ScheduleTransitionStruct::Type transitions[2];
    transitions[0].dayOfWeek       = everyDay;
    transitions[0].transitionTime  = 360; // 06:00
    transitions[0].heatingSetpoint = MakeOptional(static_cast<int16_t>(2100));
    transitions[1].dayOfWeek       = everyDay;
    transitions[1].transitionTime  = 1320; // 22:00
    transitions[1].heatingSetpoint = MakeOptional(static_cast<int16_t>(1800));
    VerifyOrDie(schedule.SetTransitions(DataModel::List<const thermostat::Structs::ScheduleTransitionStruct::Type>(transitions)) ==
                CHIP_NO_ERROR);
    schedule.SetBuiltIn(DataModel::MakeNullable(true));
    mScheduleCount = 1;
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

    if (!IsSupportedMode(mSystemMode))
    {
        mSystemMode = thermostat::SystemModeEnum::kOff;
    }

    // The cluster validates the complete setpoint state on every write; on startup the loaded
    // persisted state is re-checked the same way, and anything inconsistent falls back to defaults.
    if (!HasThermostatCluster() || !ThermostatCluster().GetSetpoints().Valid())
    {
        mMinHeatSetpointLimit = thermostat::kDefaultAbsMinHeatSetpointLimit;
        mMaxHeatSetpointLimit = thermostat::kDefaultAbsMaxHeatSetpointLimit;
        mMinCoolSetpointLimit = thermostat::kDefaultAbsMinCoolSetpointLimit;
        mMaxCoolSetpointLimit = thermostat::kDefaultAbsMaxCoolSetpointLimit;
        mHeatingSetpoint      = thermostat::kDefaultHeatingSetpoint;
        mCoolingSetpoint      = thermostat::kDefaultCoolingSetpoint;
    }

    UpdateSimulatedRunningState();
    return CHIP_NO_ERROR;
}

void LoggingThermostat::Shutdown(ClusterShutdownType type)
{
    mAttributeStorage = nullptr;
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
        static_assert(2 * kPresetCapacity + 1 <= UINT8_MAX);
        for (int handle = 1; handle <= 2 * kPresetCapacity + 1; ++handle)
        {
            const uint8_t handleByte = static_cast<uint8_t>(handle);
            const ByteSpan candidate(&handleByte, 1);
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

CHIP_ERROR LoggingThermostat::GetScheduleTypeAtIndex(size_t index, thermostat::Structs::ScheduleTypeStruct::Type & value)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(kScheduleModes), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value.systemMode        = kScheduleModes[index];
    value.numberOfSchedules = kScheduleCapacity;
    value.scheduleTypeFeatures =
        BitMask<thermostat::ScheduleTypeFeaturesBitmap>(thermostat::ScheduleTypeFeaturesBitmap::kSupportsSetpoints);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetScheduleAtIndex(size_t index, thermostat::ScheduleStructWithOwnedMembers & value)
{
    VerifyOrReturnError(index < mScheduleCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = mSchedules[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetPendingScheduleAtIndex(size_t index, thermostat::ScheduleStructWithOwnedMembers & value)
{
    VerifyOrReturnError(index < mPendingScheduleCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = mPendingSchedules[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & value)
{
    if (mActiveScheduleHandle.IsNull())
    {
        value.SetNull();
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(!value.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t handle[] = { mActiveScheduleHandle.Value() };
    return CopySpanToMutableSpan(ByteSpan(handle), value.Value());
}

CHIP_ERROR LoggingThermostat::SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & value)
{
    if (value.IsNull())
    {
        mActiveScheduleHandle.SetNull();
    }
    else
    {
        VerifyOrReturnError(value.Value().size() == 1, CHIP_ERROR_INVALID_ARGUMENT);
        mActiveScheduleHandle.SetNonNull(value.Value()[0]);
    }
    return CHIP_NO_ERROR;
}

void LoggingThermostat::InitializePendingSchedules()
{
    mPendingSchedules     = mSchedules;
    mPendingScheduleCount = mScheduleCount;
}

CHIP_ERROR LoggingThermostat::AppendToPendingScheduleList(const thermostat::ScheduleStructWithOwnedMembers & value)
{
    VerifyOrReturnError(mPendingScheduleCount < mPendingSchedules.size(), CHIP_ERROR_NO_MEMORY);
    auto & pending = mPendingSchedules[mPendingScheduleCount];
    pending        = value;
    if (pending.GetScheduleHandle().IsNull())
    {
        // Do not derive new handles from SystemMode so a new schedule cannot collide with a built-in schedule handle.
        static_assert(2 * kScheduleCapacity + 1 <= UINT8_MAX);
        for (int handle = 1; handle <= 2 * kScheduleCapacity + 1; ++handle)
        {
            const uint8_t handleByte = static_cast<uint8_t>(handle);
            const ByteSpan candidate(&handleByte, 1);
            const auto containsHandle = [&](const auto & schedules, uint8_t count) {
                for (uint8_t index = 0; index < count; ++index)
                {
                    const auto existing = schedules[index].GetScheduleHandle();
                    if (!existing.IsNull() && existing.Value().data_equal(candidate))
                    {
                        return true;
                    }
                }
                return false;
            };
            if (!containsHandle(mSchedules, mScheduleCount) && !containsHandle(mPendingSchedules, mPendingScheduleCount))
            {
                ReturnErrorOnFailure(pending.SetScheduleHandle(DataModel::MakeNullable(candidate)));
                break;
            }
        }
    }
    ++mPendingScheduleCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingThermostat::CommitPendingSchedules()
{
    mSchedules     = mPendingSchedules;
    mScheduleCount = mPendingScheduleCount;
    return CHIP_NO_ERROR;
}

std::optional<System::Clock::Milliseconds16> LoggingThermostat::GetMaxAtomicWriteTimeout(AttributeId attributeId)
{
    if (attributeId == thermostat::Attributes::Presets::Id)
    {
        return System::Clock::Milliseconds16(3000);
    }
    if (attributeId == thermostat::Attributes::Schedules::Id)
    {
        return System::Clock::Milliseconds16(9000);
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
    VerifyOrReturnValue(IsSupportedMode(value), Status::ConstraintError);
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
    // The cluster validates the complete setpoint state before saving it. Limit setters must not
    // compare against the other stored limit, which may still be awaiting its update.
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinHeatSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxHeatSetpointLimit,
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
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinHeatSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxHeatSetpointLimit,
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
    // As with heating limits, the other stored limit may not yet reflect the validated state.
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinCoolSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxCoolSetpointLimit,
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
    VerifyOrReturnValue(value >= thermostat::kDefaultAbsMinCoolSetpointLimit &&
                            value <= thermostat::kDefaultAbsMaxCoolSetpointLimit,
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
    changed = false;
    VerifyOrReturnValue(mRunningState == value, Status::Success);
    mRunningState = value;
    changed       = true;
    ChipLogProgress(AppServer, "Thermostat: RunningState set to 0x%04x", value.Raw());
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
        if ((mSystemMode == thermostat::SystemModeEnum::kHeat || mSystemMode == thermostat::SystemModeEnum::kAuto) &&
            currentTemp < mHeatingSetpoint)
        {
            newRunningMode = thermostat::ThermostatRunningModeEnum::kHeat;
            newRunningState.Set(thermostat::RelayStateBitmap::kHeat);
        }
        else if ((mSystemMode == thermostat::SystemModeEnum::kCool || mSystemMode == thermostat::SystemModeEnum::kAuto) &&
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
