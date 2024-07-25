/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <EVSEManufacturerImpl.h>
#include <EnergyEvseManager.h>

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
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
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

struct FakeReadingsData
{
    bool bEnabled;                  /* If enabled then the timer callback will re-trigger */
    EndpointId mEndpointId;         /* Which endpoint the meter is on */
    uint8_t mInterval_s;            /* Interval in seconds to callback */
    int64_t mPower_mW;              /* Active Power on the load in mW (signed value) +ve = imported */
    uint32_t mPowerRandomness_mW;   /* The amount to randomize the Power on the load in mW */
    int64_t mVoltage_mV;            /* Voltage reading in mV (signed value) */
    uint32_t mVoltageRandomness_mV; /* The amount to randomize the Voltage in mV */
    int64_t mCurrent_mA;            /* ActiveCurrent reading in mA (signed value) */
    uint32_t mCurrentRandomness_mA; /* The amount to randomize the ActiveCurrent in mA */

    /* These energy values can only be positive values.
     * however the underlying energy type (power_mWh) is signed, so keeping with that convention */
    int64_t mTotalEnergyImported    = 0; /* Cumulative Energy Imported which is updated if mPower > 0 */
    int64_t mTotalEnergyExported    = 0; /* Cumulative Energy Imported which is updated if mPower < 0 */
    int64_t mPeriodicEnergyImported = 0; /* Periodic Energy Imported which is updated if mPower > 0 */
    int64_t mPeriodicEnergyExported = 0; /* Periodic Energy Imported which is updated if mPower < 0 */
};

static FakeReadingsData gFakeReadingsData;

/* This helper routine starts and handles a callback */
/**
 * @brief   Starts a fake load/generator to periodically callback the power and energy
 *          clusters.
 * @param[in]   aEndpointId  - which endpoint is the meter to be updated on
 * @param[in]   aPower_mW    - the mean power of the load
 *                             Positive power indicates Imported energy (e.g. a load)
 *                             Negative power indicated Exported energy (e.g. a generator)
 * @param[in]   aPowerRandomness_mW  This is used to define the max randomness of the
 *                             random power values around the mean power of the load
 * @param[in]   aVoltage_mV  - the nominal voltage measurement
 * @param[in]   aVoltageRandomness_mV  This is used to define the max randomness of the
 *                             random voltage values
 * @param[in]   aCurrent_mA  - the nominal current measurement
 * @param[in]   aCurrentRandomness_mA  This is used to define the max randomness of the
 *                             random current values
 * @param[in]   aInterval_s  - the callback interval in seconds
 * @param[in]   bReset       - boolean: true will reset the energy values to 0
 */
void EVSEManufacturer::StartFakeReadings(EndpointId aEndpointId, int64_t aPower_mW, uint32_t aPowerRandomness_mW,
                                         int64_t aVoltage_mV, uint32_t aVoltageRandomness_mV, int64_t aCurrent_mA,
                                         uint32_t aCurrentRandomness_mA, uint8_t aInterval_s, bool bReset)
{
    gFakeReadingsData.bEnabled              = true;
    gFakeReadingsData.mEndpointId           = aEndpointId;
    gFakeReadingsData.mPower_mW             = aPower_mW;
    gFakeReadingsData.mPowerRandomness_mW   = aPowerRandomness_mW;
    gFakeReadingsData.mVoltage_mV           = aVoltage_mV;
    gFakeReadingsData.mVoltageRandomness_mV = aVoltageRandomness_mV;
    gFakeReadingsData.mCurrent_mA           = aCurrent_mA;
    gFakeReadingsData.mCurrentRandomness_mA = aCurrentRandomness_mA;
    gFakeReadingsData.mInterval_s           = aInterval_s;

    if (bReset)
    {
        gFakeReadingsData.mTotalEnergyImported = 0;
        gFakeReadingsData.mTotalEnergyExported = 0;
    }

    // Call update function to kick off regular readings
    FakeReadingsUpdate();
}
/**
 * @brief   Stops any active updates to the fake load data callbacks
 */
