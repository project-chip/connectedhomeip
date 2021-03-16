/*
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

#include <support/logging/CHIPLogging.h>

#include "AppConfig.h"
#include "BoltLockManager.h"

#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/enums.h"
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
        return;
    }

    if (*value)
    {
        BoltLockMgr().InitiateAction(0, BoltLockManager::LOCK_ACTION);
    }
    else
    {
        BoltLockMgr().InitiateAction(0, BoltLockManager::UNLOCK_ACTION);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}


bool emberAfGetEndpointInfoCallback(chip::EndpointId endpoint, uint8_t * returnNetworkIndex, EmberAfEndpointInfoStruct * returnEndpointInfo)
{
    return false;
}

bool emberAfPreMessageSendCallback(EmberAfMessageStruct * messageStruct, EmberStatus * status)
{
    return false;
}

void emberAfAddToCurrentAppTasksCallback(EmberAfApplicationTask tasks)
{

}

bool emberAfAttributeReadAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                        chip::AttributeId attributeId)
{
    return false;
}                                        

EmberAfStatus emberAfExternalAttributeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer)
{
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

bool emberAfAttributeWriteAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                         chip::AttributeId attributeId)
{
    return false;
}

EmberAfStatus emberAfExternalAttributeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength)
{
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}                                                   

void emberAfGroupsClusterServerInitCallback(chip::EndpointId endpoint)
{

}

void emberAfScenesClusterServerInitCallback(chip::EndpointId endpoint)
{

}

void emberAfIdentifyClusterServerInitCallback(chip::EndpointId endpoint)
{

}

void emberAfIdentifyClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId)
{

}

EmberAfStatus emberAfPreAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                                uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                uint8_t * value)
{
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}                                                

EmberAfAttributeWritePermission emberAfAllowNetworkWriteAttributeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                          chip::AttributeId attributeId, uint8_t mask,
                                                                          uint16_t manufacturerCode, uint8_t * value, uint8_t type)

{
    return EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_READ_ONLY;
}                                                                          

bool emberAfReadAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    return false;
}

bool emberAfWriteAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    return false;
}

bool emberAfConfigureReportingResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    return false;
}

bool emberAfReadReportingConfigurationResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    return false;
}

bool emberAfReportAttributesCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    return false;
}

bool emberAfDefaultResponseCallback(chip::ClusterId clusterId, chip::CommandId commandId, EmberAfStatus status)
{
    return false;
}


bool emberAfDiscoverAttributesResponseCallback(chip::ClusterId clusterId, bool discoveryComplete, uint8_t * buffer,
                                               uint16_t bufLen, bool extended)
{
    return false;
}                                               

bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand * cmd)
{
    return false;
}

EmberStatus emberAfInterpanSendMessageCallback(EmberAfInterpanHeader * header, uint16_t messageLength, uint8_t * message)
{
    return 0;
}

uint8_t emberAfGetSourceRouteOverheadCallback(chip::NodeId destination)
{
    return 0;
}

bool emberAfGroupsClusterAddGroupCallback(uint16_t groupId, uint8_t * groupName)
{
    return false;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(uint16_t groupId, uint8_t * groupName)
{
    return false;
}

bool emberAfGroupsClusterGetGroupMembershipCallback(uint8_t groupCount, /* TYPE WARNING: array array defaults to */ uint8_t *  groupList)
{
    return false;
}

bool emberAfGroupsClusterRemoveAllGroupsCallback()
{
    return false;
}

bool emberAfGroupsClusterRemoveGroupCallback(uint16_t groupId)
{
    return false;
}

bool emberAfGroupsClusterViewGroupCallback(uint16_t groupId)
{
    return false;
}

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
    return false;
}

bool emberAfIdentifyClusterIdentifyQueryCallback()
{
    return false;
}

bool emberAfIdentifyClusterIdentifyCallback(uint16_t identifyTime)
{
    return false;
}

bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName, /* TYPE WARNING: array array defaults to */ uint8_t *  extensionFieldSets)
{
    return false;
}

bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId)
{
    return false;
}

bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime)
{
    return false;
}

bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId)
{
    return false;
}

bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    return false;
}

bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    return false;
}

bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    return false;
}

bool emberAfScenesClusterMakeInvalidCallback(unsigned char)
{
    return false;
}

bool emberAfGroupsClusterEndpointInGroupCallback(unsigned char, unsigned short)
{
    return false;
}