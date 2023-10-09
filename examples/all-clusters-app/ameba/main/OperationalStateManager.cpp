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
#include <OperationalStateManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::RvcOperationalState;

CHIP_ERROR GenericOperationalStateDelegateImpl::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    if (index >= mOperationalStateList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOperationalStateDelegateImpl::GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalPhase = mOperationalPhaseList[index];
    return CHIP_NO_ERROR;
}

void GenericOperationalStateDelegateImpl::HandlePauseStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void GenericOperationalStateDelegateImpl::HandleResumeStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void GenericOperationalStateDelegateImpl::HandleStartStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void GenericOperationalStateDelegateImpl::HandleStopStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

// Init Operational State cluster

static OperationalState::Instance * gOperationalStateInstance = nullptr;
static OperationalStateDelegate * gOperationalStateDelegate   = nullptr;

void OperationalState::Shutdown()
{
    if (gOperationalStateInstance != nullptr)
    {
        delete gOperationalStateInstance;
        gOperationalStateInstance = nullptr;
    }
    if (gOperationalStateDelegate != nullptr)
    {
        delete gOperationalStateDelegate;
        gOperationalStateDelegate = nullptr;
    }
}

OperationalState::Instance * OperationalState::GetOperationalStateInstance()
{
    return gOperationalStateInstance;
}

void emberAfOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gOperationalStateInstance == nullptr && gOperationalStateDelegate == nullptr);

    gOperationalStateDelegate           = new OperationalStateDelegate;
    EndpointId operationalStateEndpoint = 0x01;
    gOperationalStateInstance = new Instance(gOperationalStateDelegate, operationalStateEndpoint, Clusters::OperationalState::Id);

    gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

    gOperationalStateInstance->Init();
}

// Init RVC Operational State cluster

static OperationalState::Instance * gRvcOperationalStateInstance  = nullptr;
static RvcOperationalStateDelegate * gRvcOperationalStateDelegate = nullptr;

OperationalState::Instance * OperationalState::GetRVCOperationalStateInstance()
{
    return gRvcOperationalStateInstance;
}

void RvcOperationalState::Shutdown()
{
    if (gRvcOperationalStateInstance != nullptr)
    {
        delete gRvcOperationalStateInstance;
        gRvcOperationalStateInstance = nullptr;
    }
    if (gRvcOperationalStateDelegate != nullptr)
    {
        delete gRvcOperationalStateDelegate;
        gRvcOperationalStateDelegate = nullptr;
    }
}

void emberAfRvcOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcOperationalStateInstance == nullptr && gRvcOperationalStateDelegate == nullptr);

    gRvcOperationalStateDelegate        = new RvcOperationalStateDelegate;
    EndpointId operationalStateEndpoint = 0x01;
    gRvcOperationalStateInstance =
        new Instance(gRvcOperationalStateDelegate, operationalStateEndpoint, Clusters::RvcOperationalState::Id);

    gRvcOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

    gRvcOperationalStateInstance->Init();
}
