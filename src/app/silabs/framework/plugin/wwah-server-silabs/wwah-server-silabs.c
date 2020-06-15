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
 * @brief Routines for the WWAH Server Silabs plugin, which implement the server
 *        side of the WWAH Silabs cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/aps-link-key-authorization/aps-link-key-authorization.h"
#include "app/framework/plugin/compact-logger/compact-logger.h"
#include "app/framework/plugin/dido/dido.h"
#include "wwah-server-silabs.h"
#include "../update-tc-link-key/update-tc-link-key.h"
#include "app/framework/plugin/wwah-connectivity-manager/wwah-connectivity-manager.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#define AMAZON_MANUFACTURING_ID 0x1217

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/plugin/wwah-server-silabs/wwah-server-silabs-test.h"
#endif

#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
#include "app/framework/plugin/ias-zone-server/ias-zone-server.h"
#endif

#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER
#include "app/framework/plugin/poll-control-server/poll-control-server.h"
#else
#define emberAfPluginPollControlServerSetIgnoreNonTrustCenter(ignoreNonTc) (void)0
#endif

#ifndef EZSP_HOST
#include "stack/include/network-formation.h"
#endif

static void (*poweringOffCb)(EmberPowerDownNotificationResult) = NULL;
static EmberAfWwahPowerNotificationReason resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_UNKNOWN;
#define INVALID_WWAH_ENDPOINT 0
static uint8_t wwahServerEndpoint = INVALID_WWAH_ENDPOINT;
static uint8_t wwahClientEndpoint = INVALID_WWAH_ENDPOINT;
static uint8_t wwahConfigurationBitmask;

static void initializeWwahClientEndpoint(uint8_t endpoint);
static bool haveWwahClientEndpoint(void);
static EmberStatus discoverWwahClientEndpoint(void);
static void setWwahClientEndpoint(uint8_t endpoint);
static void wwahClientDiscoveryCallback(const EmberAfServiceDiscoveryResult *result);
static void poweringOffNotificationComplete(EmberPowerDownNotificationResult result);
static void setSlWwahConfigurationMode(bool enable);
static inline void setParentClassificationEnabled(bool enabled);

extern EmberEventControl emberAfPluginWwahServerSilabsTriggerSurveyBeaconsNetworkEventControls[];

enum {
  WWAH_CLIENT_TICK_STATE_DONE,
  WWAH_CLIENT_TICK_STATE_WAIT_NETWORK_UP,
  WWAH_CLIENT_TICK_STATE_ENDPOINT_DISCOVERY,
  WWAH_CLIENT_TICK_STATE_SEND_POWERING_ON,
  WWAH_CLIENT_TICK_STATE_POWERING_OFF_TIMEOUT
};

static uint8_t clientTickState = WWAH_CLIENT_TICK_STATE_DONE;
#define MAX_DISCOVERY_DELAY 5000
#define MAX_DISCOVERY_RETRIES 3

// Response payload contain 10 clusters and respective statuses.
#define MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_RESPONSE_COMMAND 10
static bool isClusterInUseTcForClusterServerToken(uint16_t clusterId);
static void syncClusterWithUseTcForClusterServerToken(void);
static EmberAfStatus setUseTcForClusterServerToken(uint8_t* tokenIndex, uint16_t clusterId);
static void cleanUseTcForClusterServerToken(uint8_t tokenIndex);

static EmberApsFrame apsFrameCache;
static EmberNodeId responseDestinationCache;
static uint8_t responseSeqNumCache;

static bool ignoreNonTrustCenterForTimeCluster = false;

EmberAfStatus emberReadWwahServerSilabsAttribute(uint8_t endpoint,
                                                 EmberAfAttributeId attributeId,
                                                 const char *name,
                                                 uint8_t *data,
                                                 uint8_t size)
{
  EmberAfStatus afStatus = emberAfReadManufacturerSpecificServerAttribute(endpoint,
                                                                          ZCL_SL_WWAH_CLUSTER_ID,
                                                                          attributeId,
                                                                          AMAZON_MANUFACTURING_ID,
                                                                          data,
                                                                          size);
  if (afStatus != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSlWwahClusterPrintln("ERR: %ping %p 0x%x", "read", name, afStatus);
  }
  return afStatus;
}

EmberAfStatus emberWriteWwahServerSilabsAttribute(uint8_t endpoint,
                                                  EmberAfAttributeId attributeId,
                                                  const char *name,
                                                  uint8_t *data,
                                                  EmberAfAttributeType type)
{
  EmberAfStatus afStatus = emberAfWriteManufacturerSpecificServerAttribute(endpoint,
                                                                           ZCL_SL_WWAH_CLUSTER_ID,
                                                                           attributeId,
                                                                           AMAZON_MANUFACTURING_ID,
                                                                           data,
                                                                           type);
  if (afStatus != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSlWwahClusterPrintln("ERR: %ping %p 0x%x", "writ", name, afStatus);
  }
  return afStatus;
}

void emberAfSlWwahClusterServerTickCallback(uint8_t endpoint)
{
  static uint8_t discoRetries;    // Used to throttle discovery retries.
  uint16_t delayMs = 0;

  switch (clientTickState) {
    case WWAH_CLIENT_TICK_STATE_WAIT_NETWORK_UP:
      // Wait for the network to come up before proceeding.
      if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
        if (haveWwahClientEndpoint()) {
          // Send the power on notification command.
          clientTickState = WWAH_CLIENT_TICK_STATE_SEND_POWERING_ON;
        } else {
          // Start client endpoint discovery.
          clientTickState = WWAH_CLIENT_TICK_STATE_ENDPOINT_DISCOVERY;
        }
        // Set a random delay before continuing to the next state.
        delayMs = emberGetPseudoRandomNumber() % MAX_DISCOVERY_DELAY;
        emberAfScheduleServerTick(endpoint, ZCL_SL_WWAH_CLUSTER_ID, delayMs);
      }
      break;

    case WWAH_CLIENT_TICK_STATE_ENDPOINT_DISCOVERY:
      if (!haveWwahClientEndpoint() && (discoRetries < MAX_DISCOVERY_RETRIES)) {
        discoverWwahClientEndpoint();
        delayMs = emberGetPseudoRandomNumber() % MAX_DISCOVERY_DELAY;
        emberAfScheduleServerTick(endpoint, ZCL_SL_WWAH_CLUSTER_ID, delayMs);
        discoRetries++;
      } else {
        discoRetries = 0;   // Reset this for next time.
        if (haveWwahClientEndpoint()) {
          // Proceed to next state to send power on notification.
          clientTickState = WWAH_CLIENT_TICK_STATE_SEND_POWERING_ON;
          emberAfScheduleServerTick(endpoint, ZCL_SL_WWAH_CLUSTER_ID, 500);
        } else {
          // Give up, wwah client endpoint not found.
          clientTickState = WWAH_CLIENT_TICK_STATE_DONE;
        }
      }
      break;

    case WWAH_CLIENT_TICK_STATE_SEND_POWERING_ON:
      // Send the Powering On notification to the coordinator.
      // Send this in the Tick callback to randomize the transmission
      // after the stack comes up.
      emberAfPluginWwahServerSendPoweringOnNotification(EMBER_ZIGBEE_COORDINATOR_ADDRESS,
                                                        endpoint,
                                                        wwahClientEndpoint,
                                                        resetReason,
                                                        AMAZON_MANUFACTURING_ID,
                                                        NULL,
                                                        0);
      clientTickState = WWAH_CLIENT_TICK_STATE_DONE;
      break;

    case WWAH_CLIENT_TICK_STATE_POWERING_OFF_TIMEOUT:
      if (poweringOffCb != NULL) {
        emberAfSlWwahClusterPrintln("Powering off notification timeout");
        poweringOffNotificationComplete(EMBER_POWER_DOWN_NOTIFICATION_TIMEOUT);
      }
      clientTickState = WWAH_CLIENT_TICK_STATE_DONE;
      break;
    default:
      break;
  }
}