void EVSEManufacturer::StopFakeReadings()
{
    gFakeReadingsData.bEnabled = false;
}
/**
 * @brief   Sends fake meter data into the cluster and restarts the timer
 */
void EVSEManufacturer::FakeReadingsUpdate()
{
    /* Check to see if the fake Load is still running - don't send updates if the timer was already cancelled */
    if (!gFakeReadingsData.bEnabled)
    {
        return;
    }

    // Update readings
    // Avoid using floats - so we will do a basic rand() call which will generate a integer value between 0 and RAND_MAX
    // first compute power as a mean + some random value in range +/- mPowerRandomness_mW
    int64_t power =
        (static_cast<int64_t>(rand()) % (2 * gFakeReadingsData.mPowerRandomness_mW)) - gFakeReadingsData.mPowerRandomness_mW;
    power += gFakeReadingsData.mPower_mW; // add in the base power

    int64_t voltage =
        (static_cast<int64_t>(rand()) % (2 * gFakeReadingsData.mVoltageRandomness_mV)) - gFakeReadingsData.mVoltageRandomness_mV;
    voltage += gFakeReadingsData.mVoltage_mV; // add in the base voltage

    /* Note: whilst we could compute a current from the power and voltage,
     * there will always be some random error from the sensor
     * that measures it. To keep this simple and to avoid doing divides in integer
     * format etc use the same approach here too.
     * This is meant more as an example to show how to use the APIs, not
     * to be a real representation of laws of physics.
     */
    int64_t current =
        (static_cast<int64_t>(rand()) % (2 * gFakeReadingsData.mCurrentRandomness_mA)) - gFakeReadingsData.mCurrentRandomness_mA;
    current += gFakeReadingsData.mCurrent_mA; // add in the base current

    SendPowerReading(gFakeReadingsData.mEndpointId, power, voltage, current);

    // update the energy meter - we'll assume that the power has been constant during the previous interval
    if (gFakeReadingsData.mPower_mW > 0)
    {
        // Positive power - means power is imported
        gFakeReadingsData.mPeriodicEnergyImported = ((power * gFakeReadingsData.mInterval_s) / 3600);
        gFakeReadingsData.mPeriodicEnergyExported = 0;
        gFakeReadingsData.mTotalEnergyImported += gFakeReadingsData.mPeriodicEnergyImported;
    }
    else
    {
        // Negative power - means power is exported, but the exported energy is reported positive
        gFakeReadingsData.mPeriodicEnergyImported = 0;
        gFakeReadingsData.mPeriodicEnergyExported = ((-power * gFakeReadingsData.mInterval_s) / 3600);
        gFakeReadingsData.mTotalEnergyExported += gFakeReadingsData.mPeriodicEnergyExported;
    }

    SendPeriodicEnergyReading(gFakeReadingsData.mEndpointId, gFakeReadingsData.mPeriodicEnergyImported,
                              gFakeReadingsData.mPeriodicEnergyExported);

    SendCumulativeEnergyReading(gFakeReadingsData.mEndpointId, gFakeReadingsData.mTotalEnergyImported,
                                gFakeReadingsData.mTotalEnergyExported);

    // start/restart the timer
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(gFakeReadingsData.mInterval_s), FakeReadingsTimerExpiry, this);
}
/**
 * @brief   Timer expiry callback to handle fake load
 */
void EVSEManufacturer::FakeReadingsTimerExpiry(System::Layer * systemLayer, void * manufacturer)
{
    EVSEManufacturer * mn = reinterpret_cast<EVSEManufacturer *>(manufacturer);

    mn->FakeReadingsUpdate();
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
        break;
    case EVSECallbackType::ChargeCurrentChanged:
        ChipLogProgress(AppServer, "EVSE callback - maxChargeCurrent changed to %ld",
                        static_cast<long>(cb->ChargingCurrent.maximumChargeCurrent));
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

    default:
        ChipLogError(AppServer, "Unhandled EVSE Callback type %d", static_cast<int>(cb->type));
    }
}

