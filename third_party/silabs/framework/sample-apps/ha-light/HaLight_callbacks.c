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
//
// This file is generated. Please do not edit manually.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"

#include "app/framework/util/af-main.h"

// A short press of the button will do client commissioning while a long press
// will do server commissioning.
#define SHORT_PRESS_DURATION_MS MILLISECOND_TICKS_PER_QUARTERSECOND

// We only support EZ-Mode Commissioning for the On/Off cluster.
static uint16_t clusterIds[] = { ZCL_ON_OFF_CLUSTER_ID };

/** @brief Server Attribute Changed
 *
 * On/off cluster, server-side attribute changed.
 *
 * @param endpoint Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    bool onOff;
    EmberAfStatus status = emberAfReadAttribute(endpoint,
                                                ZCL_ON_OFF_CLUSTER_ID,
                                                ZCL_ON_OFF_ATTRIBUTE_ID,
                                                CLUSTER_MASK_SERVER,
                                                (uint8_t *)&onOff,
                                                sizeof(onOff),
                                                NULL); // data type
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfCorePrintln("ERR: reading on/off %x", status);
    } else {
      emberAfCorePrintln("Light on 0x%x is now %p",
                         endpoint,
                         onOff ? "ON" : "OFF");
      emberAfCoreFlush();
      if ( onOff ) {
        halSetLed(BOARDLED1);
      } else {
        halClearLed(BOARDLED1);
      }
    }
  }
}

/** @brief Client Complete
 *
 * This function is called by the EZ-Mode Commissioning plugin when client
 * commissioning completes.
 *
 * @param bindingIndex The binding index that was created or
 * ::EMBER_NULL_BINDING if an error occurred.  Ver.: always
 */
void emberAfPluginEzmodeCommissioningClientCompleteCallback(uint8_t bindingIndex)
{
}

/** @brief Configured
 *
 * This callback is called by the Reporting plugin whenever a reporting entry
 * is configured, including when entries are deleted or updated.  The
 * application can use this callback for scheduling readings or measurements
 * based on the minimum and maximum reporting interval for the entry.  The
 * application should return EMBER_ZCL_STATUS_SUCCESS if it can support the
 * configuration or an error status otherwise.  Note: attribute reporting is
 * required for many clusters and attributes, so rejecting a reporting
 * configuration may violate ZigBee specifications.
 *
 * @param entry   Ver.: always
 */
EmberAfStatus emberAfPluginReportingConfiguredCallback(const EmberAfPluginReportingEntry * entry)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Select File Descriptors
 *
 * This function is called when the Gateway plugin will do a select() call to
 * yield the processor until it has a timed event that needs to execute.  The
 * function implementor may add additional file descriptors that the
 * application will monitor with select() for data ready.  These file
 * descriptors must be read file descriptors.  The number of file descriptors
 * added must be returned by the function (0 for none added).
 *
 * @param list A pointer to a list of File descriptors that the function
 * implementor may append to  Ver.: always
 * @param maxSize The maximum number of elements that the function implementor
 * may add.  Ver.: always
 */
int emberAfPluginGatewaySelectFileDescriptorsCallback(int* list,
                                                      int maxSize)
{
  return 0;
}

/** @brief Group Names Supported
 *
 * This function is called by the framework when it is necessary to determine
 * whether or not group names are supported.
 *
 * @param endpoint The endpoint. Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(uint8_t endpoint)
{
  return false;
}

/** @brief Get Group Name
 *
 * This function is called by the framework to query for a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Set Group Name
 *
 * This function is called by the framework to set a group name based
 * on the endpoint and the group ID.
 *
 * @param endpoint The endpoint. Ver.: always
 * @param groupId  The group ID. Ver.: always
 * @param groupName Pointer to the group name. Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * released but the button joining plugin did not handle it.  This callback is
 * NOT called in ISR context so there are no restrictions on what code can
 * execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The length of time button was held down before
 * it was released.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
  // We assume the first endpoint (0) is the one to use for client or server
  uint8_t endpoint = emberAfPrimaryEndpoint();
  if (buttonPressDurationMs < SHORT_PRESS_DURATION_MS) {
    emberAfCorePrintln("EZ-Mode Commission %p:%x", "Client", endpoint);
    emberAfEzmodeClientCommission(endpoint,
                                  EMBER_AF_EZMODE_COMMISSIONING_SERVER_TO_CLIENT,
                                  clusterIds,
                                  COUNTOF(clusterIds));
  } else {
    emberAfCorePrintln("EZ-Mode Commission %p:%x", "Server", endpoint);
    emberAfEzmodeServerCommission(endpoint);
  }
}

static void setUnsecuredRejoinPolicy(void)
{
  bool set;
  EmberStatus status = EMBER_SUCCESS;

  set = (emberUnsignedCommandArgument(0) ? true : false);

#ifdef EZSP_HOST
  EzspDecisionBitmask policy = (EZSP_DECISION_ALLOW_JOINS | EZSP_DECISION_ALLOW_UNSECURED_REJOINS);
  status = emberAfSetEzspPolicy(EZSP_TRUST_CENTER_POLICY,
                                (set
                                 ? policy
                                 : (policy | EZSP_DECISION_IGNORE_UNSECURED_REJOINS)),
                                "Trust Center Policy",
                                (set
                                 ? "Allow preconfigured key joins"
                                 : "Ignore trust center rejoins"));
#endif

  emberAfCorePrintln("allow trust center rejoins: 0x%X", status);
}

static void setStackComplianceRevision(void)
{
#ifndef EZSP_HOST
  // This guy is declared in stack/zigbee/zigbee-device.c when we build for
  // EMBER_TEST.
  extern uint8_t emTestStackComplianceRevision;

  uint8_t revision = (uint8_t)emberUnsignedCommandArgument(0);
  emTestStackComplianceRevision = revision;
  emberAfCorePrintln("Set stack compliance revision to %d",
                     emTestStackComplianceRevision);
#else
  emberAfCorePrintln("This command is only available on SoC!");
#endif
}

EmberCommandEntry emberAfCustomCommands[] = {
  emberCommandEntryAction("set-unsecured-rejoin-policy",
                          setUnsecuredRejoinPolicy,
                          "u",
                          "Set the trust center policy on whether or not to allow unsecured rejoins."),
  emberCommandEntryAction("set-stack-compliance-revision",
                          setStackComplianceRevision,
                          "u",
                          "Set the stack compliance revision used by the stack (only available on SoC)."),
  emberCommandEntryTerminator(),
};
#ifndef EZSP_HOST
void emberIncomingNetworkStatusHandler(uint8_t errorCode,
                                       EmberNodeId target)
{
  emberAfPushCallbackNetworkIndex();
  emberAfCorePrintln("ErrorCode : 0x%1X, Target : 0x%2X ", errorCode, target);
  emberAfPopNetworkIndex();
}
#else
void ezspIncomingNetworkStatusHandler(uint8_t errorCode,
                                      EmberNodeId target)
{
  emberAfPushCallbackNetworkIndex();
  emberAfCorePrintln("ErrorCode : 0x%1X, Target : 0x%2X ", errorCode, target);
  emberAfPopNetworkIndex();
}
#endif //EZSP_HOST
