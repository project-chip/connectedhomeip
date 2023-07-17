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
#include <operational-state-delegate-impl.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

using chip::Protocols::InteractionModel::Status;

CHIP_ERROR OperationalStateDelegate::SetOperationalState(uint8_t opState)
{
    mOperationalState = opState;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateDelegate::SetPhase(const app::DataModel::Nullable<uint8_t> & phase)
{
    mOperationalPhase = phase;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateDelegate::SetCountdownTime(const app::DataModel::Nullable<uint32_t> & time)
{
    mCountdownTime = time;
    return CHIP_NO_ERROR;
}

uint8_t OperationalStateDelegate::GetCurrentOperationalState()
{
    return mOperationalState;
}

CHIP_ERROR OperationalStateDelegate::SetOperationalError(const GenericOperationalError & opErrState)
{
    mOperationalError = opErrState;
    return CHIP_NO_ERROR;
}

void OperationalStateDelegate::GetCurrentOperationalError(GenericOperationalError & error)
{
    error = mOperationalError;
}

void OperationalStateDelegate::GetCurrentPhase(app::DataModel::Nullable<uint8_t> & phase)
{
    phase = mOperationalPhase;
}

void OperationalStateDelegate::GetCountdownTime(app::DataModel::Nullable<uint32_t> & time)
{
    time = mCountdownTime;
}

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

void OperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    mOperationalState = to_underlying(OperationalStateEnum::kPaused);
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void OperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    mOperationalState = to_underlying(OperationalStateEnum::kRunning);
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void OperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    mOperationalState = to_underlying(OperationalStateEnum::kRunning);
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

void OperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    mOperationalState = to_underlying(OperationalStateEnum::kStopped);
    err.Set(to_underlying(ErrorStateEnum::kNoError));
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
