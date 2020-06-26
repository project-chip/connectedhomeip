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
 * @brief Implemented routines for prepayment server.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "prepayment-server.h"
#include "prepayment-debt-log.h"
#include "prepayment-debt-schedule.h"
#include "prepayment-modes-table.h"
#include "../calendar-client/calendar-client.h"

typedef uint16_t PaymentControlConfiguration;
typedef uint8_t  FriendlyCredit;

extern void emberAfPluginPrepaymentSnapshotStorageInitCallback(uint8_t endpoint);
extern uint8_t emberAfPluginPrepaymentServerGetSnapshotCallback(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                                uint32_t startTime,
                                                                uint32_t endTime,
                                                                uint8_t  snapshotOffset,
                                                                uint32_t snapshotCause);

void emberAfPrepaymentClusterServerInitCallback(uint8_t endpoint)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PREPAYMENT_CLUSTER_ID);

  emberAfPluginPrepaymentSnapshotStorageInitCallback(endpoint);
  emInitPrepaymentModesTable();
  emberAfPluginPrepaymentServerInitDebtLog();
  emberAfPluginPrepaymentServerInitDebtSchedule();

  if ( ep == 0xFF ) {
    return;
  }
}

bool emberAfPrepaymentClusterSelectAvailableEmergencyCreditCallback(uint32_t commandIssueDateTime,
                                                                    uint8_t originatingDevice)
{
  emberAfPrepaymentClusterPrintln("Rx: Select Available Emergency Credit");
  return true;
}

#define CUTOFF_UNCHANGED 0xFFFFFFFF
bool emberAfPrepaymentClusterChangePaymentModeCallback(uint32_t providerId,
                                                       uint32_t issuerEventId,
                                                       uint32_t implementationDateTime,
                                                       PaymentControlConfiguration proposedPaymentControlConfiguration,
                                                       uint32_t cutOffValue)
{
  // The requester can be obtained with emberAfResponseDestination;
  EmberNodeId nodeId;
  uint8_t endpoint;
  uint8_t srcEndpoint, dstEndpoint;
  FriendlyCredit friendlyCredit;
  uint32_t friendlyCreditCalendarId;
  uint32_t emergencyCreditLimit;
  uint32_t emergencyCreditThreshold;
  uint8_t  dataType;
  uint8_t  i;

  emberAfPrepaymentClusterPrintln("RX: ChangePaymentMode, pid=0x%4x, eid=0x%4x, cfg=0x%2x", providerId, issuerEventId, proposedPaymentControlConfiguration);
  endpoint = emberAfCurrentEndpoint();

  if ( cutOffValue != CUTOFF_UNCHANGED ) {
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CUT_OFF_VALUE_ATTRIBUTE
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID,
                          ZCL_CUT_OFF_VALUE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                          (uint8_t *)&cutOffValue, ZCL_INT32S_ATTRIBUTE_TYPE);
#endif
  }

  emberAfPrepaymentSchedulePrepaymentMode(emberAfCurrentEndpoint(), providerId, issuerEventId, implementationDateTime,
                                          proposedPaymentControlConfiguration);

  // Setup the friendly credit & emergency credit limit attributes.
#ifdef EMBER_AF_PLUGIN_CALENDAR_CLIENT
  i = emberAfPluginCalendarClientGetCalendarIndexByType(endpoint, EMBER_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR);
  friendlyCredit = (i < EMBER_AF_PLUGIN_CALENDAR_CLIENT_CALENDARS) ? 0x01 : 0x00;
  friendlyCreditCalendarId = emberAfPluginCalendarClientGetCalendarId(endpoint, i);
#else
  friendlyCredit = 0x00;
  friendlyCreditCalendarId = EMBER_AF_PLUGIN_CALENDAR_CLIENT_INVALID_CALENDAR_ID;
#endif

#if (!defined ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE) \
  || (!defined ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE)
#error "Prepayment Emergency Credit Limit/Allowance and Threshold attributes required for this plugin!"
#endif
  emberAfReadAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                       ZCL_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                       (uint8_t *)&emergencyCreditLimit, 4, &dataType);
  emberAfReadAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                       ZCL_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                       (uint8_t *)&emergencyCreditThreshold, 4, &dataType);
  nodeId = emberAfCurrentCommand()->source;
  srcEndpoint = emberAfGetCommandApsFrame()->destinationEndpoint;
  dstEndpoint = emberAfGetCommandApsFrame()->sourceEndpoint;
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

