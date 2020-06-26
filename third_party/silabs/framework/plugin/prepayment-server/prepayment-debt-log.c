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
#include "app/framework/util/common.h"
#include "prepayment-debt-log.h"
#include "prepayment-debt-schedule.h"

// Each entry stores information from a debt collection event where an amount was applied towards debt reduction.
typedef struct {
  uint32_t collectionTime;
  uint32_t amountCollected;
  uint32_t outstandingDebt;
  uint8_t  debtType;    // Indicates if the entry is for Debt #1, #2, or #3
} emDebtLogEntry;

#define DEBT_LOG_TABLE_SIZE  EMBER_AF_PLUGIN_PREPAYMENT_SERVER_DEBT_LOG_CAPACITY
#define DEBT_TYPE_INVALID    0xFE

#define DEBT_RECOVERY_GROUP_DELTA  0x10
#define DEBT_TYPE_INCREMENT_FLAG  0x01
#define DEBT_ATTRIB_NO_CHANGE32  0xFFFFFFFF
#define DEBT_ATTRIB_NO_CHANGE16  0xFFFF
#define DEBT_ATTRIB_NO_CHANGE8  0xFF
enum {
  CHANGE_DEBT_AMOUNT_TYPE_1_ABS = 0x00,
  CHANGE_DEBT_AMOUNT_TYPE_1_INC = 0x01,
  CHANGE_DEBT_AMOUNT_TYPE_2_ABS = 0x02,
  CHANGE_DEBT_AMOUNT_TYPE_2_INC = 0x03,
  CHANGE_DEBT_AMOUNT_TYPE_3_ABS = 0x04,
  CHANGE_DEBT_AMOUNT_TYPE_3_INC = 0x05,
};

static emDebtLogEntry DebtLogTable[DEBT_LOG_TABLE_SIZE];

static uint8_t getNextAvailableDebtLogIndex(void);
static uint8_t getMostRecentDebtLogEntry(uint8_t debtType);
static uint8_t getNthMostRecentDebtLogEntry(uint8_t debtType, uint8_t nth, uint8_t mostRecentIndex);
void emberAfPluginLogDebtCollectionEvent(uint32_t collectionTimeUtc, uint32_t amountCollected, uint32_t outstandingDebt, uint8_t debtType);

static void emberAfPluginPrepaymentServerPublishDebtLog(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                        uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                        DebtPayload *debtPayload);

// Initialize the contents of the Debt Log
void emberAfPluginPrepaymentServerInitDebtLog()
{
  uint8_t i;
  for ( i = 0; i < DEBT_LOG_TABLE_SIZE; i++ ) {
    DebtLogTable[i].collectionTime = 0;
    DebtLogTable[i].amountCollected = 0;
    DebtLogTable[i].outstandingDebt = 0;
    DebtLogTable[i].debtType = DEBT_TYPE_INVALID;
  }
}

// Returns the next index available for use.  Once the log has filled up,
// this will return the oldest log entry.
static uint8_t getNextAvailableDebtLogIndex()
{
  uint8_t i;
  uint32_t minCollectionTime = 0xFFFFFFFF;
  uint8_t  minCollectionTimeIndex = 0;

  for ( i = 0; i < DEBT_LOG_TABLE_SIZE; i++ ) {
    if ( DebtLogTable[i].collectionTime == 0 ) {
      // Found unused log entry.
      minCollectionTimeIndex = i;
      break;
    } else if ( DebtLogTable[i].collectionTime < minCollectionTime ) {
      minCollectionTime = DebtLogTable[i].collectionTime;
      minCollectionTimeIndex = i;
    }
  }
  return minCollectionTimeIndex;
}

