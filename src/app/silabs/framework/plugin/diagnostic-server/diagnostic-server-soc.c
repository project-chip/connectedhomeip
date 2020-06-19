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
 * @brief Routines for the Diagnostic Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/counters/counters.h"
#include "diagnostic-server.h"
#include "app/framework/util/attribute-storage.h"
#include "app/util/common/common.h"

bool emberAfReadDiagnosticAttribute(
  EmberAfAttributeMetadata *attributeMetadata,
  uint8_t *buffer)
{
  uint8_t emberCounter = EMBER_COUNTER_TYPE_COUNT;
  EmberStatus status;

  switch (attributeMetadata->attributeId) {
    case ZCL_MAC_RX_BCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_RX_BROADCAST;
      break;
    case ZCL_MAC_TX_BCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_TX_BROADCAST;
      break;
    case ZCL_MAC_RX_UCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_RX_UNICAST;
      break;
    case ZCL_MAC_TX_UCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS;
      break;
    case ZCL_MAC_TX_UCAST_RETRY_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_TX_UNICAST_RETRY;
      break;
    case ZCL_MAC_TX_UCAST_FAIL_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_MAC_TX_UNICAST_FAILED;
      break;
    case ZCL_APS_RX_BCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_RX_BROADCAST;
      break;
    case ZCL_APS_TX_BCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_TX_BROADCAST;
      break;
    case ZCL_APS_RX_UCAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_RX_UNICAST;
      break;
    case ZCL_APS_UCAST_SUCCESS_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS;
      break;
    case ZCL_APS_TX_UCAST_RETRY_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY;
      break;
    case ZCL_APS_TX_UCAST_FAIL_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED;
      break;
    case ZCL_ROUTE_DISC_INITIATED_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_ROUTE_DISCOVERY_INITIATED;
      break;
    case ZCL_NEIGHBOR_ADDED_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_NEIGHBOR_ADDED;
      break;
    case ZCL_NEIGHBOR_REMOVED_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_NEIGHBOR_REMOVED;
      break;
    case ZCL_NEIGHBOR_STALE_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_NEIGHBOR_STALE;
      break;
    case ZCL_JOIN_INDICATION_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_JOIN_INDICATION;
      break;
    case ZCL_CHILD_MOVED_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_CHILD_REMOVED;
      break;
    case ZCL_NWK_FC_FAILURE_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_NWK_FRAME_COUNTER_FAILURE;
      break;
    case ZCL_APS_FC_FAILURE_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_FRAME_COUNTER_FAILURE;
      break;
    case ZCL_APS_UNAUTHORIZED_KEY_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED;
      break;
    case ZCL_NWK_DECRYPT_FAILURE_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_NWK_DECRYPTION_FAILURE;
      break;
    case ZCL_APS_DECRYPT_FAILURE_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_APS_DECRYPTION_FAILURE;
      break;
    case ZCL_PACKET_BUFFER_ALLOC_FAILURES_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_ALLOCATE_PACKET_BUFFER_FAILURE;
      break;
    case ZCL_RELAYED_UNICAST_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_RELAYED_UNICAST;
      break;
    case ZCL_PHY_TO_MAC_QUEUE_LIMIT_REACHED_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED;
      break;
    case ZCL_PACKET_VALIDATE_DROP_COUNT_ATTRIBUTE_ID:
      emberCounter = EMBER_COUNTER_PACKET_VALIDATE_LIBRARY_DROPPED_COUNT;
      break;
    default:
      break;
  }
  if (emberCounter < EMBER_COUNTER_TYPE_COUNT) {
    if (attributeMetadata->size == 2) {
      emberStoreLowHighInt16u(buffer, emberCounters[emberCounter]);
    } else if (attributeMetadata->size == 4) {
      emberStoreLowHighInt32u(buffer, emberCounters[emberCounter]);
    }
    return true;
  }
  // code for handling diagnostic attributes that need to be computed
  switch (attributeMetadata->attributeId) {
    case ZCL_NUMBER_OF_RESETS_ATTRIBUTE_ID:
    {
      tokTypeStackBootCounter rebootCounter;

      uint16_t rebootCounter16;

      halCommonGetToken(&rebootCounter, TOKEN_STACK_BOOT_COUNTER);

      // in case we are using simee2 and the reboot counter is an uint32_t.
      rebootCounter16 = (uint16_t) rebootCounter;

      emberStoreLowHighInt16u(buffer, rebootCounter16);

      return true;
    }
    break;
    case ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID:
    {
      uint32_t scratch;
      uint16_t macRetriesPerAps;

      scratch = emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS]
                + emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED];
      if (scratch > 0) {
        scratch = emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY]
                  / scratch;
      }

      macRetriesPerAps = (uint16_t) scratch;
      emberStoreLowHighInt16u(buffer, macRetriesPerAps);
      return true;
    }
    break;
    case ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID:
      status = emberGetLastHopLqi(buffer);
      assert(status == EMBER_ZCL_STATUS_SUCCESS);

      emberAfCorePrintln("LQI:  %x %x", buffer[0]);

      return true;
      break;
    case ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID:
    {
      int8_t rssi;
      status = emberGetLastHopRssi(&rssi);
      assert(status == EMBER_ZCL_STATUS_SUCCESS);

      buffer[0] = (uint8_t) rssi;

      emberAfCorePrintln("RSSI:  %x %x", buffer[0]);

      return true;
    }
    break;
    default:
      break;
  }

  return false;
}