#ifdef EMBER_AF_GBCS_COMPATIBLE
  // GBCS explicitly lists some commands that need to be sent with "disable
  // default response" flag set. This is one of them.
  // We make it conditional on GBCS so it does not affect standard SE apps.
  emberAfSetDisableDefaultResponse(EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

  emberAfFillCommandPrepaymentClusterChangePaymentModeResponse(friendlyCredit, friendlyCreditCalendarId,
                                                               emergencyCreditLimit, emergencyCreditThreshold);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  return true;
}

bool emberAfPrepaymentClusterEmergencyCreditSetupCallback(uint32_t issuerEventId,
                                                          uint32_t startTime,
                                                          uint32_t emergencyCreditLimit,
                                                          uint32_t emergencyCreditThreshold)
{
  emberAfPrepaymentClusterPrintln("Rx: Emergency Credit Setup");
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE
  emberAfWriteAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                        ZCL_EMERGENCY_CREDIT_LIMIT_ALLOWANCE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                        (uint8_t *)&emergencyCreditLimit, ZCL_INT32U_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Emergency Credit Limit Allowance attribute is required for this plugin."
#endif

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE
  emberAfWriteAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                        ZCL_EMERGENCY_CREDIT_THRESHOLD_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                        (uint8_t *)&emergencyCreditThreshold, ZCL_INT32U_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Emergency Credit Threshold attribute is required for this plugin."
#endif
  return true;
}

enum {
  CREDIT_ADJUSTMENT_TYPE_INCREMENTAL = 0x00,
  CREDIT_ADJUSTMENT_TYPE_ABSOLUTE    = 0x01,
};

bool emberAfPrepaymentClusterCreditAdjustmentCallback(uint32_t issuerEventId,
                                                      uint32_t startTime,
                                                      uint8_t creditAdjustmentType,
                                                      uint32_t creditAdjustmentValue)
{
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE
  uint32_t currTimeUtc;
#endif
  int32_t currCreditAdjustmentValue;
  uint8_t  dataType;

  emberAfPrepaymentClusterPrintln("Rx: Credit Adjustment");
#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_ATTRIBUTE
  if ( creditAdjustmentType == CREDIT_ADJUSTMENT_TYPE_INCREMENTAL ) {
    // Read current value, then add it to the adjustment.
    emberAfReadAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                         ZCL_CREDIT_REMAINING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                         (uint8_t *)&currCreditAdjustmentValue, 4, &dataType);
    currCreditAdjustmentValue += creditAdjustmentValue;
  }

  emberAfWriteAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                        ZCL_CREDIT_REMAINING_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                        (uint8_t *)&creditAdjustmentValue, ZCL_INT32S_ATTRIBUTE_TYPE);
#else
  #error "Prepayment Credit Adjustment attribute is required for this plugin."
#endif

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE
  // This one is optional - we'll track it if supported.
  currTimeUtc = emberAfGetCurrentTime();
  emberAfWriteAttribute(emberAfCurrentEndpoint(), ZCL_PREPAYMENT_CLUSTER_ID,
                        ZCL_CREDIT_REMAINING_TIMESTAMP_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                        (uint8_t *)&currTimeUtc, ZCL_UTC_TIME_ATTRIBUTE_TYPE);
#endif

  return true;
}

#define MAX_SNAPSHOT_PAYLOAD_LEN  24
bool emberAfPrepaymentClusterGetPrepaySnapshotCallback(uint32_t earliestStartTime, uint32_t latestEndTime,
                                                       uint8_t snapshotOffset, uint32_t snapshotCause)
{
  EmberNodeId nodeId;
  uint8_t srcEndpoint, dstEndpoint;

  emberAfPrepaymentClusterPrintln("RX: GetPrepaySnapshot, st=0x%4x, offset=%d, cause=%d", earliestStartTime, snapshotOffset, snapshotCause);
  nodeId = emberAfCurrentCommand()->source;
  srcEndpoint = emberAfGetCommandApsFrame()->destinationEndpoint;
  dstEndpoint = emberAfGetCommandApsFrame()->sourceEndpoint;
  emberAfPrepaymentClusterPrintln("... from 0x%2x, ep=%d", nodeId, dstEndpoint);

  emberAfPluginPrepaymentServerGetSnapshotCallback(nodeId, srcEndpoint, dstEndpoint,
                                                   earliestStartTime, latestEndTime, snapshotOffset, snapshotCause);
  return true;
}
