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
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/core/ClusterEnums.h>

#include "AppConfig.h"
#include "AppTask.h"
#include "DataModelHelper.h"
#include "DishwasherManager.h"
#include "ElectricalEnergyMeasurementInstance.h"
#include "ElectricalSensorManager.h"
#include "EnergyTimeUtils.h"
#include "PowerTopologyDelegate.h"
#include <ElectricalPowerMeasurementDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::DataModel;

ElectricalSensorManager ElectricalSensorManager::sElectricalSensor;

static std::unique_ptr<ElectricalEnergyMeasurementInstance> gEEMInstance;
static std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
static std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;
static std::unique_ptr<PowerTopologyDelegate> gPTDelegate;
static std::unique_ptr<PowerTopologyInstance> gPTInstance;

static const struct
{
    PowerModeEnum PowerMode;
    int64_t Voltage;       // mV
    int64_t ActiveCurrent; // mA
    int64_t ActivePower;   // mW
    int64_t Frequency;     // Hz
} kAttributes[4] = {
    { PowerModeEnum::kAc, 120'000, 0, 0, 60 },             // kStopped
    { PowerModeEnum::kAc, 120'000, 15'000, 1800'000, 60 }, // kRunning
    { PowerModeEnum::kAc, 120'000, 125, 17'000, 60 },      // kPaused
    { PowerModeEnum::kUnknown, 0, 0, 0, 0 }                // kError
};

/*
 *  @brief  Creates a Delegate and Instance for Electrical Power/Energy Measurement and Power Topology clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR ElectricalSensorManager::Init()
{
    EndpointId EPMEndpointId = DataModelHelper::GetEndpointIdFromCluster(ElectricalPowerMeasurement::Id);
    EndpointId EEMEndpointId = DataModelHelper::GetEndpointIdFromCluster(ElectricalEnergyMeasurement::Id);
    EndpointId PTEndpointId  = DataModelHelper::GetEndpointIdFromCluster(PowerTopology::Id);
    CHIP_ERROR err;

    VerifyOrReturnError((EPMEndpointId != kInvalidEndpointId) && (EEMEndpointId != kInvalidEndpointId) &&
                            (PTEndpointId != kInvalidEndpointId),
                        CHIP_ERROR_INCORRECT_STATE, ChipLogError(AppServer, "EPM, EEM or PT Cluster not configured"));

    // Initialize EPM (Electrical Power Management)
    VerifyOrReturnError(!gEPMDelegate && !gEPMInstance, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "EPM Delegate or Instance already exist"));

    gEPMDelegate = std::make_unique<ElectricalPowerMeasurementDelegate>();
    VerifyOrReturnError(gEPMDelegate, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for EPM Delegate"));

    /* Manufacturer may optionally not support all features, commands & attributes */
    /* Turning on all optional features and attributes for test certification purposes */
    gEPMInstance = std::make_unique<ElectricalPowerMeasurementInstance>(
        EPMEndpointId, *gEPMDelegate,
        BitMask<ElectricalPowerMeasurement::Feature, uint32_t>(ElectricalPowerMeasurement::Feature::kDirectCurrent,
                                                               ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
        BitMask<ElectricalPowerMeasurement::OptionalAttributes, uint32_t>(
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeVoltage,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeActiveCurrent));

    VerifyOrReturnError(gEPMInstance, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for EPM Instance");
                        gEPMDelegate.reset());

    // Register Attribute & Command handlers
    err = gEPMInstance->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "Init failed on gEPMInstance"); gEPMInstance.reset();
                        gEPMDelegate.reset());

    // Initialize EPM attributes
    OperationalStateEnum state = GetDishwasherManager()->GetOperationalState();
    UpdateEPMAttributes(state);

    // Initialize EEM (Electrical Energy Management)
    VerifyOrReturnError(!gEEMInstance, CHIP_ERROR_INCORRECT_STATE, ChipLogError(AppServer, "EEM Instance already exist"));

    gEEMInstance = std::make_unique<ElectricalEnergyMeasurementInstance>(
        EEMEndpointId, *gEPMDelegate,
        BitMask<ElectricalEnergyMeasurement::Feature, uint32_t>(ElectricalEnergyMeasurement::Feature::kImportedEnergy,
                                                                ElectricalEnergyMeasurement::Feature::kCumulativeEnergy),
        BitMask<ElectricalEnergyMeasurement::OptionalAttributes, uint32_t>(
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    VerifyOrReturnError(gEEMInstance, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for EEM Instance"));

    // Register Attribute & Command handlers
    err = gEEMInstance->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "Init failed on gEEMInstance"); gPTInstance.reset());

    // Initialize PT (Power Topology)
    VerifyOrReturnError(!gPTDelegate && !gPTInstance, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "PowerTopology Delegate or Instance already exist"));

    gPTDelegate = std::make_unique<PowerTopologyDelegate>();
    VerifyOrReturnError(gPTDelegate, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for PT Delegate"));

    gPTInstance = std::make_unique<PowerTopologyInstance>(
        PTEndpointId, *gPTDelegate, BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology),
        BitMask<PowerTopology::OptionalAttributes, uint32_t>());

    VerifyOrReturnError(gPTInstance, CHIP_ERROR_NO_MEMORY, ChipLogError(AppServer, "Failed to allocate memory for PT Instance");
                        gPTDelegate.reset());

    // Register Attribute & Command handlers
    err = gPTInstance->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, ChipLogError(AppServer, "Init failed on gPTInstance"); gPTInstance.reset();
                        gPTDelegate.reset(););

    return CHIP_NO_ERROR;
}

void ElectricalSensorManager::Shutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */

    // Shutdown PT (Power Topology)
    if (gPTInstance)
    {
        // Deregister attribute & command handlers
        gPTInstance->Shutdown();
        gPTInstance.reset();
    }

    if (gPTDelegate)
    {
        gPTDelegate.reset();
    }

    // Shutdown EEM (Electrical Energy Management)
    if (gEEMInstance)
    {
        // Deregister attribute & command handlers
        gEEMInstance->Shutdown();
        gEEMInstance.reset();
    }

    // Shutdown EPM (Electrical Power Management)
    if (gEPMInstance)
    {
        // Deregister attribute & command handlers
        gEPMInstance->Shutdown();
        gEPMInstance.reset();
    }

    if (gEPMDelegate)
    {
        gEPMDelegate.reset();
    }
}

void ElectricalSensorManager::UpdateEPMAttributes(OperationalStateEnum state)
{
    if (gEPMDelegate)
    {
        uint8_t updateState = to_underlying(state);

        // Check state range
        if (updateState >= MATTER_ARRAY_SIZE(kAttributes))
        {
            updateState = MATTER_ARRAY_SIZE(kAttributes) - 1;
        }

        ChipLogDetail(AppServer, "UpdateAllAttributes to Operational State : %d", updateState);

        // Added to support testing using a static array for now
        gEPMDelegate->SetPowerMode(kAttributes[updateState].PowerMode);
        gEPMDelegate->SetVoltage(MakeNullable(kAttributes[updateState].Voltage));
        gEPMDelegate->SetActiveCurrent(MakeNullable(kAttributes[updateState].ActiveCurrent));
        gEPMDelegate->SetActivePower(MakeNullable(kAttributes[updateState].ActivePower));
    }
}
