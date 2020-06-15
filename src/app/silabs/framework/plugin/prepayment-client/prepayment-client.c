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
 * @brief Routines for the Prepayment Client plugin, which implements the client
 *        side of the Prepayment cluster.
 *******************************************************************************
   ******************************************************************************/

// *****************************************************************************
// * prepayment-client.c
// *
// * Implemented routines for prepayment client.
// *
// * Copyright 2014 by Silicon Laboratories, Inc.
// *****************************************************************************

#include "app/framework/include/af.h"
#include "prepayment-client.h"

void emberAfPluginPrepaymentClientChangePaymentMode(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint32_t providerId, uint32_t issuerEventId, uint32_t implementationDateTime, uint16_t proposedPaymentControlConfiguration, uint32_t cutOffValue)
{
  EmberStatus status;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(srcEndpoint, ZCL_PREPAYMENT_CLUSTER_ID);
  if ( ep == 0xFF ) {
    emberAfAppPrintln("==== NO PREPAYMENT CLIENT ENDPOINT");
    return;
  }

  emberAfFillCommandPrepaymentClusterChangePaymentMode(providerId, issuerEventId, implementationDateTime,
                                                       proposedPaymentControlConfiguration, cutOffValue);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  emberAfAppPrintln("=====   SEND PAYMENT MODE stat=0x%x", status);
}

bool emberAfPrepaymentClusterChangePaymentModeResponseCallback(uint8_t friendlyCredit, uint32_t friendlyCreditCalendarId,
                                                               uint32_t emergencyCreditLimit, uint32_t emergencyCreditThreshold)
{
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  emberAfAppPrintln("RX: Change Payment Mode Response Callback");
  return true;
}

bool emberAfPrepaymentClusterPublishPrepaySnapshotCallback(uint32_t snapshotId, uint32_t snapshotTime, uint8_t totalSnapshotsFound,
                                                           uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                           uint32_t snapshotCause, uint8_t snapshotPayloadType, uint8_t *snapshotPayload)
{
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  emberAfAppPrintln("RX: Publish Prepay Snapshot Callback");
  return true;
}

bool emberAfPrepaymentClusterPublishTopUpLogCallback(uint8_t commandIndex, uint8_t totalNumberOfCommands, uint8_t *topUpPayload)
{
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  emberAfAppPrintln("RX: Publish TopUp Log Callback");
  return true;
}

bool emberAfPrepaymentClusterPublishDebtLogCallback(uint8_t commandIndex, uint8_t totalNumberOfCommands, uint8_t *debtPayload)
{
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  emberAfAppPrintln("RX: Publish Debt Log Callback");
  emberAfPrepaymentClusterPrintln("  commandIndex=%d", commandIndex);
  emberAfPrepaymentClusterPrintln("  totalNumberOfCommands=%d", totalNumberOfCommands);
  return true;
}
