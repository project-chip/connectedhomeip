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
#include <rvc-operational-state-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcOperationalState;

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalStateAtIndex(size_t index,
                                                                   OperationalState::GenericOperationalState & operationalState)
{
    if (index >= MATTER_ARRAY_SIZE(mOperationalStateList))
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

void RvcOperationalStateDelegate::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    (mPauseRvcDeviceInstance->*mPauseCallback)(err);
}

void RvcOperationalStateDelegate::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    (mResumeRvcDeviceInstance->*mResumeCallback)(err);
}

void RvcOperationalStateDelegate::HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err)
{
    (mGoHomeRvcDeviceInstance->*mGoHomeCallback)(err);
}
