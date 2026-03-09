/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Commands.h>
#include <clusters/PowerSource/Enums.h>
#include <clusters/PowerSource/Events.h>
#include <lib/support/ScopedBuffer.h>

#include <variant>
#include <algorithm>

namespace chip {
namespace app {
namespace Clusters {

class PowerSourceCluster : public DefaultServerCluster
{
public:
    using PowerSourceStatusEnum = PowerSource::PowerSourceStatusEnum;
    using WiredCurrentTypeEnum = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum = PowerSource::WiredFaultEnum;
    using BatChargeLevelEnum = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum = PowerSource::BatFaultEnum;
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    using BatChargeStateEnum = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum = PowerSource::BatChargeFaultEnum;

    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
        PowerSource::Attributes::WiredAssessedInputVoltage::Id,
        PowerSource::Attributes::WiredAssessedInputFrequency::Id,
        PowerSource::Attributes::WiredCurrentType::Id,
        PowerSource::Attributes::WiredAssessedCurrent::Id,
        PowerSource::Attributes::WiredNominalVoltage::Id,
        PowerSource::Attributes::WiredMaximumCurrent::Id,
        PowerSource::Attributes::WiredPresent::Id,
        PowerSource::Attributes::ActiveWiredFaults::Id,
        PowerSource::Attributes::BatVoltage::Id,
        PowerSource::Attributes::BatPercentRemaining::Id,
        PowerSource::Attributes::BatTimeRemaining::Id,
        PowerSource::Attributes::BatChargeLevel::Id,
        PowerSource::Attributes::BatReplacementNeeded::Id,
        PowerSource::Attributes::BatReplaceability::Id,
        PowerSource::Attributes::BatPresent::Id,
        PowerSource::Attributes::ActiveBatFaults::Id,
        PowerSource::Attributes::BatReplacementDescription::Id,
        PowerSource::Attributes::BatCommonDesignation::Id,
        PowerSource::Attributes::BatANSIDesignation::Id,
        PowerSource::Attributes::BatIECDesignation::Id,
        PowerSource::Attributes::BatApprovedChemistry::Id,
        PowerSource::Attributes::BatCapacity::Id,
        PowerSource::Attributes::BatQuantity::Id,
        PowerSource::Attributes::BatChargeState::Id,
        PowerSource::Attributes::BatTimeToFullCharge::Id,
        PowerSource::Attributes::BatFunctionalWhileCharging::Id,
        PowerSource::Attributes::BatChargingCurrent::Id,
        PowerSource::Attributes::ActiveBatChargeFaults::Id>

    struct WiredConfiguration
    {
        CharSpan description;
        WiredCurrentTypeEnum currentType;
        uint32_t nominalVoltage;
        uint32_t maximumCurrent;

        // To force the user to specify these mandatory attributes (taking the corresponding feature into account).
        WiredConfiguration(CharSpan description, WiredCurrentTypeEnum currentType):
            description(description), currentType(currentType)
        {}
    }

    struct BatteryConfiguration
    {
        CharSpan description;
        BatReplaceabilityEnum replaceability;
        CharSpan replacementDescription;
        BatCommonDesignationEnum commonDesignation;
        CharSpan ansiDesignation;
        CharSpan iecDesignation;
        BatApprovedChemistryEnum approvedChemistry;
        uint32_t capacity;
        uint8_t quantity;

        // To force the user to specify these mandatory attributes (taking the corresponding feature into account).
        BatteryConfiguration(CharSpan description, BatReplaceabilityEnum replaceability):
            description(description), replaceability(replaceability)
        {}
    }

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer, WiredConfiguration config) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
        mFeatures(WiredFeatures()),
        mSystemLayer(systemLayer)
    {
        mAttributes.wired = WiredAttributes{};
        VerifyOrDieWithMsg()
    }

