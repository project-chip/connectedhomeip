/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Groups Client plugin, the
 *client implementation of the Groups cluster.
 *******************************************************************************
 ******************************************************************************/

// *******************************************************************
// * groups-client.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

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
    emberAfGroupsClusterPrintln("");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfGroupsClusterRemoveGroupResponseCallback(app::CommandHandler * commandObj, uint8_t status, GroupId groupId)
{
    emberAfGroupsClusterPrintln("RX: RemoveGroupResponse 0x%x, 0x%2x", status, groupId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
