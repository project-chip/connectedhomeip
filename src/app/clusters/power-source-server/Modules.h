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

#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/Enums.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>

#include <atomic>
#include <optional>
#include <type_traits>
#include <string>

namespace chip::app::Clusters::PowerSource::detail {

// without EndpointList
struct MandatoryModule
{
    PowerSourceStatusEnum status;
    uint8_t order;
    CharSpan description;
};

struct WiredMandatoryModule
{
    WiredCurrentTypeEnum wiredCurrentType;
};

struct BatteryMandatoryModule
{
    BatChargeLevelEnum batChargeLevel;
    bool batReplacementNeeded;
    BatReplaceabilityEnum batReplaceability;
};

struct ReplaceableMandatoryModule
{
    CharSpan batReplacementDescription;
    uint8_t batQuantity;
};

struct RechargeableMandatoryModule
{
    BatChargeStateEnum batChargeState;
    bool batFunctionalWhileCharging;
};

template <bool used> struct WiredAssessedInputVoltageModule {};
template <>          struct WiredAssessedInputVoltageModule<true>
{
    std::optional<uint32_t> wiredAssessedInputVoltage;
};

template <bool used> struct WiredAssessedInputFrequencyModule {};
template <>          struct WiredAssessedInputFrequencyModule<true>
{
    std::optional<uint16_t> wiredAssessedInputFrequency;
};

template <bool used> struct WiredAssessedCurrentModule {};
template <>          struct WiredAssessedCurrentModule<true>
{
    std::optional<uint32_t> wiredAssessedCurrent;
};

template <bool used> struct WiredNominalVoltageModule {};
template <>          struct WiredNominalVoltageModule<true>
{
    uint32_t wiredNominalVoltage;
};

template <bool used> struct WiredMaximumCurrentModule {};
template <>          struct WiredMaximumCurrentModule<true>
{
    uint32_t wiredMaximumCurrent;
};

template <bool used> struct WiredPresentModule {};
template <>          struct WiredPresentModule<true>
{
    bool wiredPresent;
};

template <bool used> struct ActiveWiredFaultsModule {};
template <>          struct ActiveWiredFaultsModule<true>
{
protected:
    uint8_t activeWiredFaultsBitSet;
};

template <bool used> struct BatVoltageModule {};
template <>          struct BatVoltageModule<true>
{
    std::optional<uint32_t> batVoltage;
};

template <bool used> struct BatPercentRemainingModule {};
template <>          struct BatPercentRemainingModule<true>
{
    std::optional<uint8_t> batPercentRemaining;
};

template <bool used> struct BatTimeRemainingModule {};
template <>          struct BatTimeRemainingModule<true>
{
    std::optional<uint32_t> batTimeRemaining;
};

template <bool used> struct BatPresentModule {};
template <>          struct BatPresentModule<true>
{
    bool batPresent;
};

template <bool used> struct ActiveBatFaultsModule {};
template <>          struct ActiveBatFaultsModule<true>
{
protected:
    uint8_t activeBatFaultsBitSet;
};

template <bool used> struct BatCommonDesignationModule {};
template <>          struct BatCommonDesignationModule<true>
{
    BatCommonDesignationEnum batCommonDesignation;
};

template <bool used> struct BatANSIDesignationModule {};
template <>          struct BatANSIDesignationModule<true>
{
    CharSpan batANSIDesignation;
};

template <bool used> struct BatIECDesignationModule {};
template <>          struct BatIECDesignationModule<true>
{
    CharSpan batIECDesignation;
};

template <bool used> struct BatApprovedChemistryModule {};
template <>          struct BatApprovedChemistryModule<true>
{
    BatApprovedChemistryEnum batApprovedChemistry;
};

template <bool used> struct BatCapacityModule {};
template <>          struct BatCapacityModule<true>
{
    uint32_t batCapacity;
};

template <bool used> struct BatTimeToFullChargeModule {};
template <>          struct BatTimeToFullChargeModule<true>
{
    std::optional<uint32_t> batTimeToFullCharge;
};

template <bool used> struct BatChargingCurrentModule {};
template <>          struct BatChargingCurrentModule<true>
{
    std::optional<uint32_t> batChargingCurrent;
};

template <bool used> struct ActiveBatChargeFaultsModule {};
template <>          struct ActiveBatChargeFaultsModule<true>
{
protected:
    uint16_t activeBatChargeFaultsBitSet;
};

struct EndpointListModule
{
protected:
    Platform::ScopedMemoryBuffer<EndpointId> endpointList;
    uint16_t endpointListCount = 0;
};

template <bool used, uint32_t optionalAttributeBits> struct WiredModule {};
template <uint32_t optionalAttributeBits>
struct WiredModule<true, optionalAttributeBits> :
    public WiredMandatoryModule,
    public WiredAssessedInputVoltageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredAssessedInputVoltage::Id)>,
    public WiredAssessedInputFrequencyModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredAssessedInputFrequency::Id)>,
    public WiredAssessedCurrentModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredAssessedCurrent::Id)>,
    public WiredNominalVoltageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredNominalVoltage::Id)>,
    public WiredMaximumCurrentModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredMaximumCurrent::Id)>,
    public WiredPresentModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::WiredPresent::Id)>,
    public ActiveWiredFaultsModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::ActiveWiredFaults::Id)>
{};

