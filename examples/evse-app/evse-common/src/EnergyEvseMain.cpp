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
#include <ElectricalPowerMeasurementDelegateImpl.h>
#include <EnergyEvseManager.h>
#include <EnergyManagementAppCmdLineOptions.h>
#include <PowerTopologyDelegateImpl.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
#include <device-energy-management-modes.h>
#include <energy-evse-modes.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/Nullable.h>
#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

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

// EVSE-specific instances
std::unique_ptr<EnergyEvseDelegate> gEvseDelegate;
std::unique_ptr<EvseTargetsDelegate> gEvseTargetsDelegate;
std::unique_ptr<EnergyEvseManager> gEvseInstance;
std::unique_ptr<EVSEManufacturer> gEvseManufacturer;

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

    // Link the delegate to the instance for attribute access
    gEvseDelegate->SetInstance(gEvseInstance.get());

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
        chip::BitMask<DeviceEnergyManagement::Feature> featureMap = GetFeatureMapFromCmdLine();
        TEMPORARY_RETURN_IGNORED DeviceEnergyManagementInit(endpointId, gDEMDelegate, gDEMInstance, featureMap);
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
    ChipLogDetail(AppServer, "Evse App: EvseApplicationShutdown()");

    /* Shutdown in reverse order that they were created */
    TEMPORARY_RETURN_IGNORED EVSEManufacturerShutdown();
    TEMPORARY_RETURN_IGNORED PowerTopologyShutdown(gPTInstance, gPTDelegate);
    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown(gEPMInstance, gEPMDelegate);
    TEMPORARY_RETURN_IGNORED EnergyEvseShutdown();
    DeviceEnergyManagementShutdown(gDEMInstance, gDEMDelegate);

    Clusters::DeviceEnergyManagementMode::Shutdown();
    Clusters::EnergyEvseMode::Shutdown();
}

EndpointId GetIdentifyEndpointId()
{
    return GetEnergyDeviceEndpointId();
}
