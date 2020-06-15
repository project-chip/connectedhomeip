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
 * @brief This file creates a Diagnostic Information Data Object (DIDO) and sends
 * it to a specified destination.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "dido.h"

#include "stack/include/zigbee-event-logger-gen.h"
#include "stack/config/config.h"

#include "app/framework/plugin/compact-logger/compact-logger.h"
#include "app/framework/plugin/counters/counters.h"
#include "app/framework/plugin/fragmentation/fragmentation.h"
#include "app/framework/plugin/zigbee-event-logger/zigbee-event-logger-print-gen.h"
#include "app/framework/util/util.h"

#ifdef USE_APP_BOOTLOADER_LIB
#include "hal/micro/bootloader-interface-app.h"
#endif

// The general DIDO format is currently defined as follows:
// [DIDO Header] + [Sequence of DIDO Type-Length Values]
// Individual TLV formats are defined later in this file.

// #defines
#define DIDO_TLV_HEADER_LENGTH  4

typedef enum {
  DIDO_TLV_TYPE_CRC = 0,
  DIDO_TLV_TYPE_LOG_DATA = 1,
  DIDO_TLV_TYPE_STACK_TRACE = 2,
  DIDO_TLV_TYPE_EMBER_COUNTERS = 3,
  DIDO_TLV_TYPE_NVM3_STATS = 4,
  DIDO_TLV_TYPE_VERSION = 5,
} EmberAfDidoTlvType;

// Function Prototypes
static EmberStatus insertDidoHeader(uint8_t **buffer, uint16_t *remainingLength);
static EmberStatus insertTlvHeader(uint8_t **buffer,
                                   uint16_t *remainingLength,
                                   EmberAfDidoTlvType tlvType,
                                   uint16_t tlvDataLength);
static EmberStatus insertEventLogTlvData(uint8_t **buffer, uint16_t *remainingLength);
static EmberStatus insertEmberCounterTlvData(uint8_t **buffer, uint16_t *remainingLength);
static EmberStatus insertVersionTlvData(uint8_t **buffer, uint16_t *remainingLength);
//static EmberStatus insertStackTraceTlvData(uint8_t **buffer, uint16_t *remainingLength);    // Not implemented yet.
//static EmberStatus insertNvm3StatTlvData(uint8_t **buffer, uint16_t *remainingLength);      // Not implemented yet.

static EmberStatus insertEventLogHeader(uint8_t **buffer, uint16_t *remainingLength);
static EmberStatus insertEventLogEntryHeader(uint8_t **buffer,
                                             uint16_t *remainingLength,
                                             EmberAfPluginCompactLoggerMessageInfo *info);

// Temporary, until we can use a fragmentation allocation.
#define DIDO_BUFFER_SIZE 1024
static uint8_t didoBuffer[DIDO_BUFFER_SIZE];

// TLV Handler function definitions
typedef EmberStatus (tlvInsertFunction)(uint8_t **buffer, uint16_t *remainingLength);
typedef struct {
  EmberAfDidoTlvType tlvType;
  tlvInsertFunction *insertTlvData;
} tlvHandler;

static const tlvHandler tlvHandlers[] =
{
  { DIDO_TLV_TYPE_LOG_DATA, insertEventLogTlvData },
  //{ DIDO_TLV_TYPE_STACK_TRACE, insertStackTraceTlvData },   // Not implemented yet.
  { DIDO_TLV_TYPE_EMBER_COUNTERS, insertEmberCounterTlvData },
  //{ DIDO_TLV_TYPE_NVM3_STATS, insertNvm3StatTlvData },      // Not implemented yet.
  { DIDO_TLV_TYPE_VERSION, insertVersionTlvData },
};

#define NUM_TLV_HANDLERS (sizeof(tlvHandlers) / sizeof(tlvHandler))
#define ZCL_HEADER_SIZE 5

static uint16_t filledLen;

