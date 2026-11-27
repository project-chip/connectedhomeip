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

#include <device/types/device-energy-management/EnergyManagement.h>

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::DeviceEnergyManagement;

namespace {

constexpr Structs::PowerAdjustStruct::Type kDefaultPowerAdjustEntry = {
    .minPower    = 0,
    .maxPower    = 1000,
    .minDuration = 0,
    .maxDuration = 3600,
};

// Limits reported between the Power Adjustment and Power Adjustment Clear test event triggers.
// The default limits allow a duration of 0 seconds, which ends an adjustment as soon as it starts. These limits
// have a 10 second minimum, so an adjustment requested with the minimum duration stays active for 10 seconds.
constexpr Structs::PowerAdjustStruct::Type kTestEventPowerAdjustEntry = {
    .minPower    = 5'000'000,
    .maxPower    = 30'000'000,
    .minDuration = 10,
    .maxDuration = 60,
};

constexpr uint64_t kEventTriggerEndpointMask = 0x0000'FFFF'0000'0000;

bool IsOptedOut(OptOutStateEnum optOutState, PowerAdjustReasonEnum adjustmentReason)
{
    switch (adjustmentReason)
    {
    case PowerAdjustReasonEnum::kLocalOptimizationAdjustment:
        return optOutState == OptOutStateEnum::kLocalOptOut || optOutState == OptOutStateEnum::kOptOut;
    case PowerAdjustReasonEnum::kGridOptimizationAdjustment:
        return optOutState == OptOutStateEnum::kGridOptOut || optOutState == OptOutStateEnum::kOptOut;
    default:
        return false;
    }
}

// OptOutStateEnum values are bit flags: kLocalOptOut (0x01) | kGridOptOut (0x02) == kOptOut (0x03).
OptOutStateEnum AddOptOut(OptOutStateEnum current, OptOutStateEnum added)
{
    return static_cast<OptOutStateEnum>(chip::to_underlying(current) | chip::to_underlying(added));
}

constexpr int64_t kMillisecondsPerHour = 3'600'000;

// Energy in mW*ms. The value is converted to mWh once, for the whole adjustment, so that the division does not
// drop a remainder for every interval.
int64_t EnergyMilliwattMilliseconds(int64_t powerMilliwatts, chip::System::Clock::Milliseconds64 duration)
{
    const int64_t absolutePower = powerMilliwatts < 0 ? -powerMilliwatts : powerMilliwatts;
    return absolutePower * static_cast<int64_t>(duration.count());
}

} // namespace

namespace chip::app {

EnergyManagement::EnergyManagement(const Context & context) : EnergyManagement(context, Config{}) {}

EnergyManagement::EnergyManagement(const Context & context, Config config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDeviceEnergyManagement, 1)),
    mTimerDelegate(context.timerDelegate), mTestEventTriggerDelegate(context.testEventTriggerDelegate), mConfig(config),
    mPowerAdjustEntry(&kDefaultPowerAdjustEntry)
{
    if (!mConfig.featureFlags.Has(Feature::kPowerAdjustment))
    {
        return;
    }

    Structs::PowerAdjustCapabilityStruct::Type capabilityStruct;
    capabilityStruct.powerAdjustCapability.SetNonNull(
        DataModel::List<const Structs::PowerAdjustStruct::Type>(mPowerAdjustEntry, 1));
    capabilityStruct.cause = PowerAdjustReasonEnum::kNoAdjustment;
    mPowerAdjustmentCapability.SetNonNull(capabilityStruct);
}

EnergyManagement::~EnergyManagement()
{
    mTimerDelegate.CancelTimer(this);
    mTestEventTriggerDelegate.RemoveHandler(this);
}

CHIP_ERROR EnergyManagement::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(SingleEndpoint::mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    Clusters::DeviceEnergyManagementCluster::Config config(endpoint, mConfig.featureFlags, *this);
    mDemCluster.Create(config);
    ReturnErrorOnFailure(provider.AddCluster(mDemCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(this));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void EnergyManagement::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTestEventTriggerDelegate.RemoveHandler(this);
    mTimerDelegate.CancelTimer(this);
    mActivePowerAdjustment.reset();

    UnregisterDescriptor(provider);

    if (mDemCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mDemCluster.Cluster()));
        mDemCluster.Destroy();
    }

    mESAState    = ESAStateEnum::kOnline;
    mOptOutState = OptOutStateEnum::kNoOptOut;
    SetPowerAdjustEntry(kDefaultPowerAdjustEntry);
    if (!mPowerAdjustmentCapability.IsNull())
    {
        mPowerAdjustmentCapability.Value().cause = PowerAdjustReasonEnum::kNoAdjustment;
    }
}

