/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *BoltLockManager
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <support/logging/CHIPLogging.h>

#include "AppConfig.h"
#include "PumpManager.h"

#include <app/chip-zcl-zpro-codec.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/enums-id.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    ChipLogProgress(Zcl, "%s: %d %" PRIx32 " %" PRIx32 " %d", __FUNCTION__, endpoint, clusterId, attributeId, *value);
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %" PRIx32, clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
        return;
    }

    if (*value)
    {
        PumpMgr().InitiateAction(0, PumpManager::LOCK_ACTION);
    }
    else
    {
        PumpMgr().InitiateAction(0, PumpManager::UNLOCK_ACTION);
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
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
}

bool emberAfGetEndpointInfoCallback(chip::EndpointId endpoint, uint8_t * returnNetworkIndex,
                                    EmberAfEndpointInfoStruct * returnEndpointInfo)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
    return false;
}

bool emberAfPreMessageSendCallback(EmberAfMessageStruct * messageStruct, EmberStatus * status)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, status);
    return false;
}

void emberAfAddToCurrentAppTasksCallback(EmberAfApplicationTask tasks) {}

bool emberAfAttributeReadAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                        chip::AttributeId attributeId)
{
    ChipLogProgress(Zcl, "%s: %d %" PRIx32 " %" PRIx32 " %d", __FUNCTION__, endpoint, clusterId, manufacturerCode, attributeId);
    return true;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

bool emberAfAttributeWriteAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                         chip::AttributeId attributeId)
{
    ChipLogProgress(Zcl, "%s: %d %" PRIx32 " %" PRIx32 " %d", __FUNCTION__, endpoint, clusterId, manufacturerCode, attributeId);
    return true;
}

EmberAfStatus emberAfExternalAttributeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

void emberAfGroupsClusterServerInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
}

void emberAfScenesClusterServerInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
}

void emberAfIdentifyClusterServerInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
}

void emberAfIdentifyClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
}

EmberAfStatus emberAfPreAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                                uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                uint8_t * value)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
    return EMBER_ZCL_STATUS_SUCCESS; // EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

EmberAfAttributeWritePermission emberAfAllowNetworkWriteAttributeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                          chip::AttributeId attributeId, uint8_t mask,
                                                                          uint16_t manufacturerCode, uint8_t * value, uint8_t type)

{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, endpoint);
    return EMBER_ZCL_ATTRIBUTE_WRITE_PERMISSION_READ_ONLY;
}

bool emberAfReadAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfWriteAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfConfigureReportingResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfReadReportingConfigurationResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfReportAttributesCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfDefaultResponseCallback(chip::ClusterId clusterId, chip::CommandId commandId, EmberAfStatus status)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfDiscoverAttributesResponseCallback(chip::ClusterId clusterId, bool discoveryComplete, uint8_t * buffer, uint16_t bufLen,
                                               bool extended)
{
    ChipLogProgress(Zcl, "%s: %" PRIx32, __FUNCTION__, clusterId);
    return false;
}

bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand * cmd)
{
    return false;
}

EmberStatus emberAfInterpanSendMessageCallback(EmberAfInterpanHeader * header, uint16_t messageLength, uint8_t * message)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, 0);
    return 0;
}

uint8_t emberAfGetSourceRouteOverheadCallback(chip::NodeId destination)
{
    return 0;
}

bool emberAfGroupsClusterAddGroupCallback(uint16_t groupId, uint8_t * groupName)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(uint16_t groupId, uint8_t * groupName)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfGroupsClusterGetGroupMembershipCallback(uint8_t groupCount,
                                                    /* TYPE WARNING: array array defaults to */ uint8_t * groupList)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupCount);
    return false;
}

bool emberAfGroupsClusterRemoveAllGroupsCallback()
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, 0);
    return false;
}

bool emberAfGroupsClusterRemoveGroupCallback(uint16_t groupId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfGroupsClusterViewGroupCallback(uint16_t groupId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, timeout);
    return false;
}

bool emberAfIdentifyClusterIdentifyQueryCallback()
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, 0);
    return false;
}

bool emberAfIdentifyClusterIdentifyCallback(uint16_t identifyTime)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, identifyTime);
    return false;
}

bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                          /* TYPE WARNING: array array defaults to */ uint8_t * extensionFieldSets)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, groupId);
    return false;
}

bool emberAfScenesClusterMakeInvalidCallback(unsigned char par)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, par);
    return false;
}

bool emberAfGroupsClusterEndpointInGroupCallback(unsigned char par, unsigned short)
{
    ChipLogProgress(Zcl, "%s: %d", __FUNCTION__, par);
    return false;
}
