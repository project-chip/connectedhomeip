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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <DeviceEnergyManagementDelegateImpl.h>
#include <DeviceEnergyManagementManager.h>
#include <ElectricalPowerMeasurementDelegateImpl.h>
#include <EnergyManagementAppCmdLineOptions.h>
#include <PowerTopologyDelegateImpl.h>
#include <WaterHeaterInstance.h>
#include <WaterHeaterMain.h>
#include <WaterHeaterManufacturer.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <device-energy-management-modes.h>
#include <water-heater-mode.h>

#include <DEMConfig.h>
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

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kMeasurementAccuracyRanges[] = {
    { .rangeMin   = 0,
      .rangeMax   = 1'000'000'000'000'000, // 1 million Mwh
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(500)),
      .percentMin = MakeOptional(static_cast<chip::Percent100ths>(50)) }
};

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type kMeasurementAccuracy = {
    .measurementType  = MeasurementTypeEnum::kElectricalEnergy,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 1'000'000'000'000'000, // 1 million Mwh
    .accuracyRanges   = DataModel::List<const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type>(
        kMeasurementAccuracyRanges)
};

// Common cluster instances
std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
std::unique_ptr<PowerTopologyInstance> gPTInstance;
std::unique_ptr<ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;
bool gCommonClustersInitialized = false;

// Water Heater specific instances
std::unique_ptr<WaterHeaterManagementDelegate> gWaterHeaterMgmtDelegate;
std::unique_ptr<WaterHeaterManagementInstance> gWaterHeaterMgmtInstance;
std::unique_ptr<WaterHeaterManufacturer> gWaterHeaterManufacturer;

