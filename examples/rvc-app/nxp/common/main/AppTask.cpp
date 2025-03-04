/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
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

#include <app/data-model/Nullable.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <rvc-modes.h>
#include <rvc-operational-state-delegate-impl.h>

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <map>
using namespace chip::Shell;
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcRunMode;
using namespace chip::app::Clusters::RvcCleanMode;
using namespace chip::app::Clusters::RvcOperationalState;
using namespace chip::app::Clusters::OperationalState;
using chip::Protocols::InteractionModel::Status;

#ifdef ENABLE_CHIP_SHELL
static CHIP_ERROR cliRVCOpState(int argc, char * argv[])
{
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }
	if (!strcmp(argv[0], "paused"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kPaused));
    }
    else if (!strcmp(argv[0], "resume"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
        RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kRunning));
    }
    else if (!strcmp(argv[0], "stopped"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kStopped));
    }
	else if (!strcmp(argv[0], "running"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kRunning));
    }
	else if (!strcmp(argv[0], "seekingchanger"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kSeekingCharger));
    }
	else if (!strcmp(argv[0], "charging"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kCharging));
    }
	else if (!strcmp(argv[0], "docked"))
    {
        ChipLogDetail(Shell, "RVC opstate: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kDocked));
    }
    else
    {
        inputErr = true;
    }
exit:
    if (inputErr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR cliRVCRunMode(int argc, char * argv[])
{
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }

	if (!strcmp(argv[0], "idle"))
    {
        ChipLogDetail(Shell, "RVC run mode: Set to %s mode", argv[0]);
		RvcRunMode::Instance()->UpdateCurrentMode(RvcRunMode::ModeIdle);
    }
	
    else if (!strcmp(argv[0], "cleaning"))
    {
        ChipLogDetail(Shell, "RVC run mode: Set to %s mode", argv[0]);
		RvcRunMode::Instance()->UpdateCurrentMode(RvcRunMode::ModeCleaning);
    }
    else if (!strcmp(argv[0], "mapping"))
    {
        ChipLogDetail(Shell, "RVC run mode: Set to %s mode", argv[0]);
		RvcRunMode::Instance()->UpdateCurrentMode(RvcRunMode::ModeMapping);
    }
	
    else
    {
        inputErr = true;
    }
exit:
    if (inputErr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR cliRVCCleanMode(int argc, char * argv[])
{
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }

	if (!strcmp(argv[0], "vacuum"))
    {
        ChipLogDetail(Shell, "RVC clean mode: Set to %s mode", argv[0]);
		RvcCleanMode::Instance()->UpdateCurrentMode(RvcCleanMode::ModeVacuum);
    }
	
    else if (!strcmp(argv[0], "wash"))
    {
        ChipLogDetail(Shell, "RVC clean mode: Set to %s mode", argv[0]);
		RvcCleanMode::Instance()->UpdateCurrentMode(RvcCleanMode::ModeWash);
    }
    else if (!strcmp(argv[0], "deepclean"))
    {
        ChipLogDetail(Shell, "RVC clean mode: Set to %s mode", argv[0]);
		RvcCleanMode::Instance()->UpdateCurrentMode(RvcCleanMode::ModeDeepClean);
    }
	
    else
    {
        inputErr = true;
    }
exit:
    if (inputErr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR cliRVCErrorState(int argc, char * argv[])
{
	chip::app::Clusters::detail::Structs::ErrorStateStruct::Type errcode;
    bool inputErr = false;
    if ((argc != 1) && (argc != 2))
    {
        inputErr = true;
        goto exit;
    }	
	
	if (!strcmp(argv[0], "noerror"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kNoError);
    }
	
    else if (!strcmp(argv[0], "unabletostartorresume"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kUnableToStartOrResume);
    }
    else if (!strcmp(argv[0], "unabletocompleteoperation"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kUnableToCompleteOperation);
    }
	else if (!strcmp(argv[0], "commandinvalidinstate"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kCommandInvalidInState);
    }
	else if (!strcmp(argv[0], "failedtofindchargingdock"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kFailedToFindChargingDock);
    }
	else if (!strcmp(argv[0], "stuck"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kStuck);
    }
	else if (!strcmp(argv[0], "dustbinmissing"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing);
    }
	else if (!strcmp(argv[0], "dustbinfull"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinFull);
    }
	else if (!strcmp(argv[0], "watertankempty"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankEmpty);
    }
	else if (!strcmp(argv[0], "watertankmissing"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing);
    }
	else if (!strcmp(argv[0], "watertanklidopen"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankLidOpen);
    }
	else if (!strcmp(argv[0], "mopcleaningpadmissing"))
    {
        ChipLogDetail(Shell, "RVC error state: Set to %s state", argv[0]);
		RvcOperationalState::GetRvcOperationalStateInstance()->SetOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kError));
		errcode.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing);
    }
    else
    {
        inputErr = true;
    }
	
	RvcOperationalState::GetRvcOperationalStateInstance()->OnOperationalErrorDetected(errcode);
	
exit:
    if (inputErr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}
#endif /* ENABLE_CHIP_SHELL */

void RVCApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP RVC Demo App");
}

void RVCApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void RVCApp::AppTask::AppMatter_RegisterCustomCliCommands()
{
#ifdef ENABLE_CHIP_SHELL
    /* Register application commands */
    static const shell_command_t kCommands[] = {
        { .cmd_func = cliRVCOpState,
          .cmd_name = "rvcopstate",
          .cmd_help = "Set RVC operational state. Usage:[paused|resume|stopped|running|seekingchanger|charging|docked] " },
		{ .cmd_func = cliRVCRunMode,
          .cmd_name = "rvcrunmode",
          .cmd_help = "Set RVC run mode. Usage:[idle|cleaning|mapping] " },
		{ .cmd_func = cliRVCCleanMode,
          .cmd_name = "rvccleanmode",
          .cmd_help = "Set RVC clean mode. Usage:[vacuum|wash|deepclean] " },
		{ .cmd_func = cliRVCErrorState,
          .cmd_name = "rvcerrorstate",
          .cmd_help = "Set RVC error state. Usage:[noerror|unabletostartorresume|unabletocompleteoperation|commandinvalidinstate|failedtofindchargingdock|stuck|dustbinmissing|dustbinfull|watertankempty|watertankmissing|watertanklidopen|mopcleaningpadmissing] " },
    };
    Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
#endif
}

// This returns an instance of this class.
RVCApp::AppTask & RVCApp::AppTask::GetDefaultInstance()
{
    static RVCApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return RVCApp::AppTask::GetDefaultInstance();
}