template <bool used, uint32_t optionalAttributeBits> struct OnlyBatteryModule {};
template <uint32_t optionalAttributeBits>
struct OnlyBatteryModule<true, optionalAttributeBits> :
    public BatteryMandatoryModule,
    public BatVoltageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatVoltage::Id)>,
    public BatPercentRemainingModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatPercentRemaining::Id)>,
    public BatTimeRemainingModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatTimeRemaining::Id)>,
    public BatPresentModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatPresent::Id)>,
    public ActiveBatFaultsModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::ActiveBatFaults::Id)>
{};

template <bool used, uint32_t optionalAttributeBits> struct ReplaceableModule {};
template <uint32_t optionalAttributeBits>
struct ReplaceableModule<true, optionalAttributeBits> :
    public ReplaceableMandatoryModule,
    public BatCommonDesignationModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatCommonDesignation::Id)>,
    public BatANSIDesignationModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatANSIDesignation::Id)>,
    public BatIECDesignationModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatIECDesignation::Id)>,
    public BatApprovedChemistryModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatApprovedChemistry::Id)>
{};

template <bool used, uint32_t optionalAttributeBits> struct RechargeableModule {};
template <uint32_t optionalAttributeBits>
struct RechargeableModule<true, optionalAttributeBits> :
    public RechargeableMandatoryModule,
    public BatTimeToFullChargeModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatTimeToFullCharge::Id)>,
    public BatChargingCurrentModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatChargingCurrent::Id)>,
    public ActiveBatChargeFaultsModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::ActiveBatChargeFaults::Id)>
{};

template <bool used, bool supportsReplaceable, bool supportsRechargeable, uint32_t optionalAttributeBits> struct BatteryModule {};
template <bool supportsReplaceable, bool supportsRechargeable, uint32_t optionalAttributeBits>
struct BatteryModule<true, supportsReplaceable, supportsRechargeable, optionalAttributeBits> :
    public OnlyBatteryModule<true, optionalAttributeBits>,
    public ReplaceableModule<supportsReplaceable, optionalAttributeBits>,
    public RechargeableModule<supportsRechargeable, optionalAttributeBits>,
    public BatCapacityModule<(supportsReplaceable || supportsRechargeable) && AttributeSet(optionalAttributeBits).IsSet(Attributes::BatCapacity::Id)>
{
protected:
    bool batReplaceable = false;
    bool batRechargeable = false;
    TimerDelegate * mTimerDelegate { nullptr };
};

template <std::underlying_type_t<Feature> featureBits, uint32_t optionalAttributeBits>
struct AllModulesExceptEndpointList :
    public MandatoryModule,
    public WiredModule<BitFlags<Feature>(featureBits).Has(Feature::kWired), optionalAttributeBits>,
    public BatteryModule<BitFlags<Feature>(featureBits).Has(Feature::kBattery),
                         BitFlags<Feature>(featureBits).Has(Feature::kReplaceable),
                         BitFlags<Feature>(featureBits).Has(Feature::kRechargeable), optionalAttributeBits>
{};

struct BatteryTimerContext : public TimerContext
{
    std::atomic_bool timerExpired { true };

    void TimerFired() override { timerExpired = true; }
};

template <bool used> struct BatteryTimerContextsModule {};
template <>          struct BatteryTimerContextsModule<true>
{
    BatteryTimerContext batPercentRemainingNotifyTimerContext;
    BatteryTimerContext batTimeRemainingNotifyTimerContext;
    BatteryTimerContext batTimeToFullChargeNotifyTimerContext;
};

struct DescriptionStorageModule
{
    std::string description;
};

template <bool used> struct BatReplacementDescriptionStorageModule {};
template <>          struct BatReplacementDescriptionStorageModule<true>
{
    std::string batReplacementDescription;
};

template <bool used> struct BatANSIDesignationStorageModule {};
template <>          struct BatANSIDesignationStorageModule<true>
{
    std::string batANSIDesignation;
};

template <bool used> struct BatIECDesignationStorageModule {};
template <>          struct BatIECDesignationStorageModule<true>
{
    std::string batIECDesignation;
};

template <uint32_t optionalAttributeBits>
struct StringAttributeStorageModule :
    public DescriptionStorageModule,
    public BatReplacementDescriptionStorageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatReplacementDescription::Id)>,
    public BatANSIDesignationStorageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatANSIDesignation::Id)>,
    public BatIECDesignationStorageModule<AttributeSet(optionalAttributeBits).IsSet(Attributes::BatIECDesignation::Id)>
{};

} // namespace chip::app::Clusters::PowerSource::detail