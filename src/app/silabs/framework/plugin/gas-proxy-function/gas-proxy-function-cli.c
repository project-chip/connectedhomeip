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
 * @brief CLI for the Gas Proxy Function plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "gas-proxy-function.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"

typedef struct {
  uint16_t messageCode;
  const char *  description;
} GBCSUseCaseDescription;

static GBCSUseCaseDescription useCaseDescriptions[] = {
  { CS10a_MESSAGE_CODE, "CS10a Read ZigBee Device Event Log" },
  { CS10b_MESSAGE_CODE, "CS10b Read ZigBee Device Security Log" },
  { CS11_MESSAGE_CODE, "CS11 Clear ZigBee Device Event Log" },
  { GCS01a_MESSAGE_CODE, "GCS01a Set Tariff and Price on GSME" },
  { GCS01b_MESSAGE_CODE, "GCS01b Set Price on GSME" },
  { GCS05_MESSAGE_CODE, "GCS05 Update Prepayment Configurations on GSME" },
  { GCS07_MESSAGE_CODE, "GCS07 Send Message to GSME" },
  { GCS09_MESSAGE_CODE, "GCS09 Set Change of Tenancy date on GPF" },
  { GCS13a_MESSAGE_CODE, "GCS13a Read GSME Consumption Register" },
  { GCS13b_MESSAGE_CODE, "GCS13b Read GSME Block Counters" },
  { GCS13c_MESSAGE_CODE, "GCS13c Read GSME Register (TOU)" },
  { GCS14_MESSAGE_CODE, "GCS14 Read GSME Prepayment Register(s)" },
  { GCS15b_MESSAGE_CODE, "GCS15b Read GSME Billing Data Log (change of mode / tariff triggered)" },
  { GCS15c_MESSAGE_CODE, "GCS15c Read GSME Billing Data Log (billing calendar triggered)" },
  { GCS15d_MESSAGE_CODE, "GCS15d Read GSME Billing Data Log (payment-based debt payments)" },
  { GCS15e_MESSAGE_CODE, "GCS15e Read GSME Billing Data Log (prepayment credits)" },
  { GCS16a_MESSAGE_CODE, "GCS16a Read GSME Daily Read log(s)" },
  { GCS16b_MESSAGE_CODE, "GCS16b Read GSME Daily Read log(s) (prepayment)" },
  { GCS17_MESSAGE_CODE, "GCS17 Read GSME Profile Data Log" },
  { GCS21b_MESSAGE_CODE, "GCS21b Read GSME Configuration Data Prepayment" },
  { GCS21d_MESSAGE_CODE, "GCS21d Read GSME Configuration Data Device Information (BillingCalendar)" },
  { GCS21e_MESSAGE_CODE, "GCS21e Read GSME/GPF Configuration Data Device Information (device identity)" },
  { GCS21f_MESSAGE_CODE, "GCS21f Read GSME Tariff Data" },
  { GCS21j_MESSAGE_CODE, "GCS21j Read GSME Configuration Data Device Information (Payment Mode)" },
  { GCS23_MESSAGE_CODE, "GCS23 Set CV and Conversion Factor Value(s) on the GSME" },
  { GCS25_MESSAGE_CODE, "GCS25 Set Billing Calendar on the GSME" },
  { GCS33_MESSAGE_CODE, "GCS33 Read GSME Valve Status" },
  { GCS38_MESSAGE_CODE, "GCS38 Read GSME Firmware Version" },
  { GCS44_MESSAGE_CODE, "GCS44 Write Contact Details on GSME" },
  { GCS46_MESSAGE_CODE, "GCS46 Read MPRN on the GSME" },
  { GCS60_MESSAGE_CODE, "GCS60 Read Meter Balance for GSME" },
  { GCS61_MESSAGE_CODE, "GCS61 Read gas Daily Consumption Log" },
  { TEST_ENCRYPTED_MESSAGE_CODE, "Test Message Code for Encrypted Messages" },
  { TEST_MESSAGE_CODE, "Test Message Code for Non-Encrypted Messages" },
};
#define GBCS_NUM_USE_CASES (sizeof(useCaseDescriptions) / sizeof(useCaseDescriptions[0]))

// Prototypes
void emAfPluginGasProxyFunctionCliSimulateGbzMsg(void);
void emAfPluginGasProxyFunctionCliPrintSupportedUseCases(void);

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

// plugin gas-proxy-function simulate-gbz-msg-creation
void emAfPluginGasProxyFunctionCliSimulateGbzMsgCreation(void)
{
  // simple test for creation of PublishBillingPeriod ZCL Command in GBZ
  // command.
  EmberAfGbzMessageCreatorState creator = { 0 };
  EmberAfGbzMessageCreatorState creatorAllocMem = { 0 };
  uint8_t buffer[200];
  uint8_t zclPayload[17];
  uint16_t maxLen = 200;
  uint16_t writtenByte;
  EmberAfGbzZclCommand cmd = { 0 };
  EmberAfGbzMessageCreatorResult  * result = NULL;

  MEMSET(buffer, 0x00, 200);
  MEMSET(zclPayload, 0x00, 17);
  emberAfCopyInt32u(zclPayload, 0, 0x10203040); // providerId
  emberAfCopyInt32u(zclPayload, 4, emberAfGetCurrentTime()); // issuerEventId
  emberAfCopyInt32u(zclPayload, 8, 0x50607080); // startTime
  emberAfCopyInt24u(zclPayload, 12, 0x90A0B0); // duration
  zclPayload[15] = 0x00; // minute
  zclPayload[16] = 0x00; // minute

  cmd.encryption = false;
  cmd.frameControl = 0x09;
  cmd.clusterId = ZCL_PRICE_CLUSTER_ID;
  cmd.commandId = ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID;
  cmd.payload = &zclPayload[0];
  cmd.hasFromDateTime = false;

  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creator,
                                                             EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                             0,
                                                             0,
                                                             TEST_MESSAGE_CODE,
                                                             buffer,
                                                             maxLen);
  if (!writtenByte) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to initialize GBZ message creator!");
    return;
  }

  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creatorAllocMem,
                                                             EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                             0,
                                                             0,
                                                             TEST_MESSAGE_CODE,
                                                             NULL,
                                                             0);
  if (!writtenByte) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to initialize GBZ message creator!");
    return;
  }

  cmd.payloadLength = 17;
  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);
  emberAfPluginGbzMessageControllerAppendCommand(&creatorAllocMem, &cmd);

  cmd.clusterId = ZCL_SIMPLE_METERING_CLUSTER_ID;
  cmd.commandId = ZCL_PUBLISH_SNAPSHOT_COMMAND_ID;
  cmd.payloadLength = 9;
  cmd.clusterSpecific = true;
  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);
  emberAfPluginGbzMessageControllerAppendCommand(&creatorAllocMem, &cmd);

  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creator);
  emberAfPluginGasProxyFunctionPrintln("GPF: In Place message construction");
  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ command message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ command message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creator);

  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creatorAllocMem);
  emberAfPluginGasProxyFunctionPrintln("GPF: In New Memory message construction");
  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ command message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ command message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creatorAllocMem);

  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creator,
                                                             EMBER_AF_GBZ_MESSAGE_RESPONSE,
                                                             0,
                                                             0,
                                                             TEST_MESSAGE_CODE,
                                                             &buffer[0],
                                                             maxLen);
  if (!writtenByte) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to initialize GBZ message creator!");
    return;
  }

  cmd.payloadLength = 17;

  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);

  // testing if appending 2 zcl commands will work.
  // change up length to append a "fake" zcl command.
  cmd.payloadLength = 9;

  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);
  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creator);

  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ response message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ response message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creator);

  /* Testing encrypted command */

  /* In-place append */
  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creator,
                                                             EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                             0,
                                                             0,
                                                             TEST_ENCRYPTED_MESSAGE_CODE,
                                                             &buffer[0],
                                                             maxLen);
  if (!writtenByte) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to initialize GBZ message creator!");
    return;
  }

  cmd.payloadLength = 17;
  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);
  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creator);

  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ command encrypted message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ command encrypted message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creator);

  /* Appending in new memory */

  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creatorAllocMem,
                                                             EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                             0,
                                                             0,
                                                             TEST_ENCRYPTED_MESSAGE_CODE,
                                                             NULL,
                                                             0);

  emberAfPluginGbzMessageControllerAppendCommand(&creatorAllocMem, &cmd);
  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creatorAllocMem);

  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ command encrypted message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ command encrypted message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creatorAllocMem);

  /* end of Testing encrypted command */

  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creator,
                                                             EMBER_AF_GBZ_MESSAGE_ALERT,
                                                             0x1234,
                                                             emberAfGetCurrentTime(),
                                                             TEST_MESSAGE_CODE,
                                                             &buffer[0],
                                                             maxLen);
  if (!writtenByte) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unable to initialize GBZ message creator!");
    return;
  }

  cmd.payloadLength = 17;

  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);

  // testing if appending 2 zcl commands will work.
  // change up length to append a "fake" zcl command.
  cmd.payloadLength = 9;

  emberAfPluginGbzMessageControllerAppendCommand(&creator, &cmd);
  result = emberAfPluginGbzMessageControllerCreatorAssemble(&creator);

  emberAfPluginGasProxyFunctionPrintln("GPF: GBZ alert message len: %d", result->payloadLength);
  emberAfPluginGasProxyFunctionPrint("GPF: GBZ alert message: ");
  emberAfPluginGasProxyFunctionPrintBuffer(result->payload, result->payloadLength, true);
  emberAfPluginGasProxyFunctionPrintln("");
  emberAfPluginGbzMessageControllerCreatorCleanup(&creator);

  // Simulate memory Cleanup() in case no Assemble() was called.
  writtenByte = emberAfPluginGbzMessageControllerCreatorInit(&creatorAllocMem,
                                                             EMBER_AF_GBZ_MESSAGE_COMMAND,
                                                             0,
                                                             0,
                                                             TEST_MESSAGE_CODE,
                                                             NULL,
                                                             0);
  emberAfPluginGbzMessageControllerAppendCommand(&creatorAllocMem, &cmd);
  emberAfPluginGbzMessageControllerCreatorCleanup(&creatorAllocMem);
}

