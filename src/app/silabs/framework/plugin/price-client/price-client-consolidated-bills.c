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
 * @brief The Price client plugin is responsible for keeping track of the current
 * and future prices.
 * This file handles consolidated billing.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "price-client.h"
#include "../price-common/price-common-time.h"

//  CONSOLIDATED BILL

typedef struct {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t adjustedStartTimeUtc;  // Holds the converted start time (END OF WEEK, START OF DAY, etc)
  uint32_t adjustedEndTimeUtc;    // Holds the calculated end time
  uint32_t billingPeriodStartTime;
  uint32_t billingPeriodDuration;
  uint8_t  billingPeriodDurationType;
  uint8_t  tariffType;
  uint32_t consolidatedBill;
  uint16_t currency;
  uint8_t  billTrailingDigit;
  bool valid;
} EmberAfPriceConsolidatedBill;

EmberAfPriceConsolidatedBill ConsolidatedBillsTable[EMBER_AF_PRICE_CLUSTER_CLIENT_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE];

void emberAfPriceInitConsolidatedBillsTable(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }

  for ( i = 0; i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
    ConsolidatedBillsTable[ep][i].valid = false;
  }
}

#define CANCELLATION_START_TIME  0xFFFFFFFF
/*
   static bool eventsOverlap( uint32_t newEventStartTime, uint32_t newEventEndTime, uint32_t oldEventStartTime, uint32_t oldEventEndTime ){
   return( ((newEventStartTime >= oldEventStartTime) && (newEventStartTime <= oldEventEndTime)) ||
      ((newEventEndTime >= oldEventStartTime) && (newEventEndTime <= oldEventEndTime)) ||
      ((newEventStartTime <= oldEventStartTime) && (newEventEndTime > oldEventStartTime)) );
   }

   static bool removeOverlappingAndValidateConsolidatedBills( uint32_t providerId, uint32_t issuerEventId, uint32_t startTimeUtc, uint32_t durationSeconds, uint8_t tariffType ){
   uint8_t i;
   bool cancelIndex;
   uint32_t endTimeUtc;
   uint8_t  billIsValid = true;

   endTimeUtc = startTimeUtc + durationSeconds;
   for( i=0; i<EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE; i++ ){
    cancelIndex = false;
    if( ConsolidatedBillsTable[i].valid && (ConsolidatedBillsTable[i].providerId == providerId) &&
        (ConsolidatedBillsTable[i].tariffType == tariffType) && (ConsolidatedBillsTable[i].issuerEventId <= issuerEventId) ){
      if( (ConsolidatedBillsTable[i].issuerEventId == issuerEventId) &&
          (ConsolidatedBillsTable[i].billingPeriodStartTime == CANCELLATION_START_TIME) ){
        // Cancel action received, and matching entry found.   Cancel event.
        //cancelIndex = true;
        ConsolidatedBillsTable[i].valid = false;
      }
      else if( eventsOverlap( startTimeUtc, (startTimeUtc + durationSeconds),
                           ConsolidatedBillsTable[i].adjustedStartTimeUtc,
                           ConsolidatedBillsTable[i].adjustedEndTimeUtc) ){
        if( issuerEventId > ConsolidatedBillsTable[i].issuerEventId ){
          // Overlapping event found with smaller event ID - remove.
          cancelIndex = true;
        }
        else{
          // Overlapping event found with larger event ID - new event is invalid
          billIsValid = false;
        }
      }
    }
    if( cancelIndex == true ){
      ConsolidatedBillsTable[i].valid = false;
    }
   }
   return billIsValid;
   }
 */
