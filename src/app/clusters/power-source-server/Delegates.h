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

#include <app/data-model/Nullable.h>
#include <clusters/PowerSource/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {

struct PowerSourceDelegate
{
    using PowerSourceStatusEnum = PowerSource::PowerSourceStatusEnum;
    virtual ~PowerSourceDelegate() = default;

    virtual PowerSourceStatusEnum GetStatus() = 0;
    virtual uint8_t GetOrder() = 0;
    virtual CharSpan GetDescription() = 0;
    virtual Span<const EndpointId> GetPoweredEndpoints() = 0;
};

struct WiredPowerSourceDelegate
{
    using WiredCurrentTypeEnum = PowerSource::WiredCurrentTypeEnum;
    using WiredFaultEnum = PowerSource::WiredFaultEnum;
    virtual ~WiredPowerSourceDelegate() = default;

    virtual CHIP_ERROR GetAssessedInputVoltage(Optional<uint32_t> & voltage) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetAssessedInputFrequency(Optional<uint16_t> & frequency) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual WiredCurrentTypeEnum GetCurrentType() = 0;
    virtual CHIP_ERROR GetAssessedCurrent(Optional<uint32_t> & current) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetNominalVoltage(uint32_t & voltage) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetMaximumCurrent(uint32_t & current) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR IsPresent(bool & present) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetActiveFaults(Span<const WiredFaultEnum> & activeFaults) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

struct BatteryDelegate
{
    using BatChargeLevelEnum = PowerSource::BatChargeLevelEnum;
    using BatReplaceabilityEnum = PowerSource::BatReplaceabilityEnum;
    using BatFaultEnum = PowerSource::BatFaultEnum;
    virtual ~BatteryDelegate() = default;

    virtual CHIP_ERROR GetVoltage(Optional<uint32_t> & voltage) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetPercentRemaining(Optional<uint8_t> & percent) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetTimeRemaining(Optional<uint32_t> & time) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual BatChargeLevelEnum GetChargeLevel() = 0;
    virtual bool IsReplacmentNeeded() = 0;
    virtual BatReplaceabilityEnum GetReplaceability() = 0;
    virtual CHIP_ERROR IsPresent(bool & present) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetActiveFaults(Span<const BatFaultEnum> & activeFaults) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

struct ReplacableBatteryDelegate
{
    using BatCommonDesignationEnum = PowerSource::BatCommonDesignationEnum;
    using BatApprovedChemistryEnum = PowerSource::BatApprovedChemistryEnum;
    virtual ~ReplacableBatteryDelegate() = default;

    virtual CharSpan GetReplacementDescription() = 0;
    virtual CHIP_ERROR GetCommonDesignation(BatCommonDesignationEnum & designation) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetANSIDesignation(CharSpan & designation) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetIECDesignation(CharSpan & designation) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetApprovedChemistry(BatApprovedChemistryEnum & chemistry) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual uint8_t GetQuantity() = 0;

    // same function as `RechargeableBatteryDelegate::GetCapacity`
    virtual CHIP_ERROR GetCapacity(uint32_t & capacity) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

struct RechargeableBatteryDelegate
{
    using BatChargeStateEnum = PowerSource::BatChargeStateEnum;
    using BatChargeFaultEnum = PowerSource::BatChargeFaultEnum;
    virtual ~RechargeableBatteryDelegate() = default;

    virtual BatChargeStateEnum GetChargeState() = 0;
    virtual CHIP_ERROR GetTimeToFullCharge(Optional<uint32_t> & time) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual bool IsFunctionalWhileCharging() = 0;
    virtual CHIP_ERROR GetChargingCurrent(Optional<uint32_t> & current) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
    virtual CHIP_ERROR GetActiveChargeFaults(Span<const BatChargeFaultEnum> & activeFaults) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    // same function as `ReplacableBatteryDelegate::GetCapacity`
    virtual CHIP_ERROR GetCapacity(uint32_t & capacity) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
