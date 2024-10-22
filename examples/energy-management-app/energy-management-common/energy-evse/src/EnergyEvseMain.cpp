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

#include "EnergyManagementAppCmdLineOptions.h"

#include <DeviceEnergyManagementManager.h>
#include <EVSEManufacturerImpl.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <EnergyEvseManager.h>
#include <PowerTopologyDelegate.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/power-topology-server/power-topology-server.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#define ENERGY_EVSE_ENDPOINT 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::PowerTopology;

static std::unique_ptr<EnergyEvseDelegate> gEvseDelegate;
static std::unique_ptr<EvseTargetsDelegate> gEvseTargetsDelegate;
static std::unique_ptr<EnergyEvseManager> gEvseInstance;
static std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
static std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
static std::unique_ptr<EVSEManufacturer> gEvseManufacturer;
static std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
static std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
// Electrical Energy Measurement cluster uses ember to initialise
static std::unique_ptr<ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;

static std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
static std::unique_ptr<PowerTopologyInstance> gPTInstance;

EVSEManufacturer * EnergyEvse::GetEvseManufacturer()
{
    return gEvseManufacturer.get();
}

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");

    return gDEMDelegate.get();
}

/*
 *  @brief  Creates a Delegate and Instance for DEM
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR DeviceEnergyManagementInit()
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

    BitMask<DeviceEnergyManagement::Feature> featureMap = GetFeatureMapFromCmdLine();

    /* Manufacturer may optionally not support all features, commands & attributes */
    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(EndpointId(ENERGY_EVSE_ENDPOINT), *gDEMDelegate, featureMap);

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

CHIP_ERROR DeviceEnergyManagementShutdown()
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
    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a Delegate and Instance for EVSE cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EnergyEvseInit()
{
    CHIP_ERROR err;

    if (gEvseDelegate || gEvseInstance || gEvseTargetsDelegate)
    {
        ChipLogError(AppServer, "EVSE Instance, Delegate or TargetsDelegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEvseTargetsDelegate = std::make_unique<EvseTargetsDelegate>();
    if (!gEvseTargetsDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseTargetsDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    gEvseDelegate = std::make_unique<EnergyEvseDelegate>(*gEvseTargetsDelegate);
    if (!gEvseDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseDelegate");
        gEvseTargetsDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gEvseInstance = std::make_unique<EnergyEvseManager>(
        EndpointId(ENERGY_EVSE_ENDPOINT), *gEvseDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kRfid),
        BitMask<EnergyEvse::OptionalAttributes, uint32_t>(EnergyEvse::OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                          EnergyEvse::OptionalAttributes::kSupportsRandomizationWindow,
                                                          EnergyEvse::OptionalAttributes::kSupportsApproximateEvEfficiency),
        BitMask<EnergyEvse::OptionalCommands, uint32_t>(EnergyEvse::OptionalCommands::kSupportsStartDiagnostics));

    if (!gEvseInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EnergyEvseManager");
        gEvseTargetsDelegate.reset();
        gEvseDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gEvseInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseInstance");
        gEvseTargetsDelegate.reset();
        gEvseInstance.reset();
        gEvseDelegate.reset();
        return err;
    }

    err = gEvseTargetsDelegate->LoadTargets();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to LoadTargets");
        gEvseTargetsDelegate.reset();
        gEvseInstance.reset();
        gEvseDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gEvseInstance)
    {
        /* deregister attribute & command handlers */
        gEvseInstance->Shutdown();
        gEvseInstance.reset();
    }

    if (gEvseDelegate)
    {
        gEvseDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a Delegate and Instance for PowerTopology clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR PowerTopologyInit()
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

    gPTInstance =
        std::make_unique<PowerTopologyInstance>(EndpointId(ENERGY_EVSE_ENDPOINT), *gPTDelegate,
                                                BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology),
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
 *  @brief  Creates a Delegate and Instance for Electrical Power/Energy Measurement clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EnergyMeterInit()
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
        EndpointId(ENERGY_EVSE_ENDPOINT), *gEPMDelegate,
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

CHIP_ERROR EnergyMeterShutdown()
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
 *  @brief  Creates a EVSEManufacturer class to hold the EVSE & DEM clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EVSEManufacturerInit()
{
    CHIP_ERROR err;

    if (gEvseManufacturer)
    {
        ChipLogError(AppServer, "EvseManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create EVSEManufacturer */
    gEvseManufacturer =
        std::make_unique<EVSEManufacturer>(gEvseInstance.get(), gEPMInstance.get(), gPTInstance.get(), gDEMInstance.get());
    if (!gEvseManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    gDEMDelegate.get()->SetDEMManufacturerDelegate(*gEvseManufacturer.get());

    /* Call Manufacturer specific init */
    err = gEvseManufacturer->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gEvseManufacturer");
        gEvseManufacturer.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EVSEManufacturerShutdown()
{
    if (gEvseManufacturer)
    {
        /* Shutdown the EVSEManufacturer */
        gEvseManufacturer->Shutdown();
        gEvseManufacturer.reset();
    }

    return CHIP_NO_ERROR;
}

void EvseApplicationInit()
{
    if (DeviceEnergyManagementInit() != CHIP_NO_ERROR)
    {
        return;
    }

    if (EnergyEvseInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        return;
    }

    if (EnergyMeterInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        EnergyEvseShutdown();
        return;
    }

    if (PowerTopologyInit() != CHIP_NO_ERROR)
    {
        EVSEManufacturerShutdown();
        DeviceEnergyManagementShutdown();
        EnergyEvseShutdown();
        EnergyMeterShutdown();
        return;
    }

    /* Do this last so that the instances for other clusters can be wrapped inside */
    if (EVSEManufacturerInit() != CHIP_NO_ERROR)
    {
        DeviceEnergyManagementShutdown();
        EnergyEvseShutdown();
        EnergyMeterShutdown();
        return;
    }
}

void EvseApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App (EVSE): ApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    EVSEManufacturerShutdown();       /* Free the EVSEManufacturer */
    PowerTopologyShutdown();          /* Free the PowerTopology */
    EnergyMeterShutdown();            /* Free the Energy Meter */
    EnergyEvseShutdown();             /* Free the EnergyEvse */
    DeviceEnergyManagementShutdown(); /* Free the DEM */

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::EnergyEvseMode::Shutdown();
}

void emberAfElectricalEnergyMeasurementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gEEMAttrAccess);

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
        gEEMAttrAccess->Init();

        SetMeasurementAccuracy(endpointId, accuracy);
        SetCumulativeReset(endpointId, MakeOptional(resetStruct));
    }
}
