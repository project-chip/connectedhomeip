/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "EnergyManagementAppCommonMain.h"
#include "EnergyManagementAppCmdLineOptions.h"
#include <DeviceEnergyManagementDelegateImpl.h>
#include <DeviceEnergyManagementManager.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <EnergyEvseMain.h>
#include <PowerTopologyDelegate.h>
#include <WhmMain.h>
#include <WhmManufacturer.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>
#include <water-heater-mode.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/Nullable.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::WaterHeaterManagement;

namespace {

std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
std::unique_ptr<PowerTopologyInstance> gPTInstance;
// Electrical Energy Measurement cluster uses ember to initialise
std::unique_ptr<ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;
bool gCommonClustersInitialized = false;

/*
 *  @brief  Creates a Delegate and Instance for PowerTopology clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR PowerTopologyInit(chip::EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gPTDelegate || gPTInstance)
    {
        ChipLogError(AppServer, "PowerTopology Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gPTDelegate = std::make_unique<PowerTopologyDelegate>();
    if (!gPTDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for PowerTopology Delegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    gPTInstance = std::make_unique<PowerTopologyInstance>(
        EndpointId(endpointId), *gPTDelegate, BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology),
        BitMask<PowerTopology::OptionalAttributes, uint32_t>(0));

    if (!gPTInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for PowerTopology Instance");
        gPTDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gPTInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gPTInstance");
        gPTInstance.reset();
        gPTDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gPTInstance)
    {
        /* deregister attribute & command handlers */
        gPTInstance->Shutdown();
        gPTInstance.reset();
    }

    if (gPTDelegate)
    {
        gPTDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a Delegate and Instance for Electrical Power Measurement cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR ElectricalPowerMeasurementInit(chip::EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gEPMDelegate || gEPMInstance)
    {
        ChipLogError(AppServer, "EPM Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEPMDelegate = std::make_unique<ElectricalPowerMeasurementDelegate>();
    if (!gEPMDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EPM Delegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    /* Turning on all optional features and attributes for test certification purposes */
    gEPMInstance = std::make_unique<ElectricalPowerMeasurementInstance>(
        EndpointId(endpointId), *gEPMDelegate,
        BitMask<ElectricalPowerMeasurement::Feature, uint32_t>(
            ElectricalPowerMeasurement::Feature::kDirectCurrent, ElectricalPowerMeasurement::Feature::kAlternatingCurrent,
            ElectricalPowerMeasurement::Feature::kPolyphasePower, ElectricalPowerMeasurement::Feature::kHarmonics,
            ElectricalPowerMeasurement::Feature::kPowerQuality),
        BitMask<ElectricalPowerMeasurement::OptionalAttributes, uint32_t>(
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeRanges,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeVoltage,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeActiveCurrent,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeReactiveCurrent,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeApparentCurrent,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeReactivePower,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeApparentPower,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeRMSVoltage,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeRMSCurrent,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeRMSPower,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeFrequency,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributePowerFactor,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeNeutralCurrent));

    if (!gEPMInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EPM Instance");
        gEPMDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gEPMInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEPMInstance");
        gEPMInstance.reset();
        gEPMDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gEPMInstance)
    {
        /* deregister attribute & command handlers */
        gEPMInstance->Shutdown();
        gEPMInstance.reset();
    }

    if (gEPMDelegate)
    {
        gEPMDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a Delegate and Instance for DEM
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR DeviceEnergyManagementInit(chip::EndpointId endpointId)
{
    if (gDEMDelegate || gDEMInstance)
    {
        ChipLogError(AppServer, "DEM Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gDEMDelegate = std::make_unique<DeviceEnergyManagementDelegate>();
    if (!gDEMDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    chip::BitMask<DeviceEnergyManagement::Feature> featureMap = GetFeatureMapFromCmdLine();

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(endpointId, *gDEMDelegate, featureMap);

    if (!gDEMInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        gDEMDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    gDEMDelegate->SetDeviceEnergyManagementInstance(*gDEMInstance);

    CHIP_ERROR err = gDEMInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gDEMInstance");
        gDEMInstance.reset();
        gDEMDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

void DeviceEnergyManagementShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gDEMInstance)
    {
        /* deregister attribute & command handlers */
        gDEMInstance->Shutdown();
        gDEMInstance.reset();
    }
    if (gDEMDelegate)
    {
        gDEMDelegate.reset();
    }
}

CHIP_ERROR EnergyManagementCommonClustersInit(chip::EndpointId endpointId)
{
    if (!gCommonClustersInitialized)
    {
        TEMPORARY_RETURN_IGNORED DeviceEnergyManagementInit(endpointId);
        TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementInit(endpointId);
        TEMPORARY_RETURN_IGNORED PowerTopologyInit(endpointId);
    }
    VerifyOrReturnError(gDEMDelegate && gDEMInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gEPMDelegate && gEPMInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gPTDelegate && gPTInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gEEMAttrAccess, CHIP_ERROR_INCORRECT_STATE);
    gCommonClustersInitialized = true;
    return CHIP_NO_ERROR;
}

} // namespace

void emberAfElectricalEnergyMeasurementClusterInitCallback(chip::EndpointId endpointId)
{
    /* emberAfElectricalEnergyMeasurementClusterInitCallback() is called for all endpoints
       that include the EEM endpoint (even the one we disable dynamically). So here, we only
       proceed when it's called for the right endpoint determined by GetEnergyDeviceEndpointId().
    */
    if (endpointId != GetEnergyDeviceEndpointId())
    {
        return;
    }

    VerifyOrDie(!gEEMAttrAccess); // Ensure it's not initialized yet.

    gEEMAttrAccess = std::make_unique<ElectricalEnergyMeasurementAttrAccess>(
        BitMask<ElectricalEnergyMeasurement::Feature, uint32_t>(
            ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy,
            ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kPeriodicEnergy),
        BitMask<ElectricalEnergyMeasurement::OptionalAttributes, uint32_t>(
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    // Create an accuracy entry which is between +/-0.5 and +/- 5% across the range of all possible energy readings
    ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type energyAccuracyRanges[] = {
        { .rangeMin   = 0,
          .rangeMax   = 1'000'000'000'000'000, // 1 million Mwh
          .percentMax = MakeOptional(static_cast<chip::Percent100ths>(500)),
          .percentMin = MakeOptional(static_cast<chip::Percent100ths>(50)) }
    };

    ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type accuracy = {
        .measurementType  = MeasurementTypeEnum::kElectricalEnergy,
        .measured         = true,
        .minMeasuredValue = 0,
        .maxMeasuredValue = 1'000'000'000'000'000, // 1 million Mwh
        .accuracyRanges =
            DataModel::List<const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type>(energyAccuracyRanges)
    };

    // Example of setting CumulativeEnergyReset structure - for now set these to 0
    // but the manufacturer may want to store these in non volatile storage for timestamp (based on epoch_s)
    ElectricalEnergyMeasurement::Structs::CumulativeEnergyResetStruct::Type resetStruct = {
        .importedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
        .exportedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
        .importedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
        .exportedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
    };

    if (gEEMAttrAccess)
    {
        TEMPORARY_RETURN_IGNORED gEEMAttrAccess->Init();

        TEMPORARY_RETURN_IGNORED SetMeasurementAccuracy(endpointId, accuracy);
        TEMPORARY_RETURN_IGNORED SetCumulativeReset(endpointId, MakeOptional(resetStruct));
    }
}

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");

    return gDEMDelegate.get();
}

void EvseApplicationInit()
{
    auto endpointId = GetEnergyDeviceEndpointId();
    VerifyOrDie(EnergyManagementCommonClustersInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(EnergyEvseInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(EVSEManufacturerInit(endpointId, *gEPMInstance.get(), *gPTInstance.get(), *gDEMInstance.get(),
                                     *gDEMDelegate.get()) == CHIP_NO_ERROR);
}

void EvseApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App (EVSE): EvseApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    TEMPORARY_RETURN_IGNORED EVSEManufacturerShutdown();           /* Free the EVSEManufacturer */
    TEMPORARY_RETURN_IGNORED PowerTopologyShutdown();              /* Free the PowerTopology */
    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown(); /* Free the Electrical Power Measurement */
    TEMPORARY_RETURN_IGNORED EnergyEvseShutdown();                 /* Free the EnergyEvse */
    DeviceEnergyManagementShutdown();                              /* Free the DEM */

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::EnergyEvseMode::Shutdown();
}

void WaterHeaterApplicationInit()
{
    auto endpointId = GetEnergyDeviceEndpointId();
    VerifyOrDie(EnergyManagementCommonClustersInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(WhmApplicationInit(endpointId) == CHIP_NO_ERROR);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetESAState(ESAStateEnum::kOnline);
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetESAType(ESATypeEnum::kWaterHeating);
    gDEMDelegate->SetDEMManufacturerDelegate(*GetWhmManufacturer());

    // Set the abs min and max power
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetAbsMinPower(1200000); // 1.2KW
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetAbsMaxPower(7600000); // 7.6KW
}

void WaterHeaterApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App (WaterHeater): WaterHeaterShutdown()");

    /* Shutdown in reverse order that they were created */
    TEMPORARY_RETURN_IGNORED PowerTopologyShutdown();              /* Free the PowerTopology */
    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown(); /* Free the Energy Meter */
    DeviceEnergyManagementShutdown();                              /* Free the DEM */
    TEMPORARY_RETURN_IGNORED WhmApplicationShutdown();

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::WaterHeaterMode::Shutdown();
}
