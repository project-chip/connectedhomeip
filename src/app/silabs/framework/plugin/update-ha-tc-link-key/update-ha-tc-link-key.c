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
 * @brief Routines for the Update HA TC Link Key plugin, which provides a way
 *        for trust centers to preemptively update the link key for HA devices.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"

#define R21_STACK_REVISION 21

void emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(const EmberAfDeviceInfo *info)
{
  if (info->stackRevision < R21_STACK_REVISION) {
    EmberNodeId nodeId = emberLookupNodeIdByEui64(((EmberAfDeviceInfo *)info)->eui64);
    EmberStatus status = (nodeId == EMBER_NULL_NODE_ID
                          ? EMBER_ERR_FATAL
                          : emberSendTrustCenterLinkKey(nodeId, ((EmberAfDeviceInfo *)info)->eui64));
    emberAfCorePrint("Updating trust center link key of joining device ");
    emberAfPrintBigEndianEui64(info->eui64);
    emberAfCorePrintln(": 0x%X", status);
  }
}