// Returns the most recent entry index - meaning, the entry whose
// collection time occurred closest to right now.
static uint8_t getMostRecentDebtLogEntry(uint8_t debtType)
{
  // To do this, find the most recent log entry index.
  // Then decrement (and wrap) the index 'index' times to get to the Nth most recent entry.
  uint8_t i;
  uint32_t minDeltaTime = 0xFFFFFFFF;
  uint8_t  minDeltaTimeIndex = 0xFF;
  uint32_t currentTime;
  uint32_t deltaTime;

  currentTime = emberAfGetCurrentTime();
  for ( i = 0; i < DEBT_LOG_TABLE_SIZE; i++ ) {
    if ( (DebtLogTable[i].collectionTime != 0)
         && ((debtType == DEBT_TYPE_ALL) || (debtType == DebtLogTable[i].debtType)) ) {
      deltaTime = currentTime - DebtLogTable[i].collectionTime;
      if ( deltaTime < minDeltaTime ) {
        minDeltaTime = deltaTime;
        minDeltaTimeIndex = i;
      }
    }
  }
  return minDeltaTimeIndex;
}

// Returns the "nth" most recent occuring entry - meaning, the nth entry
// whose collection time most recently occurred relative to right now.
static uint8_t getNthMostRecentDebtLogEntry(uint8_t debtType, uint8_t nth, uint8_t mostRecentIndex)
{
  // "nth" indicates which of the most recent entries should be retrieved.
  // An 'nth' value of '0' is the newest.
  // The "mostRecentiIndex" parameter should be obtained by calling getMostRecentDebtLogEntry().
  uint8_t i;
  uint8_t nthMostRecentIndex = 0xFF;

  if ( mostRecentIndex < DEBT_LOG_TABLE_SIZE ) {
    i = mostRecentIndex;

    // Go through the log until "nth" found, or until we reach the mostRecentIndex
    while ( 1 ) {
      if ( (DebtLogTable[i].debtType == debtType) || (debtType == DEBT_TYPE_ALL) ) {
        // valid matching entry
        if ( nth == 0 ) {
          nthMostRecentIndex = i;
          break;
        } else {
          nth--;
        }
      }
      if ( i == 0 ) {
        i = DEBT_LOG_TABLE_SIZE - 1;
      } else {
        i--;
      }
      if ( i == mostRecentIndex ) {
        // Wrap occurred - abort.
        break;
      }
    }
  }
  return nthMostRecentIndex;
}

#define DEBT_ATTRIBUTE_GROUP_DELTA  0x10
#define MAX_DEBT_PAYLOAD_LEN  13
#define MAX_DEBTS  3
#define CMD_INDEX_LAST_CMD  0xFE

// Handles the Get Debt Repayment Log command and sends a "Publish Debt Log" response
// command that provides the requested info from the debt log.
bool emberAfPrepaymentClusterGetDebtRepaymentLogCallback(uint32_t latestEndTime, uint8_t numberOfDebts, uint8_t debtType)
{
  EmberNodeId nodeId;
  uint8_t srcEndpoint, dstEndpoint;

  emberAfPrepaymentClusterPrintln("RX: GetDebtLog, endTime=0x%4x, numDebts=%d, debtType=%d", latestEndTime, numberOfDebts, debtType);
  nodeId = emberAfCurrentCommand()->source;
  srcEndpoint = emberAfGetCommandApsFrame()->destinationEndpoint;
  dstEndpoint = emberAfGetCommandApsFrame()->sourceEndpoint;
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);

  emberAfPluginSendPublishDebtLog(nodeId, srcEndpoint, dstEndpoint, latestEndTime, numberOfDebts, debtType);

  return true;
}

enum {
  DEBT_RECOVERY_METHOD_TIME_BASED = 0,
  DEBT_RECOVERY_METHOD_PERCENT_BASED = 1,
  DEBT_RECOVERY_METHOD_CATCH_UP_BASED = 2,
};

