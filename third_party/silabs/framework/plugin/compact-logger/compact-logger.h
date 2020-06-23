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
 * @brief APIs and defines for the Compact Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef _COMPACT_LOGGER_H_
#define _COMPACT_LOGGER_H_

typedef uint8_t EmberAfPluginCompactLoggerMessageBitmask;

typedef struct {
  uint32_t timestampSeconds;

  // Millisecond precision only available if enabled in the plugin
  uint16_t millisecondPrecision;

  uint16_t messageId;
  EmberAfPluginCompactLoggerMessageBitmask bitmask;
  uint16_t dataLength;
} EmberAfPluginCompactLoggerMessageInfo;

typedef enum {
  EMBER_AF_PLUGIN_COMPACT_LOGGER_SEVERITY_CRITICAL,
  EMBER_AF_PLUGIN_COMPACT_LOGGER_SEVERITY_NOTICE,
  EMBER_AF_PLUGIN_COMPACT_LOGGER_SEVERITY_DEBUG
} EmberCompactLoggerSeverity;

#define EMBER_AF_PLUGIN_COMPACT_LOGGER_FACILITY_ALL  0xFFFF
#define EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_UTC_TIME_SYNC 0x01
#define EMBER_AF_PLUGIN_COMPACT_LOGGER_BITMASK_MS_PRECISION  0x02

void emberAfPluginCompactLoggerInit(void);

EmberStatus emberAfPluginCompactLoggerAdd(uint16_t logMessageId,
                                          uint16_t logDataLength,
                                          uint8_t* logData);

uint16_t emberAfPluginCompactLoggerGetLogCount(void);

EmberStatus emberAfPluginCompactLoggerInitIterator(void);
EmberStatus emberAfPluginCompactLoggerGetLogMessageInfoByIterator(EmberAfPluginCompactLoggerMessageInfo* info);
EmberStatus emberAfPluginCompactLoggerGetLogMessageDataByIterator(uint16_t indexIntoEntry,
                                                                  uint16_t maxMessageSize,
                                                                  uint16_t* returnDataSize,
                                                                  uint8_t* returnData);
EmberStatus emberAfPluginCompactLoggerIteratorNextEntry(void);
EmberStatus emberAfPluginCompactLoggerUpdateAllLogsWithUtcTime(uint32_t currentUtcTimeSeconds);

void emberAfPluginCompactLoggerPrintAllMessages(void);

EmberStatus emberAfPluginCompactLoggerGetEntryByNumber(uint32_t entryNumber,
                                                       EmberAfPluginCompactLoggerMessageInfo* info);

bool emberAfPluginCompactLoggerCheckFacilityAndSeverity(EmberCompactLoggerSeverity severity,
                                                        uint16_t facility);
void emberAfPluginCompactLoggerSetFacility(uint16_t facility);
EmberStatus emberAfPluginCompactLoggerSetSeverity(EmberCompactLoggerSeverity severity);

// Callbacks
void emberAfPluginCompactLoggerUtcTimeSetCallback(uint32_t currentUtcTimeSeconds);

#endif  // #ifndef _COMPACT_LOGGER_H_
