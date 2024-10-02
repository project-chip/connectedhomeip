/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include <DEMManufacturerDelegate.h>
#include <DeviceEnergyManagementDelegateImpl.h>
#include <EVSEManufacturerImpl.h>
#include <EnergyEvseDelegateImpl.h>
#include <EnergyEvseManager.h>
#include <EnergyTimeUtils.h>

#include <EnergyTimeUtils.h>
#include <FakeReadings.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;

using Protocols::InteractionModel::Status;

CHIP_ERROR EVSEManufacturer::Init()
{
    /* Manufacturers should modify this to do any custom initialisation */

    /* Register callbacks */
    EnergyEvseDelegate * dg = GetEvseManufacturer()->GetEvseDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "EVSE Delegate is not initialized");
        return CHIP_ERROR_UNINITIALIZED;
    }

    dg->HwRegisterEvseCallbackHandler(ApplicationCallbackHandler, reinterpret_cast<intptr_t>(this));

    ReturnErrorOnFailure(InitializePowerMeasurementCluster());

    ReturnErrorOnFailure(InitializePowerSourceCluster());

    DeviceEnergyManagementDelegate * dem = GetEvseManufacturer()->GetDEMDelegate();
    VerifyOrReturnLogError(dem != nullptr, CHIP_ERROR_UNINITIALIZED);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    dem->SetESAState(ESAStateEnum::kOnline);
    dem->SetESAType(ESATypeEnum::kEvse);

    // Set the abs min and max power
    dem->SetAbsMinPower(1200000); // 1.2KW
    dem->SetAbsMaxPower(7600000); // 7.6KW

    /*
     * This is an example implementation for manufacturers to consider
     *
     * For Manufacturer to specify the hardware capability in mA:
     *  dg->HwSetMaxHardwareCurrentLimit(32000);    // 32A
     *
     * For Manufacturer to specify the CircuitCapacity in mA (e.g. from DIP switches)
     *  dg->HwSetCircuitCapacity(20000);            // 20A
     *
     */

    /* Once the system is initialised then check to see if the state was restored
     * (e.g. after a power outage), and if the Enable timer check needs to be started
     */
    dg->ScheduleCheckOnEnabledTimeout();

    return CHIP_NO_ERROR;
}

/*
 * When the EV is plugged in, and asking for demand change the state
 * and set the CableAssembly current limit
 *
 *   EnergyEvseDelegate * dg = GetEvseManufacturer()->GetEvseDelegate();
 *   VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);
 *
 *   dg->HwSetState(StateEnum::kPluggedInDemand);
 *   dg->HwSetCableAssemblyLimit(63000);    // 63A = 63000mA
 *
 *
 * If the vehicle ID can be retrieved (e.g. over Powerline)
 *   dg->HwSetVehicleID(CharSpan::fromCharString("TEST_VEHICLE_123456789"));
 *
 *
 * If the EVSE has an RFID sensor, the RFID value read can cause an event to be sent
 * (e.g. can be used to indicate if a user as tried to activate the charging)
 *
 *   uint8_t uid[10] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
 *   dg->HwSetRFID(ByteSpan(uid));
 */

CHIP_ERROR EVSEManufacturer::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR FindNextTarget(const BitMask<EnergyEvse::TargetDayOfWeekBitmap> dayOfWeekMap, uint16_t minutesPastMidnightNow_m,
                          uint16_t & targetTimeMinutesPastMidnight_m, DataModel::Nullable<Percent> & targetSoC,
                          DataModel::Nullable<int64_t> & addedEnergy_mWh, bool bAllowTargetsInPast)
{
    EnergyEvse::Structs::ChargingTargetScheduleStruct::Type entry;

    uint16_t minTimeToTarget_m = 24 * 60; // 24 hours
    bool bFound                = false;

    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrReturnError(mn != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    const DataModel::List<const EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules =
        dg->GetEvseTargetsDelegate()->GetTargets();
    for (auto & chargingTargetScheduleEntry : chargingTargetSchedules)
    {
        if (chargingTargetScheduleEntry.dayOfWeekForSequence.HasAny(dayOfWeekMap))
        {
            // We've found today's schedule - iterate through the targets on this day
            for (auto & chargingTarget : chargingTargetScheduleEntry.chargingTargets)
            {
                if ((chargingTarget.targetTimeMinutesPastMidnight < minutesPastMidnightNow_m) && (bAllowTargetsInPast == false))
                {
                    // This target is in the past so move to the next if there is one
                    continue;
                }

                if (chargingTarget.targetTimeMinutesPastMidnight < minTimeToTarget_m)
                {
                    // This is the earliest target found in the day's targets so far
                    bFound            = true;
                    minTimeToTarget_m = chargingTarget.targetTimeMinutesPastMidnight;

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
                        addedEnergy_mWh.SetNonNull(chargingTarget.addedEnergy.Value());
                    }
                    else
                    {
                        addedEnergy_mWh.SetNull();
                    }
                }
            }
        }

        if (bFound)
        {
            // Skip the rest of the search
            break;
        }
    }

    return bFound ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
}