void emberAfPluginUpdateDebtOnCollectionEvent(uint8_t endpoint, uint8_t debtType)
{
  // uint8_t i;
  uint16_t attributeId;
  uint8_t debtRecoveryMethod;
  uint32_t debtRecoveryAmount;
  uint32_t debtAmount;
  EmberAfStatus status;
  uint32_t now;
  uint32_t startTime;

  if ( debtType <= DEBT_TYPE_3 ) {
    // Reduce the debt amount if the method is TIME- or CATCH-UP-based.  Do nothing for PERCENTAGE-Based.
    attributeId = ZCL_DEBT_RECOVERY_METHOD_1_ATTRIBUTE_ID + (debtType * DEBT_RECOVERY_GROUP_DELTA);
    status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                  &debtRecoveryMethod, 1, NULL);

    if ( (status == EMBER_ZCL_STATUS_SUCCESS)
         && ((debtRecoveryMethod == DEBT_RECOVERY_METHOD_TIME_BASED) || (debtRecoveryMethod == DEBT_RECOVERY_METHOD_CATCH_UP_BASED)) ) {
      // Time based recovery.
      now = emberAfGetCurrentTime();
      attributeId = ZCL_DEBT_RECOVERY_START_TIME_1_ATTRIBUTE_ID + (debtType * DEBT_RECOVERY_GROUP_DELTA);
      status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                    (uint8_t *)&startTime, 4, NULL);

      if ( (status == EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE)
           || ( (status == EMBER_ZCL_STATUS_SUCCESS) && (now >= startTime) ) ) {
        // Valid time to do collection.
        // Get debt recovery amount and subtract it from the debt amount.
        attributeId = ZCL_DEBT_RECOVERY_AMOUNT_1_ATTRIBUTE_ID + (debtType * DEBT_RECOVERY_GROUP_DELTA);
        status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                      (uint8_t *)&debtRecoveryAmount, 4, NULL);
        attributeId = ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID + (debtType * DEBT_RECOVERY_GROUP_DELTA);
        status |= emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                       (uint8_t *)&debtAmount, 4, NULL);
        if ( (status == EMBER_ZCL_STATUS_SUCCESS) && (debtAmount > 0) && (debtRecoveryAmount > 0) ) {
          //
          if ( debtAmount >= debtRecoveryAmount ) {
            debtAmount -= debtRecoveryAmount;
          } else {
            debtAmount = 0;
          }
          emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&debtAmount, ZCL_INT32U_ATTRIBUTE_TYPE);
          // log the debt transaction to the debt log.
          emberAfPluginLogDebtCollectionEvent(now, debtRecoveryAmount, debtAmount, debtType);
        }
      }
    }
  }
}

void emberAfPluginLogDebtCollectionEvent(uint32_t collectionTimeUtc, uint32_t amountCollected, uint32_t outstandingDebt, uint8_t debtType)
{
  uint8_t i;
  i = getNextAvailableDebtLogIndex();
  if ( i < DEBT_LOG_TABLE_SIZE ) {
    DebtLogTable[i].collectionTime = collectionTimeUtc;
    DebtLogTable[i].amountCollected = amountCollected;
    DebtLogTable[i].outstandingDebt = outstandingDebt;
    DebtLogTable[i].debtType = debtType;    // = debt #0, #1, #2
  }
}

static uint8_t NumDebtPayloads;
// Formats the debt log response data prior to calling the function to send the log data.
void emberAfPluginSendPublishDebtLog(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                     uint32_t latestEndTime, uint8_t numberOfDebts, uint8_t debtType)
{
  DebtPayload debtPayloads[DEBT_LOG_TABLE_SIZE];      // Size this to max number of log entries.
  uint8_t  i, x;
  uint8_t mostRecentIndex;

  // The first returned should be the most recent with its collection time <= latest end time.
  if ( numberOfDebts == 0 ) {
    numberOfDebts = DEBT_LOG_TABLE_SIZE;
  }

  mostRecentIndex = getMostRecentDebtLogEntry(debtType);
  if ( mostRecentIndex >= DEBT_LOG_TABLE_SIZE ) {
    // No entries for this debt type
    emberAfPrepaymentClusterPrintln("No Matching Debt Log Entries");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    // Store debt log entries for transmission
    for ( i = 0; i < numberOfDebts; i++ ) {
      x = getNthMostRecentDebtLogEntry(debtType, i, mostRecentIndex);
      if ( x < DEBT_LOG_TABLE_SIZE ) {
        debtPayloads[i].collectionTime = DebtLogTable[x].collectionTime;
        debtPayloads[i].amountCollected = DebtLogTable[x].amountCollected;
        debtPayloads[i].outstandingDebt = DebtLogTable[x].outstandingDebt;
        debtPayloads[i].debtType        = DebtLogTable[x].debtType;
      } else {
        break;
      }
    }
    // "i" now indicates how many responses were put into debtPayloads[].
    NumDebtPayloads = i;
    emberAfPrepaymentClusterPrintln("Send Publish Debt Log, %d entries", NumDebtPayloads);
    emberAfPluginPrepaymentServerPublishDebtLog(nodeId, srcEndpoint, dstEndpoint, CMD_INDEX_LAST_CMD, 1, debtPayloads);
  }
}

