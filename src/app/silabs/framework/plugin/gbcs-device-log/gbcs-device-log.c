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
 * @brief Routines for the GBCS Device Log plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "gbcs-device-log.h"

#define GBCS_DEVICE_LOG_ENTRY_INUSE  BIT(1)
#define deviceLogEntryInUse(entry)   ((entry)->flags & GBCS_DEVICE_LOG_ENTRY_INUSE)

typedef struct {
  EmberEUI64 deviceId;
  EmberAfGBCSDeviceLogInfo deviceInfo;
  uint8_t flags;
} deviceLogEntry;

static deviceLogEntry deviceLog[EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE];
#define EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX  0xFF
#define isValidDeviceLogIndex(index) ((index) < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE)

#define EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_NULL_EUI { 0, 0, 0, 0, 0, 0, 0, 0 }
static const EmberEUI64 NULL_EUI   = EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_NULL_EUI;
#define isNullEui(eui)               (MEMCOMPARE(eui, NULL_EUI, EUI64_SIZE) == 0)

#define CONCAT(x, y, z) x ## y ## z
#define GET_DEVICE_TYPE(value) CONCAT(EMBER_AF_GBCS_, value, _DEVICE_TYPE)

static EmberEUI64 DEVICE1_EUI   = EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE1_EUI;
#define DEVICE1_TYPE GET_DEVICE_TYPE(EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE1_TYPE)
static EmberEUI64 DEVICE2_EUI   = EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE2_EUI;
#define DEVICE2_TYPE GET_DEVICE_TYPE(EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE2_TYPE)
static EmberEUI64 DEVICE3_EUI = EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE3_EUI;
#define DEVICE3_TYPE GET_DEVICE_TYPE(EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE3_TYPE)
static EmberEUI64 DEVICE4_EUI  = EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE4_EUI;
#define DEVICE4_TYPE GET_DEVICE_TYPE(EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE4_TYPE)

//------------------------------------------------------------------------------
// Forward declarations

static uint8_t findDeviceLogEntry(EmberEUI64 deviceId);
static uint8_t findFreeDeviceLogEntry(void);

//------------------------------------------------------------------------------
// Initialization

void emberAfPluginGbcsDeviceLogInitCallback(void)
{
  emberAfPluginGbcsDeviceLogReset();
}

//------------------------------------------------------------------------------
// Public API

void emberAfPluginGbcsDeviceLogClear(void)
{
  MEMSET(deviceLog, 0, sizeof(deviceLogEntry) * EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE);
  emberAfDebugPrintln("GBCS Device Log: Cleared");
}

void emberAfPluginGbcsDeviceLogReset(void)
{
  EmberEUI64 deviceId;
  EmberAfGBCSDeviceLogInfo deviceInfo;

  emberAfPluginGbcsDeviceLogClear();

  if (!isNullEui(DEVICE1_EUI)) {
    deviceInfo.deviceType = DEVICE1_TYPE;
    emberReverseMemCopy(deviceId, DEVICE1_EUI, EUI64_SIZE);
    emberAfPluginGbcsDeviceLogStore(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE2_EUI)) {
    deviceInfo.deviceType = DEVICE2_TYPE;
    emberReverseMemCopy(deviceId, DEVICE2_EUI, EUI64_SIZE);
    emberAfPluginGbcsDeviceLogStore(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE3_EUI)) {
    deviceInfo.deviceType = DEVICE3_TYPE;
    emberReverseMemCopy(deviceId, DEVICE3_EUI, EUI64_SIZE);
    emberAfPluginGbcsDeviceLogStore(deviceId, &deviceInfo);
  }

  if (!isNullEui(DEVICE4_EUI)) {
    deviceInfo.deviceType = DEVICE4_TYPE;
    emberReverseMemCopy(deviceId, DEVICE4_EUI, EUI64_SIZE);
    emberAfPluginGbcsDeviceLogStore(deviceId, &deviceInfo);
  }

  emberAfDebugPrintln("GBCS Device Log: Reset complete");
}

uint8_t emberAfPluginGbcsDeviceLogMaxSize(void)
{
  return EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE;
}

uint8_t emberAfPluginGbcsDeviceLogCount(void)
{
  uint8_t i;
  uint8_t count = 0;

  for (i = 0; i < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])) {
      count++;
    }
  }

  emberAfDebugPrintln("GBCS Device Log: Current number of entries is %d", count);
  return count;
}