/**
 * @brief   Simple example to demonstrate how an EVSE can compute the start time
 *          and duration of a charging schedule
 */
CHIP_ERROR EVSEManufacturer::ComputeChargingSchedule()
{
    CHIP_ERROR err;
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrReturnError(mn != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    BitMask<EnergyEvse::TargetDayOfWeekBitmap> dayOfWeekMap = 0;
    ReturnErrorOnFailure(GetLocalDayOfWeekNow(dayOfWeekMap));

    uint16_t minutesPastMidnightNow_m = 0;
    ReturnErrorOnFailure(GetMinutesPastMidnight(minutesPastMidnightNow_m));

    uint32_t now_epoch_s = 0;
    ReturnErrorOnFailure(GetEpochTS(now_epoch_s));

    DataModel::Nullable<uint32_t> startTime_epoch_s;
    DataModel::Nullable<uint32_t> targetTime_epoch_s;
    DataModel::Nullable<Percent> targetSoC;
    DataModel::Nullable<int64_t> addedEnergy_mWh;

    uint32_t power_W;
    uint32_t chargingDuration_s;
    uint32_t tempTargetTime_epoch_s;
    uint32_t tempStartTime_epoch_s;
    uint16_t targetTimeMinutesPastMidnight_m;

    // Initialise the values to Null - if the FindNextTarget finds one, then it will update the value
    targetTime_epoch_s.SetNull();
    targetSoC.SetNull();
    addedEnergy_mWh.SetNull();
    startTime_epoch_s.SetNull(); // If we FindNextTarget this will be computed below and set to a non null value

    /* We can only compute charging schedules if the EV is plugged in and the charging is enabled
     * so we know the charging current - i.e. can get the max power, and therefore can calculate
     * the charging duration and hence start time
     */
    if (dg->IsEvsePluggedIn() && dg->GetSupplyState() == SupplyStateEnum::kChargingEnabled)
    {
        uint8_t searchDay = 0;
        while (searchDay < 2)
        {
            err = FindNextTarget(dayOfWeekMap, minutesPastMidnightNow_m, targetTimeMinutesPastMidnight_m, targetSoC,
                                 addedEnergy_mWh, (searchDay != 0));
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                // We didn't find one for today, try tomorrow
                searchDay++;
                dayOfWeekMap = BitMask<EnergyEvse::TargetDayOfWeekBitmap>((dayOfWeekMap.Raw() << 1) & kAllTargetDaysMask);

                if (!dayOfWeekMap.HasAny())
                {
                    // Must be Saturday and shifted off, so set it to Sunday
                    dayOfWeekMap = BitMask<EnergyEvse::TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kSunday);
                }
            }
            else
            {
                break; // We found a target or we error'd out for some other reason
            }
        }

        if (err == CHIP_NO_ERROR)
        {
            /* Set the target Time in epoch_s format*/
            tempTargetTime_epoch_s =
                ((now_epoch_s / 60) + targetTimeMinutesPastMidnight_m + (searchDay * 1440) - minutesPastMidnightNow_m) * 60;
            targetTime_epoch_s.SetNonNull(tempTargetTime_epoch_s);

            if (!targetSoC.IsNull())
            {
                if (targetSoC.Value() != 100)
                {
                    ChipLogError(AppServer, "EVSE WARNING: TargetSoC is not 100%% and we don't know the EV SoC!");
                }
                // We don't know the Vehicle SoC so we must charge now
                // TODO make this use the SoC featureMap to determine if this is an error
                startTime_epoch_s.SetNonNull(now_epoch_s);
            }
            else
            {
                // We expect to use AddedEnergy to determine the charging start time
                if (addedEnergy_mWh.IsNull())
                {
                    ChipLogError(AppServer, "EVSE ERROR: Neither TargetSoC or AddedEnergy has been provided");
                    return CHIP_ERROR_INTERNAL;
                }
                // Simple optimizer - assume a flat tariff throughout the day
                // Compute power from nominal voltage and maxChargingRate
                // GetMaximumChargeCurrent returns mA, but to help avoid overflow
                // We use V (not mV) and compute power to the nearest Watt
                power_W = static_cast<uint32_t>((230 * dg->GetMaximumChargeCurrent()) /
                                                1000); // TODO don't use 230V - not all markets will use that
                if (power_W == 0)
                {
                    ChipLogError(AppServer, "EVSE Error: MaxCurrent = 0Amp - Can't schedule charging");
                    return CHIP_ERROR_INTERNAL;
                }

                // Time to charge(seconds) = (3600 * Energy(mWh) / Power(W)) / 1000
                // to avoid using floats we multiply by 36 and then divide by 10 (instead of x3600 and dividing by 1000)
                chargingDuration_s = static_cast<uint32_t>(((addedEnergy_mWh.Value() / power_W) * 36) / 10);

                // Add in 15 minutes leeway to account for slow starting vehicles
                // that need to condition the battery or if it is cold etc
                chargingDuration_s += (15 * 60);

                // A price optimizer can look for cheapest time of day
                // However for now we'll start charging as late as possible
                tempStartTime_epoch_s = tempTargetTime_epoch_s - chargingDuration_s;

                if (tempStartTime_epoch_s < now_epoch_s)
                {
                    // we need to turn on the EVSE now - it won't have enough time to reach the target
                    startTime_epoch_s.SetNonNull(now_epoch_s);
                    // TODO call function to turn on the EV
                }
                else
                {
                    // we turn off the EVSE for now
                    startTime_epoch_s.SetNonNull(tempStartTime_epoch_s);
                    // TODO have a periodic timer which checks if we should turn on the charger now
                }
            }
        }
    }

    // Update the attributes to allow a UI to inform the user
    dg->SetNextChargeStartTime(startTime_epoch_s);
    dg->SetNextChargeTargetTime(targetTime_epoch_s);
    dg->SetNextChargeRequiredEnergy(addedEnergy_mWh);
    dg->SetNextChargeTargetSoC(targetSoC);

    return err;
}