static void poweringOffNotificationComplete(EmberPowerDownNotificationResult result)
{
  if (poweringOffCb != NULL) {
    poweringOffCb(result);
    poweringOffCb = NULL;
  }
}

void emberAfSlWwahClusterServerDefaultResponseCallback(uint8_t endpoint,
                                                       uint8_t commandId,
                                                       EmberAfStatus status)
{
  EmberPowerDownNotificationResult result;
  if (commandId == ZCL_POWERING_OFF_NOTIFICATION_COMMAND_ID) {
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      emberAfDebugPrintln("Powering off default response: 0x%X", status);
      result = EMBER_POWER_DOWN_NOTIFICATION_SUCCESS;
    } else {
      emberAfDebugPrintln("Powering off default response error: 0x%X", status);
      result = EMBER_POWER_DOWN_NOTIFICATION_FAILURE;
    }
    poweringOffNotificationComplete(result);
  }
}

void emberAfSlWwahClusterServerInitCallback(uint8_t endpoint)
{
  uint8_t macPollFailureWaitTimeSec;

  // Get the reset reason and store it.
  // This will be sent after the stack comes up.
  uint16_t resetInfo = halGetExtendedResetInfo();
  switch (RESET_BASE_TYPE(resetInfo)) {
    case RESET_FIB:
      if (resetInfo == RESET_FIB_GO) {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_OTA_BOOTLOAD_SUCCESS;
      } else {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_BOOTLOAD_FAILURE;
      }
      break;
    case RESET_BOOTLOADER:
      if (resetInfo == RESET_BOOTLOADER_GO) {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_OTA_BOOTLOAD_SUCCESS;
      } else {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_BOOTLOAD_FAILURE;
      }
      break;
    #if defined(CORTEXM3_EFM32_MICRO)
    case RESET_BROWNOUT:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_BROWNOUT;
      break;
    case RESET_POWERON:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_POWER_BUTTON;
      break;
    #elif defined(CORTEXM3_EMBER_MICRO)
    case RESET_POWERON:
      if (resetInfo == RESET_POWERON_HV) {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_POWER_BUTTON;
      } else if (resetInfo == RESET_POWERON_LV) {
        resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_BROWNOUT;
      }
      break;
    #endif
    case RESET_EXTERNAL:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_RESET_PIN;
      break;
    case RESET_SOFTWARE:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_SOFTWARE_RESET;
      break;
    case RESET_WATCHDOG:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_WATCHDOG;
      break;
    case RESET_CRASH:
    case RESET_FATAL:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_SOFWARE_EXCEPTION;
      break;
    case RESET_FAULT:
    case RESET_FLASH:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_MEMORY_HARDWARE_FAULT;
      break;
    default:
      resetReason = EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_UNKNOWN;
      break;
  }

  // Load the WWAH configuration state bitmask
  halCommonGetToken(&wwahConfigurationBitmask,
                    TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK);

  // Set ZLL Interpan attribute
  uint8_t touchlinkEnabled = !(wwahConfigurationBitmask
                               & PLUGIN_WWAH_CONFIGURATION_MASK_ZLL_POLICY_DISABLED);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_TOUCHLINK_INTERPAN_ENABLED_ATTRIBUTE_ID,
                                      "zll policy",
                                      &touchlinkEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Load mac poll failure wait time token
  halCommonGetToken(&macPollFailureWaitTimeSec, TOKEN_WWAH_MAC_POLL_FAILURE_WAIT_TIME);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_MAC_POLL_FAILURE_WAIT_TIME_ATTRIBUTE_ID,
                                      "mac poll failure wait time",
                                      &macPollFailureWaitTimeSec,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);

  initializeWwahClientEndpoint(endpoint);
  wwahServerEndpoint = endpoint;

  // Synchronize leave without rejoin attribute with security bitmask
  EmberExtendedSecurityBitmask mask;
  emberGetExtendedSecurityBitmask(&mask);
  uint8_t data = (mask & EMBER_NWK_LEAVE_WITHOUT_REJOIN_NOT_ALLOWED) ? 0 : 1;
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_MGMT_LEAVE_WITHOUT_REJOIN_ENABLED_ATTRIBUTE_ID,
                                      "mgmt leave without rejoin enabled",
                                      &data,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Synchronize TCSecurityOnNtwkKeyRotationEnabled attribute with security bitmask
  data = (mask & EMBER_SECURE_NETWORK_KEY_ROTATION) ? 1 : 0;
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_TC_SECURITY_ON_NTWK_KEY_ROTATION_ENABLED_ATTRIBUTE_ID,
                                      "tc security on ntwk key rotation enabled",
                                      &data,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Write MACRetryCount and NWKRetryCount
  data = emberGetMaxMacRetries();
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_MAC_RETRY_COUNT_ATTRIBUTE_ID,
                                      "mac retry count",
                                      &data,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);
  data = emberGetMaxNwkRetries();
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_NWK_RETRY_COUNT_ATTRIBUTE_ID,
                                      "network retry count",
                                      &data,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);

  // Set Configuration Mode attribute, update stack
  uint8_t confModeEnabled = wwahConfigurationBitmask
                            & PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED;
  confModeEnabled = confModeEnabled ? 1 : 0;
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_CONFIGURATION_MODE_ENABLED_ATTRIBUTE_ID,
                                      "configuration mode enabled",
                                      &confModeEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);
#ifndef EZSP_HOST
  emberSetZDOConfigurationMode(confModeEnabled != 0);
#endif

  // Set Parent Classification attribute, update stack
  uint8_t parentClassificationEnabled = wwahConfigurationBitmask
                                        & PLUGIN_WWAH_CONFIGURATION_MASK_PARENT_CLASSIFICATION_ENABLED;
  parentClassificationEnabled = parentClassificationEnabled ? 1 : 0;
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_WWAH_PARENT_CLASSIFICATION_ENABLED_ATTRIBUTE_ID,
                                      "parent classification enabled",
                                      &parentClassificationEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  emberSetParentClassificationEnabled(parentClassificationEnabled);

  // Set pending network update channel and panId attributes
  uint8_t channel;
  halCommonGetToken(&channel, TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL);

  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_PENDING_NETWORK_UPDATE_CHANNEL_ATTRIBUTE_ID,
                                      "pending network update channel",
                                      &channel,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);

  uint16_t panId;
  halCommonGetToken(&panId, TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_PENDING_NETWORK_UPDATE_PANID_ATTRIBUTE_ID,
                                      "pending network update panId",
                                      (uint8_t*)&panId,
                                      ZCL_INT16U_ATTRIBUTE_TYPE);

  // Sync the ignoreNonTrustCenter setting of all supported clusters
  // with the UseTcForClusterServerToken
  syncClusterWithUseTcForClusterServerToken();

  // Update disable ota downgrades configuration and attribute from token
  uint8_t disableOtaDowngrades = (wwahConfigurationBitmask
                                  & PLUGIN_WWAH_CONFIGURATION_MASK_DISABLE_OTA_DOWNGRADES) ? 1 : 0;
  if (disableOtaDowngrades) {
    emberWriteWwahServerSilabsAttribute(endpoint,
                                        ZCL_SL_DISABLE_OTA_DOWNGRADES_ATTRIBUTE_ID,
                                        "disable OTA downgrades",
                                        &disableOtaDowngrades,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    emberAfPluginSetDisableOtaDowngrades(disableOtaDowngrades);
  }

#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
  emberAfWwahAppEventRetryManagerInitCallback(endpoint);
#endif
#if defined(EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER)
  emberAfPluginWwahConnectivityManagerInitCallback(endpoint);
#endif
}