EmberStatus emberAfDidoClusterSendDebugReport(uint8_t debugReportId,
                                              EmberNodeId nodeId,
                                              uint8_t endpoint)
{
  EmberStatus status;
  uint8_t *buffer;
  uint16_t remainingLength;
  EmberApsFrame *papsFrame;
  uint8_t i;
  uint8_t messageTag;
  uint8_t *tlvHeader;
  int32u crc;

  // buffer points to a large buffer where we can store the data for the report.
  buffer = &didoBuffer[ZCL_HEADER_SIZE];
  remainingLength = DIDO_BUFFER_SIZE - ZCL_HEADER_SIZE;

  // Insert the debugReportId at the beginning of the buffer
  // before the DIDO header.
  *buffer++ = debugReportId;
  remainingLength--;

  status = insertDidoHeader(&buffer, &remainingLength);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: Could not create Dido Header");
    return EMBER_ERR_FATAL;
  }

  for (i = 0; i < NUM_TLV_HANDLERS; i++) {
    // Get header & data for each TLV
    tlvHeader = buffer;
    buffer = buffer + DIDO_TLV_HEADER_LENGTH;

    filledLen = remainingLength;
    status = tlvHandlers[i].insertTlvData(&buffer, &remainingLength);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error inserting TLV Data[%d], status=0x%x", i, status);
      return status;
    }
    filledLen -= remainingLength; // filledLen tells how much data was copied into the buffer.
    emberAfCorePrintln("**  Storing type=%d, filledLen=%d",
                       tlvHandlers[i].tlvType, filledLen);
    status = insertTlvHeader(&tlvHeader,
                             &remainingLength,
                             tlvHandlers[i].tlvType,
                             filledLen);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error inserting TLV Header[%d], status=0x%x", i, status);
    }
  }
  // Fill in the CRC TLV.
  filledLen = buffer - didoBuffer;
  tlvHeader = buffer;
  buffer = buffer + DIDO_TLV_HEADER_LENGTH;
  status = insertTlvHeader(&tlvHeader,
                           &remainingLength,
                           DIDO_TLV_TYPE_CRC,
                           sizeof(crc));

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error inserting CRC Header, status=0x%x", status);
    return status;
  }

  // Calculate the CRC over the entire packet except the last 4 (CRC) bytes.
  // Don't include the reportId.
  crc = emberAfGetBufferCrc(&didoBuffer[ZCL_HEADER_SIZE + 1],
                            filledLen + DIDO_TLV_HEADER_LENGTH - ZCL_HEADER_SIZE - 1,
                            0);

  emberStoreLowHighInt32u(buffer, crc);
  buffer += sizeof(crc);
  filledLen = buffer - didoBuffer;

  // Reverse endpoints.
  papsFrame = emberAfGetCommandApsFrame();
  i = papsFrame->sourceEndpoint;
  emberAfSetCommandEndpoints(papsFrame->destinationEndpoint, i);

  // Set the external buffer to point to didoBuffer where we will
  // store the ZCL frame header bytes, before the DIDO report.
  // EMZIGBEE-2645 will replace didoBuffer with a pointer to a
  // fragmentation buffer to free up RAM.
  emberAfSetExternalBuffer(didoBuffer,
                           DIDO_BUFFER_SIZE,
                           &filledLen,
                           papsFrame);
  // For this fill command, pass in pointer to where the actual REPORT DATA
  // is located, which is after the ZCL header bytes, and after the reportId.
  // Modify the length parameter to reflect the number of remaining bytes
  // after the ZCL header and the debugReportId.
  emberAfFillCommandSlWwahClusterDebugReportQueryResponse(debugReportId,
                                                          &didoBuffer[ZCL_HEADER_SIZE + 1],
                                                          (filledLen - ZCL_HEADER_SIZE - 1));

  // emberAfFillCommandSlWwahClusterDebugReportQueryResponse() sets the APS options to defaults.
  // But APS Encryption is required for WWAH cluster, so enable it here.
  papsFrame->options |= EMBER_APS_OPTION_ENCRYPTION;

  // This is a response, must set the ZCL header sequence number to that of the request.
  // Offset of sequence number field depends on whether 2-byte mfg code is present.
  if ((didoBuffer[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U) {
    didoBuffer[3] = emberAfCurrentCommand()->seqNum; // fc, mfg code, seq
  } else {
    didoBuffer[1] = emberAfCurrentCommand()->seqNum; // fc, seq
  }

  status = emAfFragmentationSendUnicast(EMBER_OUTGOING_DIRECT,
                                        nodeId,
                                        papsFrame,
                                        didoBuffer,
                                        filledLen,
                                        &messageTag);
  return status;
}

// DIDO Header Definitions
#define MAGIC_NUMBER_LENGTH 8
#define DIDO_HEADER_VERSION 1
#define DIDO_HEADER_LENGTH  (MAGIC_NUMBER_LENGTH + 1)

static const uint8_t DidoHeaderMagicNumber[MAGIC_NUMBER_LENGTH] =
{
  0x3a, 0x46, 0xfa, 0xdb, 0xc8, 0x69, 0xfe, 0x50
};

// DIDO Header:
// [Magic Number(8)] + [Version(1)]
static EmberStatus insertDidoHeader(uint8_t **buffer, uint16_t *remainingLength)
{
  if (*remainingLength >= DIDO_HEADER_LENGTH) {
    MEMCOPY(*buffer, DidoHeaderMagicNumber, MAGIC_NUMBER_LENGTH);
    (*buffer)[MAGIC_NUMBER_LENGTH] = DIDO_HEADER_VERSION;
    *remainingLength -= DIDO_HEADER_LENGTH;
    *buffer += DIDO_HEADER_LENGTH;
    return EMBER_SUCCESS;
  }
  return EMBER_ERR_FATAL;
}

// TLV Header:
// [TLV Type(2)] + [Data Length(2)]
static EmberStatus insertTlvHeader(uint8_t **buffer,
                                   uint16_t *remainingLength,
                                   EmberAfDidoTlvType tlvType,
                                   uint16_t tlvDataLength)
{
  if (*remainingLength > DIDO_TLV_HEADER_LENGTH) {
    emberStoreLowHighInt16u(*buffer, tlvType);
    *buffer += 2;
    emberStoreLowHighInt16u(*buffer, tlvDataLength);
    *buffer += 2;
    *remainingLength -= 4;
    return EMBER_SUCCESS;
  }
  return EMBER_ERR_FATAL;
}

// TLV Event Log
// [Event Log Header] + [Event Log Entries]
// Each event log entry has a header & data
static EmberStatus insertEventLogTlvData(uint8_t **buffer, uint16_t *remainingLength)
{
  EmberAfPluginCompactLoggerMessageInfo msgInfo;
  EmberStatus status;
  uint16_t msgSize;
  uint8_t cnt = 0;

  // Setup Log Data TLV Header
  status = insertEventLogHeader(buffer, remainingLength);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Insert Event Log Header Error, status=0x%x", status);
    return status;
  }

  status = emberAfPluginCompactLoggerInitIterator();
  if (status == EMBER_SUCCESS) {
    do {
      // Get message info and add it to the buffer.
      status = emberAfPluginCompactLoggerGetLogMessageInfoByIterator(&msgInfo);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("Get Log Message Info Error, status=0x%x", status);
        // Corruption in Log Data TLV
        return status;
      }
      cnt++;

      status = insertEventLogEntryHeader(buffer, remainingLength, &msgInfo);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("Insert Event Log Entry Header Error, status=0x%x", status);
        // Corruption in Log Data TLV
        return status;
      }

      // Get the data from the iterator
      if (*remainingLength < msgInfo.dataLength) {
        emberAfCorePrintln("Insert Event Log Entry Error - out of space");
        // Out of space - abort and send what we could fit.
        break;
      }
      msgSize = 0;
      status = emberAfPluginCompactLoggerGetLogMessageDataByIterator(0,
                                                                     *remainingLength,
                                                                     &msgSize,
                                                                     *buffer);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("Get Log Message Error, status=0x%x", status);
        return EMBER_ERR_FATAL;
      } else if (msgInfo.dataLength < *remainingLength) {
        // Ensure we received data, and the data was not truncated
        // to fit inside the remaining length of the buffer.
        *remainingLength -= msgInfo.dataLength;
        *buffer = *buffer + msgInfo.dataLength;
      } else {
        emberAfCorePrintln("Error: Log Data Error, message length=%d, remainingLength=%d",
                           msgInfo.dataLength, *remainingLength);
        break;
      }

      // Advance the iterator.  This returns EMBER_OPERATION_IN_PROGRESS
      // until it returns the final entry, at which point it returns
      // EMBER_SUCCESS.  Continue to read as long as
      // status == EMBER_OPERATION_IN_PROGRESS.
      status = emberAfPluginCompactLoggerIteratorNextEntry();
    } while ( status == EMBER_OPERATION_IN_PROGRESS );
  }
  return EMBER_SUCCESS;
}

