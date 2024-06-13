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

#include <DeviceEnergyManagementDelegateImpl.h>
#include <EVSEManufacturerImpl.h>
#include <DEMManufacturerDelegate.h>
#include <EnergyEvseManager.h>

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <app/clusters/power-source-server/power-source-server.h>
#include <app/server/Server.h>
#include <DEMUtils.h>

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

    DeviceEnergyManagementDelegate * dem = GetEvseManufacturer()->GetDEMDelegate();
    VerifyOrReturnLogError(dem != nullptr, CHIP_ERROR_UNINITIALIZED);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    dem->SetESAState(ESAStateEnum::kOnline);

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

int64_t EVSEManufacturer::GetEnergyUse()
{
    return 300;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPowerAdjustRequest(const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause)
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

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementStartTimeAdjustRequest(const uint32_t requestedStartTime, AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPauseRequest(const uint32_t duration, AdjustmentCauseEnum cause)
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

CHIP_ERROR EVSEManufacturer::HandleModifyRequest(const uint32_t forecastID,
                                                 const DataModel::DecodableList<DeviceEnergyManagement::Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
                                                 AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturer::RequestConstraintBasedForecast(const DataModel::DecodableList<DeviceEnergyManagement::Structs::ConstraintsStruct::DecodableType> & constraints,
                                                            AdjustmentCauseEnum cause)
{
    return CHIP_NO_ERROR;
}
