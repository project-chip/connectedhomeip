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
 * @brief Compact logger Plugin
 *
 * This plugin defines an interface to add enumerated log messages with
 * a timestamp.  This plugin does not define the exact enumerations or format
 * but in general the format is the following:
 *
 * - UTC Time seconds:  4 bytes
 * - Log Message Bitmask: 1 bytes
 * - Log Message ID: 2 bytes
 * - Log Message Data Length: 2 bytes
 * - Log Message Data: Variable
 *
 * The messages definitions are meant to be enumerated by some other
 * predefined entity.  This allows the embedded code running on chip to
 * store only the enumerated value (log message ID) and the variable data.
 *
 * For example, log Message ID 3 could be defined as:
 *   Rejoining to network using channel bitmask 0x%08X
 *
 * The compact logger only needs to store ID 3, and the 32-bit value that
 * would represent the bitmask parameter.  Different message IDs may
 * have different numbers or types of parameters but this plugin
 * is ignorant of that.  A seperate tool or plugin would be necessary to
 * print the data, and define how to serialize the enumerated message.
 *
 * This implementation stores all log data in a ring buffer in RAM.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/compact-logger/compact-logger.h"
#include "app/framework/plugin/ring-buffer/ring-buffer.h"
#include "app/framework/util/time-util.h"
#include "app/framework/plugin/simple-clock/simple-clock.h"

//------------------------------------------------------------------------------
// Globals

// NOTE: The Log message length will NOT be stored because the ring
// buffer has a length field it uses.  The log message length
// will be the size of the ring buffer entry MINUS the various overhead
// fields defined below.

// Internal Format:
//   UTC Time Seconds
//   Log Message Bitmask
//   Milliseconds precision (2-bytes) : OPTIONAL
//     Enabled only if EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION
//   Log Message ID
//   Log Message Data (variable. Could be 0)

#define UTC_TIME_SIZE_BYTES 4
#define LOG_MESSAGE_ID_SIZE_BYTES 2
#define LOG_MESSAGE_BITMASK_SIZE_BYTES 1

#define MILLISECOND_PRECISION_BYTES 2

#define UTC_TIME_OFFSET 0
#define LOG_MESSAGE_BITMASK_OFFSET (UTC_TIME_SIZE_BYTES)
#define LOG_MESSAGE_ID_OFFSET (LOG_MESSAGE_BITMASK_OFFSET + LOG_MESSAGE_ID_SIZE_BYTES)

#define SERIALIZED_LOG_MESSAGE_LENGTH \
  (UTC_TIME_SIZE_BYTES                \
   + LOG_MESSAGE_BITMASK_SIZE_BYTES   \
   + LOG_MESSAGE_ID_SIZE_BYTES)

#define UTC_TIME_SYNCED_BITMASK BIT(0)

#if defined(EMBER_SCRIPTED_TEST)
  #define MAX_MESSAGE_COUNT 10
// This will define a storage space that can store MAX_MESSAGE_COUNT assuming all messages
// have no variable data.
  #define EMBER_AF_PLUGIN_COMPACT_LOGGER_STORAGE_SIZE                                         \
  ((SERIALIZED_LOG_MESSAGE_LENGTH + EMBER_AF_PLUGIN_RING_BUFFER_NARROW_LENGTH_ENTRY_OVERHEAD) \
   * MAX_MESSAGE_COUNT)
#endif

static EmberRingBuffer ring = {
  EMBER_AF_PLUGIN_COMPACT_LOGGER_STORAGE_SIZE,

  // Don't care about other variables.  Will be initialized by ring buffer init
};

static uint8_t ringBufferStorage[EMBER_AF_PLUGIN_COMPACT_LOGGER_STORAGE_SIZE];

#define MAX_PRINT_MESSAGE_COUNT 100
#define MAX_MESSAGE_DATA_PRINT_LENGTH 50
#define PRINT_BUFFER_LENGTH 10

// The severity and facility are used to filter which mesages are written
// to the logger, and which are not.  See
// emberAfPluginCompactLoggerCheckFacilityAndSeverity().
static uint8_t compactLoggerSeverity;
static uint16_t compactLoggerFacility;

// Design note:
// Storing the bitmask is somewhat of a waste because the only thing stored is
// the time sync bit.  Storing that with each log message is inefficient because
// we update all existing logs with the real time stamp once we obtain
// time sync from a time server.  The only case where this would be beneficial
// is if we couldn't atomically update all messages.  Not really sure why
// that would happen in a non-RTOS environment.
// A better way would be to store the time synchronization in a global.

//------------------------------------------------------------------------------
// Forward Declarations

