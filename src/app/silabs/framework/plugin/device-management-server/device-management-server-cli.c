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
 * @brief CLI for the Device Management Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "device-management-server.h"
#include "app/framework/plugin/device-management-server/device-management-common.h"
#ifndef EMBER_AF_GENERATE_CLI
  #error The Device Management Server plugin is not compatible with the legacy CLI.
#endif

void emAfDeviceManagementServerCliIssuerEventId(void);
void emAfDeviceManagementServerCliPassword(void);
void emAfDeviceManagementServerCliPrint(void);
void emAfDeviceManagementServerCliProviderId(void);
void emAfDeviceManagementServerCliSiteId(void);
void emAfDeviceManagementServerCliSupplier(void);
void emAfDeviceManagementServerCliTariffType(void);
void emAfDeviceManagementServerCliTenancy(void);

void emAfDeviceManagementServerCliPrint(void)
{
  emberAfDeviceManagementServerPrint();
}

void emAfDeviceManagementServerCliTenancy(void)
{
  EmberAfDeviceManagementTenancy tenancy;
  tenancy.implementationDateTime = (uint32_t) emberUnsignedCommandArgument(0);
  tenancy.tenancy = (uint32_t) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetTenancy(&tenancy,
                                          false);
}

void emAfDeviceManagementServerCliProviderId(void)
{
  uint32_t providerId = (uint32_t) emberUnsignedCommandArgument(0);
  emberAfPluginDeviceManagementSetProviderId(providerId);
}

void emAfDeviceManagementServerCliIssuerEventId(void)
{
  uint32_t issuerEventId = (uint32_t) emberUnsignedCommandArgument(0);
  emberAfPluginDeviceManagementSetIssuerEventId(issuerEventId);
}

void emAfDeviceManagementServerCliTariffType(void)
{
  EmberAfTariffType tariffType = (EmberAfTariffType) emberUnsignedCommandArgument(0);
  emberAfPluginDeviceManagementSetTariffType(tariffType);
}

void emAfDeviceManagementServerCliSupplier(void)
{
  uint8_t length;
  EmberAfDeviceManagementSupplier supplier;

  uint8_t endpoint = (uint32_t) emberUnsignedCommandArgument(0);
  supplier.proposedProviderId = (uint32_t) emberUnsignedCommandArgument(1);
  supplier.implementationDateTime = (uint32_t) emberUnsignedCommandArgument(2);
  supplier.providerChangeControl = (uint32_t) emberUnsignedCommandArgument(3);
  length = emberCopyStringArgument(4,
                                   supplier.proposedProviderName + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_NAME_LENGTH,
                                   false);
  supplier.proposedProviderName[0] = length;
  length = emberCopyStringArgument(5,
                                   supplier.proposedProviderContactDetails + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROPOSED_PROVIDER_CONTACT_DETAILS_LENGTH,
                                   false);
  supplier.proposedProviderContactDetails[0] = length;
  emberAfPluginDeviceManagementSetSupplier(endpoint, &supplier);
}

void emAfDeviceManagementServerCliSiteId(void)
{
  uint8_t length;
  EmberAfDeviceManagementSiteId siteId;

  length = emberCopyStringArgument(0,
                                   siteId.siteId + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH,
                                   false);
  siteId.siteId[0] = length;
  siteId.implementationDateTime = (uint32_t) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetSiteId(&siteId);
}

void emAfDeviceManagementServerCliCIN(void)
{
  uint8_t length;
  EmberAfDeviceManagementCIN cin;

  length = emberCopyStringArgument(0,
                                   cin.cin + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_CIN_LENGTH,
                                   false);
  cin.cin[0] = length;
  cin.implementationDateTime = (uint32_t) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetCIN(&cin);
}

void emAfDeviceManagementServerCliPassword(void)
{
  uint8_t length;
  EmberAfDeviceManagementPassword password;

  length = emberCopyStringArgument(0,
                                   password.password + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH,
                                   false);
  password.password[0] = length;
  password.implementationDateTime = (uint32_t) emberUnsignedCommandArgument(1);
  password.durationInMinutes = (uint16_t) emberUnsignedCommandArgument(2);
  password.passwordType = (uint8_t) emberUnsignedCommandArgument(3);

  emberAfPluginDeviceManagementSetPassword(&password);
}

// plugin device-management-server pub-chg-of-tenancy <dst:2> <src endpoint:1>  <dst endpoint:1>
void emAfDeviceManagementServerCliPublishChangeOfTenancy(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  emberAfDeviceManagementClusterPublishChangeOfTenancy(dstAddr, srcEndpoint, dstEndpoint);
}

// plugin device-management-server pub-chg-of-tenancy <dst:2> <src endpoint:1>  <dst endpoint:1>
void emAfDeviceManagementServerCliPublishChangeOfSupplier(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  emberAfDeviceManagementClusterPublishChangeOfSupplier(dstAddr, srcEndpoint, dstEndpoint);
}

// plugin device-management-server update-site-id <dst:2> <src endpoint:1>  <dst endpoint:1>
void emAfDeviceManagementServerCliUpdateSiteId(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  emberAfDeviceManagementClusterUpdateSiteId(dstAddr, srcEndpoint, dstEndpoint);
}

// plugin device-management-server update-cin <dst:2> <src endpoint:1>  <dst endpoint:1>
void emAfDeviceManagementServerCliUpdateCIN(void)
{
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  emberAfDeviceManagementClusterUpdateCIN(dstAddr, srcEndpoint, dstEndpoint);
}

// plugin device-management-server pendingUpdatesMask <pendingUpdatesMask:1>
void emAfDeviceManagementServerCliPendingUpdates(void)
{
  EmberAfDeviceManagementChangePendingFlags pendingUpdatesMask = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfDeviceManagementClusterSetPendingUpdates(pendingUpdatesMask);
}

void emAfDeviceManagementServerCliSendRequestNewPasswordResponse(void)
{
  uint8_t passwordType = (uint8_t)emberUnsignedCommandArgument(0);
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(1);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(3);
  emberAfDeviceManagementClusterSendRequestNewPasswordResponse(passwordType,
                                                               dstAddr,
                                                               srcEndpoint,
                                                               dstEndpoint);
}