    PowerSourceCluster(EndpointId endpointId, const OptionalAttributeSet & optionalAttributeSet, System::Layer & systemLayer) :
        DefaultServerCluster({ endpointId, PowerSource::Id }), mOptionalAttributeSet(optionalAttributeSet),
        mFeatures(BatteryFeatures(batteryDelegate.replacableBatteryDelegate != nullptr, batteryDelegate.rechargeableBatteryDelegate != nullptr)),
        mSystemLayer(systemLayer)
    {
        mAttributes.battery = BatteryAttributes{};
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    const BitFlags<PowerSource::Feature> & Features() const { return mFeatures; }

#define VALIDATE_FEATURE(feature_name, attr_name) \
    VerifyOrDieWithMsg(Features().Has(Feature::##feature_name), NotSpecified, "Attempting to read attribute `" #attr_name "` when feature `" #feature_name "` is not set.");

#define VALIDATE_OPTIONAL_ATTRIBUTE(attr_name) \
    VerifyOrDieWithMsg(mOptionalAttributeSet.IsSet(PowerSource::Attributes::##attr_name::Id), NotSpecified, "Attempting to read attribute `" #attr_name "` when it is not specified as supported optional attribute");

    // Getters

    PowerSourceStatusEnum GetStatus() const { return mAttributes.status; }

    uint8 GetOrder() const { return mAttributes.order; }

    CharSpan GetDescription() const { return mAttributes.GetDescription(); }

    Optional<uint32_t> GetWiredAssessedInputVoltage() const
    {
        // here and down, where the attribute is optional, the feature validation is done only for logging purposes,
        // because the optional attribute validation is fully enough for code correctness
        VALIDATE_FEATURE(kWired, WiredAssessedInputVoltage);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedInputVoltage);

        return mAttributes.wired.assessedInputVoltage;
    }

    Optional<uint16_t> GetWiredAssessedInputFrequency() const
    {
        VALIDATE_FEATURE(kWired, WiredAssessedInputFrequency);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedInputFrequency);

        return mAttributes.wired.assessedInputFrequency;
    }

    WiredCurrentTypeEnum GetWiredCurrentType() const
    {
        VALIDATE_FEATURE(kWired, WiredCurrentType);
        VerifyOrDieWithMsg(mAttributes.wired.currentType != WiredCurrentTypeEnum::k)

        return mAttributes.wired.currentType;
    }

    Optional<uint32_t> GetWiredAssessedCurrent() const
    {
        VALIDATE_FEATURE(kWired, WiredAssessedCurrent);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredAssessedCurrent);

        return mAttributes.wired.assessedCurrent;
    }

    uint32_t GetWiredNominalVoltage() const
    {
        VALIDATE_FEATURE(kWired, WiredNominalVoltage);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredNominalVoltage);

        return mAttributes.wired.nominalVoltage;
    }

    uint32_t GetWiredMaximumCurrent() const
    {
        VALIDATE_FEATURE(kWired, WiredMaximumCurrent);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredMaximumCurrent);

        return mAttributes.wired.maximumCurrent;
    }

    bool GetWiredPresent() const
    {
        VALIDATE_FEATURE(kWired, WiredPresent);
        VALIDATE_OPTIONAL_ATTRIBUTE(WiredPresent);

        return mAttributes.wired.isPresent;
    }

    Span<WiredFaultEnum> GetActiveWiredFaults() const
    {
        VALIDATE_FEATURE(kWired, ActiveWiredFaults);
        VALIDATE_OPTIONAL_ATTRIBUTE(ActiveWiredFaults);

        return mAttributes.wired.GetActiveFaults();
    }

    Optional<uint32_t> GetBatVoltage() const
    {
        VALIDATE_FEATURE(kBattery, BatVoltage);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatVoltage);

        return mAttributes.battery.voltage;
    }

    Optional<uint8_t> GetBatPercentRemaining() const
    {
        VALIDATE_FEATURE(kBattery, BatPercentRemaining);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatPercentRemaining);

        return mAttributes.battery.percentRemaining;
    }

    Optional<uint32_t> GetBatTimeRemaining() const
    {
        VALIDATE_FEATURE(kBattery, BatTimeRemaining);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatTimeRemaining);

