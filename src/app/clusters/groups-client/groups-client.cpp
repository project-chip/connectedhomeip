/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

bool emberAfGroupsClusterAddGroupResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId)
{
    emberAfGroupsClusterPrintln("RX: AddGroupResponse 0x%x, 0x%2x", status, groupId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfGroupsClusterViewGroupResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId,
                                                   uint8_t * groupName)
{
    emberAfGroupsClusterPrint("RX: ViewGroupResponse 0x%x, 0x%2x, \"", status, groupId);
    emberAfGroupsClusterPrintString(groupName);
    emberAfGroupsClusterPrintln("\"");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfGroupsClusterGetGroupMembershipResponseCallback(app::CommandHandler * commandObj, uint8_t capacity, uint8_t groupCount,
                                                            uint8_t * groupList)
{
    uint8_t i;
    emberAfGroupsClusterPrint("RX: GetGroupMembershipResponse 0x%x, 0x%x,", capacity, groupCount);
    for (i = 0; i < groupCount; i++)
    {
        emberAfGroupsClusterPrint(" [0x%2x]", emberAfGetInt16u(groupList + (i << 1), 0, 2));
    }
    emberAfGroupsClusterPrintln("%s", "");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfGroupsClusterRemoveGroupResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId)
{
    emberAfGroupsClusterPrintln("RX: RemoveGroupResponse 0x%x, 0x%2x", status, groupId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