// plugin gas-proxy-function simulate-future-dated-msg
void emAfPluginGasProxyFunctionCliSimulateFutureDatedMsg(void)
{
  uint32_t futureTime = emberAfGetCurrentTime() + 20000;
  emberAfPluginGasProxyFunctionPrintln("GPF: Future Time: 0x%4x", futureTime);

  // Command
  uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                           0x01, // component count

                           // component #1 - PublishCalorificValue
                           0x00,  // extended header control field
                           0x07, 0x00,  // extended header cluster id
                           0x00, 0x11,  // extended gbz command length
                           0x09,  // frame control
                           0x00,  // trans. seq number
                           0x03,  // ZCL command id - PublishCalorificValue
                           0x00, 0x00, 0x00, 0x00,  // issuer event id
                           0x00, 0x00, 0x00, 0x00,  // start time - now
                           0x04, 0x03, 0x02, 0x01,  // calorific value
                           0x01,  // calorific value unit
                           0x10,  // calorific value trailing digit
  };

  // Normal Response
  uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                               0x01, // component count

                               // component #1 - PublishConversionFactor
                               0x00,  // extended header control field
                               0x07, 0x00,  // extended header cluster id
                               0x00, 0x05,  // extended gbz command length
                               0x00,  // frame control
                               0x00,  // trans. seq number
                               0x0B,  // ZCL command id - default response
                               0x02,  // response to PublishConversionFactor
                               0x00,  // success
  };

  emberAfCopyInt32u(gbzCommand, 15, futureTime);

  emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
  emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
  emberAfPluginGasProxyFunctionPrintln("");

  emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                    sizeof(gbzCommand),
                                                    gbzCommandResp,
                                                    sizeof(gbzCommandResp),
                                                    GCS05_MESSAGE_CODE);
}