#define POWERING_ON_NOTIFICATION_MAX_DELAY_MS  5000
void emberAfPluginWwahServerSilabsStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN) {
    // Clear out the WWAH endpoint in case the node is leaving.
    setWwahClientEndpoint(INVALID_WWAH_ENDPOINT);
  } else if (status == EMBER_NETWORK_UP && clientTickState != WWAH_CLIENT_TICK_STATE_DONE) {
    emberAfScheduleServerTick(wwahServerEndpoint, ZCL_SL_WWAH_CLUSTER_ID, 500);
  }
}

// -----------------------------------------------------------------------------
// WWAH Client Endpoint Discovery

static void initializeWwahClientEndpoint(uint8_t endpoint)
{
  // Read from Token
  halCommonGetToken(&wwahClientEndpoint, TOKEN_PLUGIN_WWAH_CLIENT_ENDPOINT);
  emberAfDebugPrintln("Read client endpoint 0x%X", wwahClientEndpoint);

  // Wait for the network to come up.
  clientTickState = WWAH_CLIENT_TICK_STATE_WAIT_NETWORK_UP;
  emberAfScheduleServerTick(endpoint, ZCL_SL_WWAH_CLUSTER_ID, 500);
}

static bool haveWwahClientEndpoint()
{
  return wwahClientEndpoint != INVALID_WWAH_ENDPOINT;
}

static void setWwahClientEndpoint(uint8_t endpoint)
{
  if (wwahClientEndpoint != endpoint) {
    wwahClientEndpoint = endpoint;
    halCommonSetToken(TOKEN_PLUGIN_WWAH_CLIENT_ENDPOINT, &wwahClientEndpoint);
    emberAfDebugPrintln("Set client endpoint 0x%X", wwahClientEndpoint);
  }
}

static EmberStatus discoverWwahClientEndpoint()
{
  EmberStatus status;
  status = emberAfFindDevicesByProfileAndCluster(EMBER_ZIGBEE_COORDINATOR_ADDRESS,
                                                 HA_PROFILE_ID,
                                                 ZCL_SL_WWAH_CLUSTER_ID,
                                                 EMBER_AF_CLIENT_CLUSTER_DISCOVERY,
                                                 wwahClientDiscoveryCallback);
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send endpoint discovery: 0x%X", status);
  }
  return status;
}

static void wwahClientDiscoveryCallback(const EmberAfServiceDiscoveryResult *result)
{
  const EmberAfEndpointList *epList;
  if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      epList = (const EmberAfEndpointList*)result->responseData;
      if (epList->count >= 1) {
        setWwahClientEndpoint(epList->list[0]);
      }
    }
  }
}

// -------------------------------------------------------
// APS ACK on Unicast callbacks
// -------------------------------------------------------

// Note: APS ACK is always supported, simply reporting back success.
//       Exempt list is not stored (nor supported) as it'd have a huge token and flash footprint.
bool emberAfSlWwahClusterRequireApsAcksOnUnicastsCallback(uint8_t numberExemptClusters,
                                                          uint8_t* clusterId)
{
  EmberStatus status;

  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response for Require APS ACK on Unicasts: 0x%X", status);
  }

  return true;
}

// Note: APS ACK is always supported, simply reporting back success.
bool emberAfSlWwahClusterRemoveApsAcksOnUnicastsRequirementCallback(void)
{
  EmberStatus status;

  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response for Remove APS ACK on Unicasts: 0x%X", status);
  }

  return true;
}

// Note: APS ACK is always supported, exempt list is not stored, so simply reporting back always enabled (ie. length of exempt list is zero).
bool emberAfSlWwahClusterApsAckRequirementQueryCallback(void)
{
  EmberStatus status;

  // Build up the response for the query.
  emberAfFillCommandSlWwahClusterApsAckEnablementQueryResponse(0, NULL, 0);

  // Send the response.
  status = emberAfSendResponse();
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send response for ApsAckEnablementQuery: 0x%X", status);
  }

  return true;
}

// -------------------------------------------------------
// APS Link Key Authorization callbacks
// -------------------------------------------------------

bool emberAfSlWwahClusterEnableApsLinkKeyAuthorizationCallback(uint8_t numberExemptClusters,
                                                               uint8_t* clusterId)
{
  EmberStatus status;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_SUCCESS;

  // We can just go ahead and enable authorization, no need to worry about missing TC link key
  // as we should always have a key available.
  if (EMBER_SUCCESS != emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(numberExemptClusters, clusterId)) {
    emberAfSlWwahClusterPrintln("Failed to store all clusters to exempt list, list is full.");
    afStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }
  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterDisableApsLinkKeyAuthorizationCallback(uint8_t numberExemptClusters,
                                                                uint8_t* clusterId)
{
  EmberStatus status;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_SUCCESS;

  // We can just go ahead and enable authorization, no need to worry about missing TC link key
  // as we should always have a key available.
  if (EMBER_SUCCESS != emberAfApsLinkKeyAuthorizationDisableWithExemptClusterList(numberExemptClusters, clusterId)) {
    emberAfSlWwahClusterPrintln("Failed to store all clusters to exempt list, list is full.");
    afStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }
  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterApsLinkKeyAuthorizationQueryCallback(uint16_t clusterId)
{
  EmberStatus status;

  // Build up the response for the query
  emberAfFillCommandSlWwahClusterApsLinkKeyAuthorizationQueryResponse(
    clusterId,
    emberAfApsLinkKeyAuthorizationIsRequiredOnCluster(clusterId));

  // Send the response
  status = emberAfSendResponse();
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send response for ApsLinkKeyAuthorizationQuery: 0x%X", status);
  }

  return true;
}

// Pre-Bootload Callback, from ota-client
void emberAfPluginOtaClientPreBootloadCallback(uint8_t srcEndpoint,
                                               uint8_t serverEndpoint,
                                               EmberNodeId serverNodeId)
{
  // Attempt to send the Powering Off notification to the coordinator.
  emberAfPluginWwahServerSendPoweringOffNotification(serverNodeId,
                                                     srcEndpoint,
                                                     serverEndpoint,
                                                     EMBER_ZCL_WWAH_POWER_NOTIFICATION_REASON_OTA_BOOTLOAD_SUCCESS,
                                                     AMAZON_MANUFACTURING_ID,
                                                     NULL,
                                                     0,
                                                     NULL);
}

// Power Notification Callbacks

#define POWER_DOWN_NOTIFICATION_RESPONSE_TIMEOUT_MS  (MILLISECOND_TICKS_PER_SECOND * 3)
void emberAfPluginWwahServerSendPoweringOffNotification(EmberNodeId nodeId,
                                                        uint8_t srcEndpoint,
                                                        uint8_t dstEndpoint,
                                                        EmberAfWwahPowerNotificationReason reason,
                                                        uint16_t manufacturerId,
                                                        uint8_t *manufacturerReason,
                                                        uint8_t manufacturerReasonLen,
                                                        void (*pcallback)(EmberPowerDownNotificationResult) )
{
  EmberStatus status;
  emberAfFillCommandSlWwahClusterPoweringOffNotification(reason,
                                                         manufacturerId,
                                                         manufacturerReasonLen,
                                                         manufacturerReason,
                                                         manufacturerReasonLen);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if ((EMBER_SUCCESS == status) && (pcallback != NULL)) {
    // Track callback function so it can be called when the default response is received,
    // or if a timeout occurs.
    poweringOffCb = pcallback;
    clientTickState = WWAH_CLIENT_TICK_STATE_POWERING_OFF_TIMEOUT;
    emberAfScheduleServerTick(srcEndpoint, ZCL_SL_WWAH_CLUSTER_ID, POWER_DOWN_NOTIFICATION_RESPONSE_TIMEOUT_MS);
  } else if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send Powering Off Notification: 0x%X", status);
    if (pcallback != NULL) {
      pcallback(EMBER_POWER_DOWN_NOTIFICATION_FAILURE);
    }
  }
}

