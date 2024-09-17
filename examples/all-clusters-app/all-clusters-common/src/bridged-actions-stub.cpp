/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <bridged-actions-stub.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

CHIP_ERROR ActionsDelegateImpl::ReadActionAtIndex(uint16_t index, ActionListStructType & action)
{
    if (index >= ArraySize(kActionList))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    action.actionID          = kActionList[index].actionID;
    action.name              = kActionList[index].name;
    action.type              = kActionList[index].type;
    action.endpointListID    = kActionList[index].endpointListID;
    action.supportedCommands = kActionList[index].supportedCommands;
    action.state             = kActionList[index].state;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsDelegateImpl::ReadEndpointListAtIndex(uint16_t index, EndpointListStructType & epList)
{
    if (index >= ArraySize(kEndpointList))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    epList.endpointListID = kEndpointList[index].endpointListID;
    epList.name           = kEndpointList[index].name;
    epList.type           = kEndpointList[index].type;
    epList.endpoints      = kEndpointList[index].endpoints;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsDelegateImpl::FindActionIdInActionList(uint16_t actionId)
{
    for (uint16_t i = 0; i < kActionListSize; i++)
    {
        if (kActionList[i].actionID == actionId)
            return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

Status ActionsDelegateImpl::HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                              Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status ActionsDelegateImpl::HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}