static void convertSerializedLogDataToStruct(uint8_t* serializedData,
                                             EmberAfPluginCompactLoggerMessageInfo* info);
static void serializeLogDataFromStruct(EmberAfPluginCompactLoggerMessageInfo* info,
                                       uint8_t* serializedLogData);
static EmberStatus updateLogMessageInfoByIterator(EmberAfPluginCompactLoggerMessageInfo* info);

//------------------------------------------------------------------------------
// Public API

void emberAfPluginCompactLoggerInit(void)
{
  emberAfPluginRingBufferInitStruct(&ring,
                                    false, // wide length field? (false = 1 byte length)
                                    NULL, // delete function callback
                                    ringBufferStorage);

  compactLoggerSeverity = EMBER_AF_PLUGIN_COMPACT_LOGGER_SEVERITY_NOTICE;
  compactLoggerFacility = EMBER_AF_PLUGIN_COMPACT_LOGGER_FACILITY_ALL;
}

EmberStatus emberAfPluginCompactLoggerAdd(uint16_t logMessageId,
                                          uint16_t logDataLength,
                                          uint8_t* logData)
{
  EmberStatus status;
  uint8_t serializedLogData[SERIALIZED_LOG_MESSAGE_LENGTH];
  EmberAfPluginCompactLoggerMessageInfo info;

  MEMSET(&info, 0, sizeof(EmberAfPluginCompactLoggerMessageInfo));

  info.timestampSeconds = emberAfGetCurrentTimeSecondsWithMsPrecision(&(info.millisecondPrecision));
  info.messageId = logMessageId;
  if (emberAfPluginSimpleClockGetTimeSyncStatus()
      != EMBER_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC) {
    info.bitmask |= UTC_TIME_SYNCED_BITMASK;
  }

  serializeLogDataFromStruct(&info,
                             serializedLogData);

  status = emberAfPluginRingBufferAddEntry(&ring,
                                           serializedLogData,
                                           SERIALIZED_LOG_MESSAGE_LENGTH);
  if (status || logDataLength == 0) {
    return status;
  }

  status = emberAfPluginRingBufferAppendLastEntry(&ring,
                                                  logData,
                                                  logDataLength);

  return status;
}

uint16_t emberAfPluginCompactLoggerGetLogCount(void)
{
  return ring.entryCount;
}

void emberAfPluginCompactLoggerPrintAllMessages(void)
{
  EmberStatus status = emberAfPluginCompactLoggerInitIterator();
  bool keepGoing = true;
  EmberAfPluginCompactLoggerMessageInfo info;
  uint16_t logMessageNumber = 0;

  emberAfCorePrintln("# Timestamp           Sync ID     Length");
  // example          3 yyyy-mm-dd hh:dd:ss 0x01 0x0001 0x0000
  emberAfCorePrintln("----------------------------------------");
  if (status) {
    // No entries in log
    return;
  }

  do {
    uint16_t i;
    status = emberAfPluginCompactLoggerGetLogMessageInfoByIterator(&info);
    if (status) {
      emberAfCorePrintln("Error retrieving message info.");
      return;
    }
    emberAfCorePrint("%d: ", logMessageNumber);
    if (info.bitmask & UTC_TIME_SYNCED_BITMASK) {
      emberAfPrintTimeIsoFormat(info.timestampSeconds);
    } else {
      emberAfCorePrint("0x%8X         ", info.timestampSeconds);
    }
    emberAfCorePrint(" %s  0x%2X 0x%2X ",
                     (info.bitmask & UTC_TIME_SYNCED_BITMASK
                      ? "yes"
                      : "no "),
                     info.messageId,
                     info.dataLength);
    uint16_t indexIntoEntry = 0;
    for (i = 0; i < info.dataLength && i < MAX_MESSAGE_DATA_PRINT_LENGTH; i += PRINT_BUFFER_LENGTH) {
      uint8_t data[PRINT_BUFFER_LENGTH];
      uint16_t returnDataSize;
      EmberStatus status = emberAfPluginCompactLoggerGetLogMessageDataByIterator(indexIntoEntry,
                                                                                 PRINT_BUFFER_LENGTH,
                                                                                 &returnDataSize,
                                                                                 data);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("Error: Could not retrieve log data.");
        return;
      }
      emberAfCorePrintBuffer(data, returnDataSize, false); // false = without space
      indexIntoEntry += returnDataSize;
    }
    emberAfCorePrintln("");

    status = emberAfPluginCompactLoggerIteratorNextEntry();

    // Always increment so the print at the end is accurate for the log message count.
    logMessageNumber++;

    // The code arbitrarily limits the number of messages printed to prevent the user
    // from accidentally printing a giant log output that causes a watchdog timeout.
    if (logMessageNumber >= MAX_PRINT_MESSAGE_COUNT) {
      emberAfCorePrintln("Reached max message print count (%d)", MAX_PRINT_MESSAGE_COUNT);
      keepGoing = false;
    }

    if (status == EMBER_SUCCESS) {
      keepGoing = false;
    } else if (status != EMBER_OPERATION_IN_PROGRESS) {
      emberAfCorePrintln("Error getting next log message: 0x%X", status);
      keepGoing = false;
      return;
    }
  } while (keepGoing);

  emberAfCorePrintln("\n%d log messages", logMessageNumber);
}