bool emberAfPriceClusterPublishConsolidatedBillCallback(uint32_t providerId,
                                                        uint32_t issuerEventId,
                                                        uint32_t billingPeriodStartTime,
                                                        uint32_t billingPeriodDuration,
                                                        uint8_t billingPeriodDurationType,
                                                        uint8_t tariffType,
                                                        uint32_t consolidatedBill,
                                                        uint16_t currency,
                                                        uint8_t billTrailingDigit)
{
  uint32_t adjustedStartTime;
  uint32_t adjustedDuration = 0;
  uint8_t  i;
  // uint8_t  availableIndex;
  // uint32_t  availableStartTime;
  uint32_t timeNow = emberAfGetCurrentTime();
  uint8_t  isValid = 1;

  uint8_t  smallestEventIdIndex;
  uint32_t smallestEventId = 0xFFFFFFFF;

  uint8_t endpoint = emberAfCurrentEndpoint();
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return false;
  }

  emberAfPriceClusterPrintln("RX: PublishConsolidatedBill eventId=%d,  timeNow=0x%4x", issuerEventId, timeNow);

  if ( billingPeriodStartTime != CANCELLATION_START_TIME ) {
    if ( billingPeriodStartTime == 0 ) {
      billingPeriodStartTime = timeNow;
    }
    adjustedStartTime = emberAfPluginPriceCommonClusterGetAdjustedStartTime(billingPeriodStartTime, billingPeriodDurationType);
    adjustedDuration = emberAfPluginPriceCommonClusterConvertDurationToSeconds(billingPeriodStartTime, billingPeriodDuration, billingPeriodDurationType);
  } else {
    adjustedStartTime = CANCELLATION_START_TIME;
  }
  //isValid = removeOverlappingAndValidateConsolidatedBills( providerId, issuerEventId, adjustedStartTime, adjustedDuration, tariffType );

  if ( isValid ) {
    // Initialize these.
    //availableStartTime = 0;
    //availableIndex = EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE;
    smallestEventIdIndex = EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE;

    // Search table for matching entry, invalid entry, lowestEventId
    for ( i = 0; i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
      emberAfPriceClusterPrintln(" == i=%d, val=%d, event=%d, start=0x%4x,  timeNow=0x%4x",
                                 i, ConsolidatedBillsTable[ep][i].valid, ConsolidatedBillsTable[ep][i].issuerEventId,
                                 ConsolidatedBillsTable[ep][i].billingPeriodStartTime, timeNow);
      if ( billingPeriodStartTime == CANCELLATION_START_TIME ) {
        if ( (ConsolidatedBillsTable[ep][i].providerId == providerId)
             && (ConsolidatedBillsTable[ep][i].issuerEventId == issuerEventId) ) {
          ConsolidatedBillsTable[ep][i].valid = false;
          goto kickout;
        }
      } else if ( ConsolidatedBillsTable[ep][i].valid
                  && (ConsolidatedBillsTable[ep][i].issuerEventId == issuerEventId) ) {
        // Matching entry - reuse index
        // availableIndex = i;
        break;  // DONE
      } else if ( ConsolidatedBillsTable[ep][i].valid == false ) {
        //else if( (ConsolidatedBillsTable[i].valid == false) ||
        //  (ConsolidatedBillsTable[i].adjustedEndTimeUtc < timeNow) ){
        // invalid or expired - mark with very far-out start time
        //availableIndex = i;
        //availableStartTime = 0xFFFFFFFF;
        smallestEventIdIndex = i;
        smallestEventId = 0;    // Absolutely use this index unless a match is found.
        emberAfPriceClusterPrintln("    INVALID");
      }
      //else if( ConsolidatedBillsTable[i].adjustedStartTimeUtc > availableStartTime ){
      else if ( ConsolidatedBillsTable[ep][i].issuerEventId < issuerEventId
                && ConsolidatedBillsTable[ep][i].issuerEventId < smallestEventId ) {
        //availableIndex = i;
        //availableStartTime = ConsolidatedBillsTable[i].adjustedStartTimeUtc;
        smallestEventIdIndex = i;
        smallestEventId = ConsolidatedBillsTable[ep][i].issuerEventId;
        emberAfPriceClusterPrintln("    TIME...");
      }
    }
    // Populate the available index fields.
    i = smallestEventIdIndex;
    if ( i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE ) {
      emberAfPriceClusterPrintln("  === DONE, i=%d", i);
      ConsolidatedBillsTable[ep][i].providerId = providerId;
      ConsolidatedBillsTable[ep][i].issuerEventId = issuerEventId;
      ConsolidatedBillsTable[ep][i].adjustedStartTimeUtc = adjustedStartTime;
      ConsolidatedBillsTable[ep][i].adjustedEndTimeUtc = adjustedStartTime + adjustedDuration;
      ConsolidatedBillsTable[ep][i].billingPeriodStartTime = billingPeriodStartTime;
      ConsolidatedBillsTable[ep][i].billingPeriodDuration = billingPeriodDuration;
      ConsolidatedBillsTable[ep][i].billingPeriodDurationType = billingPeriodDurationType;
      ConsolidatedBillsTable[ep][i].tariffType = tariffType;
      ConsolidatedBillsTable[ep][i].consolidatedBill = consolidatedBill;
      ConsolidatedBillsTable[ep][i].currency = currency;
      ConsolidatedBillsTable[ep][i].billTrailingDigit = billTrailingDigit;
      ConsolidatedBillsTable[ep][i].valid = true;
      emAfPricePrintConsolidatedBillTableIndex(endpoint, i);
    }
  }
  kickout:
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