struct EVSETestEventSaveData
{
    int64_t mOldMaxHardwareCurrentLimit;
    int64_t mOldCircuitCapacity;
    int64_t mOldUserMaximumChargeCurrent;
    int64_t mOldCableAssemblyLimit;
    StateEnum mOldHwStateBasic;           /* For storing hwState before Basic Func event */
    StateEnum mOldHwStatePluggedIn;       /* For storing hwState before PluggedIn event */
    StateEnum mOldHwStatePluggedInDemand; /* For storing hwState before PluggedInDemand event */
};

static EVSETestEventSaveData sEVSETestEventSaveData;

EnergyEvseDelegate * GetEvseDelegate()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");
    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "EVSE Delegate is null");

    return dg;
}

void SetTestEventTrigger_BasicFunctionality()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldMaxHardwareCurrentLimit  = dg->HwGetMaxHardwareCurrentLimit();
    sEVSETestEventSaveData.mOldCircuitCapacity          = dg->GetCircuitCapacity();
    sEVSETestEventSaveData.mOldUserMaximumChargeCurrent = dg->GetUserMaximumChargeCurrent();
    sEVSETestEventSaveData.mOldHwStateBasic             = dg->HwGetState();

    dg->HwSetMaxHardwareCurrentLimit(32000);
    dg->HwSetCircuitCapacity(32000);
    dg->SetUserMaximumChargeCurrent(32000);
    dg->HwSetState(StateEnum::kNotPluggedIn);
}
void SetTestEventTrigger_BasicFunctionalityClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetMaxHardwareCurrentLimit(sEVSETestEventSaveData.mOldMaxHardwareCurrentLimit);
    dg->HwSetCircuitCapacity(sEVSETestEventSaveData.mOldCircuitCapacity);
    dg->SetUserMaximumChargeCurrent(sEVSETestEventSaveData.mOldUserMaximumChargeCurrent);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStateBasic);
}
void SetTestEventTrigger_EVPluggedIn()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldCableAssemblyLimit = dg->HwGetCableAssemblyLimit();
    sEVSETestEventSaveData.mOldHwStatePluggedIn   = dg->HwGetState();

    dg->HwSetCableAssemblyLimit(63000);
    dg->HwSetState(StateEnum::kPluggedInNoDemand);
}
void SetTestEventTrigger_EVPluggedInClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();
    dg->HwSetCableAssemblyLimit(sEVSETestEventSaveData.mOldCableAssemblyLimit);
    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedIn);
}

void SetTestEventTrigger_EVChargeDemand()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    sEVSETestEventSaveData.mOldHwStatePluggedInDemand = dg->HwGetState();
    dg->HwSetState(StateEnum::kPluggedInDemand);
}
void SetTestEventTrigger_EVChargeDemandClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetState(sEVSETestEventSaveData.mOldHwStatePluggedInDemand);
}
void SetTestEventTrigger_EVSEGroundFault()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kGroundFault);
}

void SetTestEventTrigger_EVSEOverTemperatureFault()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kOverTemperature);
}

void SetTestEventTrigger_EVSEFaultClear()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwSetFault(FaultStateEnum::kNoError);
}

void SetTestEventTrigger_EVSEDiagnosticsComplete()
{
    EnergyEvseDelegate * dg = GetEvseDelegate();

    dg->HwDiagnosticsComplete();
}

void SetTestEventTrigger_FakeReadingsLoadStart()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");

    int64_t aPower_mW              = 1'000'000; // Fake load 1000 W
    uint32_t aPowerRandomness_mW   = 20'000;    // randomness 20W
    int64_t aVoltage_mV            = 230'000;   // Fake Voltage 230V
    uint32_t aVoltageRandomness_mV = 1'000;     // randomness 1V
    int64_t aCurrent_mA            = 4'348;     // Fake Current (at 1kW@230V = 4.3478 Amps)
    uint32_t aCurrentRandomness_mA = 500;       // randomness 500mA
    uint8_t aInterval_s            = 2;         // 2s updates
    bool bReset                    = true;
    mn->StartFakeReadings(EndpointId(1), aPower_mW, aPowerRandomness_mW, aVoltage_mV, aVoltageRandomness_mV, aCurrent_mA,
                          aCurrentRandomness_mA, aInterval_s, bReset);
}

