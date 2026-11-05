/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <EnergyTimeUtils.h>
#include <EvseChargingSchedule.h>
#include <EvseTargetsConfig.h>

#include <algorithm>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

namespace {

constexpr int64_t kMaxRequiredEnergy_mWh = 1000000000000; // 1000 MWh

CHIP_ERROR FindNextTarget(EvseChargingScheduleContext & context, const BitMask<TargetDayOfWeekBitmap> dayOfWeekMap,
                          uint16_t minutesPastMidnightNow_m, uint16_t & targetTimeMinutesPastMidnight_m,
                          DataModel::Nullable<Percent> & targetSoC, DataModel::Nullable<int64_t> & targetAddedEnergy_mWh,
                          bool allowTargetsInPast)
{
    uint16_t minTimeToTarget_m = 24 * 60;
    bool found                 = false;

    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules =
        context.GetChargingTargets();
    for (auto & chargingTargetScheduleEntry : chargingTargetSchedules)
    {
        if (chargingTargetScheduleEntry.dayOfWeekForSequence.HasAny(dayOfWeekMap))
        {
            for (auto & chargingTarget : chargingTargetScheduleEntry.chargingTargets)
            {
                if ((chargingTarget.targetTimeMinutesPastMidnight < minutesPastMidnightNow_m) && !allowTargetsInPast)
                {
                    continue;
                }

                if (chargingTarget.targetTimeMinutesPastMidnight < minTimeToTarget_m)
                {
                    found                           = true;
                    minTimeToTarget_m               = chargingTarget.targetTimeMinutesPastMidnight;
                    targetTimeMinutesPastMidnight_m = chargingTarget.targetTimeMinutesPastMidnight;

                    if (chargingTarget.targetSoC.HasValue())
                    {
                        targetSoC.SetNonNull(chargingTarget.targetSoC.Value());
                    }
                    else
                    {
                        targetSoC.SetNull();
                    }

                    if (chargingTarget.addedEnergy.HasValue())
                    {
                        targetAddedEnergy_mWh.SetNonNull(chargingTarget.addedEnergy.Value());
                    }
                    else
                    {
                        targetAddedEnergy_mWh.SetNull();
                    }
                }
            }
        }

        if (found)
        {
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DetermineRequiredEnergy(EvseChargingScheduleContext & context, int64_t & requiredEnergy_mWh,
                                   DataModel::Nullable<Percent> & targetSoC, DataModel::Nullable<int64_t> & targetAddedEnergy_mWh)
{
    DataModel::Nullable<Percent> vehicleSoC;
    DataModel::Nullable<int64_t> batteryCapacity_mWh;

    if (!targetSoC.IsNull())
    {
        if (context.HasSoCReportingFeature())
        {
            vehicleSoC          = context.GetStateOfCharge();
            batteryCapacity_mWh = context.GetBatteryCapacity();
            if (!vehicleSoC.IsNull() && !batteryCapacity_mWh.IsNull())
            {
                requiredEnergy_mWh = std::max<int64_t>(
                    0, static_cast<int64_t>((targetSoC.Value() - vehicleSoC.Value()) * batteryCapacity_mWh.Value() / 100));
                ChipLogProgress(AppServer, "EVSE: Vehicle reports current SoC: %d    ----- target SoC: %d", vehicleSoC.Value(),
                                targetSoC.Value());
                targetAddedEnergy_mWh.SetNull();

                return CHIP_NO_ERROR;
            }
        }
        else
        {
            if (targetSoC.Value() != 100)
            {
                ChipLogError(AppServer, "EVSE WARNING: TargetSoC is not 100%% and we don't know the EV SoC!");
            }

            requiredEnergy_mWh = kMaxRequiredEnergy_mWh;

            return CHIP_NO_ERROR;
        }
    }

    if (targetAddedEnergy_mWh.IsNull())
    {
        ChipLogError(AppServer,
                     "EVSE ERROR: Cannot use TargetSoC (maybe missing VehicleSoC/BatteryCapacity?) or AddedEnergy has not been "
                     "provided - assume large battery!");

        requiredEnergy_mWh = kMaxRequiredEnergy_mWh;
        return CHIP_NO_ERROR;
    }

    requiredEnergy_mWh = targetAddedEnergy_mWh.Value();
    targetSoC.SetNull();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ComputeStartTime(EvseChargingScheduleContext & context, DataModel::Nullable<uint32_t> & startTime_epoch_s,
                            uint32_t targetTime_epoch_s, uint32_t now_epoch_s, int64_t requiredEnergy_mWh)
{
    if (requiredEnergy_mWh == 0)
    {
        startTime_epoch_s.SetNull();
        return CHIP_NO_ERROR;
    }

    uint32_t power_W = static_cast<uint32_t>((context.HwGetNominalMainsVoltage() * context.GetMaximumChargeCurrent()) / 1000000);
    if (power_W == 0)
    {
        ChipLogError(AppServer, "EVSE Error: MaxCurrent = 0Amp - Can't schedule charging");
        startTime_epoch_s.SetNull();
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t chargingDuration_s =
        static_cast<uint32_t>((static_cast<uint64_t>(requiredEnergy_mWh) * 36) / (static_cast<uint64_t>(power_W) * 10));

    chargingDuration_s += (15 * 60);

    uint32_t tempStartTime_epoch_s = targetTime_epoch_s - chargingDuration_s;

    if (chargingDuration_s > targetTime_epoch_s || tempStartTime_epoch_s < now_epoch_s)
    {
        startTime_epoch_s.SetNonNull(now_epoch_s);
    }
    else
    {
        startTime_epoch_s.SetNonNull(tempStartTime_epoch_s);
    }
    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR ComputeEvseChargingSchedule(EvseChargingScheduleContext & context)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    BitMask<TargetDayOfWeekBitmap> dayOfWeekMap = 0;
    ReturnErrorOnFailure(GetLocalDayOfWeekNow(dayOfWeekMap));

    uint16_t minutesPastMidnightNow_m = 0;
    ReturnErrorOnFailure(GetMinutesPastMidnight(minutesPastMidnightNow_m));

    uint32_t now_epoch_s = 0;
    ReturnErrorOnFailure(System::Clock::GetClock_MatterEpochS(now_epoch_s));

    DataModel::Nullable<uint32_t> startTime_epoch_s;
    DataModel::Nullable<uint32_t> targetTime_epoch_s;
    DataModel::Nullable<Percent> targetSoC;
    DataModel::Nullable<int64_t> targetAddedEnergy_mWh;

    uint32_t tempTargetTime_epoch_s;
    int64_t requiredEnergy_mWh;
    uint16_t targetTimeMinutesPastMidnight_m;

    targetTime_epoch_s.SetNull();
    targetSoC.SetNull();
    targetAddedEnergy_mWh.SetNull();
    startTime_epoch_s.SetNull();

    if (context.IsEvsePluggedIn() && context.GetSupplyState() == SupplyStateEnum::kChargingEnabled)
    {
        uint8_t searchDay = 0;
        while (searchDay < 2)
        {
            err = FindNextTarget(context, dayOfWeekMap, minutesPastMidnightNow_m, targetTimeMinutesPastMidnight_m, targetSoC,
                                 targetAddedEnergy_mWh, (searchDay != 0));
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                searchDay++;
                dayOfWeekMap = BitMask<TargetDayOfWeekBitmap>((dayOfWeekMap.Raw() << 1) & kAllTargetDaysMask);

                if (!dayOfWeekMap.HasAny())
                {
                    dayOfWeekMap = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kSunday);
                }
            }
            else
            {
                break;
            }
        }

        if (err == CHIP_NO_ERROR)
        {
            tempTargetTime_epoch_s =
                ((now_epoch_s / 60) + targetTimeMinutesPastMidnight_m + (searchDay * 1440) - minutesPastMidnightNow_m) * 60;
            targetTime_epoch_s.SetNonNull(tempTargetTime_epoch_s);

            if (DetermineRequiredEnergy(context, requiredEnergy_mWh, targetSoC, targetAddedEnergy_mWh) == CHIP_NO_ERROR)
            {
                LogErrorOnFailure(
                    ComputeStartTime(context, startTime_epoch_s, tempTargetTime_epoch_s, now_epoch_s, requiredEnergy_mWh));
            }
        }
    }

    LogErrorOnFailure(context.SetNextChargeStartTime(startTime_epoch_s));
    LogErrorOnFailure(context.SetNextChargeTargetTime(targetTime_epoch_s));
    LogErrorOnFailure(context.SetNextChargeRequiredEnergy(targetAddedEnergy_mWh));
    LogErrorOnFailure(context.SetNextChargeTargetSoC(targetSoC));

    return err;
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