bool emberAfPluginGbcsDeviceLogStore(EmberEUI64 deviceId,
                                     EmberAfGBCSDeviceLogInfo *deviceInfo)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    emberAfDebugPrint("GBCS Device Log: Updating device at index %d. EUI64=", i);
    emberAfDebugDebugExec(emberAfPrintBigEndianEui64(deviceId));
    emberAfDebugPrintln(", type=%d", deviceInfo->deviceType);
    deviceLog[i].deviceInfo.deviceType     = deviceInfo->deviceType;
    return true;
  }

  i = findFreeDeviceLogEntry();
  if (isValidDeviceLogIndex(i)) {
    emberAfDebugPrint("GBCS Device Log: Adding device at index %d. EUI64=", i);
    emberAfDebugDebugExec(emberAfPrintBigEndianEui64(deviceId));
    emberAfDebugPrintln(", type=%d", deviceInfo->deviceType);
    MEMCOPY(deviceLog[i].deviceId, deviceId, EUI64_SIZE);
    deviceLog[i].deviceInfo.deviceType     = deviceInfo->deviceType;
    deviceLog[i].flags |= GBCS_DEVICE_LOG_ENTRY_INUSE;
    return true;
  }

  emberAfPluginGbcsDeviceLogPrintln("%p%p%p",
                                    "Error: ",
                                    "Cannot add device to GBCS Device Log: ",
                                    "Too many entries");
  return false;
}

bool emberAfPluginGbcsDeviceLogRemove(EmberEUI64 deviceId)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    emberAfDebugPrint("GBCS Device Log: Removing device at index %d. EUI64=", i);
    emberAfDebugDebugExec(emberAfPrintBigEndianEui64(deviceId));
    emberAfDebugPrintln("");
    MEMSET(&deviceLog[i], 0, sizeof(deviceLogEntry));
    emberAfPluginGbcsDeviceLogDeviceRemovedCallback(deviceId);
    return true;
  }

  emberAfPluginGbcsDeviceLogPrintln("%p%p%p",
                                    "Error: ",
                                    "Cannot remove device from GBCS Device Log: ",
                                    "Device does not exist");
  return false;
}

bool emberAfPluginGbcsDeviceLogRetrieveByIndex(uint8_t index,
                                               EmberEUI64 returnDeviceId,
                                               EmberAfGBCSDeviceLogInfo* returnDeviceInfo)
{
  if (!isValidDeviceLogIndex(index)) {
    return false;
  }

  if (!deviceLogEntryInUse(&deviceLog[index])) {
    return false;
  }

  MEMCOPY(returnDeviceId, deviceLog[index].deviceId, EUI64_SIZE);
  *returnDeviceInfo = deviceLog[index].deviceInfo;
  return true;
}

bool emberAfPluginGbcsDeviceLogGet(EmberEUI64 deviceId,
                                   EmberAfGBCSDeviceLogInfo *deviceInfo)
{
  uint8_t i;

  i = findDeviceLogEntry(deviceId);
  if (isValidDeviceLogIndex(i)) {
    // emberAfDebugPrint("GBCS Device Log: Retrieving device info at index %d. EUI64=", i);
    // emberAfDebugDebugExec(emberAfPrintBigEndianEui64(deviceId));
    // emberAfDebugPrintln(", type=%d, deviceLog[i].deviceInfo.deviceType);
    deviceInfo->deviceType = deviceLog[i].deviceInfo.deviceType;
    return true;
  }

  emberAfPluginGbcsDeviceLogPrintln("%p%p%p",
                                    "Error: ",
                                    "Cannot retrieve device info from GBCS Device Log: ",
                                    "Device does not exist");
  return false;
}

bool emberAfPluginGbcsDeviceLogExists(EmberEUI64 deviceId,
                                      EmberAfGBCSDeviceType deviceType)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])
        && (deviceLog[i].deviceInfo.deviceType == deviceType)
        && (MEMCOMPARE(deviceId, deviceLog[i].deviceId, EUI64_SIZE) == 0)) {
      return true;
    }
  }

  return false;
}

void emberAfPluginGbcsDeviceLogPrintEntries(void)
{
  uint8_t i;
  bool entryPrinted = false;

  for (i = 0; i < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])) {
      emberAfPluginGbcsDeviceLogPrint("Device info at index %d. EUI64=", i);
      emberAfPluginGbcsDeviceLogDebugExec(emberAfPrintBigEndianEui64(deviceLog[i].deviceId));
      emberAfPluginGbcsDeviceLogPrintln(", type=%d", deviceLog[i].deviceInfo.deviceType);
      entryPrinted = true;
    }
  }

  if (!entryPrinted) {
    emberAfPluginGbcsDeviceLogPrintln("There are currently no entries in the GBCS Device Log");
  }
}

//------------------------------------------------------------------------------
// Internal functions

static uint8_t findDeviceLogEntry(EmberEUI64 deviceId)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (deviceLogEntryInUse(&deviceLog[i])
        && (MEMCOMPARE(deviceId, deviceLog[i].deviceId, EUI64_SIZE) == 0)) {
      return i;
    }
  }

  return EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX;
}

static uint8_t findFreeDeviceLogEntry(void)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_DEVICE_LOG_SIZE; i++) {
    if (!deviceLogEntryInUse(&deviceLog[i])) {
      return i;
    }
  }

  return EMBER_AF_PLUGIN_GBCS_DEVICE_LOG_INVALID_INDEX;
}