void emberAfPluginWwahServerSendPoweringOnNotification(EmberNodeId nodeId,
                                                       uint8_t srcEndpoint,
                                                       uint8_t dstEndpoint,
                                                       EmberAfWwahPowerNotificationReason reason,
                                                       uint16_t manufacturerId,
                                                       uint8_t *manufacturerReason,
                                                       uint8_t manufacturerReasonLen)
{
  EmberStatus status;
  emberAfFillCommandSlWwahClusterPoweringOnNotification(reason,
                                                        manufacturerId,
                                                        manufacturerReasonLen,
                                                        manufacturerReason,
                                                        manufacturerReasonLen);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send Powering On Notification: 0x%X", status);
  }
}

static void setSlWwahConfigurationMode(bool enable)
{
  uint8_t confModeEnabled = (enable ? 0x01 : 0x00);
  EmberStatus status;
  EmberAfStatus afStatus;
  afStatus = emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                                 ZCL_SL_CONFIGURATION_MODE_ENABLED_ATTRIBUTE_ID,
                                                 "configuration mode enabled",
                                                 (uint8_t *)&confModeEnabled,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  if (afStatus == EMBER_ZCL_STATUS_SUCCESS) {
#ifndef EZSP_HOST
    // Configure stack
    emberSetZDOConfigurationMode(enable);

    // Write token only if value has changed
    bool currentlyEnabled = wwahConfigurationBitmask
                            & PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED;
    if (currentlyEnabled != enable) {
      if (enable) {
        wwahConfigurationBitmask |=
          PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED;
      } else {
        wwahConfigurationBitmask &=
          ~PLUGIN_WWAH_CONFIGURATION_MASK_CONFIGURATION_MODE_ENABLED;
      }
      halCommonSetToken(TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK,
                        &wwahConfigurationBitmask);
    }
#endif
  } else {
    emberAfSlWwahClusterPrintln("ERR: writing configuration attribute %x", afStatus);
  }

  // Send default response
  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }
}

bool emberAfSlWwahClusterDisableConfigurationModeCallback(void)
{
  setSlWwahConfigurationMode(false);
  return true;
}

bool emberAfSlWwahClusterEnableConfigurationModeCallback(void)
{
  setSlWwahConfigurationMode(true);
  return true;
}

bool emberAfSlWwahClusterSetMacPollFailureWaitTimeCallback(uint8_t waitTime)
{
  EmberStatus status;
  EmberAfStatus afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                               ZCL_SL_MAC_POLL_FAILURE_WAIT_TIME_ATTRIBUTE_ID,
                                                               "mac poll failure wait time",
                                                               &waitTime,
                                                               ZCL_INT8U_ATTRIBUTE_TYPE);

  if (afStatus == EMBER_ZCL_STATUS_SUCCESS) {
    halCommonSetToken(TOKEN_WWAH_MAC_POLL_FAILURE_WAIT_TIME, &waitTime);
  }

  status = emberAfSendImmediateDefaultResponse(afStatus);

  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

// TC Link Key Rotation callbacks

bool emberAfSlWwahClusterRequestNewApsLinkKeyCallback(void)
{
  EmberStatus status = emberAfPluginUpdateTcLinkKeyStart();
  if (status == EMBER_SUCCESS) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
  }
  return true;
}

bool emberAfSlWwahClusterEnableTcSecurityOnNtwkKeyRotationCallback(void)
{
  // Set the attribute such that all network key updates are only processed through unicast messages.
  bool secureNwkKeyRotationEnabled = true;
  EmberStatus status = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                           ZCL_SL_TC_SECURITY_ON_NTWK_KEY_ROTATION_ENABLED_ATTRIBUTE_ID,
                                                           "tc security on ntwk key rotation enabled",
                                                           (uint8_t *)&secureNwkKeyRotationEnabled,
                                                           ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    EmberExtendedSecurityBitmask extendedBitmask;
    emberGetExtendedSecurityBitmask(&extendedBitmask);
    extendedBitmask |= EMBER_SECURE_NETWORK_KEY_ROTATION;
    emberSetExtendedSecurityBitmask(extendedBitmask);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
  }
  return true;
}

bool emberAfSlWwahClusterClearBindingTableCallback(void)
{
  EmberStatus status;

  status = emberClearBindingTable();

  if (status == EMBER_SUCCESS) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
  }

  return true;
}

bool emberAfSlWwahClusterDisableWwahRejoinAlgorithmCallback(void)
{
  emberAfPluginWwahConnectivityManagerDisableRejoinAlgorithm(emberAfCurrentEndpoint());
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

// -----------------------------------------------------------------------------
// ZCL command callback stub implementations.
// Causes return status "unsupported manufacturer cluster command".

bool emberAfSlWwahClusterDebugReportQueryCallback(uint8_t debugReportId)
{
#ifdef EMBER_AF_PLUGIN_DIDO
  EmberStatus status;
  status = emberAfDidoClusterSendDebugReport(debugReportId,
                                             emberAfCurrentCommand()->source,
                                             emberAfCurrentEndpoint());
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send debug report, status=0x%x", status);
  }
  return true;
#else
  return false;
#endif
}

bool emberAfSlWwahClusterDisableMgmtLeaveWithoutRejoinCallback(void)
{
  EmberStatus status = EMBER_SUCCESS;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_SUCCESS;
  uint8_t data = 0;

  // Clear MGMTLeaveWithoutRejoinEnabled attribute
  afStatus = emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                                 ZCL_SL_MGMT_LEAVE_WITHOUT_REJOIN_ENABLED_ATTRIBUTE_ID,
                                                 "mgmt leave without rejoin enabled",
                                                 &data,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (afStatus != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to write MGMTLeaveWithoutRejoinEnabled attribute: 0x%X", afStatus);
  } else {
    // Modify extended security bitmask to disable leave without rejoin
    EmberExtendedSecurityBitmask mask;
    emberGetExtendedSecurityBitmask(&mask);
    emberSetExtendedSecurityBitmask(mask | EMBER_NWK_LEAVE_WITHOUT_REJOIN_NOT_ALLOWED);
  }

  // Send default response
  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }
  return true;
}

