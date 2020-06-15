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
 * @brief Routines for interacting with the prepayment-server.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "prepayment-client.h"

/******
   TEMPORARY FIX -
    According to Bob, we believe these types should be auto-generated and put into enums.h
    Currently that is not the case.  For now, define these here.
******/

typedef uint16_t PaymentControlConfiguration;
typedef uint32_t PrepaySnapshotPayloadCause;
typedef uint8_t  PrepaySnapshotPayloadType;
typedef uint8_t  FriendlyCredit;

void emAfPrepaymentClientCliChangePaymentMode(void);

void emAfPrepaymentClientCliChangePaymentMode()
{
  EmberNodeId nodeId;
  uint8_t srcEndpoint, dstEndpoint;
  uint32_t providerId, issuerEventId;
  uint32_t implementationDateTime;
  PaymentControlConfiguration proposedPaymentControlConfiguration;
  uint32_t cutOffValue;

  nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  providerId = (uint32_t)emberUnsignedCommandArgument(3);
  issuerEventId = (uint32_t)emberUnsignedCommandArgument(4);

  implementationDateTime = (uint32_t)emberUnsignedCommandArgument(5);
  proposedPaymentControlConfiguration = (PaymentControlConfiguration)emberUnsignedCommandArgument(6);
  cutOffValue = (uint32_t)emberUnsignedCommandArgument(7);

  //emberAfAppPrintln("RX Publish Prepay Snapshot Cmd, varLen=%d", i );
  emberAfAppPrintln("Change Payment Mode, srcEp=%d, dstEp=%d, addr=%2x", srcEndpoint, dstEndpoint, nodeId);
  emberAfPluginPrepaymentClientChangePaymentMode(nodeId, srcEndpoint, dstEndpoint, providerId, issuerEventId,
                                                 implementationDateTime, proposedPaymentControlConfiguration,
                                                 cutOffValue);
}
