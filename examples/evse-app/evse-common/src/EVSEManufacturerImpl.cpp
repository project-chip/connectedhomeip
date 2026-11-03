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
#include <ElectricalSensorManager.h>
#include <EnergyEvseDelegateImpl.h>
#include <EnergyEvseManager.h>
#include <EvseChargingSchedule.h>

#include <FakeReadings.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
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

namespace {

class EvseInstanceChargingScheduleContext : public EvseChargingScheduleContext
{
public:
    EvseInstanceChargingScheduleContext(EnergyEvseDelegate * delegate, EnergyEvse::Instance * instance) :
        mDelegate(delegate), mInstance(instance)
    {}

    const DataModel::List<const EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> & GetChargingTargets() const override
    {
        return mDelegate->GetEvseTargetsDelegate()->GetTargets();
    }

    bool IsEvsePluggedIn() const override { return mDelegate->IsEvsePluggedIn(); }

    SupplyStateEnum GetSupplyState() const override { return mDelegate->GetSupplyState(); }

    bool HasSoCReportingFeature() const override { return mInstance->HasFeature(EnergyEvse::Feature::kSoCReporting); }

    DataModel::Nullable<Percent> GetStateOfCharge() const override { return mDelegate->GetStateOfCharge(); }

    DataModel::Nullable<int64_t> GetBatteryCapacity() const override { return mDelegate->GetBatteryCapacity(); }

    int64_t HwGetNominalMainsVoltage() const override { return mDelegate->HwGetNominalMainsVoltage(); }

    int64_t GetMaximumChargeCurrent() const override { return mDelegate->GetMaximumChargeCurrent(); }

    CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> value) override
    {
        return mInstance->SetNextChargeStartTime(value);
    }

    CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> value) override
    {
        return mInstance->SetNextChargeTargetTime(value);
    }

    CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> value) override
    {
        return mInstance->SetNextChargeRequiredEnergy(value);
    }

    CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> value) override
    {
        return mInstance->SetNextChargeTargetSoC(value);
    }

private:
    EnergyEvseDelegate * mDelegate;
    EnergyEvse::Instance * mInstance;
};

} // namespace

CHIP_ERROR EVSEManufacturer::Init(chip::EndpointId powerSourceEndpointId)
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

    ReturnErrorOnFailure(InitializePowerSourceCluster(powerSourceEndpointId));

    DeviceEnergyManagementDelegate * dem = GetEvseManufacturer()->GetDEMDelegate();
    VerifyOrReturnLogError(dem != nullptr, CHIP_ERROR_UNINITIALIZED);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    TEMPORARY_RETURN_IGNORED dem->SetESAState(ESAStateEnum::kOnline);
    TEMPORARY_RETURN_IGNORED dem->SetESAType(ESATypeEnum::kEvse);

    // Set the abs min and max power
    TEMPORARY_RETURN_IGNORED dem->SetAbsMinPower(1200000); // 1.2KW
    TEMPORARY_RETURN_IGNORED dem->SetAbsMaxPower(7600000); // 7.6KW

    /*
     * Manufacturers should replace these example values with the real hardware configuration.
     * They are set here, before any client request can be processed, so that a UserMaximumChargeCurrent
     * with no stored value is defaulted from CircuitCapacity before a client can write it.
     */

    // Hardware mains voltage in mV
    VerifyOrReturnError(dg->HwSetNominalMainsVoltage(230000) == Protocols::InteractionModel::Status::Success,
                        CHIP_ERROR_INTERNAL); // 230V

    // Hardware capability in mA
    VerifyOrReturnError(dg->HwSetMaxHardwareChargeCurrentLimit(32000) == Protocols::InteractionModel::Status::Success,
                        CHIP_ERROR_INTERNAL); // 32A
    VerifyOrReturnError(dg->HwSetMaxHardwareDischargeCurrentLimit(32000) == Protocols::InteractionModel::Status::Success,
                        CHIP_ERROR_INTERNAL); // 32A

    // CircuitCapacity in mA, set by the installer (e.g. from DIP switches)
    VerifyOrReturnError(dg->HwSetCircuitCapacity(20000) == Protocols::InteractionModel::Status::Success,
                        CHIP_ERROR_INTERNAL); // 20A

    /* Once the system is initialised then check to see if the state was restored
     * (e.g. after a power outage), and if the Enable timer check needs to be started.
     * If the check cannot be scheduled, the restored enable is disabled so it cannot outlive its deadline.
     */
    if (dg->ScheduleDeadlineCheckOrDisable() != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(AppServer, "EVSE: unable to schedule restored enable deadline check, EVSE disabled");
    }

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
 *   dg->HwSetVehicleID("TEST_VEHICLE_123456789"_span);
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
 * @brief   Simple example to demonstrate how an EVSE can compute the start time
 *          and duration of a charging schedule
 */