void emAfPricePrintConsolidatedBillTableIndex(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE ) {
    emberAfPriceClusterPrintln("Print PublishConsolidatedBill [%d]", i);
    emberAfPriceClusterPrintln("  isValid=%d", ConsolidatedBillsTable[ep][i].valid);
    emberAfPriceClusterPrintln("  providerId=%d", ConsolidatedBillsTable[ep][i].providerId);
    emberAfPriceClusterPrintln("  issuerEventId=%d", ConsolidatedBillsTable[ep][i].issuerEventId);
    emberAfPriceClusterPrintln("  billingPeriodStartTime=0x%4x", ConsolidatedBillsTable[ep][i].billingPeriodStartTime);
    emberAfPriceClusterPrintln("  billingPeriodDuration=0x%d", ConsolidatedBillsTable[ep][i].billingPeriodDuration);
    emberAfPriceClusterPrintln("  billingPeriodDurationType=0x%X", ConsolidatedBillsTable[ep][i].billingPeriodDurationType);
    emberAfPriceClusterPrintln("  tariffType=%d", ConsolidatedBillsTable[ep][i].tariffType);
    emberAfPriceClusterPrintln("  consolidatedBill=%d", ConsolidatedBillsTable[ep][i].consolidatedBill);
    emberAfPriceClusterPrintln("  currency=%d", ConsolidatedBillsTable[ep][i].currency);
    emberAfPriceClusterPrintln("  billTrailingDigit=%d", ConsolidatedBillsTable[ep][i].billTrailingDigit);
  }
}

uint8_t emAfPriceConsolidatedBillTableGetIndexWithEventId(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  for ( i = 0; i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
    if ( ConsolidatedBillsTable[ep][i].valid && (issuerEventId == ConsolidatedBillsTable[ep][i].issuerEventId) ) {
      break;
    }
  }
  return i;
}

uint8_t emAfPriceConsolidatedBillTableGetCurrentIndex(uint8_t endpoint)
{
  uint32_t currTime = emberAfGetCurrentTime();
  uint32_t largestEventId = 0;
  uint8_t  largestEventIdIndex = EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE;
  uint8_t  i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }

  emberAfPriceClusterPrintln("=======  GET CURRENT INDEX, timeNow=0x%4x", currTime);

  for ( i = 0; i < EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
    if ( ConsolidatedBillsTable[ep][i].valid
         && (ConsolidatedBillsTable[ep][i].adjustedStartTimeUtc <= currTime)
         && (ConsolidatedBillsTable[ep][i].issuerEventId >= largestEventId) ) {
      largestEventId = ConsolidatedBillsTable[ep][i].issuerEventId;
      largestEventIdIndex = i;
    }
  }
  return largestEventIdIndex;
}