/*
 *  @brief  Creates a Delegate and Instance for Water Heater Management cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR WaterHeaterManagementInit(EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gWaterHeaterMgmtDelegate || gWaterHeaterMgmtInstance)
    {
        ChipLogError(AppServer, "WaterHeaterManager Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gWaterHeaterMgmtDelegate = std::make_unique<WaterHeaterManagementDelegate>(endpointId);
    if (!gWaterHeaterMgmtDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WaterHeaterManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gWaterHeaterMgmtInstance = std::make_unique<WaterHeaterManagementInstance>(
        EndpointId(endpointId), *gWaterHeaterMgmtDelegate,
        BitMask<WaterHeaterManagement::Feature>(WaterHeaterManagement::Feature::kEnergyManagement,
                                                WaterHeaterManagement::Feature::kTankPercent));
    if (!gWaterHeaterMgmtInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WaterHeaterManagementInstance");
        gWaterHeaterMgmtDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Register Attribute & Command handlers */
    err = gWaterHeaterMgmtInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "gWaterHeaterMgmtInstance->Init failed: %" CHIP_ERROR_FORMAT, err.Format());
        gWaterHeaterMgmtInstance.reset();
        gWaterHeaterMgmtDelegate.reset();
        return err;
    }

    gWaterHeaterMgmtDelegate->SetWaterHeaterManagementInstance(*gWaterHeaterMgmtInstance);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WaterHeaterManagementShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gWaterHeaterMgmtInstance)
    {
        /* Deregister attribute & command handlers */
        gWaterHeaterMgmtInstance->Shutdown();
        gWaterHeaterMgmtInstance.reset();
    }

    if (gWaterHeaterMgmtDelegate)
    {
        gWaterHeaterMgmtDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a WaterHeaterManufacturer class to hold the Water Heater Management cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR WaterHeaterManufacturerInit()
{
    CHIP_ERROR err;

    if (gWaterHeaterManufacturer)
    {
        ChipLogError(AppServer, "WaterHeaterManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create WaterHeaterManufacturer */
    gWaterHeaterManufacturer = std::make_unique<WaterHeaterManufacturer>(gWaterHeaterMgmtInstance.get());
    if (!gWaterHeaterManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WaterHeaterManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Call Manufacturer specific init */
    err = gWaterHeaterManufacturer->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gWaterHeaterManufacturer");
        gWaterHeaterManufacturer.reset();
        return err;
    }

    // Let the Water Heater Management Delegate know about the WaterHeaterManufacturer object.
    gWaterHeaterMgmtDelegate->SetWaterHeaterManufacturer(*gWaterHeaterManufacturer);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WaterHeaterManufacturerShutdown()
{
    if (gWaterHeaterManufacturer)
    {
        /* Shutdown the WaterHeaterManufacturer */
        TEMPORARY_RETURN_IGNORED gWaterHeaterManufacturer->Shutdown();
        gWaterHeaterManufacturer.reset();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyManagementCommonClustersInit(chip::EndpointId endpointId)
{
    if (!gCommonClustersInitialized)
    {
        TEMPORARY_RETURN_IGNORED DeviceEnergyManagementInit(endpointId, gDEMDelegate, gDEMInstance, GetFeatureMapFromCmdLine());
        // These features and optional attributes are used to make the app pass certification
        // We recommend implementers of the app to modify these to fit their needs
        TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementInit(
            endpointId, gEPMDelegate, gEPMInstance,
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
        TEMPORARY_RETURN_IGNORED PowerTopologyInit(endpointId, gPTDelegate, gPTInstance);
    }
    VerifyOrReturnError(gDEMDelegate && gDEMInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gEPMDelegate && gEPMInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gPTDelegate && gPTInstance, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gEEMAttrAccess, CHIP_ERROR_INCORRECT_STATE);
    gCommonClustersInitialized = true;
    return CHIP_NO_ERROR;
}

} // namespace

DeviceEnergyManagement::DeviceEnergyManagementDelegate * GetDEMDelegate()
{
    VerifyOrDieWithMsg(gDEMDelegate.get() != nullptr, AppServer, "DEM Delegate is null");
    return gDEMDelegate.get();
}

WaterHeaterManufacturer * WaterHeaterManagement::GetWaterHeaterManufacturer()
{
    return gWaterHeaterManufacturer.get();
}

void emberAfElectricalEnergyMeasurementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(!gEEMAttrAccess);

    gEEMAttrAccess = std::make_unique<ElectricalEnergyMeasurementAttrAccess>(
        BitMask<ElectricalEnergyMeasurement::Feature, uint32_t>(
            ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy,
            ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kPeriodicEnergy),
        BitMask<ElectricalEnergyMeasurement::OptionalAttributes, uint32_t>(
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    ElectricalEnergyMeasurement::Structs::CumulativeEnergyResetStruct::Type resetStruct = {
        .importedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
        .exportedResetTimestamp = MakeOptional(MakeNullable(static_cast<uint32_t>(0))),
        .importedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
        .exportedResetSystime   = MakeOptional(MakeNullable(static_cast<uint64_t>(0))),
    };

    if (gEEMAttrAccess)
    {
        TEMPORARY_RETURN_IGNORED gEEMAttrAccess->Init();
        TEMPORARY_RETURN_IGNORED SetMeasurementAccuracy(endpointId, kMeasurementAccuracy);
        TEMPORARY_RETURN_IGNORED SetCumulativeReset(endpointId, MakeOptional(resetStruct));
    }
}

void WaterHeaterApplicationInit()
{
    auto endpointId = GetEnergyDeviceEndpointId();
    VerifyOrDie(EnergyManagementCommonClustersInit(endpointId) == CHIP_NO_ERROR);

    // Initialize Water Heater specific clusters
    VerifyOrDie(WaterHeaterManagementInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(WaterHeaterManufacturerInit() == CHIP_NO_ERROR);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetESAState(ESAStateEnum::kOnline);
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetESAType(ESATypeEnum::kWaterHeating);
    gDEMDelegate->SetDEMManufacturerDelegate(*gWaterHeaterManufacturer.get());

    // Set the abs min and max power
    // Simulating a heat pump water heater with a max power of 5.6KW
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetAbsMinPower(0);       // 0KW
    TEMPORARY_RETURN_IGNORED gDEMDelegate->SetAbsMaxPower(5600000); // 5.6KW
}

void WaterHeaterApplicationShutdown()
{
    ChipLogDetail(AppServer, "Water Heater App: WaterHeaterShutdown()");

    /* Shutdown in reverse order that they were created */
    TEMPORARY_RETURN_IGNORED PowerTopologyShutdown(gPTInstance, gPTDelegate);                /* Free the PowerTopology */
    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown(gEPMInstance, gEPMDelegate); /* Free the Energy Meter */
    DeviceEnergyManagementShutdown(gDEMInstance, gDEMDelegate);                              /* Free the DEM */

    // Shutdown Water Heater specific clusters
    TEMPORARY_RETURN_IGNORED WaterHeaterManufacturerShutdown();
    TEMPORARY_RETURN_IGNORED WaterHeaterManagementShutdown();

    DeviceEnergyManagementMode::Shutdown();
    WaterHeaterMode::Shutdown();
}

EndpointId GetIdentifyEndpointId()
{
    return GetEnergyDeviceEndpointId();
}