bool emberAfSlWwahClusterDisableOtaDowngradesCallback(void)
{
  uint8_t disableOtaDowngrades = 1;
  EmberAfStatus afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                               ZCL_SL_DISABLE_OTA_DOWNGRADES_ATTRIBUTE_ID,
                                                               "disable OTA downgrades",
                                                               &disableOtaDowngrades,
                                                               ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  if (afStatus != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to write DisableOtaDowngrades attribute: 0x%X", afStatus);
  } else {
    wwahConfigurationBitmask |= PLUGIN_WWAH_CONFIGURATION_MASK_DISABLE_OTA_DOWNGRADES;
    halCommonSetToken(TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK, &wwahConfigurationBitmask);
    emberAfPluginSetDisableOtaDowngrades(disableOtaDowngrades);
  }

  // Send default response
  EmberStatus status = emberAfSendImmediateDefaultResponse(afStatus);
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterDisablePeriodicRouterCheckInsCallback(void)
{
  EmberStatus status = EMBER_ERR_FATAL;

#if defined(EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER)
  status = emberAfPluginWwahConnectivityManagerDisablePeriodicRouterCheckIns();
#else
  emberAfSlWwahClusterPrintln("ERROR - Enable Periodic Router Check Ins Callback: Functionality is not supported.");
#endif

  EmberAfStatus afStatus = (status == EMBER_SUCCESS) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterDisableTouchlinkInterpanMessageSupportCallback(void)
{
  uint8_t touchlinkEnabled = 0;
  EmberStatus status;
  EmberAfStatus afStatus;
  bool valueChanged = false;

  // Already disabled?
  if (wwahConfigurationBitmask & PLUGIN_WWAH_CONFIGURATION_MASK_ZLL_POLICY_DISABLED) {
    afStatus = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                   ZCL_SL_TOUCHLINK_INTERPAN_ENABLED_ATTRIBUTE_ID,
                                                   "zll policy",
                                                   &touchlinkEnabled,
                                                   ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    valueChanged = true;
  }

  status = emberAfSendImmediateDefaultResponse(afStatus);

  if (afStatus == EMBER_ZCL_STATUS_SUCCESS) {
    if (valueChanged) {
      wwahConfigurationBitmask |= PLUGIN_WWAH_CONFIGURATION_MASK_ZLL_POLICY_DISABLED;
      halCommonSetToken(TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK,
                        &wwahConfigurationBitmask);
    }
  } else {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterDisableWwahAppEventRetryAlgorithmCallback(void)
{
  EmberStatus status;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_SUCCESS;

#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
  // Setting up retry parameters
  emberAfWwahAppEventRetryManagerDisable(emberAfCurrentEndpoint());
#else
  emberAfSlWwahClusterPrintln("ERROR - Disable Wwah App Event Retry Algorithm Callback: Functionality is not supported.");
  // TODO: Is EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND what we want here?
  afStatus = EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND;
#endif

  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterDisableWwahBadParentRecoveryCallback(void)
{
  emAfPluginWwahConnectivityManagerDisableBadParentRecovery(emberAfCurrentEndpoint());
  EmberStatus status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterEnablePeriodicRouterCheckInsCallback(uint16_t checkInInterval)
{
  EmberStatus status = EMBER_ERR_FATAL;

#if defined(EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER)
  status = emberAfPluginWwahConnectivityManagerEnablePeriodicRouterCheckIns(checkInInterval);
#else
  emberAfSlWwahClusterPrintln("ERROR - Enable Periodic Router Check Ins Callback: Functionality is not supported.");
#endif

  EmberAfStatus afStatus = (status == EMBER_SUCCESS) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;

  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterEnableWwahAppEventRetryAlgorithmCallback(uint8_t firstBackoffTimeSeconds,
                                                                  uint8_t backoffSeqCommonRatio,
                                                                  uint32_t maxBackoffTimeSeconds,
                                                                  uint8_t maxRedeliveryAttempts)
{
  EmberStatus status;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_SUCCESS;

#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
  // Setting up retry parameters
  if (EMBER_SUCCESS != emberAfWwahAppEventRetryManagerEnableWithParams(emberAfCurrentEndpoint(),
                                                                       firstBackoffTimeSeconds,
                                                                       backoffSeqCommonRatio,
                                                                       maxBackoffTimeSeconds,
                                                                       maxRedeliveryAttempts)) {
    afStatus = EMBER_ZCL_STATUS_INVALID_VALUE;
    emberAfSlWwahClusterPrintln("ERROR - Enable Wwah App Event Retry Algorithm Callback: Invalid parameter.");
  }

#else
  emberAfSlWwahClusterPrintln("ERROR - Enable Wwah App Event Retry Algorithm Callback: Functionality is not supported.");
  // TODO: Is EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND what we want here?
  afStatus = EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND;
#endif

  status = emberAfSendImmediateDefaultResponse(afStatus);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterEnableWwahBadParentRecoveryCallback(void)
{
  emAfPluginWwahConnectivityManagerEnableBadParentRecovery(emberAfCurrentEndpoint(), 0);
  EmberStatus status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

static inline void setParentClassificationEnabled(bool enabled)
{
  EmberStatus status;
  EmberAfStatus afStatus;
  bool valueChanged = false;
  uint8_t parentClassificationEnabled = enabled ? 1 : 0;

  if (enabled
      == (bool)(wwahConfigurationBitmask
                & PLUGIN_WWAH_CONFIGURATION_MASK_PARENT_CLASSIFICATION_ENABLED)) {
    afStatus = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    // Update the ZCL attribute
    afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                   ZCL_SL_WWAH_PARENT_CLASSIFICATION_ENABLED_ATTRIBUTE_ID,
                                                   "parent classification enabled",
                                                   &parentClassificationEnabled,
                                                   ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    valueChanged = true;
  }

  if (EMBER_ZCL_STATUS_SUCCESS == afStatus) {
    if (valueChanged) {
      // Configure the stack
      emberSetParentClassificationEnabled(enabled);

      // Persist the token
      if (enabled) {
        wwahConfigurationBitmask |=
          PLUGIN_WWAH_CONFIGURATION_MASK_PARENT_CLASSIFICATION_ENABLED;
      } else {
        wwahConfigurationBitmask &=
          ~PLUGIN_WWAH_CONFIGURATION_MASK_PARENT_CLASSIFICATION_ENABLED;
      }
      halCommonSetToken(TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK,
                        &wwahConfigurationBitmask);
    }
  } else {
    emberAfSlWwahClusterPrintln("WWahParentClassificationCallback failed to "
                                "update ZCL attribute value: 0x%X", afStatus);
  }

  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (EMBER_SUCCESS != status) {
    emberAfSlWwahClusterPrintln("WWahParentClassificationCallback failed to "
                                "send default response: 0x%X", status);
  }
}

bool emberAfSlWwahClusterEnableWwahParentClassificationCallback(void)
{
  setParentClassificationEnabled(true);
  return true;
}

bool emberAfSlWwahClusterDisableWwahParentClassificationCallback(void)
{
  setParentClassificationEnabled(false);
  return true;
}

bool emberAfSlWwahClusterEnableWwahRejoinAlgorithmCallback(uint16_t fastRejoinTimeoutSeconds,
                                                           uint16_t durationBetweenRejoinsSeconds,
                                                           uint16_t fastRejoinFirstBackoffSeconds,
                                                           uint16_t maxBackoffTimeSeconds,
                                                           uint16_t maxBackoffIterations)
{
  // Backoffs implemented in a future release
  emberAfPluginWwahConnectivityManagerSetRejoinParameters(fastRejoinTimeoutSeconds,
                                                          durationBetweenRejoinsSeconds,
                                                          fastRejoinFirstBackoffSeconds,
                                                          maxBackoffTimeSeconds,
                                                          maxBackoffIterations);
  emberAfPluginWwahConnectivityManagerEnableRejoinAlgorithm(emberAfCurrentEndpoint());
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfSlWwahClusterEnableZCLGroupCommandsFromNonTCCallback(void)
{
  return false;
}

bool emberAfSlWwahClusterRequestTimeCallback(void)
{
  EmberStatus status;
  uint8_t timeAttributeIds[] = {
    LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_DST_START_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_DST_START_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_DST_END_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_DST_END_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_DST_SHIFT_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_DST_SHIFT_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_LOCAL_TIME_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_LOCAL_TIME_ATTRIBUTE_ID),
  };
  emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID,
                                                       timeAttributeIds,
                                                       sizeof(timeAttributeIds));
  emberAfSetCommandEndpoints(emberAfGetCommandApsFrame()->destinationEndpoint,  // source endpoint
                             emberAfGetCommandApsFrame()->sourceEndpoint);      // destination endpoint

  EmberNodeId sourceNodeId = emberAfCurrentCommand()->source;
  if (ignoreNonTrustCenterForTimeCluster
      && (sourceNodeId != EMBER_TRUST_CENTER_NODE_ID)) {
    emberAfSlWwahClusterPrintln("Ignoring non trust center node 0x%2X and requesting time from TC",
                                sourceNodeId);
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, EMBER_TRUST_CENTER_NODE_ID);
  } else {
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sourceNodeId);
  }

  if (status != EMBER_SUCCESS) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    emberAfSlWwahClusterPrintln("Failed to request time attributes on server: 0x%X", status);
  } else {
    // There is no defined ZCL response for the Request Time command - send a default response.
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  }
  return true;
}

void emAfPluginSlWwahReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                    uint8_t *buffer,
                                                    uint16_t bufLen)
{
  uint16_t i;
  uint16_t attributeId;
  uint32_t utcTime;
  uint8_t status;
  if ( clusterId == ZCL_TIME_CLUSTER_ID ) {
    i = 0;
    while ((i + 3) <= bufLen) { // 3 to ensure we can read at least the attribute ID and the status
      attributeId = buffer[i] + (buffer[i + 1] << 8);
      status = buffer[i + 2];
      i += 3;
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        if ((i + 1) > bufLen) {
          // Too short, dump the message.
          return;
        }
        i++;  // skip the type of the attribute.  We already know what it should be.
        if ( attributeId == ZCL_TIME_ATTRIBUTE_ID ) {
          if ((i + 4) > bufLen) {
            // Too short, dump the message.
            return;
          }
          utcTime = (buffer[i] + (buffer[i + 1] << 8) + (buffer[i + 2] << 16) + (buffer[i + 3] << 24));
          emberAfSlWwahClusterPrintln("Received Utc Time Update, time=%d", utcTime);
#ifdef EMBER_AF_PLUGIN_SIMPLE_CLOCK
          emberAfSetTimeCallback(utcTime);
#endif
#ifdef EMBER_AF_PLUGIN_TIME_SERVER
          emAfTimeClusterServerSetCurrentTime(utcTime);
#endif
#ifdef EMBER_AF_PLUGIN_COMPACT_LOGGER
          emberAfPluginCompactLoggerUpdateAllLogsWithUtcTime(utcTime);
#endif
          break;
        }
      }
    }
  }
}

bool emberAfSlWwahClusterSetIasZoneEnrollmentMethodCallback(uint8_t enrollmentMode)
{
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
  EmberAfStatus status;
  status = emberAfPluginIasZoneClusterSetEnrollmentMethod(emberAfGetCommandApsFrame()->destinationEndpoint,
                                                          enrollmentMode);
  emberAfSendImmediateDefaultResponse(status);
  return true;
#else
  return false;
#endif
}

bool emberAfSlWwahClusterSetPendingNetworkUpdateCallback(uint8_t channel,
                                                         uint16_t panId)
{
  EmberStatus status;
  EmberAfStatus afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                               ZCL_SL_PENDING_NETWORK_UPDATE_CHANNEL_ATTRIBUTE_ID,
                                                               "pending network update channel",
                                                               &channel,
                                                               ZCL_INT8U_ATTRIBUTE_TYPE);

  if (afStatus == EMBER_ZCL_STATUS_SUCCESS) {
    afStatus = emberWriteWwahServerSilabsAttribute(emberAfCurrentEndpoint(),
                                                   ZCL_SL_PENDING_NETWORK_UPDATE_PANID_ATTRIBUTE_ID,
                                                   "pending network update panId",
                                                   (uint8_t*)&panId,
                                                   ZCL_INT16U_ATTRIBUTE_TYPE);
  }

  if (afStatus == EMBER_ZCL_STATUS_SUCCESS) {
    halCommonSetToken(TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL, &channel);
    halCommonSetToken(TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID, &panId);
  }

  status = emberAfSendImmediateDefaultResponse(afStatus);

  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send default response: 0x%X", status);
  }

  return true;
}

//----------------------------------
// Survey Beacons implementation (SoC only)

#ifndef EZSP_HOST

#ifndef EMBER_TEST
static EmberMessageBuffer surveyBeaconDataCache;
static uint8_t surveyBeaconDataCount;
#endif // EMBER_TEST

static uint8_t hubConnectivityIndex, longUptimeIndex, rssiIndex, sortStartIndex, sortEndIndex, targetIndex;

void cacheApsFrameData(void)
{
  apsFrameCache.destinationEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  apsFrameCache.sourceEndpoint = emberAfCurrentEndpoint();
  apsFrameCache.profileId = emberAfProfileIdFromIndex(emberAfIndexFromEndpoint(apsFrameCache.destinationEndpoint));
  responseDestinationCache = emberAfCurrentCommand()->source;
  responseSeqNumCache = emberAfCurrentCommand()->seqNum;
}

bool setupSurveyBeaconProcedure(void)
{
  surveyBeaconDataCache = emberAllocateStackBuffer();
  if (surveyBeaconDataCache == EMBER_NULL_MESSAGE_BUFFER) {
    return false;
  }
  emberSetLinkedBuffersLength(surveyBeaconDataCache, sizeof(EmberBeaconSurvey) * EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT);
  surveyBeaconDataCount = 0;

  hubConnectivityIndex = 0;
  longUptimeIndex = 0;
  rssiIndex = 0;
  sortStartIndex = 0;
  sortEndIndex = 0;
  targetIndex = 0;

  return true;
}

bool emSurveyBeaconsScanCompleteHandler(uint8_t channel, EmberStatus status)
{
  EmberBeaconSurvey beaconSurveyResults[EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT];

  // Compile the results. If there were more than we can fit in a payload, filter the results so we
  // pass the best survey results.
  if (surveyBeaconDataCount > EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT) {
    surveyBeaconDataCount = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT;
  }
  emberCopyFromLinkedBuffers(surveyBeaconDataCache,
                             0,
                             beaconSurveyResults,
                             sizeof(EmberBeaconSurvey) * surveyBeaconDataCount);

  emberAfFillCommandSlWwahClusterSurveyBeaconsResponse(surveyBeaconDataCount,
                                                       (uint8_t *) beaconSurveyResults,
                                                       surveyBeaconDataCount * sizeof(EmberBeaconSurvey));

  EmberApsFrame *apsFrame = NULL;
  apsFrame = emberAfGetCommandApsFrame();
  apsFrame->destinationEndpoint = apsFrameCache.destinationEndpoint;
  apsFrame->sourceEndpoint = apsFrameCache.sourceEndpoint;
  apsFrame->profileId = apsFrameCache.profileId;

  // This is a response, must set the ZCL header sequence number to that of the request.
  // Offset of sequence number field depends on whether 2-byte mfg code is present.
  if ((emAfZclBuffer[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U) {
    emAfZclBuffer[3] = responseSeqNumCache; // fc, mfg code, seq
  } else {
    emAfZclBuffer[1] = responseSeqNumCache; // fc, seq
  }

  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, responseDestinationCache);
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send SurveyBeaconResponse: 0x%X",
                                status);
  }

  emberReleaseMessageBuffer(surveyBeaconDataCache);

  return true;
}