CHIP_ERROR EVSEManufacturer::ComputeChargingSchedule()
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrReturnError(mn != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrReturnError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    EnergyEvse::Instance * instance = dg->GetInstance();
    VerifyOrReturnError(instance != nullptr, CHIP_ERROR_UNINITIALIZED);

    EvseInstanceChargingScheduleContext context(dg, instance);
    return ComputeEvseChargingSchedule(context);
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
CHIP_ERROR EVSEManufacturer::InitializePowerSourceCluster(chip::EndpointId endpointId)
{
    Protocols::InteractionModel::Status status;

    status = PowerSource::Attributes::Status::Set(endpointId, PowerSourceStatusEnum::kActive);
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::FeatureMap::Set(endpointId, static_cast<uint32_t>(PowerSource::Feature::kWired));
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::WiredNominalVoltage::Set(endpointId, 230'000); // 230V in mv
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::WiredMaximumCurrent::Set(endpointId, 32'000); // 32A in mA
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);

    status = PowerSource::Attributes::WiredCurrentType::Set(endpointId, PowerSource::WiredCurrentTypeEnum::kAc);
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);
    status = PowerSource::Attributes::Description::Set(endpointId, "Primary Mains Power"_span);
    VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_ERROR_INTERNAL);

    chip::EndpointId endpointArray[] = { endpointId };
    Span<EndpointId> endpointList    = Span<EndpointId>(endpointArray);

    // Note per API - we do not need to maintain the span after the SetEndpointList has been called
    // since it takes a copy (see power-source-server/codegen/power-source-server.cpp)
    return PowerSourceServer::Instance().SetEndpointList(endpointId, endpointList);
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
        TEMPORARY_RETURN_IGNORED pClass->ComputeChargingSchedule();
        break;
    case EVSECallbackType::ChargeCurrentChanged:
        ChipLogProgress(AppServer, "EVSE callback - maxChargeCurrent changed to %ld",
                        static_cast<long>(cb->ChargingCurrent.maximumChargeCurrent));
        TEMPORARY_RETURN_IGNORED pClass->ComputeChargingSchedule();
        pClass->UpdateEVFakeReadings(cb->ChargingCurrent.maximumChargeCurrent);
        break;
    case EVSECallbackType::DischargeCurrentChanged:
        ChipLogProgress(AppServer, "EVSE callback - maxDischargeCurrent changed to %ld",
                        static_cast<long>(cb->DischargingCurrent.maximumDischargeCurrent));
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
        TEMPORARY_RETURN_IGNORED pClass->ComputeChargingSchedule();
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

CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPowerRangeAdjustRequest(const Optional<int64_t> minPower,
                                                                                 const Optional<int64_t> maxPower,
                                                                                 const uint32_t durationS,
                                                                                 AdjustmentCauseEnum cause)
{
    // Currently not implemented by our EVSE app
    return CHIP_NO_ERROR;
}
CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementPowerRangeAdjustCompletion()
{
    // Currently not implemented by our EVSE app
    return CHIP_NO_ERROR;
}
CHIP_ERROR EVSEManufacturer::HandleDeviceEnergyManagementCancelPowerRangeAdjustRequest(CauseEnum cause)
{
    // Currently not implemented by our EVSE app
    return CHIP_NO_ERROR;
}