/**
 * @brief   Allows a client application to initialise the Accuracy, Measurement types etc
 */
CHIP_ERROR EVSEManufacturer::InitializePowerMeasurementCluster()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrReturnError(mn != nullptr, CHIP_ERROR_UNINITIALIZED);

    ElectricalPowerMeasurementDelegate * dg = mn->GetEPMDelegate();
    VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    ReturnErrorOnFailure(dg->SetPowerMode(PowerModeEnum::kAc));

    return CHIP_NO_ERROR;
}

/**
 * @brief   Allows a client application to initialise the PowerSource cluster
 */
CHIP_ERROR EVSEManufacturer::InitializePowerSourceCluster()
{
    Protocols::InteractionModel::Status status;

    status = PowerSource::Attributes::Status::Set(EndpointId(0) /*RootNode*/, PowerSourceStatusEnum::kActive);
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status =
        PowerSource::Attributes::FeatureMap::Set(EndpointId(0 /*RootNode*/), static_cast<uint32_t>(PowerSource::Feature::kWired));
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::WiredNominalVoltage::Set(EndpointId(0 /*RootNode*/), 230'000); // 230V in mv
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::WiredMaximumCurrent::Set(EndpointId(0 /*RootNode*/), 32'000); // 32A in mA
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);

    status = PowerSource::Attributes::WiredCurrentType::Set(EndpointId(0 /*RootNode*/), PowerSource::WiredCurrentTypeEnum::kAc);
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::Description::Set(EndpointId(0 /*RootNode*/), CharSpan::fromCharString("Primary Mains Power"));
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);

    chip::EndpointId endpointArray[] = { 1 /* EVSE Endpoint */ };
    Span<EndpointId> endpointList    = Span<EndpointId>(endpointArray);

    // Note per API - we do not need to maintain the span after the SetEndpointList has been called
    // since it takes a copy (see power-source-server.cpp)
    PowerSourceServer::Instance().SetEndpointList(0 /* Root Node */, endpointList);

    return CHIP_NO_ERROR;
}

/**
 * @brief   Allows a client application to send in power readings into the system
 *
 * @param[in]  aEndpointId       - Endpoint to send to EPM Cluster
 * @param[in]  aActivePower_mW   - ActivePower measured in milli-watts
 * @param[in]  aVoltage_mV       - Voltage measured in milli-volts
 * @param[in]  aActiveCurrent_mA - ActiveCurrent measured in milli-amps
 */
CHIP_ERROR EVSEManufacturer::SendPowerReading(EndpointId aEndpointId, int64_t aActivePower_mW, int64_t aVoltage_mV,
                                              int64_t aActiveCurrent_mA)
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrReturnError(mn != nullptr, CHIP_ERROR_UNINITIALIZED);

    ElectricalPowerMeasurementDelegate * dg = mn->GetEPMDelegate();
    VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    dg->SetActivePower(MakeNullable(aActivePower_mW));
    dg->SetVoltage(MakeNullable(aVoltage_mV));
    dg->SetActiveCurrent(MakeNullable(aActiveCurrent_mA));

    return CHIP_NO_ERROR;
}