// TLV Event Log Header
// [BootCount (4)] + [NumTlvEntries (2)] + [LoggerVersion (1)]
#define LOG_DATA_TLV_SIZE 7
static EmberStatus insertEventLogHeader(uint8_t **buffer, uint16_t *remainingLength)
{
  uint16_t length;
  uint32_t bootCnt;

  if (*remainingLength < LOG_DATA_TLV_SIZE) {
    return EMBER_ERR_FATAL;
  }

  // Setup Event Log TLV Header
  halCommonGetToken(&bootCnt, TOKEN_STACK_BOOT_COUNTER);
  emberStoreLowHighInt32u(*buffer, bootCnt);
  *buffer += sizeof(bootCnt);
  length = emberAfPluginCompactLoggerGetLogCount();
  emberStoreLowHighInt16u(*buffer, length);
  *buffer += sizeof(length);
  **buffer = ZIGBEE_EVENT_LOGGER_VERSION;
  *buffer += 1;
  *remainingLength -= LOG_DATA_TLV_SIZE;
  return EMBER_SUCCESS;
}

// Event Log Entry Header:
// [Length (1)] + [Bitmask (1)] + [TimeValue (4)] + [MSPrecision (0/2)] +
// [BootCnt (0/4)] + [MsgId (2)]
#define MAX_LOG_DATA_HEADER_SIZE  (sizeof(EmberAfPluginCompactLoggerMessageInfo) + 4)    // Worst Case, include boot count
#define EVENT_LOG_ENTRY_INDEX_LENGTH  0
#define EVENT_LOG_ENTRY_INDEX_BITMASK 1
static EmberStatus insertEventLogEntryHeader(uint8_t **buffer,
                                             uint16_t *remainingLength,
                                             EmberAfPluginCompactLoggerMessageInfo *info)
{
  uint16_t responseLength;
  uint8_t *pdst = *buffer;
  if (*remainingLength < MAX_LOG_DATA_HEADER_SIZE) {
    return EMBER_ERR_FATAL;
  }
  pdst[EVENT_LOG_ENTRY_INDEX_BITMASK] = info->bitmask;
  emberStoreLowHighInt32u(&pdst[2], info->timestampSeconds);
  responseLength = 6;

  if (info->bitmask & EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION) {
    emberStoreLowHighInt16u(&pdst[responseLength], info->millisecondPrecision);
    responseLength += sizeof(info->millisecondPrecision);
  }
  /*if( info->bitmask & EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_BOOT_COUNT)
     {
     // Store boot counter from current log entry.
     // TODO:  This feature is not currently supported.
     emberStoreLowHighInt32u( &buffer[responseLength], ??? );
     responseLength += sizeof(???);
     }*/
  emberStoreLowHighInt16u(&pdst[responseLength], info->messageId);
  responseLength += sizeof(info->messageId);
  pdst[EVENT_LOG_ENTRY_INDEX_LENGTH] = responseLength + info->dataLength;

  *remainingLength -= responseLength;
  *buffer += responseLength;

  return EMBER_SUCCESS;
}

