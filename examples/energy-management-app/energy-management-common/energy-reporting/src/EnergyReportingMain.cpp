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

static constexpr int POWER_TOPOLOGY_ENDPOINT=1;
static constexpr int ENERGY_METER_ENDPOINT=1;

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::PowerTopology;

std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
// Electrical Energy Measurement cluster uses ember to initialise
std::unique_ptr<ElectricalEnergyMeasurementAttrAccess> gEEMAttrAccess;

std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
std::unique_ptr<chip::app::Clusters::PowerTopology::PowerTopologyInstance> gPTInstance;

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
        std::make_unique<PowerTopologyInstance>(EndpointId(POWER_TOPOLOGY_ENDPOINT), *gPTDelegate,
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
        EndpointId(ENERGY_METER_ENDPOINT), *gEPMDelegate,
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
