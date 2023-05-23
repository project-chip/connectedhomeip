/*
*
*    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <operational-state-delegates.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using chip::Protocols::InteractionModel::Status;


static OperationalErrorStateStruct & setOperationalNoError(OperationalErrorStateStruct & error)
{
    char opNoError[64] = "No Error";
    char opNoErrorDetails[64] = "No Error Details";

    error.ErrorStateID = static_cast<uint8_t>(ErrorStateEnum::kNoError);
    memset(error.ErrorStateLabel, 0, sizeof(error.ErrorStateLabel));
    memset(error.ErrorStateDetails, 0, sizeof(error.ErrorStateDetails));
    memcpy(error.ErrorStateLabel, opNoError, sizeof(opNoError));
    memcpy(error.ErrorStateDetails, opNoErrorDetails, sizeof(opNoErrorDetails));

    return error;
}
//-- Operational state delegate functions

CHIP_ERROR OperationalStateDelegate::Init()
{
    return CHIP_NO_ERROR;
}

void OperationalStateDelegate::HandlePauseState(OperationalStateStruct & state, OperationalErrorStateStruct & error)
{
    ChipLogDetail(Zcl, "Op: HandlePauseState");
    state.OperationalStateID = static_cast<uint8_t>(OperationalStateEnum::kPaused);
    setOperationalNoError(error);
}

void OperationalStateDelegate::HandleResumeState(OperationalStateStruct & state, OperationalErrorStateStruct & error)
{
    ChipLogDetail(Zcl, "Op: HandleResumeState");
    state.OperationalStateID = static_cast<uint8_t>(OperationalStateEnum::kRunning);
    setOperationalNoError(error);
}

void OperationalStateDelegate::HandleStartState(OperationalStateStruct & state, OperationalErrorStateStruct & error)
{
    ChipLogDetail(Zcl, "Op: HandleStartState");
    state.OperationalStateID = static_cast<uint8_t>(OperationalStateEnum::kRunning);
    setOperationalNoError(error);
}

void OperationalStateDelegate::HandleStopState(OperationalStateStruct & state, OperationalErrorStateStruct & error)
{
    ChipLogDetail(Zcl, "Op: HandleStopState");
    state.OperationalStateID = static_cast<uint8_t>(OperationalStateEnum::kStopped);
    setOperationalNoError(error);
}
