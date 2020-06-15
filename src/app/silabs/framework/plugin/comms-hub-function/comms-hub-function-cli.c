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
 * @brief CLI for the Comms Hub Function plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/command-interpreter2.h"
#include "comms-hub-function.h"
#include "comms-hub-tunnel-endpoints.h"
#include "tunnel-manager.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"

typedef struct {
  uint16_t messageCode;
  const char *  description;
} GBCSUseCaseDescription;

static GBCSUseCaseDescription useCaseDescriptions[] = {
  { GCS05_MESSAGE_CODE, "GCS05 Update Prepayment Configurations on GSME" },
  { GCS06_MESSAGE_CODE, "GCS06 Activate Emergency Credit Remotely on GSME" },
  { GCS07_MESSAGE_CODE, "GCS07 Send Message to GSME" },
  { GCS11_MESSAGE_CODE, "GCS11 Disable Privacy PIN Protection on GSME" },
  { GCS23_MESSAGE_CODE, "GCS23 Set CV and Conversion Factor Value(s) on the GSME" },
  { GCS44_MESSAGE_CODE, "GCS44 Write Contact Details on GSME" },
  { GCS01b_MESSAGE_CODE, "GCS01b Set Price on GSME" },
};
#define GBCS_NUM_USE_CASES (sizeof(useCaseDescriptions) / sizeof(useCaseDescriptions[0]))

#define EMBER_AF_COMMS_HUB_FUNCTION_MSG_CACHE   (3)
#define EMBER_AF_COMMS_HUB_FUNCTION_SEND_LENGTH (350)

static uint8_t messagePayload[EMBER_AF_COMMS_HUB_FUNCTION_MSG_CACHE][EMBER_AF_COMMS_HUB_FUNCTION_SEND_LENGTH];
static uint8_t nextMessage = 0;

// Prototypes
static void sendMessage(EmberEUI64 deviceId, uint16_t length, uint8_t *message, bool includeHeader, uint16_t messageCode);
void emAfPluginCommsHubFunctionCliPrintSupportedUseCases(void);

// Internal Functions
static uint16_t findUseCaseDescription(uint16_t messageCode)
{
  uint16_t index;

  for (index = 0; index < GBCS_NUM_USE_CASES; index++) {
    if (useCaseDescriptions[index].messageCode == messageCode) {
      return index;
    }
  }

  return index;
}

// External Functions