Protocols::InteractionModel::Status EnergyManagement::PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                                         AdjustmentCauseEnum cause)
{
    using Protocols::InteractionModel::Status;

    VerifyOrReturnValue(!mPowerAdjustmentCapability.IsNull(), Status::Failure);

    mTimerDelegate.CancelTimer(this);
    VerifyOrReturnValue(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(duration)) == CHIP_NO_ERROR, Status::Failure,
                        EndPowerAdjustment(CauseEnum::kCancelled));

    const System::Clock::Timestamp now = mTimerDelegate.GetCurrentMonotonicTimestamp();
    const bool isNewAdjustment         = !mActivePowerAdjustment.has_value();
    if (isNewAdjustment)
    {
        mActivePowerAdjustment =
            ActivePowerAdjustment{ .startTime = now, .intervalStartTime = now, .power = power, .energyMilliwattMilliseconds = 0 };
    }
    else
    {
        ActivePowerAdjustment & adjustment = *mActivePowerAdjustment;
        adjustment.energyMilliwattMilliseconds += EnergyMilliwattMilliseconds(adjustment.power, now - adjustment.intervalStartTime);
        adjustment.intervalStartTime = now;
        adjustment.power             = power;
    }

    LogErrorOnFailure(SetESAState(ESAStateEnum::kPowerAdjustActive));
    LogErrorOnFailure(SetPowerAdjustmentCause(AdjustmentCauseToPowerAdjustReason(cause)));

    if (isNewAdjustment && mDemCluster.IsConstructed())
    {
        mDemCluster.Cluster().GenerateEvent(Events::PowerAdjustStart::Type{});
    }
    return Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::CancelPowerAdjustRequest()
{
    EndPowerAdjustment(CauseEnum::kCancelled);
    return Protocols::InteractionModel::Status::Success;
}

void EnergyManagement::TimerFired()
{
    EndPowerAdjustment(CauseEnum::kNormalCompletion);
}

void EnergyManagement::EndPowerAdjustment(CauseEnum cause)
{
    mTimerDelegate.CancelTimer(this);
    LogErrorOnFailure(SetESAState(ESAStateEnum::kOnline));
    if (!mPowerAdjustmentCapability.IsNull())
    {
        LogErrorOnFailure(SetPowerAdjustmentCause(PowerAdjustReasonEnum::kNoAdjustment));
    }

    VerifyOrReturn(mActivePowerAdjustment.has_value());
    const ActivePowerAdjustment adjustment = *mActivePowerAdjustment;
    mActivePowerAdjustment.reset();

    const System::Clock::Timestamp now = mTimerDelegate.GetCurrentMonotonicTimestamp();
    Events::PowerAdjustEnd::Type event;
    event.cause     = cause;
    event.duration  = static_cast<uint32_t>(((now - adjustment.startTime).count() + 500) / 1000);
    event.energyUse = (adjustment.energyMilliwattMilliseconds +
                       EnergyMilliwattMilliseconds(adjustment.power, now - adjustment.intervalStartTime)) /
        kMillisecondsPerHour;

    VerifyOrReturn(mDemCluster.IsConstructed());
    mDemCluster.Cluster().GenerateEvent(event);
}

void EnergyManagement::SetOptOutState(OptOutStateEnum state)
{
    if (mOptOutState != state)
    {
        mOptOutState = state;
        NotifyAttributeChanged(Attributes::OptOutState::Id);
    }

    if (mActivePowerAdjustment.has_value() && !mPowerAdjustmentCapability.IsNull() &&
        IsOptedOut(mOptOutState, mPowerAdjustmentCapability.Value().cause))
    {
        EndPowerAdjustment(CauseEnum::kUserOptOut);
    }
}

