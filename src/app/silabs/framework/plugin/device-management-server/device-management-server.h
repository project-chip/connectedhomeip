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
/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Management Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/plugin/device-management-server/device-management-common.h"

bool emberAfPluginDeviceManagementSetTenancy(EmberAfDeviceManagementTenancy *tenancy,
                                             bool validateOptionalFields);
bool emberAfPluginDeviceManagementGetTenancy(EmberAfDeviceManagementTenancy *tenancy);

bool emberAfPluginDeviceManagementSetSupplier(uint8_t endpoint, EmberAfDeviceManagementSupplier *supplier);
bool emberAfPluginDeviceManagementGetSupplier(EmberAfDeviceManagementSupplier *supplier);

bool emberAfPluginDeviceManagementSetInfoGlobalData(uint32_t providerId,
                                                    uint32_t issuerEventId,
                                                    uint8_t tariffType);

bool emberAfPluginDeviceManagementSetSiteId(EmberAfDeviceManagementSiteId *siteId);
bool emberAfPluginDeviceManagementGetSiteId(EmberAfDeviceManagementSiteId *siteId);

bool emberAfPluginDeviceManagementSetCIN(EmberAfDeviceManagementCIN *cin);
bool emberAfPluginDeviceManagementGetCIN(EmberAfDeviceManagementCIN *cin);

bool emberAfPluginDeviceManagementSetPassword(EmberAfDeviceManagementPassword *password);
bool emberAfPluginDeviceManagementGetPassword(EmberAfDeviceManagementPassword *password,
                                              uint8_t passwordType);

void emberAfDeviceManagementServerPrint(void);

bool emberAfDeviceManagementClusterUpdateSiteId(EmberNodeId dstAddr,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint);

bool emberAfPluginDeviceManagementSetProviderId(uint32_t providerId);
bool emberAfPluginDeviceManagementSetIssuerEventId(uint32_t issuerEventId);
bool emberAfPluginDeviceManagementSetTariffType(EmberAfTariffType tariffType);

bool emberAfDeviceManagementClusterPublishChangeOfTenancy(EmberNodeId dstAddr,
                                                          uint8_t srcEndpoint,
                                                          uint8_t dstEndpoint);
bool emberAfDeviceManagementClusterPublishChangeOfSupplier(EmberNodeId dstAddr,
                                                           uint8_t srcEndpoint,
                                                           uint8_t dstEndpoint);

void emberAfDeviceManagementClusterSetPendingUpdates(EmberAfDeviceManagementChangePendingFlags pendingUpdatesMask);
void emberAfDeviceManagementClusterGetPendingUpdates(EmberAfDeviceManagementChangePendingFlags *pendingUpdatesMask);
bool emberAfDeviceManagementClusterUpdateCIN(EmberNodeId dstAddr,
                                             uint8_t srcEndpoint,
                                             uint8_t dstEndpoint);

bool emberAfDeviceManagementClusterSendRequestNewPasswordResponse(uint8_t passwordType,
                                                                  EmberNodeId dstAddr,
                                                                  uint8_t srcEndpoint,
                                                                  uint8_t dstEndpoint);