void SetTestEventTrigger_FakeReadingsGeneratorStart()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");

    int64_t aPower_mW              = -3'000'000; // Fake Generator -3000 W
    uint32_t aPowerRandomness_mW   = 20'000;     // randomness 20W
    int64_t aVoltage_mV            = 230'000;    // Fake Voltage 230V
    uint32_t aVoltageRandomness_mV = 1'000;      // randomness 1V
    int64_t aCurrent_mA            = -13'043;    // Fake Current (at -3kW@230V = -13.0434 Amps)
    uint32_t aCurrentRandomness_mA = 500;        // randomness 500mA
    uint8_t aInterval_s            = 5;          // 5s updates
    bool bReset                    = true;
    mn->StartFakeReadings(EndpointId(1), aPower_mW, aPowerRandomness_mW, aVoltage_mV, aVoltageRandomness_mV, aCurrent_mA,
                          aCurrentRandomness_mA, aInterval_s, bReset);
}

void SetTestEventTrigger_FakeReadingsStop()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");
    mn->StopFakeReadings();
}

bool HandleEnergyEvseTestEventTrigger(uint64_t eventTrigger)
{
    EnergyEvseTrigger trigger = static_cast<EnergyEvseTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyEvseTrigger::kBasicFunctionality:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => Basic Functionality install");
        SetTestEventTrigger_BasicFunctionality();
        break;
    case EnergyEvseTrigger::kBasicFunctionalityClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => Basic Functionality clear");
        SetTestEventTrigger_BasicFunctionalityClear();
        break;
    case EnergyEvseTrigger::kEVPluggedIn:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV plugged in");
        SetTestEventTrigger_EVPluggedIn();
        break;
    case EnergyEvseTrigger::kEVPluggedInClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV unplugged");
        SetTestEventTrigger_EVPluggedInClear();
        break;
    case EnergyEvseTrigger::kEVChargeDemand:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV Charge Demand");
        SetTestEventTrigger_EVChargeDemand();
        break;
    case EnergyEvseTrigger::kEVChargeDemandClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EV Charge NoDemand");
        SetTestEventTrigger_EVChargeDemandClear();
        break;
    case EnergyEvseTrigger::kEVSEGroundFault:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE has a GroundFault fault");
        SetTestEventTrigger_EVSEGroundFault();
        break;
    case EnergyEvseTrigger::kEVSEOverTemperatureFault:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE has a OverTemperature fault");
        SetTestEventTrigger_EVSEOverTemperatureFault();
        break;
    case EnergyEvseTrigger::kEVSEFaultClear:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE faults have cleared");
        SetTestEventTrigger_EVSEFaultClear();
        break;
    case EnergyEvseTrigger::kEVSEDiagnosticsComplete:
        ChipLogProgress(Support, "[EnergyEVSE-Test-Event] => EVSE Diagnostics Completed");
        SetTestEventTrigger_EVSEDiagnosticsComplete();
        break;

    default:
        return false;
    }

    return true;
}

bool HandleEnergyReportingTestEventTrigger(uint64_t eventTrigger)
{
    EnergyReportingTrigger trigger = static_cast<EnergyReportingTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyReportingTrigger::kFakeReadingsStop:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Stop Fake load");
        SetTestEventTrigger_FakeReadingsStop();
        break;
    case EnergyReportingTrigger::kFakeReadingsLoadStart_1kW_2s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake load 1kW @2s Import");
        SetTestEventTrigger_FakeReadingsLoadStart();
        break;
    case EnergyReportingTrigger::kFakeReadingsGenStart_3kW_5s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake generator 3kW @5s Export");
        SetTestEventTrigger_FakeReadingsGeneratorStart();
        break;

    default:
        return false;
    }

    return true;
}