// plugin comms-hub-function simulate-gbz-msg <uint16_t:messageCode>
void emAfPluginCommsHubFunctionCliSimulateGbzMsg(void)
{
  EmberEUI64 destinationDeviceId;
  uint16_t messageCode = (uint16_t)emberUnsignedCommandArgument(1);
  uint32_t currentTime = emberAfGetCurrentTime();
  uint16_t index = findUseCaseDescription(messageCode);

  emberAfCopyBigEndianEui64Argument(0, destinationDeviceId);

  if (index >= GBCS_NUM_USE_CASES) {
    emberAfPluginCommsHubFunctionPrintln("Unsupported message code: 0x%2x", messageCode);
    emAfPluginCommsHubFunctionCliPrintSupportedUseCases();
    return;
  }

  emberAfPluginCommsHubFunctionPrintln("GBCS Use Case: %p", useCaseDescriptions[index].description);
  emberAfPluginCommsHubFunctionPrintln("Current Time: 0x%4x", currentTime);

  if (messageCode == GCS05_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x0B, // component count - 8-14

                             // only testing simple cmds for now.
                             // component #0 - Emergency Credit Setup
                             0x00,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x13,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x03,  // ZCL command id
                             0x00, 0x00, 0x00, 0x00,  // issuerEventId - cur utc time // index-11
                             0x00, 0x00, 0x00, 0x00,  // startTime - now
                             0x00, 0x00, 0x00, 0x00,  // emergency credit limit
                             0x00, 0x00, 0x00, 0x00,  // emergency credit threshold

                             // component #1 - Set Overall Debt Cap
                             0x00,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x13,  // extended gbz command length
                             0x01,  // frame control
                             0x01,  // trans. seq number
                             0x0C,  // ZCL command id
                             0x00, 0x00, 0x00, 0x00,  // provider id - 0x00
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - cur utc time // index-39
                             0x00, 0x00, 0x00, 0x00,  // implementation date/time - now
                             0x44, 0x33, 0x22, 0x11,  // OverallDebtCap

                             // component #2 - Set Low Credit Warning Level
                             0x00,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x07,  // extended gbz command length
                             0x01,  // frame control
                             0x02,  // trans. seq number
                             0x09,  // ZCL command id
                             0x44, 0x33, 0x22, 0x11,  // low credit warning level

                             // component #3 - Set Maximum Credit Limit
                             0x00,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x17,  // extended gbz command length
                             0x01,  // frame control
                             0x03,  // trans. seq number
                             0x0B,  // ZCL command id
                             0x00, 0x00, 0x00, 0x00,  // provider id - unused
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - // index-75
                             0x00, 0x00, 0x00, 0x00,  // implementation date/time -now
                             0x44, 0x33, 0x22, 0x11,  // max meter balance
                             0x00, 0x33, 0x22, 0x11,  // largest value of any one credit

                             // component #4 - PublishCalendar
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x19,  // extended gbz command length
                             0x09,  // frame control
                             0x04,  // trans. seq number
                             0x00,  // command id - PublishCalendar
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-103
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-107
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x03,  // calendar type - 3 / Friendly Credit Calendar
                             0x00,  // calendarTimeRef - 0x00 - UTC time
                             0x00,  // calendar name
                             0x04,  // number of seasons
                             0x02,  // number of week profiles
                             0x02,  // number of day profiles

                             // component #5 - PublishDayProfile
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x1D,  // extended gbz command length
                             0x09,  // frame control
                             0x05,  // trans. seq number
                             0x01,  // command id - PublishDayProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-133
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-137
                             0x01,   // day id
                             0x03,  // total number of schedule entries
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x03,  // calendar type - 3 / Friendly Credit Calendar
                             0x00, 0x00, 0x01,  //day sch entry 0 - 12:00 AM, tier 1
                             0xE0, 0x01, 0x02,  //day sch entry 1 - 08:00 AM, tier 2
                             0xC0, 0x03, 0x03,  //day sch entry 2 - 04:00 PM, tier 3

                             // component #6 - PublishDayProfile
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x1D,  // extended gbz command length
                             0x09,  // frame control
                             0x06,  // trans. seq number
                             0x01,  // command id - PublishDayProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-167
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-171
                             0x02,  // day id
                             0x03,  // total number of schedule entries
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x03,  // calendar type - 3 / Friendly Credit Calendar
                             0x00, 0x00, 0x04,  //day sch entry 0 - 12:00 AM, tier 4
                             0xE0, 0x01, 0x05,  //day sch entry 1 - 08:00 AM, tier 5
                             0xC0, 0x03, 0x06,  //day sch entry 2 - 04:00 PM, tier 6

                             // component #7 - PublishWeekProfile
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x17,  // extended gbz command length
                             0x09,  // frame control
                             0x07,  // trans. seq number
                             0x02,  // command id - PublishWeekProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-201
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-205
                             0x01,  // week id
                             0x01,  // mon
                             0x01,  // tue
                             0x01,  // wed
                             0x01,  // thu
                             0x01,  // fri
                             0x01,  // sat
                             0x01,  // sun

                             // component #8 - PublishWeekProfile
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x17,  // extended gbz command length
                             0x09,  // frame control
                             0x08,  // trans. seq number
                             0x02,  // command id - PublishWeekProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-229
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-233
                             0x02,  // week id
                             0x02,  // mon
                             0x02,  // tue
                             0x02,  // wed
                             0x02,  // thu
                             0x02,  // fri
                             0x02,  // sat
                             0x02,  // sun

                             // component #9 - PublishSeasons
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x25,  // extended gbz command length
                             0x09,  // frame control
                             0x09,  // trans. seq number
                             0x03,  // command id - PublishSeasons
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-257
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-261
                             0x00,  // command index
                             0x01,  // number of commands
                             0x75, 0x01, 0x01, 0x00, 0x01,  // season entry 0 - 01/01/2017, wk 1
                             0x75, 0x04, 0x01, 0x00, 0x02,  // season entry 0 - 04/01/2017, wk 2
                             0x75, 0x07, 0x01, 0x00, 0x02,  // season entry 0 - 07/01/2017, wk 2
                             0x75, 0x0A, 0x01, 0x00, 0x01,  // season entry 0 - 10/01/2017, wk 1

                             // component #10 - PublishSpecialDays
                             0x01,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x1C,  // extended gbz command length
                             0x09,  // frame control
                             0x0A,  // trans. seq number
                             0x04,  // command id - PublishSpecialDays
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-299
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-303
                             0x00, 0x00, 0x00, 0x00,  // start time
                             0x03,  // calendar type - Friendly Credit Calendar
                             0x01,  // total number of specialdays
                             0x00,  // Command Index
                             0x01,  // total number of commands
                             0x75, 0x06, 0x01, 0x00, 0x01,  // special day - 06/01/2017, ref day id - 1
    };

    emberAfCopyInt32u(gbzCommand, 11, currentTime);
    emberAfCopyInt32u(gbzCommand, 39, currentTime);
    emberAfCopyInt32u(gbzCommand, 75, currentTime);
    emberAfCopyInt32u(gbzCommand, 103, currentTime);
    emberAfCopyInt32u(gbzCommand, 107, currentTime);
    emberAfCopyInt32u(gbzCommand, 133, currentTime);
    emberAfCopyInt32u(gbzCommand, 137, currentTime);
    emberAfCopyInt32u(gbzCommand, 167, currentTime);
    emberAfCopyInt32u(gbzCommand, 171, currentTime);
    emberAfCopyInt32u(gbzCommand, 201, currentTime);
    emberAfCopyInt32u(gbzCommand, 205, currentTime);
    emberAfCopyInt32u(gbzCommand, 229, currentTime);
    emberAfCopyInt32u(gbzCommand, 233, currentTime);
    emberAfCopyInt32u(gbzCommand, 257, currentTime);
    emberAfCopyInt32u(gbzCommand, 261, currentTime);
    emberAfCopyInt32u(gbzCommand, 299, currentTime);
    emberAfCopyInt32u(gbzCommand, 303, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS06_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01, // extended header control field
                             0x07, 0x05, // extended header cluster id
                             0x00, 0x08, // extended gbz command length
                             0x01, // frame control
                             0x00, // trans. seq number
                             0x00, // ZCL command id
                             0x00, 0x00, 0x00, 0x00, // Date and time command is issued
                             0x00, // Source: 0x00=Energy Service Interface
    };

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS07_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01, // extended header control field
                             0x07, 0x03, // extended header cluster id
                             0x00, 0x12, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x00, // ZCL command id - Display Message
                             0x00, 0x00, 0x00, 0x00, // msg id - curtime
                             0x00, // msg control - normal transmission
                             0x00, 0x00, 0x00, 0x00, // start time - now
                             0xFF, 0xFF, // duration in minutes - until changed
                             0x03, // msg length
                             0x41, 0x42, 0x43 // msg = "ABC"
    };
    emberAfCopyInt32u(gbzCommand, 11, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS11_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01, // extended header control field
                             0x07, 0x08, // extended header cluster id
                             0x00, 0x0F, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x02, // ZCL command id - Display Message
                             0x00, 0x00, 0x00, 0x00, // issuer id - curtime
                             0x00, 0x00, 0x00, 0x00, // start time - now
                             0x00, 0x00, // PIN is valid until changed
                             0x02, // 2 = access to the consumer menu
                             0x00, //Password, 0-length string
    };
    emberAfCopyInt32u(gbzCommand, 11, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS23_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x11, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x03, // ZCL command id
                             0x00, 0x00, 0x00, 0x00, // issuer event id - curtime
                             0x00, 0x00, 0x00, 0x00, // current time - curtime
                             0x04, 0x03, 0x02, 0x01, // calorific value
                             0x01, // calorific value unit - MJ/m3
                             0x10, // calorific value trailing digit

                             // component #2
                             0x01, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x10, // extended gbz command length
                             0x09, // frame control
                             0x01, // trans. seq number
                             0x02, // ZCL command id
                             0x00, 0x00, 0x00, 0x00, // issuer event id - curtime
                             0x00, 0x00, 0x00, 0x00, // current time - curtime
                             0x0D, 0x0C, 0x0B, 0x0A, // conversion factor -
                             0x50, // conversion factor trailing digit
    };
    emberAfCopyInt32u(gbzCommand, 11, currentTime);
    emberAfCopyInt32u(gbzCommand, 33, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS44_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01, // extended header control field
                             0x07, 0x08, // extended header cluster id
                             0x00, 0x1C, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x01, // ZCL command id - ChangeOfSupplier
                             0x00, 0x00, 0x00, 0x00, // current provider id
                             0x00, 0x00, 0x00, 0x00, // issuer event id - curtime
                             0x00, // tariffType - 0
                             0x00, 0x00, 0x00, 0x00, // proposed provider id
                             0x00, 0x00, 0x00, 0x00, // proposed change implementation time - curtime
                             0x00, 0x00, 0x00, 0x00, // provider change control
                             0x01, 0xAA, // proposed provider name
                             0x01, 0xBB // proposed provider contact details
    };
    emberAfCopyInt32u(gbzCommand, 15, currentTime);
    emberAfCopyInt32u(gbzCommand, 24, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  } else if (messageCode == GCS01b_MESSAGE_CODE) {
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x27, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x04, // ZCL command id - PublishTariffInfo
                             0x00, 0x00, 0x00, 0x00, // provider id
                             0x00, 0x00, 0x00, 0x00, // issuer event id - curtime
                             0x01, 0x00, 0x00, 0x00, // issuer tariff id - tariff switching table
                             0x00, 0x00, 0x00, 0x00, // starttime - 0
                             0x00, // tarifftype - 0
                             0x01, 0xAA, // tariff label - 0
                             0x01, // number of price tiers in use
                             0x01, // number of block tiers in use
                             0x00, // unit of measurement
                             0x0A, 0x33, // currency - GBP
                             0x00, // price trailing digit
                             0x00, 0x00, 0x00, 0x00, // standing charge
                             0xFF, // tier block mode
                             0x00, 0x00, 0x00, // block threshold multiplier
                             0x00, 0x00, 0x00, // block threshold divisor

                             // component #2 - PublishPriceMatrix
                             0x01, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x16, // extended gbz command length
                             0x09, // frame control
                             0x01, // trans. seq number
                             0x05, // ZCL command id - PublishPriceMatrix
                             0x00, 0x00, 0x00, 0x00, // provider id - unused
                             0x00, 0x00, 0x00, 0x00, // issuer event id - curtime
                             0x00, 0x00, 0x00, 0x00, // start time - 0
                             0x01, 0x00, 0x00, 0x00, // issuer tariff id - 1, tariff switching table
                             0x00, // command index
                             0x01, // command index
                             0x00, // sub-payload control
                             0x01, // tier block id
                             0x04, 0x03, 0x02, 0x01 // tier block id price
    };
    emberAfCopyInt32u(gbzCommand, 14, currentTime);
    emberAfCopyInt32u(gbzCommand, 58, currentTime);

    emberAfPluginCommsHubFunctionPrint("GBZ Command: ");
    emberAfPluginCommsHubFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginCommsHubFunctionPrintln("");

    sendMessage(destinationDeviceId, sizeof(gbzCommand), gbzCommand, true, messageCode);
  }
}