        return mAttributes.battery.timeRemaining;
    }

    BatChargeLevelEnum GetBatChargeLevel() const
    {
        VALIDATE_FEATURE(kBattery, BatChargeLevel);

        return mAttributes.battery.chargeLevel;
    }

    bool GetBatReplacementNeeded() const
    {
        VALIDATE_FEATURE(kBattery, BatReplacementNeeded);

        return mAttributes.battery.replacementNeeded;
    }

    BatReplaceabilityEnum GetBatReplaceability() const
    {
        VALIDATE_FEATURE(kBattery, BatReplaceability);

        return mAttributes.battery.replaceability;
    }

    bool GetBatPresent() const
    {
        VALIDATE_FEATURE(kBattery, BatPresent);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatPresent);

        return mAttributes.battery.isPresent;
    }

    Span<BatFaultEnum> GetActiveBatFaults() const
    {
        VALIDATE_FEATURE(kBattery, ActiveBatFaults);
        VALIDATE_OPTIONAL_ATTRIBUTE(ActiveBatFaults);

        return mAttributes.battery.GetActiveFaults();
    }

    CharSpan GetBatReplacementDescription() const
    {
        VALIDATE_FEATURE(kBattery, BatReplacementDescription);
        VALIDATE_FEATURE(kReplaceable, BatReplacementDescription);

        return mAttributes.battery.replaceable.GetReplacementDescription();
    }

    BatCommonDesignationEnum GetBatCommonDesignation() const
    {
        VALIDATE_FEATURE(kBattery, BatCommonDesignation);
        VALIDATE_FEATURE(kReplaceable, BatCommonDesignation);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatCommonDesignation);

        return mAttributes.battery.replaceable.commonDesignation;
    }

    CharSpan GetBatANSIDesignation() const
    {
        VALIDATE_FEATURE(kBattery, BatANSIDesignation);
        VALIDATE_FEATURE(kReplaceable, BatANSIDesignation);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatANSIDesignation);

        return mAttributes.battery.replaceable.GetANSIDesignation();
    }

    CharSpan GetBatIECDesignation() const
    {
        VALIDATE_FEATURE(kBattery, BatIECDesignation);
        VALIDATE_FEATURE(kReplaceable, BatIECDesignation);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatIECDesignation);

        return mAttributes.battery.replaceable.GetIECDesignation();
    }

    BatApprovedChemistryEnum GetBatApprovedChemistry() const
    {
        VALIDATE_FEATURE(kBattery, BatApprovedChemistry);
        VALIDATE_FEATURE(kReplaceable, BatApprovedChemistry);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatApprovedChemistry);

        return mAttributes.battery.replaceable.approvedChemistry;
    }

    uint32_t GetBatCapacity() const
    {
        VALIDATE_FEATURE(kBattery, BatCapacity);
        VerifyOrDieWithMsg(Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), NotSpecified, "Attempting to read attribute `BatCapacity` when neither feature `kReplaceable` nor feature `kRechargeable` is set.");
        VALIDATE_OPTIONAL_ATTRIBUTE(BatCapacity);

        return mAttributes.battery.capacity;
    }

    uint8_t GetBatQuantity() const
    {
        VALIDATE_FEATURE(kBattery, BatQuantity);
        VALIDATE_FEATURE(kReplaceable, BatQuantity);

        return mAttributes.battery.replaceable.quantity;
    }

    BatChargeStateEnum GetBatChargeState() const
    {
        VALIDATE_FEATURE(kBattery, BatChargeState);
        VALIDATE_FEATURE(kRechargeable, BatChargeState);

        return mAttributes.battery.rechargeable.chargeState;
    }

    Optional<uint32> GetBatTimeToFullCharge() const
    {
        VALIDATE_FEATURE(kBattery, BatTimeToFullCharge);
        VALIDATE_FEATURE(kRechargeable, BatTimeToFullCharge);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatTimeToFullCharge);

        return mAttributes.battery.rechargeable.timeToFullCharge;
    }

    bool GetBatFunctionalWhileCharging() const
    {
        VALIDATE_FEATURE(kBattery, BatFunctionalWhileCharging);
        VALIDATE_FEATURE(kRechargeable, BatFunctionalWhileCharging);

        return mAttributes.battery.rechargeable.functionalWhileCharging;
    }

    Optional<uint32_t> GetBatChargingCurrent() const
    {
        VALIDATE_FEATURE(kBattery, BatChargingCurrent);
        VALIDATE_FEATURE(kRechargeable, BatChargingCurrent);
        VALIDATE_OPTIONAL_ATTRIBUTE(BatChargingCurrent);

        return mAttributes.battery.rechargeable.chargingCurrent;
    }

    Span<BatChargeFaultEnum> GetActiveBatChargeFaults() const
    {
        VALIDATE_FEATURE(kBattery, ActiveBatChargeFaults);
        VALIDATE_FEATURE(kRechargeable, ActiveBatChargeFaults);
        VALIDATE_OPTIONAL_ATTRIBUTE(ActiveBatChargeFaults);

        return mAttributes.battery.rechargeable.GetActiveChargeFaults();
    }

    Span<EndpointId> GetEndpointList() const { return mAttributes.GetPoweredEndpoints(); }

