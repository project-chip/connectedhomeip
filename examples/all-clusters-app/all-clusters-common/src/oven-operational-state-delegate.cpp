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
#include <oven-operational-state-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OvenCavityOperationalState;

static OvenCavityOperationalState::Instance * gOvenCavityOperationalStateInstance = nullptr;
static OvenCavityOperationalStateDelegate * gOvenCavityOperationalStateDelegate   = nullptr;

OvenCavityOperationalState::Instance * OvenCavityOperationalState::GetOperationalStateInstance()
{
    return gOvenCavityOperationalStateInstance;
}

void OvenCavityOperationalState::Shutdown()
{
    if (gOvenCavityOperationalStateInstance != nullptr)
    {
        delete gOvenCavityOperationalStateInstance;
        gOvenCavityOperationalStateInstance = nullptr;
    }
    if (gOvenCavityOperationalStateDelegate != nullptr)
    {
        delete gOvenCavityOperationalStateDelegate;
        gOvenCavityOperationalStateDelegate = nullptr;
    }
}

void emberAfOvenCavityOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gOvenCavityOperationalStateInstance == nullptr && gOvenCavityOperationalStateDelegate == nullptr);

    gOvenCavityOperationalStateDelegate = new OvenCavityOperationalStateDelegate;
    EndpointId operationalStateEndpoint = 0x01;
    gOvenCavityOperationalStateInstance =
        new OvenCavityOperationalState::Instance(gOvenCavityOperationalStateDelegate, operationalStateEndpoint);

    gOvenCavityOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

    gOvenCavityOperationalStateInstance->Init();
}

CHIP_ERROR
OvenCavityOperationalStateDelegate::GetOperationalStateAtIndex(size_t index,
                                                               OperationalState::GenericOperationalState & operationalState)
{
    if (index >= mOperationalStateList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR
OvenCavityOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

void OvenCavityOperationalStateDelegate::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    OperationalState::OperationalStateEnum state =
        static_cast<OperationalState::OperationalStateEnum>(GetInstance()->GetCurrentOperationalState());

    if (state == OperationalState::OperationalStateEnum::kStopped || state == OperationalState::OperationalStateEnum::kError)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void OvenCavityOperationalStateDelegate::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{

    OperationalState::OperationalStateEnum state =
        static_cast<OperationalState::OperationalStateEnum>(GetInstance()->GetCurrentOperationalState());

    if (state == OperationalState::OperationalStateEnum::kStopped || state == OperationalState::OperationalStateEnum::kError)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }

    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void OvenCavityOperationalStateDelegate::HandleStartStateCallback(OperationalState::GenericOperationalError & err)
{
    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);

    if (current_err.errorStateID != to_underlying(OperationalState::ErrorStateEnum::kNoError))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume));
        return;
    }

    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void OvenCavityOperationalStateDelegate::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}