EmberStatus emberAfPluginCompactLoggerInitIterator(void)
{
  return emberAfPluginRingBufferInitIterator(&ring);
}

EmberStatus emberAfPluginCompactLoggerGetLogMessageInfoByIterator(EmberAfPluginCompactLoggerMessageInfo* info)
{
  uint8_t serializedLogData[SERIALIZED_LOG_MESSAGE_LENGTH];
  uint16_t entrySize;
  uint16_t returnDataSize;
  EmberStatus status;

  status = emberAfPluginRingBufferGetEntryByIterator(&ring,
                                                     0,
                                                     &entrySize,
                                                     SERIALIZED_LOG_MESSAGE_LENGTH,
                                                     &returnDataSize,
                                                     serializedLogData);

  if (status) {
    return status;
  }

  convertSerializedLogDataToStruct(serializedLogData, info);
  info->dataLength = entrySize - returnDataSize;

  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginCompactLoggerGetLogMessageDataByIterator(uint16_t indexIntoEntry,
                                                                  uint16_t maxMessageSize,
                                                                  uint16_t* returnDataSize,
                                                                  uint8_t* returnData)
{
  EmberStatus status;
  uint16_t entrySize;
  uint16_t headerSize = SERIALIZED_LOG_MESSAGE_LENGTH;

  status = emberAfPluginRingBufferGetEntryByIterator(&ring,
                                                     headerSize + indexIntoEntry,
                                                     &entrySize,
                                                     maxMessageSize,
                                                     returnDataSize,
                                                     returnData);
  return status;
}

// This should be called when the embedded code synchronizes
// with UTC time, meaning subsequent calls to emberAfGetCurrentTime()
// return the real UTC time and not seconds since boot.
EmberStatus emberAfPluginCompactLoggerUpdateAllLogsWithUtcTime(uint32_t currentUtcTimeSeconds)
{
  // Foreach log message, take the delta from the current clock tick seconds to the
  // clock tick seconds that was used when the log message was added.
  // Set the log message's time to be the currentUtcTimeSeconds - delta.

  EmberStatus status;
  bool keepGoing = true;
  uint16_t entryNumber = 0;

  status = emberAfPluginCompactLoggerInitIterator();
  if (status != EMBER_SUCCESS) {
    return status;
  }

  do {
    EmberAfPluginCompactLoggerMessageInfo info;
    status = emberAfPluginCompactLoggerGetLogMessageInfoByIterator(&info);
    if (status) {
      emberAfCorePrintln("Error: Compact logger couldn't get log info to update timestamp.");
      return status;
    }

    if (!(info.bitmask & UTC_TIME_SYNCED_BITMASK)) {
      uint32_t logMessageSecondsSinceBoot = info.timestampSeconds;
      uint32_t currentSecondsSinceBoot = halCommonGetInt32uMillisecondTick() / MILLISECOND_TICKS_PER_SECOND;

      info.timestampSeconds = (currentUtcTimeSeconds
                               - (currentSecondsSinceBoot - logMessageSecondsSinceBoot));
      info.bitmask |= UTC_TIME_SYNCED_BITMASK;

      status = updateLogMessageInfoByIterator(&info);

      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("Error: Compact logger failed to update current time for entry %d.",
                           entryNumber);
        return status;
      }
    }

    status = emberAfPluginCompactLoggerIteratorNextEntry();
    entryNumber++;
    if (status == EMBER_SUCCESS) {
      return EMBER_SUCCESS;
    } else if (status != EMBER_OPERATION_IN_PROGRESS) {
      emberAfCorePrintln("Error: Compact logger couldn't get next entry in logs.");
      return status;
    }
  } while (keepGoing);

  // Issue a callback notifying that UTC time has been set
  emberAfPluginCompactLoggerUtcTimeSetCallback(currentUtcTimeSeconds);

  return status;
}