// Sends the Publish Debt Log command.
// NOTE:  DebtPayload is a struct type.  See se-meter-gas/gen/simulation/af-structs.h
static void emberAfPluginPrepaymentServerPublishDebtLog(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                        uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                        DebtPayload *debtPayload)
{
  EmberStatus status;
  // uint16_t debtPayloadLen=0;
  uint8_t i;

  // For GBCS use cases, we should be setting the disable default response bit.
  emberAfFillExternalBuffer( (ZCL_CLUSTER_SPECIFIC_COMMAND
                              | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef EMBER_AF_GBCS_COMPATIBLE
                              | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                              ),
                             ZCL_PREPAYMENT_CLUSTER_ID,
                             ZCL_PUBLISH_DEBT_LOG_COMMAND_ID,
                             "uu",
                             commandIndex, totalNumberOfCommands);
  for ( i = 0; i < NumDebtPayloads; i++ ) {
    emberAfPutInt32uInResp(debtPayload[i].collectionTime);
    emberAfPutInt32uInResp(debtPayload[i].amountCollected);
    emberAfPutInt8uInResp(debtPayload[i].debtType);
    emberAfPutInt32uInResp(debtPayload[i].outstandingDebt);
  }

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if ( status != EMBER_SUCCESS ) {
    emberAfPrepaymentClusterPrintln("Error in Publish Debt Log %x", status);
  }
}

