/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024,2026 NXP
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#include <app/server/Server.h>
#include <DEMConfig.h>
#include <DEMManufacturerDelegate.h>
#include <EnergyEvseMain.h>
#include <EVSEManufacturerImpl.h>
#include "UserInterfaceFeedback.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "BLEApplicationManager.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;
using namespace chip::app::Clusters::EnergyEvse;

constexpr chip::EndpointId kEvseEndpoint        = 1;
constexpr const char * kEvseApp                 = "evse";
constexpr const char * kValidApps[]             = { kEvseApp };
constexpr EndpointId kValidEndpoints[]          = { kEvseEndpoint };

bool CHARGING_ON = false;

/* The amount to randomize the Power on the load in mW */
uint32_t gPowerRandomness_mW = 20000;
/* Active Power on the load in mW (signed value) +ve = imported */
int64_t gPower_mW = 22000000;

/* Voltage reading in mV (signed value) */
int64_t gVoltage_mV = 230000;
/* The amount to randomize the Voltage in mV */
uint32_t gVoltageRandomness_mV = 1000;

/* ActiveCurrent reading in mA (signed value) */
int64_t gCurrent_mA = 95000;
/* The amount to randomize the ActiveCurrent in mA */
uint32_t gCurrentRandomness_mA = 500;

/* Cumulative Energy Imported which is updated if mPower > 0 */
int64_t gTotalEnergyImported = 0;

/* Cumulative Energy Imported which is updated if mPower < 0 */
int64_t gTotalEnergyExported = 0;

/* Periodic Energy Imported which is updated if mPower > 0 */
int64_t gPeriodicEnergyImported = 0;

/* Periodic Energy Imported which is updated if mPower < 0 */
int64_t gPeriodicEnergyExported = 0;

/* Interval in seconds to callback */
uint8_t gInterval_s = 5;

uint8_t gStateOfCharge = 0;

int64_t gFullChargingEnergy = 66000000;

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kPowerForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);

// Make EVSE the default app
static const char * spApp        = kEvseApp;
static EndpointId sAppEndpointId = kEvseEndpoint;