// plugin gas-proxy-function simulate-gbz-msg <uint16_t:messageCode>
void emAfPluginGasProxyFunctionCliSimulateGbzMsg(void)
{
  uint16_t messageCode = (uint16_t)emberUnsignedCommandArgument(0);
  uint32_t currentTime = emberAfGetCurrentTime();

  uint16_t index = findUseCaseDescription(messageCode);
  if (index >= GBCS_NUM_USE_CASES) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Unsupported message code: 0x%2x", messageCode);
    emAfPluginGasProxyFunctionCliPrintSupportedUseCases();
    return;
  }

  emberAfPluginGasProxyFunctionPrintln("GPF: GBCS Use Case: %p", useCaseDescriptions[index].description);
  emberAfPluginGasProxyFunctionPrintln("GPF: Current Time: 0x%4x", currentTime);

  if (messageCode == GCS23_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1 - PublishCalorificValue
                             0x00,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x03,  // ZCL command id - PublishCalorificValue
                             0x00, 0x00, 0x00, 0x00,  // issuer event id
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x04, 0x03, 0x02, 0x01,  // calorific value
                             0x01,  // calorific value unit
                             0x10,  // calorific value trailing digit

                             // component #2 - PublishConversionFactor
                             0x01,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x10,  // extended gbz command length
                             0x09,  // frame control
                             0x01,  // trans. seq number
                             0x02,  // ZCL command id - PublishConversionFactor
                             0x00, 0x00, 0x00, 0x00,  // issuer event id
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x0D, 0x0C, 0x0B, 0x0A,  // conversion factor -
                             0x50,  // conversion factor trailing digit
    };
    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x02, // component count

                                 // component #1 - PublishConversionFactor
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x02,  // response to PublishConversionFactor
                                 0x00,  // success

                                 // component #2 - PublishCalorificValue
                                 0x01,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x01,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x03,  // response to PublishCalorificValue
                                 0x00   // success
    };

    emberAfCopyInt32u(gbzCommand, 11, currentTime);
    emberAfCopyInt32u(gbzCommand, 33, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == GCS21d_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1 - GetBillingPeriod
                             0x01,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x0D,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x0B,  // ZCL command id - GetBillingPeriod
                             0x00, 0x00, 0x00, 0x00,  // earliest start time
                             0xFF, 0xFF, 0xFF, 0xFF,  // min issuer event id
                             0x01,  // number of commands
                             0x00   // tariff type - delivered tariff
    };
    emberAfCopyInt32u(gbzCommand, 11, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS21d_MESSAGE_CODE);
  } else if (messageCode == GCS01a_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x0A, // component count

                             // component #0 - PublishCalendar
                             0x00,  // extended header control field
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x19,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // command id - PublishCalendar
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index-15
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-19
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x00,  // calendar type - 0x00 - delivered tariff
                             0x00,  // calendarTimeRef - 0x00 - UTC time
                             0x00,  // calendar name
                             0x04,  // number of seasons
                             0x02,  // number of week profiles
                             0x02,  // number of day profiles

                             // component #1 - PublishDayProfile
                             0x00,  // extended header control field // index 33
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x1D,  // extended gbz command length
                             0x09,  // frame control
                             0x01,  // trans. seq number
                             0x01,  // command id - PublishDayProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id  // index-45
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index-49
                             0x01,   // day id
                             0x03,  // total number of schedule entries
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x00,  // calendar type - 0 / Delivered Calendar
                             0x00, 0x00, 0x01,  //day sch entry 0 - 12:00 AM, tier 1
                             0xE0, 0x01, 0x02,  //day sch entry 1 - 08:00 AM, tier 2
                             0xC0, 0x03, 0x03,  //day sch entry 2 - 04:00 PM, tier 3

                             // component #2 - PublishDayProfile
                             0x00,  // extended header control field // index 67
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x1D,  // extended gbz command length
                             0x09,  // frame control
                             0x02,  // trans. seq number
                             0x01,  // command id - PublishDayProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 79
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index 83
                             0x02,   // day id
                             0x03,  // total number of schedule entries
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x00,  // calendar type - 0 / Delivered Calendar
                             0x00, 0x00, 0x04,  //day sch entry 0 - 12:00 AM, tier 4
                             0xE0, 0x01, 0x05,  //day sch entry 1 - 08:00 AM, tier 5
                             0xC0, 0x03, 0x06,  //day sch entry 2 - 04:00 PM, tier 6

                             // component #3 - PublishWeekProfile
                             0x00,  // extended header control field  // index 101
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x17,  // extended gbz command length
                             0x09,  // frame control
                             0x03,  // trans. seq number
                             0x02,  // command id - PublishWeekProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 113
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index 117
                             0x01,  // week id
                             0x01,  // mon
                             0x01,  // tue
                             0x01,  // wed
                             0x01,  // thu
                             0x01,  // fri
                             0x01,  // sat
                             0x01,  // sun

                             // component #4 - PublishWeekProfile
                             0x00,  // extended header control field // index 129
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x17,  // extended gbz command length
                             0x09,  // frame control
                             0x04,  // trans. seq number
                             0x02,  // command id - PublishWeekProfile
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 141
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index 145
                             0x02,  // week id
                             0x02,  // mon
                             0x02,  // tue
                             0x02,  // wed
                             0x02,  // thu
                             0x02,  // fri
                             0x02,  // sat
                             0x02,  // sun

                             // component #5 - PublishSeasons
                             0x00,  // extended header control field // index 157
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x25,  // extended gbz command length
                             0x09,  // frame control
                             0x05,  // trans. seq number
                             0x03,  // command id - PublishSeasons
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 169
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index 173
                             0x00,  // command index
                             0x01,  // number of commands
                             0x75, 0x01, 0x01, 0x00, 0x01,  // season entry 0 - 01/01/2017, wk 1
                             0x75, 0x04, 0x01, 0x00, 0x02,  // season entry 1 - 04/01/2017, wk 2
                             0x75, 0x07, 0x01, 0x00, 0x02,  // season entry 2 - 07/01/2017, wk 2
                             0x75, 0x0A, 0x01, 0x00, 0x01,  // season entry 3 - 10/01/2017, wk 1

                             // component #6 - PublishSpecialDays
                             0x00,  // extended header control field // index 199
                             0x07, 0x07,  // extended header cluster id
                             0x00, 0x21,  // extended gbz command length
                             0x09,  // frame control
                             0x06,  // trans. seq number
                             0x04,  // command id - PublishSpecialDays
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 211
                             0x00, 0x00, 0x00, 0x00,  // issuer calendar id // index 215
                             0x00, 0x00, 0x00, 0x00,  // start time
                             0x00,  // calendar type - delivered calendar
                             0x02,  // number of special days
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x75, 0x01, 0x02, 0x00, 0x01,  // season entry 0 - 01/02/2017, day id 1
                             0x75, 0x04, 0x02, 0x00, 0x02,  // season entry 0 - 04/02/2017, day id 2

                             // component #7 - PublishTariffInfo
                             0x00,  // extended header control field // index 237
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x26,  // extended gbz command length
                             0x09,  // frame control
                             0x07,  // trans. seq number
                             0x04,  // command id - PublishTariffInfo
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 249
                             0x01, 0x00, 0x00, 0x00,  // issuer tariff id
                             0x00, 0x00, 0x00, 0x00,  // start time
                             0x10,  // Block tariff
                             0x00,  // tariff label
                             0x01,  // number of price tiers in use
                             0x02,  // number of block thresholds in use
                             0x00,  // unit of measure
                             0x3A, 0x03,  // currency
                             0x50,  // price trailing digit
                             0x00, 0x00, 0x00, 0x00,  // standing charge
                             0xFF,  // tier block mode
                             0x01, 0x00, 0x00,  // block thresholds multiplier
                             0xE8, 0x03, 0x00,  // block thresholds divisor

                             // component #8 - PublishBlockThresholds
                             0x00,  // extended header control field // index 282
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x23,  // extended gbz command length
                             0x09,  // frame control
                             0x08,  // trans. seq number
                             0x06,  // command id - PublishBlockThresholds
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 294
                             0x00, 0x00, 0x00, 0x00,  // start time
                             0x01, 0x00, 0x00, 0x00,  // issuer tariff id
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x01,  // sub-payload control - Block thresholds apply to all TOU tiers / block only charging in operation
                             0x02,  // tier / number of block thesholds
                             0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // block thresholds 1
                             0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, // block thresholds 2

                             // component #9 - PublishPriceMatrix
                             0x01,  // extended header control field // 322
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x1B,  // extended gbz command length
                             0x09,  // frame control
                             0x09,  // trans. seq number
                             0x05,  // command id - PublishPriceMatrix
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id // index 334
                             0x00, 0x00, 0x00, 0x00,  // start time
                             0x01, 0x00, 0x00, 0x00,  // issuer tariff id
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x01,  // sub-payload control - The information stored in the sub payload is TOU based
                             0x00,  // tier block 0
                             0x04, 0x03, 0x02, 0x01,  // tier block price
    };

    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x0A, // component count

                                 // component #0 - PublishCalender
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x00,  // resp to PublishCalender
                                 0x00,  // success

                                 // component #1 - PublishDayProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x01,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x01,  // resp to PublishDayProfile
                                 0x00,  // success

                                 // component #2 - PublishDayProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x02,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x01,  // resp to PublishDayProfile
                                 0x00,  // success

                                 // component #3 - PublishWeekProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x03,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x02,  // resp to PublishWeekProfile
                                 0x00,  // success

                                 // component #4 - PublishWeekProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x04,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x02,  // resp to PublishWeekProfile
                                 0x00,  // success

                                 // component #5 - PublishSeasons
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x05,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x03,  // resp to PublishSeasons
                                 0x00,  // success

                                 // component #6 - PublishSpecialDays
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x06,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // resp to PublishSeasons
                                 0x00,  // success

                                 // component #7 - PublishTariffInformation
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x07,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // resp to PublishTariffInformation
                                 0x00,  // success

                                 // component #8 - PublishBlockThresholds
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x08,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x06,  // resp to PublishTariffInformation
                                 0x00,  // success

                                 // component #9 - PublishPriceMatrix
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended header gbz command length
                                 0x00,  // frame control
                                 0x09,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x05,  // resp to PublishTariffInformation
                                 0x00,  // success
    };

    emberAfCopyInt32u(gbzCommand, 15, currentTime);
    emberAfCopyInt32u(gbzCommand, 19, currentTime);
    emberAfCopyInt32u(gbzCommand, 45, currentTime);
    emberAfCopyInt32u(gbzCommand, 49, currentTime);
    emberAfCopyInt32u(gbzCommand, 79, currentTime);
    emberAfCopyInt32u(gbzCommand, 83, currentTime);
    emberAfCopyInt32u(gbzCommand, 113, currentTime);
    emberAfCopyInt32u(gbzCommand, 117, currentTime);
    emberAfCopyInt32u(gbzCommand, 141, currentTime);
    emberAfCopyInt32u(gbzCommand, 145, currentTime);
    emberAfCopyInt32u(gbzCommand, 169, currentTime);
    emberAfCopyInt32u(gbzCommand, 173, currentTime);
    emberAfCopyInt32u(gbzCommand, 211, currentTime);
    emberAfCopyInt32u(gbzCommand, 215, currentTime);
    emberAfCopyInt32u(gbzCommand, 249, currentTime);
    emberAfCopyInt32u(gbzCommand, 294, currentTime);
    emberAfCopyInt32u(gbzCommand, 334, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == GCS07_MESSAGE_CODE ) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1 - DisplayMessage
                             0x01,  // extended header control field
                             0x07, 0x03,  // extended header cluster id
                             0x00, 0x18,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // command id - DisplayMessage
                             0x00, 0x00, 0x00, 0x00,  // message id
                             0x00,  // message control
                             0x00, 0x00, 0x00, 0x00,  // startTime - now
                             0x0A, 0x00,  // duration in minutes
                             0x09, 0x64, 0x75, 0x6D, 0x70, 0x6C, 0x69, 0x6E, 0x67, 0x73,  // message
    };

    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x01, // component count

                                 // component #1 - DisplayMessage
                                 0x01,  // extended header control field
                                 0x07, 0x03,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x00,  // resp to DisplayMessage
                                 0x00,  // success
    };

    emberAfCopyInt32u(gbzCommand, 11, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == GCS17_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x0C,  // extended gbz command length
                             0x11,  // frame control
                             0x00,  // trans. seq number
                             0x08,  // ZCL command id - GetSampledData
                             0x00, 0x00,  // Sample Request ID = Profile Data Log
                             0x00, 0x00, 0x00, 0x00,  // Earliest Sample Start Time
                             0x00,  // Sample Type = Consumption Delivered
                             0x70, 0x4A   // Number Of Samples = 19056
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS17_MESSAGE_CODE);
  } else if (messageCode == GCS44_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1 - PublishChangeOfSupplier
                             0x01,  // extended header control field
                             0x07, 0x08,  // extended header cluster id
                             0x00, 0x26,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x01,  // command id - PublishChangeOfSupplier
                             0x00, 0x00, 0x00, 0x00,  // current provider id - 0
                             0x00, 0x00, 0x00, 0x00,  // issuer event id
                             0x00,  // tariffType
                             0x00, 0x00, 0x00, 0x00,  // proposed provider id
                             0x00, 0x00, 0x00, 0x00,  // implementation time - now
                             0x00, 0x00, 0x00, 0x00,  // provider change control
                             0x09, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,  // proposed provider name
                             0x03, 0xCC, 0xBB, 0xAA,  // proposed provider contact details
    };
    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x01, // component count

                                 // component #1
                                 0x01,  // extended header control field
                                 0x07, 0x08,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x00,  // resp to PublishChangeOfSupplier
                                 0x00,  // success
    };

    emberAfCopyInt32u(gbzCommand, 15, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == CS11_MESSAGE_CODE) {
    // TOM Command
    uint8_t tomGbzCmd[] = { 0x01, 0x09, // profile id
                            0x01,  // component count

                            // component #1 - ClearEventLog
                            // Clearing the GPF Esi's event log
                            0x00,   // extended header control field
                            0x07, 0x09,   // extended header cluster id
                            0x00, 0x04,   // extended gbz command length
                            0x01,   // frame control
                            0x00,   // trans. seq number
                            0x01,   // zcl command id
                            0x03,   // logId - 3 / General Event Log
    };

    // TOM Normal Response
    uint8_t tomGbzCmdResp[] = { 0x01, 0x09, // profile id
                                0x01,  // component count

                                // component #1
                                0x01,   // extended header control field
                                0x07, 0x09,   // extended header cluster id
                                0x00, 0x04,   // extended gbz command length
                                0x09,   // frame control
                                0x00,   // trans. seq number
                                0x0B,   // ZCL command id - ClearEventLogResponse
                                0x08,   // success
    };

    // Non-TOM Command
    uint8_t nonTomGbzCmd[] = { 0x01, 0x09, // profile id
                               0x04, // component count

                               // component #1 - ClearEventLog
                               0x00, // extended header control field
                               0x07, 0x09, // extended header cluster id
                               0x00, 0x04, // extended gbz command length
                               0x01, // frame control
                               0x00, // trans. seq number
                               0x01, // zcl command id
                               0x03, // logId - 3 / General Event Log

                               // component #2 - ClearEventLog
                               0x00, // extended header control field
                               0x07, 0x09, // extended header cluster id
                               0x00, 0x04, // extended gbz command length
                               0x01, // frame control
                               0x01, // trans. seq number
                               0x01, // zcl command id
                               0x04, // logId - 4 / Security Event Log

                               // component #3 - ClearEventLog
                               // Clearing the GSME Mirror's event log
                               0x00, // extended header control field
                               0x07, 0x09, // extended header cluster id
                               0x00, 0x04, // extended gbz command length
                               0x01, // frame control
                               0x02, // trans. seq number
                               0x01, // zcl command id
                               0x06, // logId - 3 / General Event Log

                               // component #4 - ClearEventLog
                               // Clearing the GSME Mirror's security event log
                               0x01, // extended header control field
                               0x07, 0x09, // extended header cluster id
                               0x00, 0x04, // extended gbz command length
                               0x01, // frame control
                               0x03, // trans. seq number
                               0x01, // zcl command id
                               0x07, // logId - 4 / Security Event Log
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(tomGbzCmd, sizeof(tomGbzCmd), true);
    emberAfPluginGasProxyFunctionPrintln("");

    // Since CS11 can be both a TOM and a non-TOM we will test both by first
    // treating it as a TOM then treating it as a non-TOM.  In the real application
    // the code that receives the message from the WAN and calls one of the
    // following two functions will need to decide which one based on whether
    // the message was destined for the GSME (TOM) or the GPF (non-TOM).
    emberAfPluginGasProxyFunctionTapOffMessageHandler(tomGbzCmd,
                                                      sizeof(tomGbzCmd),
                                                      tomGbzCmdResp,
                                                      sizeof(tomGbzCmdResp),
                                                      messageCode);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(nonTomGbzCmd, sizeof(nonTomGbzCmd), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(nonTomGbzCmd,
                                                         sizeof(nonTomGbzCmd),
                                                         CS11_MESSAGE_CODE);
  } else if (messageCode == GCS01b_MESSAGE_CODE ) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x03, // component count

                             // component #0 - PublishTariffInformation
                             0x00,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x26,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x04,  // zcl command id - PublishTariffInformation
                             0x00, 0x00, 0x00, 0x00,  // ProviderId
                             0x00, 0x00, 0x00, 0x00,  // issuer event id
                             0x01, 0x00, 0x00, 0x00,  // tariff switching table
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x00,  // tarifftype / charging scheme
                             0x00,  // tariff label
                             0x01,  // number of price tiers in use
                             0x02,  // number of block thresholds in use
                             0x00,  // unit of measure - kWh
                             0x3A, 0x03,  // currency = GBP
                             0x50,  // price trailing digit
                             0x00, 0x00, 0x00, 0x00,  // standing charge
                             0x00,  // tier block mode
                             0x01, 0x00, 0x00,  // block threshold multiplier
                             0xE8, 0x03, 0x00,  // block threshold divisor

                             // component #1 - PublishBlockThresholds
                             0x00,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x23,  // extended gbz command length
                             0x09,  // frame control
                             0x01,  // trans. seq number
                             0x06,  // command id - PublishBlockThresholds
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - cur utc time /
                             0x00, 0x00, 0x00, 0x00,  // start time  - now
                             0x01, 0x00, 0x00, 0x00,  // issuer tariff id
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x01,  // sub-payload control - Block thresholds apply to all TOU tiers / block only charging in operation
                             0x02,  // tier / number of block thesholds
                             0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // block thresholds 1
                             0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, // block thresholds 2

                             // component #2 - PublishPriceMatrix
                             0x01,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x1B,  // extended gbz command length
                             0x09,  // frame control
                             0x02,  // trans. seq number
                             0x05,  // command id - PublishPriceMatrix
                             0x00, 0x00, 0x00, 0x00,  // provider id
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - cur utc time  / 292
                             0x00, 0x00, 0x00, 0x00,  // start time - now
                             0x01, 0x00, 0x00, 0x00,  // issuer tariff id
                             0x00,  // command index
                             0x01,  // total number of commands
                             0x01,  // sub-payload control - The information stored in the sub payload is TOU based
                             0x00,  // tier block 0
                             0x04, 0x03, 0x02, 0x01,  // tier block price
    };
    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x03, // component count

                                 // component #0
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // resp to PublishTariffInformation
                                 0x00,  // success

                                 // component #1
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x01,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // resp to PublishTariffInformation
                                 0x00,  // success

                                 // component #2
                                 0x01,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x02,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // resp to PublishTariffInformation
                                 0x00,  // success
    };

    emberAfCopyInt32u(gbzCommand, 15, currentTime);
    emberAfCopyInt32u(gbzCommand, 58, currentTime);
    emberAfCopyInt32u(gbzCommand, 98, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == GCS13a_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x0B,  // extended gbz command length
                             0x10,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x02, 0x03,  // Attribute ID - Divisor
                             0x01, 0x03,  // Attribute ID - Multiplier
                             0x00, 0x03,  // Attribute ID - UnitOfMeasure
                             0x00, 0x00,  // Attribute ID - CurrentSummationDelivered
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS13a_MESSAGE_CODE);
  } else if (messageCode == GCS13c_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x13,  // extended gbz command length
                             0x10,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x02, 0x03,  // Attribute ID - Divisor
                             0x01, 0x03,  // Attribute ID - Multiplier
                             0x00, 0x03,  // Attribute ID - UnitOfMeasure
                             0x00, 0x00,  // Attribute ID - CurrentSummationDelivered
                             0x00, 0x01,  // Attribute ID - CurrentTier1SummationDelivered
                             0x02, 0x01,  // Attribute ID - CurrentTier2SummationDelivered
                             0x04, 0x01,  // Attribute ID - CurrentTier3SummationDelivered
                             0x06, 0x01,  // Attribute ID - CurrentTier4SummationDelivered
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS13c_MESSAGE_CODE);
  } else if (messageCode == GCS13b_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count
                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x10,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x00, 0x07,  // Attribute ID - CurrentNoTierBlock1SummationDelivered
                             0x01, 0x07,  // Attribute ID - CurrentNoTierBlock2SummationDelivered
                             0x02, 0x07,  // Attribute ID - CurrentNoTierBlock3SummationDelivered
                             0x03, 0x07,  // Attribute ID - CurrentNoTierBlock4SummationDelivered
                             0x02, 0x03,  // Attribute ID - Divisor
                             0x01, 0x03,  // Attribute ID - Multiplier
                             0x00, 0x03,  // Attribute ID - UnitOfMeasure
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS13b_MESSAGE_CODE);
  } else if (messageCode == GCS14_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x07,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x17, 0x06,  // Attribute ID - Price Trailing Digit
                             0x16, 0x06,  // Attribute ID - Currency

                             // component #2
                             0x01,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x0F,  // extended gbz command length
                             0x00,  // frame control
                             0x01,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x05, 0x00,  // Attribute ID - Accumulated Debt
                             0x02, 0x00,  // Attribute ID - EmergencyCreditRemaining
                             0x01, 0x00,  // Attribute ID - Credit Remaining
                             0x31, 0x02,  // Attribute ID - Payment-based Debt
                             0x11, 0x02,  // Attribute ID - Time-based Debt (1)
                             0x21, 0x02,  // Attribute ID - Time-based Debt (2)
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS14_MESSAGE_CODE);
  } else if (messageCode == GCS21e_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x03, // component count

                             // component #1
                             0x00,  // extended header control field
                             0x00, 0x00,  // extended header cluster id
                             0x00, 0x09,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x04, 0x00,  // Attribute ID - ManufacturerName
                             0x05, 0x00,  // Attribute ID - ModelIdentifier
                             0x03, 0x00,  // Attribute ID - HWVersion

                             // component #2
                             0x00,  // extended header control field
                             0x07, 0x08,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x01,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x01, 0x01,  // Attribute ID - Provider Name

                             // component #3
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x07,  // extended gbz command length
                             0x00,  // frame control
                             0x02,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x08, 0x06,  // Attribute ID - SupplyDepletionState
                             0x07, 0x06,  // Attribute ID - SupplyTamperState
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS21e_MESSAGE_CODE);
  } else if (messageCode == GCS21j_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x00, 0x00,  // Attribute ID - Payment Control Configuration
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS21j_MESSAGE_CODE);
  } else if (messageCode == GCS46_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count
                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x07, 0x03,  // Attribute ID - Site ID
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS46_MESSAGE_CODE);
  } else if (messageCode == GCS33_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x07,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x05, 0x02,  // Attribute ID - RemainingBatteryLifeinDays
                             0x14, 0x00,  // Attribute ID - SupplyStatus
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS33_MESSAGE_CODE);
  } else if (messageCode == GCS38_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x06, 0x02,  // Attribute ID - CurrentMeterID (used for current firmware version)
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS38_MESSAGE_CODE);
  } else if (messageCode == GCS60_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x00, 0x0A,  // Attribute ID - BillToDateDelivered

                             // component #2
                             0x01,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x05,  // extended gbz command length
                             0x00,  // frame control
                             0x01,  // trans. seq number
                             0x00,  // ZCL command id - ReadAttributes
                             0x01, 0x00,  // Attribute ID - Credit Remaining
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS60_MESSAGE_CODE);
  } else if (messageCode == GCS05_MESSAGE_CODE) {
    // Command
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

    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x0B, // component count

                                 // component #0 - Resp to Emergency Credit Setup
                                 0x00,  // extended header control field
                                 0x07, 0x05,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x08,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x03,  // response id
                                 0x00,  // emberstatus success

                                 // component #1 - Resp to Set Overall Debt Cap
                                 0x00,  // extended header control field
                                 0x07, 0x05,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x08,  // frame control
                                 0x01,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x0C,  // response id
                                 0x00,  // emberstatus success

                                 // component #2 - Resp to Set Low Credit Warning Level
                                 0x00,  // extended header control field
                                 0x07, 0x05,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x08,  // frame control
                                 0x02,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x09,  // response id
                                 0x00,  // emberstatus success

                                 // component #3 - Resp to Set Maximum Credit Limit
                                 0x00,  // extended header control field
                                 0x07, 0x05,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x08,  // frame control
                                 0x03,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x0B,  // response id
                                 0x00,  // emberstatus success

                                 // component #4 - Resp to PublishCalendar
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x04,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x00,  // response id
                                 0x00,  // emberstatus success

                                 // component #5 - Resp to PublishDayProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x05,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x01,  // response id
                                 0x00,  // emberstatus success

                                 // component #6 - Resp to PublishDayProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x06,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x01,  // response id
                                 0x00,  // emberstatus success

                                 // component #7 - Resp to PublishWeekProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x07,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x02,  // response id
                                 0x00,  // emberstatus success

                                 // component #8 - Resp to PublishWeekProfile
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x08,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x02,  // response id
                                 0x00,  // emberstatus success

                                 // component #9 - Resp to PublishSeasons
                                 0x00,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x09,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x03,  // response id
                                 0x00,  // emberstatus success

                                 // component #A - Resp to PublishSpecialDays
                                 0x01,  // extended header control field
                                 0x07, 0x07,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x0A,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x04,  // response id
                                 0x00,  // emberstatus success
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

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == CS10a_MESSAGE_CODE) {
    // This use case can be used to get the event log associated with the GPF
    // or the proxy copy of the GSME event log stored in the mirror. The
    // "Event Control / Log ID" field indicates which log to return. Since there
    // are two we will create a single gbz command for this use case and execute
    // them both.
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00,  // extended header control field
                             0x07, 0x09,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - GetEventLog
                             0x13,  // Event Control / Log ID - GPF General Log
                             0x00, 0x00,  // Event id
                             0x00, 0x00, 0x00, 0x00,  // Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // End Time
                             0x00,  // Number of Events
                             0x00, 0x00,  // Event Offset

                             // component #2
                             0x01,  // extended header control field
                             0x07, 0x09,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x01,  // frame control
                             0x01,  // trans. seq number
                             0x00,  // ZCL command id - GetEventLog
                             0x16,  // Event Control / Log ID - GSME General Log
                             0x00, 0x00,  // Event id
                             0x00, 0x00, 0x00, 0x00,  // Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // End Time
                             0x00,  // Number of Events
                             0x00, 0x00,  // Event Offset
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         CS10a_MESSAGE_CODE);
  } else if (messageCode == CS10b_MESSAGE_CODE) {
    // This use case can be used to get the security event log associated with the GPF
    // or the proxy copy of the GSME security event log stored in the mirror. The
    // "Event Control / Log ID" field indicates which log to return. Since there
    // are two we will create a single gbz command for this use case and execute
    // them both.
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count

                             // component #1
                             0x00,  // extended header control field
                             0x07, 0x09,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x00,  // ZCL command id - GetEventLog
                             0x04,  // Event Control / Log ID - GPF Security Log
                             0x00, 0x00,  // Event id
                             0x00, 0x00, 0x00, 0x00,  // Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // End Time
                             0x00,  // Number of Events
                             0x00, 0x00,  // Event Offset

                             // component #2
                             0x01,  // extended header control field
                             0x07, 0x09,  // extended header cluster id
                             0x00, 0x11,  // extended gbz command length
                             0x01,  // frame control
                             0x01,  // trans. seq number
                             0x00,  // ZCL command id - GetEventLog
                             0x07,  // Event Control / Log ID - GSME Security Log
                             0x00, 0x00,  // Event id
                             0x00, 0x00, 0x00, 0x00,  // Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // End Time
                             0x00,  // Number of Events
                             0x00, 0x00,  // Event Offset
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         CS10b_MESSAGE_CODE);
  } else if (messageCode == GCS61_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x0C,  // extended gbz command length
                             0x11,  // frame control
                             0x00,  // trans. seq number
                             0x08,  // ZCL command id - GetSampledData
                             0x01, 0x00,  // Sample Request ID = Daily Consumption Log
                             0x00, 0x00, 0x00, 0x00,  // Earliest Sample Start Time
                             0x00,  // Sample Type = Consumption Delivered
                             0xDB, 0x02   // Number Of Samples = 731
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS61_MESSAGE_CODE);
  } else if (messageCode == GCS16a_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // TODO: only sending 1 request for now

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x02,  // extended header cluster id
                             0x00, 0x10,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x06,  // ZCL command id - GetSnapshot
                             0x00, 0x00, 0x00, 0x00,  // Earliest Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // Latest Start Time
                             0x00,  // Snapshot Offset
                             0x01, 0x00, 0x00, 0x00   // Snapshot Cause (General)
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS16a_MESSAGE_CODE);
  } else if (messageCode == GCS16b_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // TODO: only sending 1 request for now

                             // component #1
                             0x01,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x10,  // extended gbz command length
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x07,  // ZCL command id - GetPrepaySnapshot
                             0x00, 0x00, 0x00, 0x00,  // Earliest Start Time
                             0xFF, 0xFF, 0xFF, 0xFF,  // Latest Start Time
                             0x00,  // Snapshot Offset
                             0x01, 0x00, 0x00, 0x00   // Snapshot Cause (General)
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS16b_MESSAGE_CODE);
  } else if (messageCode == GCS25_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x02, // component count - 8-14

                             // only testing simple cmds for now.
                             // component #0 - PublishBillingPeriod
                             0x00,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x14,  // extended gbz command length
                             0x09,  // frame control
                             0x00,  // trans. seq number
                             0x09,  // ZCL command id
                             0x00, 0x00, 0x00, 0x00,  // providerId
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - now
                             0x00, 0x00, 0x00, 0x00,  // billing period start time
                             0x01, 0x00, 0x00,  // duration
                             0x01,  // billing period duration type
                             0x00,  // tariff type

                             // component #1 - PublishBlockPeriod
                             0x01,  // extended header control field
                             0x07, 0x00,  // extended header cluster id
                             0x00, 0x16,  // extended gbz command length
                             0x09,  // frame control
                             0x01,  // trans. seq number
                             0x01,  // ZCL command id
                             0x00, 0x00, 0x00, 0x00,  // provider id - 0x00
                             0x00, 0x00, 0x00, 0x00,  // issuer event id - cur utc time
                             0x00, 0x00, 0x00, 0x00,  // block period start time
                             0x01, 0x00, 0x00,  // block period duration
                             0x02,  // block period control
                             0x01,  // block period duration type
                             0x00,  // tariff type
                             0x00,  // tariff resolution period.
    };

    // Normal Response
    uint8_t gbzCommandResp[] = { 0x01, 0x09, // profile id
                                 0x02, // component count

                                 // component #0 - Resp to Emergency Credit Setup
                                 0x00,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x00,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x09,  // response id
                                 0x00,  // emberstatus success

                                 // component #1 - Resp to Set Overall Debt Cap
                                 0x01,  // extended header control field
                                 0x07, 0x00,  // extended header cluster id
                                 0x00, 0x05,  // extended gbz command length
                                 0x00,  // frame control
                                 0x01,  // trans. seq number
                                 0x0B,  // ZCL command id - default response
                                 0x01,  // response id
                                 0x00,  // emberstatus success
    };

    emberAfCopyInt32u(gbzCommand, 15, currentTime);
    emberAfCopyInt32u(gbzCommand, 40, currentTime);

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionTapOffMessageHandler(gbzCommand,
                                                      sizeof(gbzCommand),
                                                      gbzCommandResp,
                                                      sizeof(gbzCommandResp),
                                                      messageCode);
  } else if (messageCode == GCS15b_MESSAGE_CODE) {
    // Command
    // Command Length
    //   = 2:profileId + 1:componentCount + (12*21):GetPrepaySnapshot + (12*21):GetSnapshot
    //   = 507
    uint8_t i;
    uint8_t gbzCommand[507];
    uint16_t gbzCommandIndex = 0;

    gbzCommand[gbzCommandIndex++] = 0x01;                          // profile id
    gbzCommand[gbzCommandIndex++] = 0x09;
    emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x18);          // component count
    gbzCommandIndex += 1;
    for (i = 0; i < 12; i++) {
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x00);        // extended header control field
      gbzCommandIndex += 1;

      gbzCommand[gbzCommandIndex++] = 0x07;                          // extended header cluster id
      gbzCommand[gbzCommandIndex++] = 0x05;
      gbzCommand[gbzCommandIndex++] = 0x00;                          // extended gbz command length
      gbzCommand[gbzCommandIndex++] = 0x10;

      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);        // frame control
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // trans. seq number
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x07);        // ZCL command id - GetPrepaySnapshot
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000000); // Earliest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0xFFFFFFFF); // Latest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // Snapshot Offset
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00006008); // Snapshot Cause (General)
      gbzCommandIndex += 4;
    }
    for (i = 0; i < 12; i++) {
      if (i == 11) {
        emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);      // extended header control field
      } else {
        emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x00);      // extended header control field
      }
      gbzCommandIndex += 1;

      gbzCommand[gbzCommandIndex++] = 0x07;                          // extended header cluster id
      gbzCommand[gbzCommandIndex++] = 0x02;
      gbzCommand[gbzCommandIndex++] = 0x00;                          // extended gbz command length
      gbzCommand[gbzCommandIndex++] = 0x10;

      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);        // frame control
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i + 12);        // trans. seq number
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x06);        // ZCL command id - GetPrepaySnapshot
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000000); // Earliest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0xFFFFFFFF); // Latest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // Snapshot Offset
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00006008); // Snapshot Cause (General)
      gbzCommandIndex += 4;
    }

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS15b_MESSAGE_CODE);
  } else if (messageCode == GCS15c_MESSAGE_CODE) {
    // Command
    // Command Length
    //   = 2:profileId + 1:componentCount + (12*21) GetPrepaySnapshot+(12*21):GetSnapshot
    //   = 507
    uint8_t i;
    uint8_t gbzCommand[507];
    uint16_t gbzCommandIndex = 0;

    gbzCommand[gbzCommandIndex++] = 0x01;                         // profile id
    gbzCommand[gbzCommandIndex++] = 0x09;

    //updating usecase based on IRP328 (change 3)
    emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x18);          // component count
    gbzCommandIndex += 1;
    for (i = 0; i < 12; i++) {
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x00);        // extended header control field
      gbzCommandIndex += 1;

      gbzCommand[gbzCommandIndex++] = 0x07;                          // extended header cluster id
      gbzCommand[gbzCommandIndex++] = 0x05;
      gbzCommand[gbzCommandIndex++] = 0x00;                          // extended gbz command length
      gbzCommand[gbzCommandIndex++] = 0x10;

      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);        // frame control
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // trans. seq number
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x07);        // ZCL command id - GetPrepaySnapshot
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000000); // Earliest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0xFFFFFFFF); // Latest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // Snapshot Offset
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000002); // Snapshot Cause (General)
      gbzCommandIndex += 4;
    }
    for (i = 0; i < 12; i++) {
      if (i == 11) {
        emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);      // extended header control field
      } else {
        emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x00);      // extended header control field
      }
      gbzCommandIndex += 1;

      gbzCommand[gbzCommandIndex++] = 0x07;                          // extended header cluster id
      gbzCommand[gbzCommandIndex++] = 0x02;
      gbzCommand[gbzCommandIndex++] = 0x00;                          // extended gbz command length
      gbzCommand[gbzCommandIndex++] = 0x10;

      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x01);        // frame control
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i + 12);        // trans. seq number
      gbzCommandIndex += 1;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, 0x06);        // ZCL command id - GetSnapshot
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000000); // Earliest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0xFFFFFFFF); // Latest Start Time
      gbzCommandIndex += 4;
      emberAfCopyInt8u(gbzCommand, gbzCommandIndex, i);           // Snapshot Offset
      gbzCommandIndex += 1;
      emberAfCopyInt32u(gbzCommand, gbzCommandIndex, 0x00000002); // Snapshot Cause (General)
      gbzCommandIndex += 4;
    }

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS15c_MESSAGE_CODE);
  } else if (messageCode == GCS15d_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x11,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x09,  // extended gbz command length
                             0x00, 0x00, 0x00, 0x00,  // extended header from date (TODO)
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x0A,  // ZCL command id - GetDebtRepaymentLog
                             0xFE, 0xFF, 0xFF, 0xFF,  // Latest End Time
                             0x0A,  // Number of Debts
                             0x02,  // Debt Type
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS15d_MESSAGE_CODE);
  } else if (messageCode == GCS15e_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x11,  // extended header control field
                             0x07, 0x05,  // extended header cluster id
                             0x00, 0x08,  // extended gbz command length
                             0x00, 0x00, 0x00, 0x00,  // extended header from date (TODO)
                             0x01,  // frame control
                             0x00,  // trans. seq number
                             0x08,  // ZCL command id - GetTopUpLog
                             0xFE, 0xFF, 0xFF, 0xFF,  // Latest End Time
                             0x0A,  // Number of Records
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS15e_MESSAGE_CODE);
  } else if (messageCode == GCS21f_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x06, // component count

                             // component #1
                             0x00, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x27, // extended gbz command length
                             0x00, // frame control
                             0x00, // trans. seq number
                             0x00, // ZCL command id - ReadAttributes
                             0x03, 0x02, // Attribute ID - Block Period: ThresholdDivisor
                             0x02, 0x02, // Attribute ID - Block Period: ThresholdMultiplier
                             0x00, 0x01, // Attribute ID - Block Threshold: Block1Threshold
                             0x01, 0x01, // Attribute ID - Block Threshold: Block2Threshold
                             0x02, 0x01, // Attribute ID - Block Threshold: Block3Threshold
                             0x15, 0x06, // Attribute ID - Tariff Information: UnitofMeasure
                             0x12, 0x06, // Attribute ID - Tariff Information: NumberofBlockThresholdsInUse
                             0x00, 0x04, // Attribute ID - NoTierBlock1Price
                             0x01, 0x04, // Attribute ID - NoTierBlock2Price
                             0x02, 0x04, // Attribute ID - NoTierBlock3Price
                             0x03, 0x04, // Attribute ID - NoTierBlock4Price
                             0x10, 0x04, // Attribute ID - Tier1Block1Price
                             0x20, 0x04, // Attribute ID - Tier2Block1Price
                             0x30, 0x04, // Attribute ID - Tier3Block1Price
                             0x40, 0x04, // Attribute ID - Tier4Block1Price
                             0x16, 0x06, // Attribute ID - Currency
                             0x17, 0x06, // Attribute ID - Tariff Information: Price Trailing Digit
                             0x01, 0x03, // Attribute ID - Standing Charge

                             // component #2
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0D, // extended gbz command length
                             0x01, // frame control
                             0x01, // trans. seq number
                             0x01, // ZCL command id - GetDayProfiles
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFF, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID
                             0x01, // Start Day ID
                             0x00, // Number of Days

                             // component #3
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0D, // extended gbz command length
                             0x01, // frame control
                             0x02, // trans. seq number
                             0x02, // ZCL command id - GetWeekProfiles
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFF, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID
                             0x01, // Start Week ID
                             0x00, // Number of Weeks

                             // component #4
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0B, // extended gbz command length
                             0x01, // frame control
                             0x03, // trans. seq number
                             0x03, // ZCL command id - GetSeasons
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFF, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID

                             // component #5
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x11, // extended gbz command length
                             0x01, // frame control
                             0x04, // trans. seq number
                             0x04, // ZCL command id - GetSpecialDays
                             0x00, 0x00, 0x00, 0x00, // Start Time
                             0x00, // Number of Events
                             0x00, // Calendar Type = Delivered
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFF, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID

                             // component #6
                             0x01, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x04, // extended gbz command length
                             0x01, // frame control
                             0x05, // trans. seq number
                             0x00, // ZCL command id - GetCurrentPrice
                             0x00, // Active Price
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS21f_MESSAGE_CODE);
  } else if (messageCode == GCS21b_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x06, // component count

                             // component #1
                             0x00, // extended header control field
                             0x07, 0x05, // extended header cluster id
                             0x00, 0x1B, // extended gbz command length
                             0x00, // frame control
                             0x00, // trans. seq number
                             0x00, // ZCL command id - ReadAttributes
                             0x39, 0x02, // Attribute ID - Debt Attribute: DebtRecoveryTopUpPercentage#3
                             0x06, 0x00, // Attribute ID - Prepayment Information: OverallDebtCap
                             0x16, 0x02, // Attribute ID - Debt: DebtRecoveryFrequency#1
                             0x26, 0x02, // Attribute ID - Debt: DebtRecoveryFrequency#2
                             0x40, 0x00, // Attribute ID - Prepayment Information: CutOffValue
                             0x10, 0x00, // Attribute ID - Prepayment Information: EmergencyCreditLimit
                             0x11, 0x00, // Attribute ID - Prepayment Information: EmergencyCreditThreshold
                             0x31, 0x00, // Attribute ID - Prepayment Information: LowCreditWarningLevel
                             0x21, 0x00, // Attribute ID - Prepayment Information: MaxCreditLimit
                             0x22, 0x00, // Attribute ID - Prepayment Information: MaxCreditPerTopUp
                             0x17, 0x02, // Attribute ID - Debt Attribute: DebtRecoveryAmount#1
                             0x27, 0x02,  //Attribute ID - Debt Attribute: DebtRecoveryAmount#2

                             // component #2
                             0x00, // extended header control field
                             0x07, 0x00, // extended header cluster id
                             0x00, 0x05, // extended gbz command length
                             0x00, // frame control
                             0x01, // trans. seq number
                             0x00, // ZCL command id - ReadAttributes
                             0x17, 0x06, //Attribute ID - Price Trailing Digits

                             // component #2
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0D, // extended gbz command length
                             0x01, // frame control
                             0x02, // trans. seq number
                             0x01, // ZCL command id - GetDayProfiles
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFE, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID
                             0x01, // Start Day ID
                             0x00, // Number of Days

                             // component #3
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0D, // extended gbz command length
                             0x01, // frame control
                             0x03, // trans. seq number
                             0x02, // ZCL command id - GetWeekProfiles
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFE, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID
                             0x01, // Start Week ID
                             0x00, // Number of Weeks

                             // component #4
                             0x00, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x0B, // extended gbz command length
                             0x01, // frame control
                             0x04, // trans. seq number
                             0x03, // ZCL command id - GetSeasons
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFE, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID

                             // component #5
                             0x01, // extended header control field
                             0x07, 0x07, // extended header cluster id
                             0x00, 0x11, // extended gbz command length
                             0x01, // frame control
                             0x05, // trans. seq number
                             0x04, // ZCL command id - GetSpecialDays
                             0x00, 0x00, 0x00, 0x00, // Start Time
                             0x00, // Number of Events
                             0x03, // Calendar Type = Friendly Credit Calendar
                             0x00, 0x00, 0x00, 0x00, // Provider ID - unused
                             0xFE, 0xFF, 0xFF, 0xFF, // Issuer Calendar ID
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS21b_MESSAGE_CODE);
  } else if (messageCode == GCS09_MESSAGE_CODE) {
    // Command
    uint8_t gbzCommand[] = { 0x01, 0x09, // profile id
                             0x01, // component count

                             // component #1
                             0x01, // extended header control field
                             0x07, 0x08, // extended header cluster id
                             0x00, 0x14, // extended gbz command length
                             0x09, // frame control
                             0x00, // trans. seq number
                             0x00, // ZCL command id - PublishChangeOfTenancy
                             0x00, 0x00, 0x00, 0x00, // provider id
                             0x00, 0x00, 0x00, 0x00, // current utc time
                             0x00, // tariff type
                             0x00, 0x00, 0x00, 0x00, // now
                             0x44, 0x33, 0x22, 0x11, // proposed tenancy change control
    };

    emberAfPluginGasProxyFunctionPrint("GPF: GBZ Command: ");
    emberAfPluginGasProxyFunctionPrintBuffer(gbzCommand, sizeof(gbzCommand), true);
    emberAfPluginGasProxyFunctionPrintln("");

    emberAfPluginGasProxyFunctionNonTapOffMessageHandler(gbzCommand,
                                                         sizeof(gbzCommand),
                                                         GCS09_MESSAGE_CODE);
  }
}

void emAfPluginGasProxyFunctionCliPrintSupportedUseCases(void)
{
  uint16_t index;

  emberAfPluginGasProxyFunctionPrintln("GPF: Supported Use Cases");
  for (index = 0; index < GBCS_NUM_USE_CASES; index++) {
    emberAfPluginGasProxyFunctionPrintln("GPF: Message Code: 0x%2x, GBCS Use Case: %p",
                                         useCaseDescriptions[index].messageCode,
                                         useCaseDescriptions[index].description);
  }
}
