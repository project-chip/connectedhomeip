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
 * @brief Routines for the Door Lock Server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "door-lock-server.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/af.h>

static void setActuatorEnable(void)
{
    // The Door Lock cluster test spec expects this attribute set to be true by
    // default...
    bool troo = true;
    EmberAfStatus status =
        emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_ACTUATOR_ENABLED_ATTRIBUTE_ID,
                                    (uint8_t *) &troo, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to write ActuatorEnabled attribute: 0x%X", status);
    }
}

static void setDoorState(void)
{
    uint8_t state        = EMBER_ZCL_DOOR_STATE_ERROR_UNSPECIFIED;
    EmberAfStatus status = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                       ZCL_DOOR_STATE_ATTRIBUTE_ID, (uint8_t *) &state, ZCL_ENUM8_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to write DoorState attribute: 0x%X", status);
    }
}

static void setLanguage(void)
{
    uint8_t englishString[] = { 0x02, 'e', 'n' };
    EmberAfStatus status    = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID,
                                                       ZCL_LANGUAGE_ATTRIBUTE_ID, englishString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfDoorLockClusterPrintln("Failed to write Language attribute: 0x%X", status);
    }
}

void emberAfPluginDoorLockServerInitCallback(void)
{
    emAfPluginDoorLockServerInitUser();
    emAfPluginDoorLockServerInitSchedule();

    setActuatorEnable();
    setDoorState();
    setLanguage();
}

void emAfPluginDoorLockServerWriteAttributes(const EmAfPluginDoorLockServerAttributeData * data, uint8_t dataLength,
                                             const char * description)
{
    for (uint8_t i = 0; i < dataLength; i++)
    {
        EmberAfStatus status = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, data[i].id,
                                                           (uint8_t *) &data[i].value, ZCL_INT16U_ATTRIBUTE_TYPE);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfDoorLockClusterPrintln("Failed to write %s attribute 0x%2X: 0x%X", data[i].id, status, description);
        }
    }
}

EmberAfStatus emAfPluginDoorLockServerNoteDoorStateChanged(EmberAfDoorState state)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_STATE_ATTRIBUTE
    status = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, ZCL_DOOR_STATE_ATTRIBUTE_ID,
                                         (uint8_t *) &state, ZCL_ENUM8_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
#endif

#if defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_OPEN_EVENTS_ATTRIBUTE) ||                                                             \
    defined(ZCL_USING_DOOR_LOCK_CLUSTER_DOOR_CLOSED_EVENTS_ATTRIBUTE)
    if (state == EMBER_ZCL_DOOR_STATE_OPEN || state == EMBER_ZCL_DOOR_STATE_CLOSED)
    {
        AttributeId attributeId =
            (state == EMBER_ZCL_DOOR_STATE_OPEN ? ZCL_DOOR_OPEN_EVENTS_ATTRIBUTE_ID : ZCL_DOOR_CLOSED_EVENTS_ATTRIBUTE_ID);
        uint32_t events;
        status = emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, attributeId, (uint8_t *) &events,
                                            sizeof(events));
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            events++;
            status = emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT, ZCL_DOOR_LOCK_CLUSTER_ID, attributeId,
                                                 (uint8_t *) &events, ZCL_INT32U_ATTRIBUTE_TYPE);
        }
    }
#endif

    return status;
}