EmberStatus emberAfPluginCompactLoggerIteratorNextEntry(void)
{
  return emberAfPluginRingBufferIteratorNextEntry(&ring);
}

EmberStatus emberAfPluginCompactLoggerGetEntryByNumber(uint32_t entryNumber,
                                                       EmberAfPluginCompactLoggerMessageInfo* info)
{
  uint8_t serializedLogData[SERIALIZED_LOG_MESSAGE_LENGTH];
  uint16_t returnEntryTotalSize;
  uint16_t returnDataSize;
  EmberStatus status = emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                                    entryNumber,
                                                                    0,   // index
                                                                    &returnEntryTotalSize,
                                                                    SERIALIZED_LOG_MESSAGE_LENGTH,
                                                                    &returnDataSize,
                                                                    serializedLogData);
  if (status != EMBER_SUCCESS) {
    return status;
  }
  convertSerializedLogDataToStruct(serializedLogData, info);
  return EMBER_SUCCESS;
}

bool emberAfPluginCompactLoggerCheckFacilityAndSeverity(EmberCompactLoggerSeverity severity,
                                                        uint16_t facility)
{
  return ((severity <= compactLoggerSeverity)
          && ((facility & compactLoggerFacility) == facility));
}

EmberStatus emberAfPluginCompactLoggerSetSeverity(EmberCompactLoggerSeverity severity)
{
  EmberStatus status = EMBER_BAD_ARGUMENT;
  if ( severity <= EMBER_AF_PLUGIN_COMPACT_LOGGER_SEVERITY_DEBUG ) {
    compactLoggerSeverity = severity;
    status = EMBER_SUCCESS;
  }
  return status;
}

void emberAfPluginCompactLoggerSetFacility(uint16_t facility)
{
  compactLoggerFacility = facility;
}

//------------------------------------------------------------------------------
// Internal API

static void convertSerializedLogDataToStruct(uint8_t* serializedLogData,
                                             EmberAfPluginCompactLoggerMessageInfo* info)

{
  uint8_t i;

  MEMSET(info, 0, sizeof(EmberAfPluginCompactLoggerMessageInfo));

  for (i = 0; i < UTC_TIME_SIZE_BYTES; i++) {
    info->timestampSeconds += ((uint32_t)(*serializedLogData)) << (i * 8);
    serializedLogData++;
  }

  info->bitmask = (EmberAfPluginCompactLoggerMessageBitmask)(*serializedLogData);
  serializedLogData++;
  if (info->bitmask & EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION) {
    info->millisecondPrecision = *serializedLogData;
    info->millisecondPrecision += (*serializedLogData) << 8;
  }
  info->messageId = *serializedLogData;
  serializedLogData++;
  info->messageId += (*serializedLogData) << 8;
}

static void serializeLogDataFromStruct(EmberAfPluginCompactLoggerMessageInfo* info,
                                       uint8_t* serializedLogData)
{
  uint8_t i;
  uint8_t* ptr;

  MEMSET(serializedLogData, 0, SERIALIZED_LOG_MESSAGE_LENGTH);

  ptr = &serializedLogData[UTC_TIME_OFFSET];

  for (i = 0; i < UTC_TIME_SIZE_BYTES; i++) {
    *ptr = LOW_BYTE(info->timestampSeconds >> (i * 8));
    ptr++;
  }

  if (info->bitmask & UTC_TIME_SYNCED_BITMASK) {
    *ptr |= UTC_TIME_SYNCED_BITMASK;
  }
#if defined(EMBER_AF_PLUGIN_COMPACT_LOGGER_MILLISECOND_PRECISION)
  *ptr |= EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION;
#endif
  ptr++;

#if defined(EMBER_AF_PLUGIN_COMPACT_LOGGER_MILLISECOND_PRECISION)
  for (i = 0; i < MILLISECOND_PRECISION_BYTES; i++) {
    *ptr = LOW_BYTE((info.millisecondPrecision >> (i * 8)));
    ptr++;
  }
#endif

  for (i = 0; i < LOG_MESSAGE_ID_SIZE_BYTES; i++) {
    *ptr = LOW_BYTE(info->messageId >> (i * 8));
    ptr++;
  }
}

static EmberStatus updateLogMessageInfoByIterator(EmberAfPluginCompactLoggerMessageInfo* info)
{
  uint8_t serializedLogData[SERIALIZED_LOG_MESSAGE_LENGTH];

  serializeLogDataFromStruct(info, serializedLogData);

  return emberAfPluginRingBufferUpdateEntryByIterator(&ring,
                                                      0,  // data index in entry
                                                      serializedLogData,
                                                      SERIALIZED_LOG_MESSAGE_LENGTH);
}