// Handles the prepayment Change Debt command.
bool emberAfPrepaymentClusterChangeDebtCallback(uint32_t issuerEventId,
                                                uint8_t* debtLabel,
                                                uint32_t debtAmount,
                                                uint8_t debtRecoveryMethod,
                                                uint8_t debtAmountType,
                                                uint32_t debtRecoveryStartTime,
                                                uint16_t debtRecoveryCollectionTime,
                                                uint8_t debtRecoveryFrequency,
                                                uint32_t debtRecoveryAmount,
                                                uint16_t debtRecoveryBalancePercentage)
{
  uint8_t endpoint;
  uint16_t attributeId;
  uint8_t  attributeGroupOffset;
  uint32_t currentDebt;
  uint8_t  debtType;

  endpoint = emberAfCurrentEndpoint();

  // Look at the debtAmountType field to see how to process the new debt -
  // could be applied incrementally or absolutely.
  // The debtAmountType also indicates which debt type is being updated.

  if ( debtAmountType <= CHANGE_DEBT_AMOUNT_TYPE_1_INC ) {
    attributeGroupOffset = 0;   // DEBT_#1
    debtType = 0;
  } else if ( debtAmountType <= CHANGE_DEBT_AMOUNT_TYPE_2_INC ) {
    attributeGroupOffset = DEBT_RECOVERY_GROUP_DELTA;   // DEBT_#2
    debtType = 1;
  } else if ( debtAmountType <= CHANGE_DEBT_AMOUNT_TYPE_3_INC ) {
    attributeGroupOffset = (DEBT_RECOVERY_GROUP_DELTA * 2);   // DEBT_#3
    debtType = 2;
  } else {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // TODO:  Ensure the event Id is > any previous event ID for this debt type.

  currentDebt = 0;
  if ( (debtAmount != DEBT_ATTRIB_NO_CHANGE32) && (debtAmountType & DEBT_TYPE_INCREMENT_FLAG) ) {
    attributeId = ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId,
                         CLUSTER_MASK_SERVER, (uint8_t *)&currentDebt, 4, NULL);

    debtAmount += currentDebt;    // Increment new value to the current debt.
  }

  // Update attribute values, but only if they are not set to NO_CHANGES values
  if ( debtLabel[0] != 0xFF ) {
    attributeId = ZCL_DEBT_LABEL_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          debtLabel, ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
  }

  if ( debtAmount != DEBT_ATTRIB_NO_CHANGE32 ) {
    attributeId = ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          (uint8_t *)&debtAmount, ZCL_INT32U_ATTRIBUTE_TYPE);
  }

  if ( debtRecoveryMethod != DEBT_ATTRIB_NO_CHANGE8 ) {
    attributeId = ZCL_DEBT_RECOVERY_METHOD_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          &debtRecoveryMethod, ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  if ( debtRecoveryStartTime != DEBT_ATTRIB_NO_CHANGE32 ) {
    attributeId = ZCL_DEBT_RECOVERY_START_TIME_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          (uint8_t *)&debtRecoveryStartTime, ZCL_UTC_TIME_ATTRIBUTE_TYPE);
  } else {
    attributeId = ZCL_DEBT_RECOVERY_START_TIME_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                         (uint8_t *)&debtRecoveryStartTime, 4, NULL);
  }

  if ( debtRecoveryCollectionTime != DEBT_ATTRIB_NO_CHANGE16 ) {
    attributeId = ZCL_DEBT_RECOVERY_COLLECTION_TIME_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          (uint8_t *)&debtRecoveryCollectionTime, ZCL_INT16U_ATTRIBUTE_TYPE);
  } else {
    attributeId = ZCL_DEBT_RECOVERY_COLLECTION_TIME_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                         (uint8_t *)&debtRecoveryCollectionTime, 2, NULL);
  }

  if ( debtRecoveryFrequency != DEBT_ATTRIB_NO_CHANGE8 ) {
    attributeId = ZCL_DEBT_RECOVERY_FREQUENCY_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          &debtRecoveryFrequency, ZCL_INT8U_ATTRIBUTE_TYPE);
  } else {
    attributeId = ZCL_DEBT_RECOVERY_FREQUENCY_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                         &debtRecoveryFrequency, 1, NULL);
  }

  if ( debtRecoveryAmount != DEBT_ATTRIB_NO_CHANGE32 ) {
    attributeId = ZCL_DEBT_RECOVERY_AMOUNT_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          (uint8_t *)&debtRecoveryAmount, ZCL_INT32U_ATTRIBUTE_TYPE);
  }

  if ( debtRecoveryBalancePercentage != DEBT_ATTRIB_NO_CHANGE16 ) {
    attributeId = ZCL_DEBT_RECOVERY_TOP_UP_PERCENTAGE_1_ATTRIBUTE_ID + attributeGroupOffset;
    emberAfWriteAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                          (uint8_t *)&debtRecoveryBalancePercentage, ZCL_INT16U_ATTRIBUTE_TYPE);
  }
  emberAfPrepaymentClusterPrintln("RX Change Debt");
  emberAfPluginPrepaymentServerScheduleDebtRepayment(endpoint, issuerEventId, debtType, debtRecoveryCollectionTime,
                                                     debtRecoveryStartTime, debtRecoveryFrequency);
  return true;
}

void emberAfPluginPrepaymentPrintDebtLogIndex(uint8_t index)
{
  if ( index < DEBT_LOG_TABLE_SIZE ) {
    emberAfPrepaymentClusterPrintln("= Debt Log Entry %d:", index);
    emberAfPrepaymentClusterPrintln("  collectionTime=0x%4x", DebtLogTable[index].collectionTime);
    emberAfPrepaymentClusterPrintln("  amountCollected=%d", DebtLogTable[index].amountCollected);
    emberAfPrepaymentClusterPrintln("  outstandingDebt=%d", DebtLogTable[index].outstandingDebt);
    emberAfPrepaymentClusterPrintln("  debtType=%d", DebtLogTable[index].debtType);
  } else {
    emberAfPrepaymentClusterPrintln("ERROR: Debt Log index %d out of bounds", index);
  }
}