CHIP_ERROR EnergyManagement::HandleEventTrigger(uint64_t eventTrigger)
{
    const EndpointId triggerEndpoint = static_cast<EndpointId>((eventTrigger & kEventTriggerEndpointMask) >> 32);
    VerifyOrReturnError(triggerEndpoint == kRootEndpointId || triggerEndpoint == GetEndpointId(), CHIP_ERROR_INVALID_ARGUMENT);

    const bool supportsPowerAdjustment = mConfig.featureFlags.Has(Feature::kPowerAdjustment);

    switch (static_cast<DeviceEnergyManagementTrigger>(clearEndpointInEventTrigger(eventTrigger)))
    {
    case DeviceEnergyManagementTrigger::kPowerAdjustment:
        VerifyOrReturnError(supportsPowerAdjustment, CHIP_ERROR_INVALID_ARGUMENT);
        SetPowerAdjustEntry(kTestEventPowerAdjustEntry);
        return CHIP_NO_ERROR;
    case DeviceEnergyManagementTrigger::kPowerAdjustmentClear:
        VerifyOrReturnError(supportsPowerAdjustment, CHIP_ERROR_INVALID_ARGUMENT);
        SetPowerAdjustEntry(kDefaultPowerAdjustEntry);
        return CHIP_NO_ERROR;
    case DeviceEnergyManagementTrigger::kUserOptOutLocalOptimization:
        SetOptOutState(AddOptOut(mOptOutState, OptOutStateEnum::kLocalOptOut));
        return CHIP_NO_ERROR;
    case DeviceEnergyManagementTrigger::kUserOptOutGridOptimization:
        SetOptOutState(AddOptOut(mOptOutState, OptOutStateEnum::kGridOptOut));
        return CHIP_NO_ERROR;
    case DeviceEnergyManagementTrigger::kUserOptOutClearAll:
        SetOptOutState(OptOutStateEnum::kNoOptOut);
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void EnergyManagement::SetPowerAdjustEntry(const Structs::PowerAdjustStruct::Type & entry)
{
    VerifyOrReturn(mPowerAdjustEntry != &entry);

    // The active adjustment was validated against the previous limits.
    if (mActivePowerAdjustment.has_value())
    {
        EndPowerAdjustment(CauseEnum::kCancelled);
    }

    mPowerAdjustEntry = &entry;
    NotifyAttributeChanged(Attributes::AbsMinPower::Id);
    NotifyAttributeChanged(Attributes::AbsMaxPower::Id);

    VerifyOrReturn(!mPowerAdjustmentCapability.IsNull());
    mPowerAdjustmentCapability.Value().powerAdjustCapability.SetNonNull(
        DataModel::List<const Structs::PowerAdjustStruct::Type>(mPowerAdjustEntry, 1));
    NotifyAttributeChanged(Attributes::PowerAdjustmentCapability::Id);
}

void EnergyManagement::NotifyAttributeChanged(AttributeId attribute)
{
    VerifyOrReturn(mDemCluster.IsConstructed());
    mDemCluster.Cluster().NotifyAttributeChanged(attribute);
}

Protocols::InteractionModel::Status EnergyManagement::StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                                             AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::ResumeRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
EnergyManagement::ModifyForecastRequest(const uint32_t forecastID,
                                        const DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
                                        AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
EnergyManagement::RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::Type> & constraints,
                                                 AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::CancelRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::PowerRangeAdjustRequest(const Optional<int64_t> minPower,
                                                                              const Optional<int64_t> maxPower,
                                                                              const uint32_t duration, AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::CancelPowerRangeAdjustRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

ESATypeEnum EnergyManagement::GetESAType()
{
    return mConfig.esaType;
}

bool EnergyManagement::GetESACanGenerate()
{
    return mConfig.esaCanGenerate;
}

ESAStateEnum EnergyManagement::GetESAState()
{
    return mESAState;
}

int64_t EnergyManagement::GetAbsMinPower()
{
    return mPowerAdjustEntry->minPower;
}

int64_t EnergyManagement::GetAbsMaxPower()
{
    return mPowerAdjustEntry->maxPower;
}

OptOutStateEnum EnergyManagement::GetOptOutState()
{
    return mOptOutState;
}

const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & EnergyManagement::GetPowerAdjustmentCapability()
{
    return mPowerAdjustmentCapability;
}

const DataModel::Nullable<Structs::ForecastStruct::Type> & EnergyManagement::GetForecast()
{
    return mForecast;
}

const DataModel::Nullable<Structs::PowerRangeAdjustStruct::Type> & EnergyManagement::GetPowerRangeAdjustment()
{
    return mPowerRangeAdjustment;
}

CHIP_ERROR EnergyManagement::SetESAState(ESAStateEnum state)
{
    VerifyOrReturnValue(mESAState != state, CHIP_NO_ERROR);
    mESAState = state;

    NotifyAttributeChanged(Attributes::ESAState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyManagement::SetPowerAdjustmentCause(PowerAdjustReasonEnum cause)
{
    VerifyOrReturnError(!mPowerAdjustmentCapability.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mPowerAdjustmentCapability.Value().cause != cause, CHIP_NO_ERROR);
    mPowerAdjustmentCapability.Value().cause = cause;

    NotifyAttributeChanged(Attributes::PowerAdjustmentCapability::Id);
    return CHIP_NO_ERROR;
}

} // namespace chip::app