// TLV Ember Counter Data
//  [#Counters(1)] + [Counter0Id(1)] + [Counter0Value(2)] + ...
#define COUNTER_ENTRY_SIZE  (1 + sizeof(emberCounters[0]) )
#define COUNTER_TLV_INDEX_NUM_COUNTERS  0
static EmberStatus insertEmberCounterTlvData(uint8_t **buffer, uint16_t *remainingLength)
{
  uint16_t responseLength = 1;  // Reserve 1st byte for # counters
  uint8_t i = 0;
  uint8_t *pdst = *buffer;
  for (i = 0; i < EMBER_COUNTER_TYPE_COUNT; i++) {
    if ((responseLength + COUNTER_ENTRY_SIZE) > *remainingLength) {
      // Response is too large.
      emberAfCorePrintln("Error: Out of Space with Ember Counters, i=%d, maxLen=%d",
                         i, *remainingLength);
      // Rather than return an error, break out of the for() loop
      // and report all the counter values that we could fit.
      break;
    }
    pdst[responseLength++] = i;
    emberStoreLowHighInt16u(&pdst[responseLength], emberCounters[i]);
    responseLength += 2;
  }
  pdst[COUNTER_TLV_INDEX_NUM_COUNTERS] = i; // Number of Counters
  *buffer += responseLength;
  *remainingLength -= responseLength;
  return EMBER_SUCCESS;
}

