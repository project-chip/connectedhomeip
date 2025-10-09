/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/clusters/actions-server/actions-server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <vector>

#include "Device.h"
#include "main.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

class LinuxActionsDelegateImpl : public Actions::Delegate
{
public:
    LinuxActionsDelegateImpl() : mEndpointId(1) {}

    void SetEndpointId(chip::EndpointId endpointId) { mEndpointId = endpointId; }
    chip::EndpointId GetEndpointId() const { return mEndpointId; }

    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override;
    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override;
    bool HaveActionWithId(uint16_t actionId, uint16_t & actionIndex) override;

    Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId) override;
    Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;
    Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override;
    Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override;

private:
    chip::EndpointId mEndpointId;
};

LinuxActionsDelegateImpl gLinuxActionsDelegateImpl;
std::unique_ptr<Actions::ActionsServer> sActionsServer;

CHIP_ERROR LinuxActionsDelegateImpl::ReadActionAtIndex(uint16_t index, ActionStructStorage & action)
{
    std::vector<Action *> actionList = GetActionListInfo(mEndpointId);

    uint16_t visibleCount = 0;
    for (auto actionPtr : actionList)
    {
        if (actionPtr->getIsVisible())
        {
            if (visibleCount == index)
            {
                // Found the visible action at the requested index
                action.Set(actionPtr->getActionId(), CharSpan::fromCharString(actionPtr->getName().c_str()), actionPtr->getType(),
                           actionPtr->getEndpointListId(), actionPtr->getSupportedCommands(), actionPtr->getStatus());
                return CHIP_NO_ERROR;
            }
            visibleCount++;
        }
    }

    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR LinuxActionsDelegateImpl::ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList)
{
    std::vector<EndpointListInfo> infoList = GetEndpointListInfo(mEndpointId);

    if (index >= infoList.size())
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    // Get the endpoint list at the specified index
    EndpointListInfo info = infoList[index];
    DataModel::List<const EndpointId> endpointList(info.GetEndpointListData(), info.GetEndpointListSize());

    epList.Set(info.GetEndpointListId(), CharSpan::fromCharString(info.GetName().c_str()), info.GetType(), endpointList);

    return CHIP_NO_ERROR;
}

bool LinuxActionsDelegateImpl::HaveActionWithId(uint16_t actionId, uint16_t & actionIndex)
{
    std::vector<Action *> actionList = GetActionListInfo(mEndpointId);

    uint16_t visibleCount = 0;
    for (size_t i = 0; i < actionList.size(); i++)
    {
        if (actionList[i]->getIsVisible())
        {
            if (actionList[i]->getActionId() == actionId)
            {
                actionIndex = visibleCount;
                return true;
            }
            visibleCount++;
        }
    }

    return false;
}

Status LinuxActionsDelegateImpl::HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    EndpointId endpointID  = mEndpointId;
    bool hasInvokeID       = invokeId.HasValue();
    uint32_t invokeIDValue = invokeId.ValueOr(0);

    Action * targetAction            = nullptr;
    std::vector<Action *> actionList = GetActionListInfo(mEndpointId);
    for (auto * action : actionList)
    {
        if (action->getActionId() == actionId && action->getIsVisible())
        {
            targetAction = action;
            break;
        }
    }

    if (targetAction)
    {
        // Determine the room and if it's an "on" action based on action ID
        Room * targetRoom = nullptr;
        bool turnOn       = true;

        // Find room with matching endpoint list ID
        std::vector<Room *> roomList = GetRoomListInfo(mEndpointId);
        for (auto * room : roomList)
        {
            if (room->getEndpointListId() == targetAction->getEndpointListId())
            {
                targetRoom = room;
                break;
            }
        }

        // Determine if this is an "on" or "off" action based on name
        if (targetAction->getName().find("Off") != std::string::npos)
        {
            turnOn = false;
        }

        if (targetRoom)
        {
            runOnOffRoomAction(targetRoom, turnOn, endpointID, actionId, invokeIDValue, hasInvokeID);
            return Status::Success;
        }
    }

    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime,
                                                                   Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

Status LinuxActionsDelegateImpl::HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId)
{
    // Not implemented
    return Status::NotFound;
}

} // anonymous namespace

void emberAfActionsClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(endpoint == 1,
                   ChipLogError(Zcl, "Actions cluster delegate is not implemented for endpoint with id %d.", endpoint));
    VerifyOrReturn(emberAfContainsServer(endpoint, Actions::Id) == true,
                   ChipLogError(Zcl, "Endpoint %d does not support Actions cluster.", endpoint));

    VerifyOrReturn(!sActionsServer);

    gLinuxActionsDelegateImpl.SetEndpointId(endpoint);
    sActionsServer = std::make_unique<Actions::ActionsServer>(endpoint, gLinuxActionsDelegateImpl);
    sActionsServer->Init();
}