#undef VALIDATE_FEATURE
#undef VALIDATE_OPTIONAL_ATTRIBUTE

#define VERIFY_FEATURE_OR_RETURN_ERROR(feature_name) \
    VerifyOrReturnError(Features().Has(Feature::##feature_name), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(attr_name) \
    VerifyOrReturnError(mOptionalAttributeSet.IsSet(PowerSource::Attributes::##attr_name::Id), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

#define VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(list_var_name, max_size) \
    VerifyOrReturnError(list_var_name##.size() <= max_size, CHIP_ERROR_INVALID_LIST_LENGTH);

#define VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(string_var_name, max_size) \
    VerifyOrReturnError(string_var_name##.size() <= max_size, CHIP_ERROR_INVALID_STRING_LENGTH);

    // Setters

    /// Some attributes' setters are private because they are marked with the `Fixed` quality,
    /// It is possible to set them with the constructor. If it is needed to set them after construction,
    /// a setter function named like `SetConfiguration` can be added, or separate setters can be made public.

    /// `Fixed` attributes are `Description`, `WiredCurrentType`, `WiredNominalVoltage`, `WiredMaximumCurrent`, `BatReplaceability`,
    /// `BatReplacementDescription`, `BatCommonDesignation`, `BatANSIDesignation`, `BatIECDesignation`, `BatApprovedChemistry`,
    /// `BatCapacity` and `BatQuantity`

    CHIP_ERROR SetStatus(PowerSourceStatusEnum val)
    {
        SetAndNotify(mAttributes.status, val, PowerSource::Attributes::Status::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetOrder(uint8_t val)
    {
        SetAndNotify(mAttributes.order, val, PowerSource::Attributes::Order::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredAssessedInputVoltage(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedInputVoltage);

        mAttributes.wired.assessedInputVoltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredAssessedInputFrequency(Optional<uint16_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedInputFrequency);

        mAttributes.wired.assessedInputFrequency = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredAssessedCurrent(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredAssessedCurrent);

        mAttributes.wired.assessedCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredPresent(bool val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredPresent);

        SetAndNotify(mAttributes.wired.isPresent, val, PowerSource::Attributes::WiredPresent::Id)
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveWiredFaults(Span<WiredFaultEnum> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveWiredFaults);
        VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.wired.kMaxActiveFaults);

        if (mAttributes.wired.GetActiveFaults().data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        PowerSource::Events::WiredFaultChange::Type event_data{mAttributes.wired.GetActiveFaults(), val};
        CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
        if (err != CHIP_NO_ERROR) return err;

        std::copy(val.begin(), val.end(), mAttributes.wired.mActiveFaultsBuffer);
        NotifyAttributeChanged(PowerSource::Attributes::ActiveWiredFaults::Id);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatVoltage(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatVoltage);

        mAttributes.battery.voltage = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatPercentRemaining(Optional<uint8_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatPercentRemaining);
        if (val.HasValue())
        {
            // Maximum value of 200 representing 100% battery.
            VerifyOrReturnError(*val <= 200, CHIP_ERROR_INVALID_INTEGER_VALUE);
        }

        // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
        // Or this attribute is to be reported if it changes to or from null.

        // If value changes from or to null, change, notify and return.
        if (mAttributes.battery.percentRemaining == NullOptional || val = NullOptional)
        {
            SetAndNotify(mAttributes.battery.percentRemaining, val, PowerSource::Attributes::BatPercentRemaining::Id);
            return CHIP_NO_ERROR;
        }

        // If the reporting interval has expired, update the value, notify, and restart the timer.
        {
            SetAndNotify(mAttributes.battery.percentRemaining, val, PowerSource::Attributes::BatPercentRemaining::Id);
            mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatPercentRemainingNotifyTimerExpired);
            mBatPercentRemainingNotifyTimerExpired = false;
            return CHIP_NO_ERROR;
        }

        // Otherwise, the reporting interval is still active, do not notify.
        mAttributes.battery.percentRemaining = val;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatTimeRemaining(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatTimeRemaining);

        // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
        // Or this attribute is to be reported if it changes to or from null.

        // If value changes from or to null, change, notify and return.
        if (mAttributes.battery.timeRemaining == NullOptional || val = NullOptional)
        {
            SetAndNotify(mAttributes.battery.timeRemaining, val, PowerSource::Attributes::BatTimeRemaining::Id);
            return CHIP_NO_ERROR;
        }

        // If the reporting interval has expired, update the value, notify, and restart the timer.
        {
            SetAndNotify(mAttributes.battery.timeRemaining, val, PowerSource::Attributes::BatTimeRemaining::Id);
            mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeRemainingNotifyTimerExpired);
            mBatTimeRemainingNotifyTimerExpired = false;
            return CHIP_NO_ERROR;
        }

        // Otherwise, the reporting interval is still active, do not notify.
        mAttributes.battery.timeRemaining = val;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatChargeLevel(BatChargeLevelEnum val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);

        SetAndNotify(mAttributes.battery.chargeLevel, val, PowerSource::Attributes::BatChargeLevel::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatReplacementNeeded(bool val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);

        SetAndNotify(mAttributes.battery.replacementNeeded, val, PowerSource::Attributes::BatReplacementNeeded::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatPresent(bool val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatPresent);

        SetAndNotify(mAttributes.battery.isPresent, val, PowerSource::Attributes::BatPresent::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveBatFaults(Span<BatFaultEnum> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveBatFaults);
        VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.battery.kMaxActiveFaults);

        if (mAttributes.battery.GetActiveFaults().data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        PowerSource::Events::BatFaultChange::Type event_data{mAttributes.battery.GetActiveFaults(), val};
        CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
        if (err != CHIP_NO_ERROR) return err;

        std::copy(val.begin(), val.end(), mAttributes.battery.mActiveFaultsBuffer);
        NotifyAttributeChanged(PowerSource::Attributes::ActiveBatFaults::Id);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatChargeState(BatChargeStateEnum val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);

        SetAndNotify(mAttributes.battery.rechargeable.chargeState, val, PowerSource::Attributes::BatChargeState::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatTimeToFullCharge(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatTimeToFullCharge);

        // This attribute is marked with `Quieter Reporting` quality, with a time interval between change reports.
        // Or this attribute is to be reported if it changes to or from null.

        // If value changes from or to null, change, notify and return.
        if (mAttributes.battery.rechargeable.timeToFullCharge == NullOptional || val = NullOptional)
        {
            SetAndNotify(mAttributes.battery.rechargeable.timeToFullCharge, val, PowerSource::Attributes::BatTimeToFullCharge::Id);
            return CHIP_NO_ERROR;
        }

        // If the reporting interval has expired, update the value, notify, and restart the timer.
        {
            SetAndNotify(mAttributes.battery.rechargeable.timeToFullCharge, val, PowerSource::Attributes::BatTimeToFullCharge::Id);
            mSystemLayer.StartTimer(notifyTimerDuration, SetTimerExpired, &mBatTimeToFullChargeNotifyTimerExpired);
            mBatTimeToFullChargeNotifyTimerExpired = false;
            return CHIP_NO_ERROR;
        }

        // Otherwise, the reporting interval is still active, do not notify.
        mAttributes.battery.rechargeable.timeToFullCharge = val;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatFunctionalWhileCharging(bool val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);

        SetAndNotify(mAttributes.battery.rechargeable.functionalWhileCharging, val, PowerSource::Attributes::BatFunctionalWhileCharging::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatChargingCurrent(Optional<uint32_t> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatChargingCurrent);

        mAttributes.battery.rechargeable.chargingCurrent = val; // no notifying because attribute marked with 'Changes Omitted' quality
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveBatChargeFaults(Span<BatChargeFaultEnum> val)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kRechargeable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(ActiveBatChargeFaults);
        VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, mAttributes.battery.rechargeable.kMaxActiveChargeFaults);

        if (mAttributes.battery.rechargeable.GetActiveChargeFaults().data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        PowerSource::Events::BatChargeFaultChange::Type event_data{mAttributes.battery.rechargeable.GetActiveChargeFaults(), val};
        CHIP_ERROR err = mContext->interactionContext.eventsGenerator.GenerateEvent(event_data, GetEndpointId());
        if (err != CHIP_NO_ERROR) return err;

        std::copy(val.begin(), val.end(), mAttributes.battery.rechargeable.mActiveChargeFaultsBuffer);
        NotifyAttributeChanged(PowerSource::Attributes::ActiveBatChargeFaults::Id);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetEndpointList(Span<EndpointId> val)
    {
        if (mAttributes.GetPoweredEndpoints().data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        if (val.size() > mAttributes.mPoweredEndpointsBuffer.AllocatedSize())
        {
            mAttributes.mPoweredEndpointsBuffer.Calloc(val.size());
        }

        mAttributes.mPoweredEndpointsCount = val.size();

        std::copy(val.begin(), val.end(), mAttributes.mPoweredEndpointsBuffer.Get());
        NotifyAttributeChanged(PowerSource::Attributes::EndpointList::Id);

        return CHIP_NO_ERROR;
    }

private:

    // Setters for `Fixed` attributes

    CHIP_ERROR SetDescription(CharSpan val, bool notify = true)
    {
        VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, PowerSource::Attributes::Description::TypeInfo::MaxLength());

        return SetStringAndNotify(
            val,
            mAttributes.GetDescription(),
            mAttributes.mDescriptionBuffer,
            PowerSource::Attributes::Description::TypeInfo::MaxLength(),
            PowerSource::Attributes::Description::Id,
            notify
        );
    }

    CHIP_ERROR SetWiredCurrentType(WiredCurrentTypeEnum val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);

        SetAndNotify(mAttributes.wired.currentType, val, PowerSource::Attributes::WiredCurrentType::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredNominalVoltage(uint32_t val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredNominalVoltage);

        SetAndNotify(mAttributes.wired.nominalVoltage, val, PowerSource::Attributes::WiredNominalVoltage::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetWiredMaximumCurrent(uint32_t val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kWired);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(WiredMaximumCurrent);

        SetAndNotify(mAttributes.wired.maximumCurrent, val, PowerSource::Attributes::WiredMaximumCurrent::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatReplaceability(BatReplaceabilityEnum val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);

        SetAndNotify(mAttributes.battery.replaceability, val, PowerSource::Attributes::BatReplaceability::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatReplacementDescription(CharSpan val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatReplacementDescription);
        VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength());

        return SetStringAndNotify(
            val,
            mAttributes.battery.replaceable.GetReplacementDescription(),
            mAttributes.battery.replaceable.mReplacementDescriptionBuffer,
            PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength(),
            PowerSource::Attributes::BatReplacementDescription::Id,
            notify
        );
    }

    CHIP_ERROR SetBatCommonDesignation(BatCommonDesignationEnum val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatCommonDesignation);

        SetAndNotify(mAttributes.battery.replaceable.commonDesignation, PowerSource::Attributes::BatCommonDesignation::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatANSIDesignation(CharSpan val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatANSIDesignation);
        VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength());

        return SetStringAndNotify(
            val,
            mAttributes.battery.replaceable.GetANSIDesignation(),
            mAttributes.battery.replaceable.mANSIDesignationBuffer,
            PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength(),
            PowerSource::Attributes::BatANSIDesignation::Id,
            notify
        );
    }

    CHIP_ERROR SetBatIECDesignation(CharSpan val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatIECDesignation);
        VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR(val, PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength());

        return SetStringAndNotify(
            val,
            mAttributes.battery.replaceable.GetIECDesignation(),
            mAttributes.battery.replaceable.mIECDesignationBuffer,
            PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength(),
            PowerSource::Attributes::BatIECDesignation::Id,
            notify
        );
    }

    CHIP_ERROR SetBatApprovedChemistry(BatApprovedChemistryEnum val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatApprovedChemistry);

        SetAndNotify(mAttributes.battery.replaceable.approvedChemistry, PowerSource::Attributes::BatApprovedChemistry::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatCapacity(uint32_t val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VerifyOrReturnError(Features().Has(Feature::kReplaceable) || Features().Has(Feature::kRechargeable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR(BatCapacity);

        SetAndNotify(mAttributes.batttery.capacity, val, PowerSource::Attributes::BatCapacity::Id, notify);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetBatQuantity(uint8_t val, bool notify = true)
    {
        VERIFY_FEATURE_OR_RETURN_ERROR(kBattery);
        VERIFY_FEATURE_OR_RETURN_ERROR(kReplaceable);

        SetAndNotify(mAttributes.battery.replaceable.quantity, val, PowerSource::Attributes::BatQuantity::Id, notify);
        return CHIP_NO_ERROR;
    }

#undef VERIFY_FEATURE_OR_RETURN_ERROR
#undef VERIFY_OPTIONAL_ATTRIBUTE_OR_RETURN_ERROR
#undef VERIFY_LIST_ATTRIBUTE_SIZE_OR_RETURN_ERROR
#undef VERIFY_STRING_ATTRIBUTE_SIZE_OR_RETURN_ERROR

public:
    struct WiredAttributes
    {
        Optional<uint32_t> assessedInputVoltage{};
        Optional<uint16_t> assessedInputFrequency{};
        WiredCurrentTypeEnum currentType{};
        Optional<uint32_t> assessedCurrent{};
        uint32_t nominalVoltage{};
        uint32_t maximumCurrent{};
        bool isPresent{};
        Span<WiredFaultEnum> GetActiveFaults() { return Span<WiredFaultEnum>(mActiveFaultsBuffer, mActiveFaultsSize); }

        constexpr size_t kMaxActiveFaults = 8;
        size_t mActiveFaultsSize = 0;
        WiredFaultEnum mActiveFaultsBuffer[kMaxActiveFaults];
    };

    struct ReplaceableBatteryAttributes
    {
        CharSpan GetReplacementDescription() { return CharSpan(mReplacementDescriptionBuffer, strlen(mReplacementDescriptionBuffer)); }
        BatCommonDesignationEnum commonDesignation = BatCommonDesignationEnum::kUnspecified;
        CharSpan GetANSIDesignation() { return CharSpan(mANSIDesignationBuffer, strlen(mANSIDesignationBuffer)); }
        CharSpan GetIECDesignation() { return CharSpan(mIECDesignationBuffer, strlen(mIECDesignationBuffer)); }
        BatApprovedChemistryEnum approvedChemistry = BatApprovedChemistryEnum::kUnspecified;
        uint8_t quantity{};

        char mReplacementDescriptionBuffer[PowerSource::Attributes::BatReplacementDescription::TypeInfo::MaxLength() + 1] = { 0 };
        char mANSIDesignationBuffer[PowerSource::Attributes::BatANSIDesignation::TypeInfo::MaxLength() + 1] = { 0 };
        char mIECDesignationBuffer[PowerSource::Attributes::BatIECDesignation::TypeInfo::MaxLength() + 1] = { 0 };
    };

    struct RechargeableBatteryAttributes
    {
        BatChargeStateEnum chargeState = BatChargeStateEnum::kUnknown;
        Optional<uint32_t> timeToFullCharge{};
        bool functionalWhileCharging{};
        Optional<uint32_t> chargingCurrent{};
        Span<BatChargeFaultEnum> GetActiveChargeFaults() { return Span<BatChargeFaultEnum>(mActiveChargeFaultsBuffer, mActiveChargeFaultsSize); }

        constexpr size_t kMaxActiveChargeFaults = 16;
        size_t mActiveChargeFaultsSize = 0;
        BatChargeFaultEnum mActiveChargeFaultsBuffer[kMaxActiveChargeFaults];
    };

    struct BatteryAttributes
    {
        Optional<uint32_t> voltage{};
        Optional<uint8_t> percentRemaining{};
        Optional<uint32_t> timeRemaining{};
        BatChargeLevelEnum chargeLevel = BatChargeLevelEnum::kOk;
        bool replacementNeeded{};
        BatReplaceabilityEnum replaceability = BatReplaceabilityEnum::kUnspecified;
        bool isPresent{};
        Span<BatFaultEnum> GetActiveFaults() { return Span<BatFaultEnum>(mActiveFaultsBuffer, mActiveFaultsSize); }
        uint32_t capacity{};

        constexpr size_t kMaxActiveFaults = 8;
        size_t mActiveFaultsSize = 0;
        BatFaultEnum mActiveFaultsBuffer[kMaxActiveFaults];

        ReplaceableBatteryAttributes replaceable{};
        RechargeableBatteryAttributes rechargeable{};
    };

    struct Attributes
    {
        PowerSourceStatusEnum status = PowerSourceStatusEnum::kUnspecified;
        uint8_t order{};
        CharSpan GetDescription() { return CharSpan(mDescriptionBuffer, strlen(mDescriptionBuffer)); }
        Span<EndpointId> GetPoweredEndpoints() { return Span(mPoweredEndpointsBuffer.Get(), mPoweredEndpointsCount); };

        char mDescriptionBuffer[PowerSource::Attributes::Description::TypeInfo::MaxLength() + 1] = { 0 };

        Platform::ScopedMemoryBufferWithSize<EndpointId> mPoweredEndpointsBuffer;
        size_t mPoweredEndpointsCount = 0;

        union
        {
            WiredAttributes wired;
            BatteryAttributes battery;
        };
    };

    static inline BitFlags<PowerSource::Feature> WiredFeatures()
    {
        return BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired);
    }

    static inline BitFlags<PowerSource::Feature> BatteryFeatures(bool replaceable, bool rechargeable)
    {
        auto flags = BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery);
        return flags.Set(PowerSource::Feature::kReplaceable, replaceable)
                    .Set(PowerSource::Feature::kRechargeable, rechargeable);
    }

    void forceOptionalAttributesValidity();

    template <class T>
    void SetAndNotify(T & current_val, const T & new_val, AttributeId id, bool notify = true)
    {
        VerifyOrReturn(current_val != new_val && notify); // no-op if equal

        current_val = new_val;
        NotifyAttributeChanged(id);
    }

    // maxSize without null byte
    CHIP_ERROR SetStringAndNotify(CharSpan val, CharSpan current, char* buffer, size_t maxSize, AttributeId id, bool notify = true)
    {
        if (current.data_equal(val))
        {
            return CHIP_NO_ERROR; // no-op if equal
        }

        auto err = CopyCharSpanToMutableCharSpan(val, Span(buffer, maxSize))
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        buffer[val.size()] = 0; // null byte
        if (notify)
        {
            NotifyAttributeChanged(id);
        }
        return CHIP_NO_ERROR;
    }

    EndpointId GetEndpointId()
    {
        return GetPaths()[0].mEndpointId;
    }

    const BitFlags<PowerSource::Feature> mFeatures;
    OptionalAttributeSet mOptionalAttributeSet;
    struct Attributes mAttributes;

    // context
    System::Layer & mSystemLayer;

    // notify timer stuff
    std::atomic_bool mBatPercentRemainingNotifyTimerExpired{true};
    std::atomic_bool mBatTimeRemainingNotifyTimerExpired{true};
    std::atomic_bool mBatTimeToFullChargeNotifyTimerExpired{true};

    constexpr System::Clock::Timeout notifyTimerDuration = System::Clock::Seconds16(10);

    static void SetTimerExpired(System::Layer *, void * pAtomicBool)
    {
        std::atomic_bool* p = reinterpret_cast<std::atomic_bool>(pAtomicBool);
        *p = true;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip