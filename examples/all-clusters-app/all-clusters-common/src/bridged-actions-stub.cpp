/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

namespace {
std::unique_ptr<Clusters::Actions::ActionsDelegateImpl> sActionsDelegateImpl;
std::unique_ptr<Clusters::Actions::ActionsServer> sActionsServer;
} // namespace

CHIP_ERROR ActionsDelegateImpl::ReadActionAtIndex(uint16_t index, ActionStructStorage & action)
{
    if (index >= kActionList.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    action = kActionList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsDelegateImpl::ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList)
{
    if (index >= kEndpointList.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    epList = kEndpointList[index];
    return CHIP_NO_ERROR;
}

bool ActionsDelegateImpl::HaveActionWithId(uint16_t actionId, uint16_t & actionIndex)
{
    for (size_t i = 0; i < kEndpointList.size(); i++)
    {
        if (kActionList[i].actionID == actionId)
        {
            actionIndex = (uint16_t) i;
            return true;
        }
    }
    return false;
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

void emberAfActionsClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(endpoint == 1,
                   ChipLogError(Zcl, "Actions cluster delegate is not implemented for endpoint with id %d.", endpoint));
    VerifyOrReturn(emberAfContainsServer(endpoint, Actions::Id) == true,
                   ChipLogError(Zcl, "Endpoint %d does not support Actions cluster.", endpoint));
    VerifyOrReturn(!sActionsDelegateImpl && !sActionsServer);

    sActionsDelegateImpl = std::make_unique<Actions::ActionsDelegateImpl>();
    sActionsServer       = std::make_unique<Actions::ActionsServer>(endpoint, *sActionsDelegateImpl.get());

    sActionsServer->Init();
}