void sortBeaconSurveyResult(EmberBeaconSurvey surveyResult)
{
  // Determine if we have space in our array for this beacon survey result
  if (surveyResult.classificationMask & 0x02 // HUB_CONNECTIVITY_MASK
      && surveyResult.classificationMask & 0x01) { // LONG_UPTIME_MASK
    hubConnectivityIndex++;
    longUptimeIndex++;
    rssiIndex++;
    sortStartIndex = 0;
    sortEndIndex = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT <= hubConnectivityIndex ? EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT : hubConnectivityIndex;
  } else if (surveyResult.classificationMask & 0x02) { // HUB_CONNECTIVITY_MASK
    longUptimeIndex++;
    rssiIndex++;
    sortStartIndex = hubConnectivityIndex;
    sortEndIndex = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT <= longUptimeIndex ? EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT : longUptimeIndex;
  } else if (surveyResult.classificationMask & 0x01) { // LONG_UPTIME_MASK
    rssiIndex++;
    sortStartIndex = longUptimeIndex;
    sortEndIndex = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT <= rssiIndex ? EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT : rssiIndex;
  } else {
    sortStartIndex = rssiIndex;
    sortEndIndex = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT <= surveyBeaconDataCount + 1 ? EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT : surveyBeaconDataCount + 1;
  }

  if (sortStartIndex >= sortEndIndex) {
    return;
  }

  // Determine index in array to insert this beacon survey result
  int8_t j;
  EmberBeaconSurvey temp;
  for (j = sortStartIndex; j < sortEndIndex; j++) {
    emberCopyFromLinkedBuffers(surveyBeaconDataCache,
                               j * sizeof(EmberBeaconSurvey),
                               &temp,
                               sizeof(EmberBeaconSurvey));
    if (j == sortEndIndex - 1 || surveyResult.rssi >= temp.rssi) {
      targetIndex = j;
      break;
    }
  }

  // Shift inferior results out of the array and insert survey result
  for (j = EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_SURVEY_BEACON_RESULT_LIMIT - 2; j >= targetIndex; j--) {
    emberCopyFromLinkedBuffers(surveyBeaconDataCache,
                               j * sizeof(EmberBeaconSurvey),
                               &temp,
                               sizeof(EmberBeaconSurvey));
    emberCopyToLinkedBuffers(&temp,
                             surveyBeaconDataCache,
                             (j + 1) * sizeof(EmberBeaconSurvey),
                             sizeof(EmberBeaconSurvey));
  }
  emberCopyToLinkedBuffers(&surveyResult,
                           surveyBeaconDataCache,
                           targetIndex * sizeof(EmberBeaconSurvey),
                           sizeof(EmberBeaconSurvey));
}