// When a Top Up occurs, check if a percentage of the Top Off must be applied towards debt.
// Return that amount.
uint32_t emberAfPluginPrepaymentGetDebtRecoveryTopUpPercentage(uint8_t endpoint, uint32_t topUpValue)
{
  EmberAfStatus status;
  uint32_t reduction;
  uint16_t attributeId;
  uint16_t percentage;
  uint16_t totalPercentage = 0;
  uint8_t  recoveryMethod;
  uint8_t  i;

  for ( i = 0; i < 3; i++ ) {
    attributeId = ZCL_DEBT_RECOVERY_TOP_UP_PERCENTAGE_1_ATTRIBUTE_ID + (i * DEBT_RECOVERY_GROUP_DELTA);
    status =  emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                   (uint8_t *)&percentage, 2, NULL);

    attributeId = ZCL_DEBT_RECOVERY_METHOD_1_ATTRIBUTE_ID + (i * DEBT_RECOVERY_GROUP_DELTA);
    status |= emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                   (uint8_t *)&recoveryMethod, 1, NULL);
    if ( (status == EMBER_ZCL_STATUS_SUCCESS) && (recoveryMethod == DEBT_RECOVERY_METHOD_PERCENT_BASED) ) {
      totalPercentage += percentage;
    }
  }
  // TOP_UP_PERCENTAGE is a u16, stored as xxx.xxx%.  Max value is 100.00%, or 10,000, or 0x2710.
  // To compute the actual top up reduction, multiply by the total percentage, then divide by 10,000.
  // Values are rounded down to the nearest integer.
  if ( totalPercentage ) {
    reduction = (topUpValue * totalPercentage) / 10000;
    emberAfPrepaymentClusterPrintln("Appying Debt Recovery to Top Up %d percent, or %d", (totalPercentage / 100), reduction);
  } else {
    emberAfPrepaymentClusterPrintln("No Top Up Debt Recovery found");
    reduction = 0;
  }
  return reduction;
}

void emberAfPluginPrepaymentPrintDebtAttributes(uint8_t endpoint, uint8_t index)
{
  uint16_t attributeId;
  EmberAfStatus status;
  uint32_t val32u;
  uint16_t val16u;
  uint8_t  val8u;
  uint8_t  valString[14];

  if ( index >= 3 ) {
    emberAfPrepaymentClusterPrintln("= ERROR: Debt Attrib Set %d out of bounds", index);
    return;
  }

  emberAfPrepaymentClusterPrintln("= Debt Attribute Set #%d:", index);

  attributeId = ZCL_DEBT_LABEL_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&valString, 14, NULL);

  emberAfPrepaymentClusterPrintln("  DebtLabel status=0x%x, val=0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x...", status, valString[0], valString[1],
                                  valString[2], valString[3], valString[4], valString[5], valString[6], valString[7],
                                  valString[8], valString[9], valString[10], valString[11], valString[12], valString[13]);

  attributeId = ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val32u, 4, NULL);
  emberAfPrepaymentClusterPrintln("  DebtAmount status=0x%x, val=%d", status, val32u);

  attributeId = ZCL_DEBT_RECOVERY_METHOD_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val8u, 1, NULL);
  emberAfPrepaymentClusterPrintln("  RecovMethod status=0x%x, val=%d", status, val8u);

  attributeId = ZCL_DEBT_RECOVERY_START_TIME_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val32u, 4, NULL);
  emberAfPrepaymentClusterPrintln("  RecovStartTime status=0x%x, val=%d", status, val32u);

  attributeId = ZCL_DEBT_RECOVERY_COLLECTION_TIME_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val16u, 2, NULL);
  emberAfPrepaymentClusterPrintln("  RecovCollectTime status=0x%x, val=%d", status, val16u);

  attributeId = ZCL_DEBT_RECOVERY_FREQUENCY_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val8u, 1, NULL);
  emberAfPrepaymentClusterPrintln("  RecovFreq status=0x%x, val=%d", status, val8u);

  attributeId = ZCL_DEBT_RECOVERY_AMOUNT_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val32u, 4, NULL);
  emberAfPrepaymentClusterPrintln("  RecovAmount status=0x%x, val=%d", status, val32u);

  attributeId = ZCL_DEBT_RECOVERY_TOP_UP_PERCENTAGE_1_ATTRIBUTE_ID + (index * DEBT_RECOVERY_GROUP_DELTA);
  status = emberAfReadAttribute(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, attributeId, CLUSTER_MASK_SERVER,
                                (uint8_t *)&val16u, 2, NULL);
  emberAfPrepaymentClusterPrintln("  RecovPercent status=0x%x, val=%d", status, val16u);
}