using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

/**
 * @brief   Allows a client application to send cumulative energy readings into the system
 *
 *          This is a helper function to add timestamps to the readings
 *
 * @param[in]  aCumulativeEnergyImported -total energy imported in milli-watthours
 * @param[in]  aCumulativeEnergyExported -total energy exported in milli-watthours
 */
CHIP_ERROR EVSEManufacturer::SendCumulativeEnergyReading(EndpointId aEndpointId, int64_t aCumulativeEnergyImported,
                                                         int64_t aCumulativeEnergyExported)
{
    MeasurementData * data = MeasurementDataForEndpoint(aEndpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyMeasurementStruct::Type energyImported;
    EnergyMeasurementStruct::Type energyExported;

    /** IMPORT */
    // Copy last endTimestamp into new startTimestamp if it exists
    energyImported.startTimestamp.ClearValue();
    energyImported.startSystime.ClearValue();
    if (data->cumulativeImported.HasValue())
    {
        energyImported.startTimestamp = data->cumulativeImported.Value().endTimestamp;
        energyImported.startSystime   = data->cumulativeImported.Value().endSystime;
    }

    energyImported.energy = aCumulativeEnergyImported;

    /** EXPORT */
    // Copy last endTimestamp into new startTimestamp if it exists
    energyExported.startTimestamp.ClearValue();
    energyExported.startSystime.ClearValue();
    if (data->cumulativeExported.HasValue())
    {
        energyExported.startTimestamp = data->cumulativeExported.Value().endTimestamp;
        energyExported.startSystime   = data->cumulativeExported.Value().endSystime;
    }

    energyExported.energy = aCumulativeEnergyExported;

    // Get current timestamp
    uint32_t currentTimestamp;
    CHIP_ERROR err = GetEpochTS(currentTimestamp);
    if (err == CHIP_NO_ERROR)
    {
        // use EpochTS
        energyImported.endTimestamp.SetValue(currentTimestamp);
        energyExported.endTimestamp.SetValue(currentTimestamp);
    }
    else
    {
        ChipLogError(AppServer, "GetEpochTS returned error getting timestamp %" CHIP_ERROR_FORMAT, err.Format());

        // use systemTime as a fallback
        System::Clock::Milliseconds64 system_time_ms =
            std::chrono::duration_cast<System::Clock::Milliseconds64>(chip::Server::GetInstance().TimeSinceInit());
        uint64_t nowMS = static_cast<uint64_t>(system_time_ms.count());

        energyImported.endSystime.SetValue(nowMS);
        energyExported.endSystime.SetValue(nowMS);
    }

    // call the SDK to update attributes and generate an event
    if (!NotifyCumulativeEnergyMeasured(aEndpointId, MakeOptional(energyImported), MakeOptional(energyExported)))
    {
        ChipLogError(AppServer, "Failed to notify Cumulative Energy reading.");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

/**
 * @brief   Allows a client application to send periodic energy readings into the system
 *
 *          This is a helper function to add timestamps to the readings
 *
 * @param[in]  aPeriodicEnergyImported - energy imported in milli-watthours in last period
 * @param[in]  aPeriodicEnergyExported - energy exported in milli-watthours in last period
 */
CHIP_ERROR EVSEManufacturer::SendPeriodicEnergyReading(EndpointId aEndpointId, int64_t aPeriodicEnergyImported,
                                                       int64_t aPeriodicEnergyExported)
{
    MeasurementData * data = MeasurementDataForEndpoint(aEndpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyMeasurementStruct::Type energyImported;
    EnergyMeasurementStruct::Type energyExported;

    /** IMPORT */
    // Copy last endTimestamp into new startTimestamp if it exists
    energyImported.startTimestamp.ClearValue();
    energyImported.startSystime.ClearValue();
    if (data->periodicImported.HasValue())
    {
        energyImported.startTimestamp = data->periodicImported.Value().endTimestamp;
        energyImported.startSystime   = data->periodicImported.Value().endSystime;
    }

    energyImported.energy = aPeriodicEnergyImported;

    /** EXPORT */
    // Copy last endTimestamp into new startTimestamp if it exists
    energyExported.startTimestamp.ClearValue();
    energyExported.startSystime.ClearValue();
    if (data->periodicExported.HasValue())
    {
        energyExported.startTimestamp = data->periodicExported.Value().endTimestamp;
        energyExported.startSystime   = data->periodicExported.Value().endSystime;
    }

    energyExported.energy = aPeriodicEnergyExported;

    // Get current timestamp
    uint32_t currentTimestamp;
    CHIP_ERROR err = GetEpochTS(currentTimestamp);
    if (err == CHIP_NO_ERROR)
    {
        // use EpochTS
        energyImported.endTimestamp.SetValue(currentTimestamp);
        energyExported.endTimestamp.SetValue(currentTimestamp);
    }
    else
    {
        ChipLogError(AppServer, "GetEpochTS returned error getting timestamp");

        // use systemTime as a fallback
        System::Clock::Milliseconds64 system_time_ms =
            std::chrono::duration_cast<System::Clock::Milliseconds64>(chip::Server::GetInstance().TimeSinceInit());
        uint64_t nowMS = static_cast<uint64_t>(system_time_ms.count());

        energyImported.endSystime.SetValue(nowMS);
        energyExported.endSystime.SetValue(nowMS);
    }

    // call the SDK to update attributes and generate an event
    if (!NotifyPeriodicEnergyMeasured(aEndpointId, MakeOptional(energyImported), MakeOptional(energyExported)))
    {
        ChipLogError(AppServer, "Failed to notify Cumulative Energy reading.");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void EVSEManufacturer::UpdateEVFakeReadings(const Amperage_mA maximumChargeCurrent)
{
    FakeReadings::GetInstance().SetCurrent(maximumChargeCurrent);
    // Note we have to divide by 1000 to make ma * mv = mW
    FakeReadings::GetInstance().SetPower((FakeReadings::GetInstance().GetVoltage() * maximumChargeCurrent) / 1000);
}

/**
 * @brief    Main Callback handler - to be implemented by Manufacturer
 *
 * @param    EVSECbInfo describes the type of call back, and a union of structs
 *           which contain relevant info for the specific callback type
 *
 * @param    arg - optional pointer to some context information (see register function)
 */
void EVSEManufacturer::ApplicationCallbackHandler(const EVSECbInfo * cb, intptr_t arg)
{
    EVSEManufacturer * pClass = reinterpret_cast<EVSEManufacturer *>(arg);

    switch (cb->type)
    {
    case EVSECallbackType::StateChanged:
        ChipLogProgress(AppServer, "EVSE callback - state changed");
        pClass->ComputeChargingSchedule();
        break;
    case EVSECallbackType::ChargeCurrentChanged:
        ChipLogProgress(AppServer, "EVSE callback - maxChargeCurrent changed to %ld",
                        static_cast<long>(cb->ChargingCurrent.maximumChargeCurrent));
        pClass->ComputeChargingSchedule();
        pClass->UpdateEVFakeReadings(cb->ChargingCurrent.maximumChargeCurrent);
        break;
    case EVSECallbackType::EnergyMeterReadingRequested:
        ChipLogProgress(AppServer, "EVSE callback - EnergyMeterReadingRequested");
        if (cb->EnergyMeterReadingRequest.meterType == ChargingDischargingType::kCharging)
        {
            *(cb->EnergyMeterReadingRequest.energyMeterValuePtr) = pClass->mLastChargingEnergyMeter;
        }
        else
        {
            *(cb->EnergyMeterReadingRequest.energyMeterValuePtr) = pClass->mLastDischargingEnergyMeter;
        }
        break;

    case EVSECallbackType::ChargingPreferencesChanged:
        ChipLogProgress(AppServer, "EVSE callback - ChargingPreferencesChanged");
        pClass->ComputeChargingSchedule();
        break;

    default:
        ChipLogError(AppServer, "Unhandled EVSE Callback type %d", static_cast<int>(cb->type));
    }
}

// The PowerAdjustEnd event needs to report the approximate energy used by the ESA during the session.
int64_t EVSEManufacturer::GetApproxEnergyDuringSession()
{
    return 300;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPowerAdjustRequest(const int64_t powerMw, const uint32_t durationS,
                                                                            AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPowerAdjustCompletion()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementCancelPowerAdjustRequest(CauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementStartTimeAdjustRequest(const uint32_t requestedStartTimeUtc,
                                                                                AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPauseRequest(const uint32_t durationS, AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementCancelPauseRequest(CauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPauseCompletion()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementCancelRequest()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleModifyForecastRequest(
    const uint32_t forecastID,
    const DataModel::DecodableList<DeviceEnergyManagement::Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
    AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::RequestConstraintBasedForecast(
    const DataModel::DecodableList<DeviceEnergyManagement::Structs::ConstraintsStruct::DecodableType> & constraints,
    AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}