bool emSurveyBeaconsNetworkFoundHandler(EmberZigbeeNetwork* networkFound, uint8_t lqi, int8_t rssi, EmberNodeId nodeId, uint8_t parentClassification)
{
  EmberPanId networkPanId = emberGetPanId();
  if (networkFound->panId == networkPanId) {
    EmberBeaconSurvey surveyResult;
    surveyResult.nodeId = nodeId;
    surveyResult.rssi = rssi;
    surveyResult.classificationMask = parentClassification;

    sortBeaconSurveyResult(surveyResult);

    surveyBeaconDataCount++;
  }

  return true;
}

#endif // EZSP_HOST

void emberAfPluginWwahServerSilabsTriggerSurveyBeaconsNetworkEventHandler(void)
{
  #ifndef EZSP_HOST
  EmberStatus status;;
  status = emberSurveyBeacons(false,
                              &emSurveyBeaconsNetworkFoundHandler,
                              &emSurveyBeaconsScanCompleteHandler);

  if (status != EMBER_SUCCESS) {
    emberReleaseMessageBuffer(surveyBeaconDataCache);
    emberAfSlWwahClusterPrintln("Failed to start survey beacon procedure 0x%X", status);
  }

  emberAfNetworkEventControlSetInactive(emberAfPluginWwahServerSilabsTriggerSurveyBeaconsNetworkEventControls);
  #endif //EZSP_HOST
}

bool emberAfSlWwahClusterSurveyBeaconsCallback(uint8_t useStandardBeacons)
{
  EmberStatus status = EMBER_SUCCESS;

  // Survey beacons is currently only available on SoC devices
  #ifndef EZSP_HOST
  // Enhanced beacon support for survey beacons is currently unavailable on 2.4ghz applications
  if (useStandardBeacons == 0) {
    // i.e. use enhanced beacon - unsupported - return INVALID_FIELD per WWAH cluster spec rev 023
    status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
  } else if (setupSurveyBeaconProcedure()) {
    cacheApsFrameData();
    emberAfNetworkEventControlSetDelayMS(emberAfPluginWwahServerSilabsTriggerSurveyBeaconsNetworkEventControls, 1000);
  } else {
    status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  }
  #else
  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  #endif // EZSP_HOST

  if (status != EMBER_SUCCESS) {
    emberReleaseMessageBuffer(surveyBeaconDataCache);
    emberAfSlWwahClusterPrintln("Failed to start survey beacon procedure 0x%X", status);
  }

  return true;
}

static bool isClusterInUseTcForClusterServerToken(uint16_t clusterId)
{
  uint16_t storedClusterId;
  for (uint8_t i = 0; i < MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN; i++) {
    halCommonGetIndexedToken(&storedClusterId, TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, i);
    if (clusterId == storedClusterId) {
      // Found a match
      return true;
    } else if (storedClusterId == PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE) {
      // We always store the supported cluster IDs at the beginning of
      // the UseTcForClusterServerToken. If we see a NULL_VALUE here,
      // it means the rest would be the NULL_VALUE as well,
      // we can return earlier here.
      return false;
    }
  }
  return false;
}

static void syncClusterWithUseTcForClusterServerToken(void)
{
  // UseTcForClusterServer for the OTA cluster
  if (isClusterInUseTcForClusterServerToken(ZCL_OTA_BOOTLOAD_CLUSTER_ID)) {
    emberAfPluginOtaClientSetIgnoreNonTrustCenter(true);
  } else {
    emberAfPluginOtaClientSetIgnoreNonTrustCenter(false);
  }

  // UseTcForClusterServer for the Time cluster
  if (isClusterInUseTcForClusterServerToken(ZCL_TIME_CLUSTER_ID)) {
    ignoreNonTrustCenterForTimeCluster = true;
  } else {
    ignoreNonTrustCenterForTimeCluster = false;
  }

  // UseTcForClusterServer for the Poll Control cluster
  if (isClusterInUseTcForClusterServerToken(ZCL_POLL_CONTROL_CLUSTER_ID)) {
    emberAfPluginPollControlServerSetIgnoreNonTrustCenter(true);
  } else {
    emberAfPluginPollControlServerSetIgnoreNonTrustCenter(false);
  }

  // UseTcForClusterServer for the Keep Alive cluster
  // Note: In our SDK, we now only use the trust center
  // as the Keep Alive cluster server (see trust-center-keepalive.c).
  // If we support non trust center as Keep Alive cluster servers in the future,
  // we need to update this code as well.
}

static EmberAfStatus setUseTcForClusterServerToken(uint8_t* tokenIndex, uint16_t clusterId)
{
  uint16_t storedClusterId;
  // Check the token value first to avoid unnecessary token writing.
  halCommonGetIndexedToken(&storedClusterId, TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, *tokenIndex);
  if ((storedClusterId != clusterId)
      && (*tokenIndex < MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN)) {
    halCommonSetIndexedToken(TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, *tokenIndex, &clusterId);
  }
  (*tokenIndex)++;
  return EMBER_ZCL_STATUS_SUCCESS;
}

static void cleanUseTcForClusterServerToken(uint8_t tokenIndex)
{
  uint16_t clusterId;
  for (uint8_t i = tokenIndex; i < MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN; i++) {
    halCommonGetIndexedToken(&clusterId, TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, i);
    if (clusterId != PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE) {
      clusterId = PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE;
      halCommonSetIndexedToken(TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, tokenIndex, &clusterId);
    }
  }
}

