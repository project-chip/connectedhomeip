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
 * @brief CLI for the Zigbee Event Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "stack/include/zigbee-event-logger-gen.h"
#include "zigbee-event-logger-print-gen.h"

#include "app/framework/plugin/compact-logger/compact-logger.h"

#define ZIGBEE_EVENT_LOGGER_MAX_DATA_SIZE  32

static void printMessageTimestamp(EmberAfPluginCompactLoggerMessageInfo *msgInfo);

// plugin zigbee-event-logger print
void emAfZigbeeEventLoggerPrintEvents(void)
{
  EmberAfPluginCompactLoggerMessageInfo msgInfo;
  EmberStatus status;
  printFunction *pfunc;
  uint16_t msgSize;
  uint8_t  msgData[ZIGBEE_EVENT_LOGGER_MAX_DATA_SIZE];
  bool ret;

  emberAfCorePrintln("Printing All Events");
  emberAfCorePrintln("-------------------");
  status = emberAfPluginCompactLoggerInitIterator();
  if ( status == EMBER_SUCCESS ) {
    do {
      // Get the data from the iterator
      emberAfPluginCompactLoggerGetLogMessageInfoByIterator(&msgInfo);
      emberAfPluginCompactLoggerGetLogMessageDataByIterator(0,
                                                            ZIGBEE_EVENT_LOGGER_MAX_DATA_SIZE,
                                                            &msgSize,
                                                            msgData);

      printMessageTimestamp(&msgInfo);
      // Now call the print function based on the messageId.
      pfunc = emberAfPluginZigBeeEventLoggerLookupPrintFunction(msgInfo.messageId);
      if ( pfunc != NULL ) {
        ret = pfunc(msgData, msgInfo.dataLength);
        if ( ret == false ) {
          emberAfPluginZigBeeEventLoggerPrintHexData(msgData, msgInfo.dataLength);
        }
      }

      // Advance the iterator.  This returns EMBER_OPERATION_IN_PROGRESS
      // until it returns the final entry, at which point it returns
      // EMBER_SUCCESS.  Continue to read as long as
      // status == EMBER_OPERATION_IN_PROGRESS.
      status = emberAfPluginCompactLoggerIteratorNextEntry();
    } while ( status == EMBER_OPERATION_IN_PROGRESS );
  }
}

static void printMessageTimestamp(EmberAfPluginCompactLoggerMessageInfo *msgInfo)
{
  if ( msgInfo->bitmask & EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION ) {
    emberAfCorePrint("[%d.%d] ", msgInfo->timestampSeconds, msgInfo->millisecondPrecision);
  }
  emberAfCorePrint("[%d] ", msgInfo->timestampSeconds);
}
