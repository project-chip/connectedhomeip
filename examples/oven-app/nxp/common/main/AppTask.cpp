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
#include <oven-modes.h>
#include <oven-operational-state-delegate.h>

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
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;

static CHIP_ERROR cliOvenOp(int argc, char * argv[])
{
    if ((argc != 1) && (argc != 2))
    {
        ChipLogError(Shell, "Invalid Argument");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    OperationalState::Instance * operationalStateInstance = OvenCavityOperationalState::GetOperationalStateInstance();
    if ((!strcmp(argv[0], "start")) || (!strcmp(argv[0], "resume")))
    {
        ChipLogDetail(Shell, "OpOven: Oven running");
        TEMPORARY_RETURN_IGNORED operationalStateInstance->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kRunning));
    }
    else if (!strcmp(argv[0], "stop"))
    {
        ChipLogDetail(Shell, "OpOven: Oven stopped");
        TEMPORARY_RETURN_IGNORED operationalStateInstance->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kStopped));
    }
    else if (!strcmp(argv[0], "pause"))
    {
        ChipLogDetail(Shell, "OpOven: Oven paused");
        TEMPORARY_RETURN_IGNORED operationalStateInstance->SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kPaused));
    }
    else if (!strcmp(argv[0], "onfault"))
    {
        ChipLogDetail(Shell, "OpOven: Send Oven UnableToCompleteOperation event");
        uint8_t event_id = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);

        OperationalState::GenericOperationalError err(event_id);
        operationalStateInstance->OnOperationalErrorDetected(err);
    }
    else
    {
        ChipLogError(Shell, "Invalid State to set");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

void OvenApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Oven Demo App");
}

void OvenApp::AppTask::PostInitMatterStack()
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

void OvenApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliOvenOp,
          .cmd_name = "oven",
          .cmd_help = "Change oven op, usage: oven start|stop|pause|resume|onfault" },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
OvenApp::AppTask & OvenApp::AppTask::GetDefaultInstance()
{
    static OvenApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return OvenApp::AppTask::GetDefaultInstance();
}