bool emberAfSlWwahClusterUseTrustCenterForClusterServerCallback(uint8_t numberOfClusters,
                                                                uint8_t* clusterId)
{
  EmberStatus status;
  // n clusters * ( 2 bytes for cluster and 1 byte for status)
  uint8_t clusterStatusRsp[MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_RESPONSE_COMMAND * (2 + 1)];
  uint8_t clusterStatusLength = 0;
  uint8_t arrayLen = 0;
  uint8_t tokenIndex = 0;
  uint16_t rcvdClusterId;
  EmberAfStatus afStatus;

  // Response payload:
  // CMD ID 0x9E | CmdStatus | ClusterCount | ClusterId1 | Status1 | ClusterId2 | Status2 |.... | clusterIdn | Statusn |
  //               (1-byte)     (1-byte)        (2-bytes)  (1-bytes)
  for (clusterStatusLength = 0; clusterStatusLength < numberOfClusters
       && clusterStatusLength < MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_RESPONSE_COMMAND;
       clusterStatusLength++) {
    // fetch a clusterId.
    rcvdClusterId = emberFetchLowHighInt16u(clusterId + (clusterStatusLength * 2));

    // Currently we support to force a device to use trust center only for
    // OTA, TIME, POLL_CONTROL and KEEP_ALIVE clusters.
    // All other cluster are unsupported.
    if (rcvdClusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
#if defined(ZCL_USING_TIME_CLUSTER_CLIENT)
        || rcvdClusterId == ZCL_TIME_CLUSTER_ID
#endif
#if defined(ZCL_USING_POLL_CONTROL_CLUSTER_SERVER) && defined(EMBER_AF_PLUGIN_POLL_CONTROL_SERVER)
        || rcvdClusterId == ZCL_POLL_CONTROL_CLUSTER_ID
#endif
#if defined(ZCL_USING_KEEPALIVE_CLUSTER_CLIENT) && defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE)
        || rcvdClusterId == ZCL_KEEPALIVE_CLUSTER_ID
#endif
        ) {
      // Based on WWAH Test Cases, OTA cluster is mandatory for UseTcForClusterServer;
      // Time cluster requires the time client is implemented;
      // Poll Control cluster requires the poll control server is implemented;
      // Keep Alive cluster requires the keep alive client is implemented.
      afStatus = setUseTcForClusterServerToken(&tokenIndex, rcvdClusterId);
    } else {
      afStatus = EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
    }

    // Append clusterId in response payload
    emberStoreLowHighInt16u(clusterStatusRsp + arrayLen, rcvdClusterId);
    arrayLen += 2;
    // Append status to indicate whether a cluster support to use TC or not.
    clusterStatusRsp[arrayLen++] = afStatus;
  }

  // Clean the TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER
  // that could be set values by the previous UseTrustCenterForClusterServer command
  // and the values don't match with the current UseTrustCenterForClusterServer command
  cleanUseTcForClusterServerToken(tokenIndex);

  // Sync the ignoreNonTrustCenter setting of all supported clusters
  // with the UseTcForClusterServerToken
  syncClusterWithUseTcForClusterServerToken();

  emberAfFillCommandSlWwahClusterUseTrustCenterForClusterServerResponse(EMBER_ZCL_STATUS_SUCCESS,
                                                                        clusterStatusLength,
                                                                        clusterStatusRsp,
                                                                        arrayLen);

  status = emberAfSendResponse();
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send UseTrustCenterForClusterServerResponse: 0x%X",
                                status);
  }

  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);

  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

bool emberAfSlWwahClusterTrustCenterForClusterServerQueryCallback(void)
{
  // n clusters * 2 bytes for cluster IDs
  uint8_t clusterRsp[MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN * 2];
  uint8_t arrayLen = 0;
  uint8_t numberOfStoredClusters = 0;
  uint16_t clusterId;
  EmberStatus status;

  // Response payload:
  // CMD ID 0x08 | ClusterCount | ClusterId1 | ClusterId2 | .... | clusterIdn |
  //                (1-byte)       (2-bytes)
  for (uint8_t i = 0; i < MAX_CLUSTER_CAPACITY_FOR_USE_TC_FOR_CLUSTER_SERVER_TOKEN; i++) {
    halCommonGetIndexedToken(&clusterId, TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER, i);
    if (clusterId != PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_NULL_VALUE) {
      emberStoreLowHighInt16u(clusterRsp + arrayLen, clusterId);
      arrayLen += 2;
      numberOfStoredClusters++;
    }
  }

  emberAfFillCommandSlWwahClusterTrustCenterForClusterServerQueryResponse(numberOfStoredClusters, clusterRsp, arrayLen);
  status = emberAfSendResponse();
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send TrustCenterForClusterServerQueryResponse: 0x%X",
                                status);
  }

  status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  if (status != EMBER_SUCCESS) {
    emberAfSlWwahClusterPrintln("Failed to send default success response: 0x%X", status);
  }

  return true;
}

void emberAfSlWwahClusterServerManufacturerSpecificAttributeChangedCallback(uint8_t endpoint,
                                                                            EmberAfAttributeId attributeId,
                                                                            uint16_t manufacturerCode)
{
  if (manufacturerCode != AMAZON_MANUFACTURING_ID) {
    return;
  }

  switch (attributeId) {
    case ZCL_SL_TOUCHLINK_INTERPAN_ENABLED_ATTRIBUTE_ID:
    {
      EmberStatus status;

      // Since touchlink can only be disabled, not enabled, we don't need to read the attribute
      #ifdef EZSP_HOST
      EmberZllPolicy policy;
      status = ezspGetPolicy(EZSP_ZLL_POLICY, &policy);
      if (status == EMBER_SUCCESS) {
        status = (EmberStatus)emberAfSetEzspPolicy(EZSP_ZLL_POLICY,
                                                   policy & ~EMBER_ZLL_POLICY_ENABLED,
                                                   "ZLL policy",
                                                   "disable");
      }
      #else
      status = emberZllSetPolicy(emberZllGetPolicy() & ~EMBER_ZLL_POLICY_ENABLED);
      #endif

      if (status != EMBER_SUCCESS && status != EMBER_LIBRARY_NOT_PRESENT) {
        emberAfSlWwahClusterPrintln("Failed to disable touchlink interpan.");
      }
      break;
    }
    case ZCL_SL_MAC_POLL_FAILURE_WAIT_TIME_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      uint8_t macPollFailureWaitTimeSec;
      status = emberReadWwahServerSilabsAttribute(endpoint,
                                                  ZCL_SL_MAC_POLL_FAILURE_WAIT_TIME_ATTRIBUTE_ID,
                                                  "mac poll failure wait time",
                                                  &macPollFailureWaitTimeSec,
                                                  sizeof(macPollFailureWaitTimeSec));

      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberSetMacPollFailureWaitTime(macPollFailureWaitTimeSec * MILLISECOND_TICKS_PER_SECOND);
      }
      break;
    }
    case ZCL_SL_PENDING_NETWORK_UPDATE_CHANNEL_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      uint8_t channel;
      status = emberReadWwahServerSilabsAttribute(endpoint,
                                                  ZCL_SL_PENDING_NETWORK_UPDATE_CHANNEL_ATTRIBUTE_ID,
                                                  "pending network update channel",
                                                  &channel,
                                                  sizeof(channel));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberSetPendingNetworkUpdateChannel(channel);
      }
      break;
    }
    case ZCL_SL_PENDING_NETWORK_UPDATE_PANID_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      uint16_t panId;
      status = emberReadWwahServerSilabsAttribute(endpoint,
                                                  ZCL_SL_PENDING_NETWORK_UPDATE_PANID_ATTRIBUTE_ID,
                                                  "pending network update panId",
                                                  (uint8_t*)&panId,
                                                  sizeof(panId));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberSetPendingNetworkUpdatePanId(panId);
      }
      break;
    }
    default:
      break;
  }
}

/*
 * -----------------------------------------------------------------------------
 * Unimplemented callbacks
 *
   void emberAfSlWwahClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                           EmberAfAttributeId attributeId)
   {
   }

   void emberAfSlWwahClusterServerMessageSentCallback(EmberOutgoingMessageType type,
                                                      uint16_t indexOrDestination,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t msgLen,
                                                      uint8_t *message,
                                                      EmberStatus status)
   {
   }

   EmberAfStatus emberAfSlWwahClusterServerPreAttributeChangedCallback(uint8_t endpoint,
                                                                       EmberAfAttributeId attributeId,
                                                                       EmberAfAttributeType attributeType,
                                                                       uint8_t size,
                                                                       uint8_t *value)
   {
   }

 * -----------------------------------------------------------------------------
 */