void emAfPluginCommsHubFunctionCliPrintSupportedUseCases(void)
{
  uint16_t index;

  emberAfPluginCommsHubFunctionPrintln("Supported Use Cases");
  for (index = 0; index < GBCS_NUM_USE_CASES; index++) {
    emberAfPluginCommsHubFunctionPrintln("Message Code: 0x%2x, GBCS Use Case: %p",
                                         useCaseDescriptions[index].messageCode,
                                         useCaseDescriptions[index].description);
  }
}

void emAfPluginCommsHubFunctionCliSend(void)
{
  EmberEUI64 deviceId;
  uint8_t length;
  uint8_t message[255];
  emberAfCopyBigEndianEui64Argument(0, deviceId);
  length = emberCopyStringArgument(1,
                                   message,
                                   255,
                                   false);
  sendMessage(deviceId, length, message, false, TEST_MESSAGE_CODE);
}

void emAfPluginCommsHubFunctionCliTimeout(void)
{
  uint32_t timeout = (uint32_t)emberUnsignedCommandArgument(0);
  emAfPluginCommsHubFunctionSetDefaultTimeout(timeout);
}

static void sendMessage(EmberEUI64 deviceId, uint16_t length, uint8_t *message, bool includeHeader, uint16_t messageCode)
{
  uint8_t i;
  uint8_t *data = &(messagePayload[nextMessage][0]);
  uint16_t headerLength = 0;
  EmberAfPluginCommsHubFunctionStatus status;

  // Tack on a dummy header
  if (includeHeader) {
    for (i = 0; i < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_GBZ_OFFSET; i++) {
      *data++ = i;
    }
    headerLength = EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_GBZ_OFFSET;
  }

  if (headerLength + length > EMBER_AF_COMMS_HUB_FUNCTION_SEND_LENGTH) {
    emberAfPluginCommsHubFunctionPrintln("Message is too long.  Truncating excess bytes.");
    length = EMBER_AF_COMMS_HUB_FUNCTION_SEND_LENGTH - headerLength;
  }
  MEMCOPY(data, message, length);

  status = emberAfPluginCommsHubFunctionSend(deviceId, headerLength + length, messagePayload[nextMessage], messageCode);
  if (status == EMBER_AF_CHF_STATUS_SUCCESS) {
    emberAfPluginCommsHubFunctionPrintln("Message has been successfully sent or queued to be sent to the destination");
  }
  if (++nextMessage == EMBER_AF_COMMS_HUB_FUNCTION_MSG_CACHE) {
    nextMessage = 0;
  }
}

