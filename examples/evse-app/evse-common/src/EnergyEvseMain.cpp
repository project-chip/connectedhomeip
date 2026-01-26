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

#include <EnergyEvseMain.h>

#include <DeviceEnergyManagementDelegateImpl.h>
#include <DeviceEnergyManagementManager.h>
#include <EVSEManufacturerImpl.h>
#include <ElectricalPowerMeasurementDelegate.h>
#include <EnergyEvseManager.h>
#include <EnergyManagementAppCmdLineOptions.h>
#include <PowerTopologyDelegate.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/data-model/Nullable.h>
#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::PowerTopology;

namespace {

// Common cluster instances
std::unique_ptr<DeviceEnergyManagementDelegate> gDEMDelegate;
std::unique_ptr<DeviceEnergyManagementManager> gDEMInstance;
std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
std::unique_ptr<PowerTopologyInstance> gPTInstance;
std::unique_ptr<ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;
bool gCommonClustersInitialized = false;

// EVSE-specific instances
std::unique_ptr<EnergyEvseDelegate> gEvseDelegate;
std::unique_ptr<EvseTargetsDelegate> gEvseTargetsDelegate;
std::unique_ptr<EnergyEvseManager> gEvseInstance;
std::unique_ptr<EVSEManufacturer> gEvseManufacturer;

/*
 *  @brief  Creates a Delegate and Instance for PowerTopology cluster
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
        EndpointId(endpointId), *gPTDelegate, BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology));

    if (!gPTInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for PowerTopology Instance");
        gPTDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gPTInstance->Init();
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
    if (gPTInstance)
    {
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

    err = gEPMInstance->Init();
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
    if (gEPMInstance)
    {
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
 *  @brief  Creates a Delegate and Instance for Device Energy Management cluster
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

    gDEMInstance = std::make_unique<DeviceEnergyManagementManager>(endpointId, *gDEMDelegate, featureMap);

    if (!gDEMInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for DeviceEnergyManagementManager");
        gDEMDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    gDEMDelegate->SetDeviceEnergyManagementInstance(*gDEMInstance);

    CHIP_ERROR err = gDEMInstance->Init();
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
    if (gDEMInstance)
    {
        gDEMInstance->Shutdown();
        gDEMInstance.reset();
    }
    if (gDEMDelegate)
    {
        gDEMDelegate.reset();
    }
}

/*
 *  @brief  Creates a Delegate and Instance for Energy EVSE cluster
 */
CHIP_ERROR EnergyEvseInit(chip::EndpointId endpointId)
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

    gEvseInstance = std::make_unique<EnergyEvseManager>(
        EndpointId(endpointId), *gEvseDelegate,
        BitMask<EnergyEvse::Feature, uint32_t>(EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kRfid,
                                               EnergyEvse::Feature::kSoCReporting, EnergyEvse::Feature::kPlugAndCharge,
                                               EnergyEvse::Feature::kV2x),
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

    err = gEvseInstance->Init();
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
    if (gEvseInstance)
    {
        gEvseInstance->Shutdown();
        gEvseInstance.reset();
    }
    if (gEvseDelegate)
    {
        gEvseDelegate.reset();
    }
    if (gEvseTargetsDelegate)
    {
        gEvseTargetsDelegate.reset();
    }
    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates the EVSEManufacturer to coordinate EVSE & DEM clusters
 *
 *  The Instance is a container around the Delegate, so
 *  create the Delegate first, then wrap it in the Instance
 *  Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR EVSEManufacturerInit(chip::EndpointId powerSourceEndpointId)
{
    CHIP_ERROR err;

    if (gEvseManufacturer)
    {
        ChipLogError(AppServer, "EvseManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gEvseManufacturer =
        std::make_unique<EVSEManufacturer>(gEvseInstance.get(), gEPMInstance.get(), gPTInstance.get(), gDEMInstance.get());
    if (!gEvseManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for EvseManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    gDEMDelegate->SetDEMManufacturerDelegate(*gEvseManufacturer.get());

    err = gEvseManufacturer->Init(powerSourceEndpointId);
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
        TEMPORARY_RETURN_IGNORED gEvseManufacturer->Shutdown();
        gEvseManufacturer.reset();
    }
    return CHIP_NO_ERROR;
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
    if (endpointId != GetEnergyDeviceEndpointId())
    {
        return;
    }

    VerifyOrDie(!gEEMAttrAccess);

    gEEMAttrAccess = std::make_unique<ElectricalEnergyMeasurementAttrAccess>(
        BitMask<ElectricalEnergyMeasurement::Feature, uint32_t>(
            ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy,
            ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kPeriodicEnergy),
        BitMask<ElectricalEnergyMeasurement::OptionalAttributes, uint32_t>(
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

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
        .maxMeasuredValue = 1'000'000'000'000'000,
        .accuracyRanges =
            DataModel::List<const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type>(energyAccuracyRanges)
    };

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

EVSEManufacturer * EnergyEvse::GetEvseManufacturer()
{
    return gEvseManufacturer.get();
}

void EvseApplicationInit()
{
    auto endpointId = GetEnergyDeviceEndpointId();
    VerifyOrDie(EnergyManagementCommonClustersInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(EnergyEvseInit(endpointId) == CHIP_NO_ERROR);
    VerifyOrDie(EVSEManufacturerInit(endpointId) == CHIP_NO_ERROR);
}

void EvseApplicationShutdown()
{
    ChipLogDetail(AppServer, "Energy Management App (EVSE): EvseApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    TEMPORARY_RETURN_IGNORED EVSEManufacturerShutdown();
    TEMPORARY_RETURN_IGNORED PowerTopologyShutdown();
    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown();
    TEMPORARY_RETURN_IGNORED EnergyEvseShutdown();
    DeviceEnergyManagementShutdown();

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::EnergyEvseMode::Shutdown();
}

EndpointId GetIdentifyEndpointId()
{
    return GetEnergyDeviceEndpointId();
}
