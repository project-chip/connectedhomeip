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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "af.h"

/** @brief Identify Cluster Identify Query Response
 *
 *
 *
 * @param timeout   Ver.: always
 */
bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
    return false;
}

/** @brief IAS Zone Cluster Zone Enroll Request
 *
 *
 *
 * @param zoneType   Ver.: always
 * @param manufacturerCode   Ver.: always
 */
bool emberAfIasZoneClusterZoneEnrollRequestCallback(uint16_t zoneType, uint16_t manufacturerCode)
{
    return false;
}

/** @brief IAS Zone Cluster Zone Status Change Notification
 *
 *
 *
 * @param zoneStatus   Ver.: always
 * @param extendedStatus   Ver.: always
 * @param zoneId   Ver.: since ha-1.2-05-3520-29
 * @param delay   Ver.: since ha-1.2-05-3520-29
 */
bool emberAfIasZoneClusterZoneStatusChangeNotificationCallback(uint16_t zoneStatus, uint8_t extendedStatus, uint8_t zoneId,
                                                               uint16_t delay)
{
    return false;
}

// endpoint_config.h callbacks, grep'd from SDK, comment these out as clusters come in

void emberAfIasZoneClusterClientInitCallback(uint8_t endpoint) {}

void emberAfPollControlClusterServerAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId) {}
void emberAfPollControlClusterServerInitCallback(uint8_t endpoint) {}
void emberAfPluginPollControlServerStackStatusCallback(EmberStatus status) {}
EmberAfStatus emberAfPollControlClusterServerPreAttributeChangedCallback(uint8_t endpoint, EmberAfAttributeId attributeId,
                                                                         EmberAfAttributeType attributeType, uint8_t size,
                                                                         uint8_t * value)
{
    return EMBER_ZCL_STATUS_SUCCESS;
}

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message, uint16_t length) {}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen) {}
