/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      This file provides the Shell implementation of Matter.
 *      It can be also used in ASR AT Command implementation.
 */

#include "matter_shell.h"
#include "AppConfig.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <lega_rtos_api.h>
#include <lib/core/CHIPError.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <string>
#include <vector>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "init_OTARequestor.h"
#endif

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#ifdef CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"

using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;
#endif

void asr_matter_reset(Reset_t type)
{
    if (type == WIFI_RESET)
    {
        ConnectivityMgr().ClearWiFiStationProvision();
        chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
        chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    }
    else if (type == FACTORY_RESET)
    {
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
    else if (type == COMMISSIONING_RESET)
    {
        chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
        auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                   CommissioningWindowAdvertisement::kDnssdOnly);
    }
}

void ShutdownChip()
{
    Server::GetInstance().Shutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();
}

void asr_matter_onoff(int value)
{
    ChipLogProgress(Zcl, "updating on/off = %d", value);

    Protocols::InteractionModel::Status status = chip::app::Clusters::OnOff::Attributes::OnOff::Set(
        /* endpoint ID */ 1, (uint8_t *) &value);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: updating on/off %x", to_underlying(status));
    }
}

void asr_matter_sensors(bool enable, int temp, int humi, int pressure)
{
#ifdef ASR_BOARD_ENABLE_SENSORS
    if (enable)
    {
        chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, static_cast<int16_t>(temp));

        chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, static_cast<int16_t>(humi));

        chip::app::Clusters::PressureMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, static_cast<int16_t>(pressure));
    }
#else
    ChipLogProgress(Zcl, "Sensor is not supported!");
#endif
}

void asr_matter_ota(uint32_t timeout)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTAInitializer::Instance().ReloadQueryTimeout(timeout);
#else
    ChipLogProgress(Zcl, "OTA is not supported!");
#endif
}

#ifdef CONFIG_ENABLE_CHIP_SHELL
static CHIP_ERROR sLightCommandHandler(int argc, char ** argv)
{
    if (argc == 1 && strcmp(argv[0], "on") == 0)
    {
        asr_matter_onoff(1);
        return CHIP_NO_ERROR;
    }
    if (argc == 1 && strcmp(argv[0], "off") == 0)
    {
        asr_matter_onoff(0);
        return CHIP_NO_ERROR;
    }
    streamer_printf(streamer_get(), "Usage: OnOff [on|off]");
    return CHIP_NO_ERROR;
}

void RegisterLightCommands()
{
    static const shell_command_t sLightCommand = { sLightCommandHandler, "OnOff", "OnOff commands. Usage: OnOff [on|off]" };
    Engine::Root().RegisterCommands(&sLightCommand, 1);
}
#endif
