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
#include "operational-state-delegate.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using OperationalStateEnum = chip::app::Clusters::OperationalState::OperationalStateEnum;

// Operational State command callbacks
CHIP_ERROR OperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    if (index > mOperationalStateList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase)
{
    if (index > mOperationalPhaseList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalPhase = mOperationalPhaseList[index];
    return CHIP_NO_ERROR;
}

void OperationalStateDelegate::SetOpStatePauseCallback(
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback)
{
    mPauseCallback = Platform::MakeUnique<std::function<void(Clusters::OperationalState::GenericOperationalError & err)>>(aCallback);
}

void OperationalStateDelegate::SetOpStateResumeCallback(
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback)
{
    mResumeCallback = Platform::MakeUnique<std::function<void(Clusters::OperationalState::GenericOperationalError & err)>>(aCallback);
}

void OperationalStateDelegate::SetOpStateStartCallback(
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback)
{
    mStartCallback = Platform::MakeUnique<std::function<void(Clusters::OperationalState::GenericOperationalError & err)>>(aCallback);
}

void OperationalStateDelegate::SetOpStateStopCallback(
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback)
{
    mStopCallback = Platform::MakeUnique<std::function<void(Clusters::OperationalState::GenericOperationalError & err)>>(aCallback);
}

void OperationalStateDelegate::SetOpStateGetCountdownTimeCallback(std::function<app::DataModel::Nullable<uint32_t>(void)> aCallback)
{
    mGetCountdownTimeCallback = Platform::MakeUnique<std::function<app::DataModel::Nullable<uint32_t>(void)>>(aCallback);
}

app::DataModel::Nullable<uint32_t> OperationalStateDelegate::OperationalStateDelegate::GetCountdownTime()
{
    return (*mGetCountdownTimeCallback.get())();
}

void OperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    (*mPauseCallback.get())(err);
}

void OperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    (*mResumeCallback.get())(err);
}

void OperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    (*mStartCallback.get())(err);
}

void OperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    (*mStopCallback.get())(err);
}