// plugin comms-hub-function get-tunnel-endpoint <uint16_t:nodeId>
void emAfPluginCommsHubFunctionCliGetTunnelEndpoint()
{
  uint8_t  endpoint;
  uint16_t nodeId;

  nodeId = (uint16_t)emberUnsignedCommandArgument(0);
  endpoint = emberAfPluginGetDeviceTunnelingEndpoint(nodeId);
  if ( endpoint == INVALID_TUNNELING_ENDPOINT ) {
    emberAfPluginCommsHubFunctionPrintln("Invalid Tunneling Endpoint for 0x%2x", nodeId);
  } else {
    emberAfPluginCommsHubFunctionPrintln("Tunnel Endpoint=0x%x", endpoint);
  }
}

void emAfPluginCommsHubFunctionCliCloseTunnel()
{
  EmberEUI64 deviceId;

  emberAfCopyBigEndianEui64Argument(0, deviceId);
  emAfPluginCommsHubFunctionTunnelClose(deviceId);
}

void emAfPluginCommsHubFunctionCliCreateTunnel()
{
  EmberEUI64 deviceId;
  uint8_t endpoint;

  endpoint = (uint8_t)emberUnsignedCommandArgument(1);

  emberAfCopyBigEndianEui64Argument(0, deviceId);
  emAfPluginCommsHubFunctionTunnelCreate(deviceId, endpoint);
}

void emAfPluginCommsHubFunctionCliPrint()
{
  emAfPluginCommsHubFunctionPrint();
}