chip::BitMask<Feature> GetFeatureMapFromCmdLine()
{
    return sFeatureMap;
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

chip::EndpointId GetEnergyDeviceEndpointId()
{
    return sAppEndpointId;
}

static CHIP_ERROR energyReadingHandler(int argc, char * argv[])
{
    uint16_t power = 0;
    uint16_t voltage = 0;
    uint16_t current = 0;
    if ((argc != 1) && (argc != 3))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    power = (uint16_t) strtoul(argv[0], nullptr, 10);
    voltage = (uint16_t) strtoul(argv[1], nullptr, 10);
    current = (uint16_t) strtoul(argv[2], nullptr, 10);

    TEMPORARY_RETURN_IGNORED GetDEMDelegate()->GetDEMManufacturerDelegate()->SendPowerReading(1, power, voltage, current);

    ChipLogProgress(DeviceLayer, "Set power to: %d mW, voltage to: %d wV, current to: %d wA", static_cast<uint16_t>(power), static_cast<uint16_t>(voltage), static_cast<uint16_t>(current));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR cliSimulateCharging(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!strcmp(argv[0], "toggle"))
    {
        ChipLogDetail(Shell, "Toggle charging status");
		TEMPORARY_RETURN_IGNORED EVSEApp::AppTask::GetDefaultInstance().ProcessSetStateClusterHandler();
    }
    else
    {
        ChipLogError(Shell, "Invalid command");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

void EVSEApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP EVSE Demo App");
}

void EVSEApp::AppTask::PostInitMatterStack()
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#ifdef APP_BT_DEVICE_NAME
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(APP_BT_DEVICE_NAME);
#endif
    /* BLEApplicationManager implemented per platform or left blank */
    chip::NXP::App::BleAppMgr().Init();
#endif
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void EVSEApp::AppTask::PostInitMatterServerInstance()
{
    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();
    if (pTestEventDelegate != nullptr)
    {
        static EnergyEvseTestEventTriggerHandler sEnergyEvseTestEventTriggerHandler;
        static EnergyReportingTestEventTriggerHandler sEnergyReportingTestEventTriggerHandler;
        static DeviceEnergyManagementTestEventTriggerHandler sDeviceEnergyManagementTestEventTriggerHandler;
    
        VerifyOrDie(pTestEventDelegate->AddHandler(&sEnergyEvseTestEventTriggerHandler) == CHIP_NO_ERROR);
        VerifyOrDie(pTestEventDelegate->AddHandler(&sEnergyReportingTestEventTriggerHandler) == CHIP_NO_ERROR);
        VerifyOrDie(pTestEventDelegate->AddHandler(&sDeviceEnergyManagementTestEventTriggerHandler) == CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot add handler for delegate");
    }
    EvseApplicationInit();
}

void EVSEApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = energyReadingHandler,
          .cmd_name = "setenergy",
          .cmd_help = "report mocked energy readings: power(mW) voltage(mV) current(mA), example: setenergy 10000 5000 2000" },
        { .cmd_func = cliSimulateCharging,
          .cmd_name = "charging",
          .cmd_help = "Toggle the charging status, acts just like the SW2 button, example: charging toggle" },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
EVSEApp::AppTask & EVSEApp::AppTask::GetDefaultInstance()
{
    static EVSEApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return EVSEApp::AppTask::GetDefaultInstance();
}

bool EVSEApp::AppTask::CheckStateClusterHandler(void)
{
    bool val = false;
	if(CHARGING_ON)
    {
        val = false;
        CHARGING_ON = false;
        EVSEApp::AppTask::GetDefaultInstance().StopChargingHandler();
    }
    else
    {
        val = true;
        CHARGING_ON = true;
    }
    return val;
}

CHIP_ERROR EVSEApp::AppTask::ProcessSetStateClusterHandler(void)
{
    FeedbackMgr().RestoreState();
    return CHIP_NO_ERROR;
}

bool EVSEApp::AppTask::isCommissioningcomplete(void)
{
    return ConnectivityMgr().IsWiFiStationProvisioned();
}

void EVSEApp::AppTask::StopChargingHandler(void)
{
    TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(StopChargingInternal, 0);
}

void EVSEApp::AppTask::CharingUpdateHandler(void)
{
    TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(UpdateChargingInternal, 0);
}

void EVSEApp::AppTask::UpdateChargingInternal(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "EVSE charging information updating.");
    // Update readings
    int64_t power = (static_cast<int64_t>(rand()) % (2 * gPowerRandomness_mW)) - gPowerRandomness_mW;
    power += gPower_mW; // add in the base power
	
    int64_t voltage = (static_cast<int64_t>(rand()) % (2 * gVoltageRandomness_mV)) - gVoltageRandomness_mV;
    voltage += gVoltage_mV; // add in the base voltage
	
    int64_t current = (static_cast<int64_t>(rand()) % (2 * gCurrentRandomness_mA)) - gCurrentRandomness_mA;
    current += gCurrent_mA; // add in the base current
	
    TEMPORARY_RETURN_IGNORED GetDEMDelegate()->GetDEMManufacturerDelegate()->SendPowerReading(kEvseEndpoint, power, voltage, current);
    
	
    // update the energy meter - we'll assume that the power has been constant during the previous interval
    if (gPower_mW > 0)
    {
        // Positive power - means power is imported
        gPeriodicEnergyImported = ((power * gInterval_s) / 3600);
        gPeriodicEnergyExported = 0;
        gTotalEnergyImported += gPeriodicEnergyImported;
    }
    else
    {
        // Negative power - means power is exported, but the exported energy is reported positive
        gPeriodicEnergyImported = 0;
        gPeriodicEnergyExported = ((-power * gInterval_s) / 3600);
        gTotalEnergyExported += gPeriodicEnergyExported;
    }
	
    TEMPORARY_RETURN_IGNORED GetDEMDelegate()->GetDEMManufacturerDelegate()->SendPeriodicEnergyReading(kEvseEndpoint, gPeriodicEnergyImported,
                                                                              gPeriodicEnergyExported);
	
    TEMPORARY_RETURN_IGNORED GetDEMDelegate()->GetDEMManufacturerDelegate()->SendCumulativeEnergyReading(kEvseEndpoint, gTotalEnergyImported,
                                                                            gTotalEnergyExported);

    if(gStateOfCharge <= 100)
    {
        EVSEManufacturer * mn = GetEvseManufacturer();
        VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");
        EnergyEvseDelegate * dg = mn->GetEvseDelegate();
        VerifyOrDieWithMsg(dg != nullptr, AppServer, "EVSE Delegate is null");
        EnergyEvse::Instance * instance = dg->GetInstance();
        VerifyOrDieWithMsg(instance != nullptr, AppServer, "EVSE Instance is null");
        gStateOfCharge = (gTotalEnergyImported * 100) / gFullChargingEnergy;
        dg->HwSetState(StateEnum::kPluggedInDemand);
        TEMPORARY_RETURN_IGNORED instance->SetStateOfCharge(gStateOfCharge);
    }

	if(gTotalEnergyImported >= gFullChargingEnergy)
	{
        gTotalEnergyImported = 0;
        gStateOfCharge = 0;
        TEMPORARY_RETURN_IGNORED EVSEApp::AppTask::GetDefaultInstance().ProcessSetStateClusterHandler();
	}
}

void EVSEApp::AppTask::StopChargingInternal(intptr_t arg)
{
    EVSEManufacturer * mn = GetEvseManufacturer();
    VerifyOrDieWithMsg(mn != nullptr, AppServer, "EVSEManufacturer is null");
    EnergyEvseDelegate * dg = mn->GetEvseDelegate();
    VerifyOrDieWithMsg(dg != nullptr, AppServer, "EVSE Delegate is null");
    dg->HwSetState(StateEnum::kPluggedInNoDemand);

    ChipLogProgress(DeviceLayer, "EVSE charging stopped.");
}