/** @brief External Attribute Read
 *
 * Like emberAfExternalAttributeWriteCallback above, this function is called
 * when the framework needs to read an attribute that is not stored within the
 * Application Framework's data structures.
        All of the important
 * information about the attribute itself is passed as a pointer to an
 * EmberAfAttributeMetadata struct, which is stored within the application and
 * used to manage the attribute. A complete description of the
 * EmberAfAttributeMetadata struct is provided in
 * app/framework/include/af-types.h
        This function assumes that the
 * application is able to read the attribute, write it into the passed buffer,
 * and return immediately. Any attributes that require a state machine for
 * reading and writing are not really candidates for externalization at the
 * present time. The Application Framework does not currently include a state
 * machine for reading or writing attributes that must take place across a
 * series of application ticks. Attributes that cannot be read in a timely
 * manner should be stored within the Application Framework and updated
 * occasionally by the application code from within the
 * emberAfMainTickCallback.
        If the application was successfully able
 * to read the attribute and write it into the passed buffer, it should return
 * a value of EMBER_ZCL_STATUS_SUCCESS. Any other return value indicates the
 * application was not able to read the attribute.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfStackDiagnosticAttributeReadCallback(uint8_t endpoint,
                                                          EmberAfClusterId clusterId,
                                                          EmberAfAttributeMetadata * attributeMetadata,
                                                          uint16_t manufacturerCode,
                                                          uint8_t * buffer)
{
  if (emberAfReadDiagnosticAttribute(attributeMetadata, buffer)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  } else {
    return EMBER_ZCL_STATUS_FAILURE;
  }
}

bool emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage)
{
  EmberStatus status;
  EmberAfStatus afStatus;
  uint8_t data;
  uint16_t macRetriesPerAps;

  uint8_t fixedEndpoints[] = FIXED_ENDPOINT_ARRAY;

  uint8_t endpoint = fixedEndpoints[0];

  // grab last hop LQI and RSSI here.  write them to the framework.
  status = emberGetLastHopLqi(&data);
  if (EMBER_SUCCESS == status) {
    afStatus = emberAfWriteServerAttribute(
      endpoint,
      ZCL_DIAGNOSTICS_CLUSTER_ID,                                     // 0x0b05
      ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID,                              // 0x011c
      &data,
      ZCL_INT8U_ATTRIBUTE_TYPE);                                      // 0x20

    if (EMBER_ZCL_STATUS_SUCCESS != afStatus) {
      emberAfCorePrintln("Diagnostic Server (SOC): failed to write value 0x%x "
                         " to cluster 0x%x attribute ID 0x%x: error 0x%x",
                         data,
                         ZCL_DIAGNOSTICS_CLUSTER_ID,
                         ZCL_LAST_MESSAGE_LQI_ATTRIBUTE_ID,
                         afStatus);
    }
  } else {
    emberAfCorePrintln("Diagnostic Server (SOC): failed to get last hop LQI: "
                       "error 0x%x", status);
  }

  status = emberGetLastHopRssi((int8_t *)&data);
  if (EMBER_SUCCESS == status) {
    afStatus = emberAfWriteServerAttribute(
      endpoint,
      ZCL_DIAGNOSTICS_CLUSTER_ID,                                     // 0x0b05
      ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID,                             // 0x011d
      &data,
      ZCL_INT8S_ATTRIBUTE_TYPE);                                      // 0x28

    if (EMBER_ZCL_STATUS_SUCCESS != afStatus) {
      emberAfCorePrintln("Diagnostic Server (SOC): failed to write value 0x%x "
                         " to cluster 0x%x attribute ID 0x%x: error 0x%x",
                         data,
                         ZCL_DIAGNOSTICS_CLUSTER_ID,
                         ZCL_LAST_MESSAGE_RSSI_ATTRIBUTE_ID,
                         afStatus);
    }
  } else {
    emberAfCorePrintln("Diagnostic Server (SOC): failed to get last hop RSSI: "
                       "error 0x%x", status);
  }

  {
    uint32_t scratch;

    scratch = emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS]
              + emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED];
    if (scratch > 0) {
      scratch = emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY]
                / scratch;
    }

    macRetriesPerAps = (uint16_t)scratch;
    afStatus = emberAfWriteServerAttribute(
      endpoint,
      ZCL_DIAGNOSTICS_CLUSTER_ID,                     // 0x0b05
      ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID,             // 0x011b
      (uint8_t *) &macRetriesPerAps,
      ZCL_INT16U_ATTRIBUTE_TYPE);                      // 0x21

    if (EMBER_ZCL_STATUS_SUCCESS != afStatus) {
      emberAfCorePrintln("Diagnostic Server (SOC): failed to write value 0x%x "
                         " to cluster 0x%x attribute ID 0x%x: error 0x%x",
                         macRetriesPerAps,
                         ZCL_DIAGNOSTICS_CLUSTER_ID,
                         ZCL_AVERAGE_MAC_RETRY_PER_APS_MSG_SENT_ATTRIBUTE_ID,
                         afStatus);
    }
  }

  return false;
}