enum {
  STACK_ID_ZIGBEE  = 0,
  STACK_ID_THREAD  = 1,
  STACK_ID_BLE     = 2,
  STACK_ID_CONNECT = 3,
};

// TLV Version Data
// [StackId(1)] + [StackVersion(2)] + [VersionType(1)] +
// [CustomerVersion(2)] + [BootloaderType(1)] + [BootloaderVersion(2)]
#define VERSION_DATA_SIZE   9

#ifndef CUSTOMER_APPLICATION_VERSION
#ifdef EMBER_AF_PLUGIN_OTA_CLIENT
#define CUSTOMER_APPLICATION_VERSION  EMBER_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION
#else
#define CUSTOMER_APPLICATION_VERSION  0x0000
#endif
#endif

static EmberStatus insertVersionTlvData(uint8_t **buffer, uint16_t *remainingLength)
{
  EmberStatus status = EMBER_ERR_FATAL;
  uint8_t *pdst = *buffer;
  int16u bootloaderVersion = 0;

  if (*remainingLength >= VERSION_DATA_SIZE) {
#ifdef USE_APP_BOOTLOADER_LIB
    bootloaderVersion = halAppBootloaderGetVersion();
#endif
    pdst[0] = STACK_ID_ZIGBEE;
    pdst[1] = LOW_BYTE(EMBER_FULL_VERSION);
    pdst[2] = HIGH_BYTE(EMBER_FULL_VERSION);
    pdst[3] = EMBER_VERSION_TYPE;
    pdst[4] = LOW_BYTE(CUSTOMER_APPLICATION_VERSION);
    pdst[5] = HIGH_BYTE(CUSTOMER_APPLICATION_VERSION);
    pdst[6] = 0;    // Bootloader Type - Legacu or Gecko.  How to detect?
    pdst[7] = LOW_BYTE(bootloaderVersion);
    pdst[8] = HIGH_BYTE(bootloaderVersion);
    *remainingLength -= VERSION_DATA_SIZE;
    *buffer += VERSION_DATA_SIZE;
    status = EMBER_SUCCESS;
  }
  return status;
}

#if 0
// TLV STACK TRACE DATA
// TODO - Implement this feature - EMZIGBEE-2821
static EmberStatus insertStackTraceTlvData(uint8_t **buffer, uint16_t *remainingLength)
{
  return EMBER_SUCCESS;
}

// TLV NVM3 STATUS DATA
// TODO - Implement this feature - EMZIGBEE-2822
static EmberStatus insertNvm3StatTlvData(uint8_t **buffer, uint16_t *remainingLength)
{
  return EMBER_SUCCESS;
}

#endif  // #if 0
