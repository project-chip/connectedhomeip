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
 * @brief Routines for the Throughput plugin, which provides a mechanism for
 *        measuring network throughput.
 *******************************************************************************
   ******************************************************************************/

#include <stdint.h>
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "math.h"
#ifdef UNIX_HOST
#include <time.h>
#endif // UNIX_HOST
#define MAX_PACKET_LENGTH 127
#define MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH   80
#define ZIGBEE_TX_TEST_MAX_INFLIGHT         5

typedef struct {
  bool inUse;
  uint32_t startTime;
  uint8_t seqn;
} InflightInfo;

static struct {
  EmberApsOption apsOptions;
  EmberNodeId destination;
  uint8_t payloadLength;
  uint8_t headerLength;
  uint8_t packetLength;
  uint16_t messageTotalCount;
  uint16_t messageRunningCount;
  uint16_t messageSuccessCount;
  uint8_t maxInFlight;
  uint8_t currentInFlight;
  uint16_t txIntervalMs;
  uint32_t startTime;
  uint32_t runTime;
  uint32_t testTimeout;
  uint32_t minSendTimeMs;
  uint32_t maxSendTimeMs;
  uint32_t sumSendTimeMs;
  uint32_t varSendTimeMs;
  InflightInfo inflightInfoTable[ZIGBEE_TX_TEST_MAX_INFLIGHT];
} testParams;

// CLI-runnable functions
void emAfPluginThroughputCliSetAllParameters(void);
void emAfPluginThroughputCliStartTest(void);
void emAfPluginThroughputCliStopTest(void);
void emAfPluginThroughputCliSetDestination(void);
void emAfPluginThroughputCliSetInterval(void);
void emAfPluginThroughputCliSetTestTimeout(void);
void emAfPluginThroughputCliSetTestCount(void);
void emAfPluginThroughputCliSetInFlightCount(void);
void emAfPluginThroughputCliSetPacketSize(void);
void emAfPluginThroughputCliSetApsAckOff(void);
void emAfPluginThroughputCliSetApsAckOn(void);
void emAfPluginThroughputCliPrintParameters(void);
void emAfPluginThroughputCliPrintResult(void);
void emAfPluginThroughputCliPrintCounters(void);
void emAfPluginThroughputCliClearCounters(void);

//stack events and handlers
EmberEventControl emberAfPluginThroughputPacketSendEventControl;
void emberAfPluginThroughputPacketSendEventHandler(void);

//Other declarations
static uint8_t isRunning = 0;
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];
extern const char * titleStrings[];
static void getHeaderLen(void);
static void printParameter(uint8_t printSelection);
static void printCounter(uint8_t);
enum printSelection {
  DESTINATION = 0,
  COUNT,
  INTERVAL,
  HEADER,
  PAYLOAD,
  PACKET,
  INFLIGHT,
  APSOPTIONS,
  TESTTIMEOUT
};

static void printParameter(uint8_t printSelection)
{
  switch (printSelection) {
    case DESTINATION:
      emberAfCorePrintln("Destination nodeID: 0x%2x ", testParams.destination);
      break;
    case COUNT:
      emberAfCorePrintln("Packets to send: %d ", testParams.messageTotalCount);
      break;
    case INTERVAL:
      emberAfCorePrintln("Transmit interval: %d ms", testParams.txIntervalMs);
      break;
    case HEADER:
      emberAfCorePrintln("Header size: %dB", testParams.headerLength);
      break;
    case PAYLOAD:
      emberAfCorePrintln("Payload size: %dB", testParams.payloadLength);
      break;
    case PACKET:
      emberAfCorePrintln("Packet size: %dB", testParams.packetLength);
      break;
    case INFLIGHT:
      emberAfCorePrintln("Packets in flight: %d", testParams.maxInFlight);
      break;
    case APSOPTIONS:
      emberAfCorePrintln("APS Options = 0x%2x", testParams.apsOptions);
      break;
    case TESTTIMEOUT:
      emberAfCorePrintln("Timeout: %d ms", testParams.testTimeout);
      break;
    default:
      emberAfCorePrintln("Err");
  }
}

void emAfPluginThroughputCliPrintParameters(void)
{
  emberAfCorePrintln(" ");
  emberAfCorePrintln("TEST PARAMETERS");
  printParameter(DESTINATION);
  printParameter(COUNT);
  printParameter(INTERVAL);
  printParameter(PAYLOAD);
  printParameter(PACKET);
  printParameter(INFLIGHT);
  printParameter(APSOPTIONS);
  printParameter(TESTTIMEOUT);
}

void emAfPluginThroughputCliSetAllParameters(void)
{
  static uint8_t minPacketLen;

  testParams.destination = (uint16_t)emberUnsignedCommandArgument(0);
  testParams.messageTotalCount = (uint16_t)emberUnsignedCommandArgument(1);
  testParams.txIntervalMs = (uint16_t)emberUnsignedCommandArgument(2);
  testParams.packetLength = (uint8_t)emberUnsignedCommandArgument(3);
  testParams.maxInFlight = (uint8_t)emberUnsignedCommandArgument(4);
  testParams.apsOptions = (EmberApsOption)emberUnsignedCommandArgument(5);
  testParams.testTimeout = (uint32_t)emberUnsignedCommandArgument(6);

  // Check packet length parameter
  getHeaderLen();
  minPacketLen = testParams.headerLength + 4;
  if (testParams.packetLength < minPacketLen) {
    testParams.packetLength = minPacketLen;
  } else if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.packetLength = MAX_PACKET_LENGTH;
  }
  testParams.payloadLength = testParams.packetLength - testParams.headerLength;

  // Check in-flight parameter
  if (testParams.maxInFlight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    testParams.maxInFlight = ZIGBEE_TX_TEST_MAX_INFLIGHT;
  } else if (testParams.maxInFlight <= 0) {
    testParams.maxInFlight = 1;
  }
  emAfPluginThroughputCliPrintParameters();
}

void emAfPluginThroughputCliSetDestination(void)
{
  testParams.destination = (uint16_t)emberUnsignedCommandArgument(0);
  getHeaderLen();
  testParams.packetLength = testParams.payloadLength + testParams.headerLength;
  if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.payloadLength = MAX_PACKET_LENGTH - testParams.headerLength;
  }
  printParameter(DESTINATION);
  printParameter(PACKET);
}

void emAfPluginThroughputCliSetTestCount(void)
{
  testParams.messageTotalCount = (uint16_t)emberUnsignedCommandArgument(0);
  printParameter(COUNT);
}

void emAfPluginThroughputCliSetInterval(void)
{
  testParams.txIntervalMs = (uint16_t)emberUnsignedCommandArgument(0);
  printParameter(INTERVAL);
}

void emAfPluginThroughputCliSetPacketSize(void)
{
  static uint8_t minPacketLen;

  testParams.packetLength = (uint8_t)emberUnsignedCommandArgument(0);
  getHeaderLen();
  minPacketLen = testParams.headerLength + 4;

  if (testParams.packetLength < minPacketLen) {
    testParams.packetLength = minPacketLen;
  } else if (testParams.packetLength > MAX_PACKET_LENGTH) {
    testParams.packetLength = MAX_PACKET_LENGTH;
  }
  testParams.payloadLength = testParams.packetLength - testParams.headerLength;

  emberAfCorePrintln("Max packet: 127");
  printParameter(HEADER);
  printParameter(PAYLOAD);
  printParameter(PACKET);
}

static void getHeaderLen(void)
{
  EmberApsFrame* testFrame;
  testFrame = emberAfGetCommandApsFrame();
  uint8_t maxPayloadLen = emberAfMaximumApsPayloadLength(
    EMBER_OUTGOING_DIRECT,
    testParams.destination,
    testFrame) - EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD;
  testParams.headerLength = MAX_PACKET_LENGTH - maxPayloadLen;
}

void emAfPluginThroughputCliSetInFlightCount(void)
{
  testParams.maxInFlight = (uint8_t)emberUnsignedCommandArgument(0);
  if (testParams.maxInFlight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    testParams.maxInFlight = ZIGBEE_TX_TEST_MAX_INFLIGHT;
  } else if (testParams.maxInFlight <= 0) {
    testParams.maxInFlight = 1;
  }
  printParameter(INFLIGHT);
}

void emAfPluginThroughputCliSetApsAckOn(void)
{
  testParams.apsOptions |= EMBER_APS_OPTION_RETRY;
  printParameter(APSOPTIONS);
}

void emAfPluginThroughputCliSetApsAckOff(void)
{
  testParams.apsOptions &= ~EMBER_APS_OPTION_RETRY;
  printParameter(APSOPTIONS);
}

void emAfPluginThroughputCliSetTestTimeout(void)
{
  testParams.testTimeout = (uint32_t)emberUnsignedCommandArgument(0);
  printParameter(TESTTIMEOUT);
}

void emAfPluginThroughputCliClearCounters(void)
{
  emberAfCorePrintln("Clearing counters");
  emberAfPluginCountersClear();
}

void emAfPluginThroughputCliStopTest(void)
{
  emberEventControlSetInactive(emberAfPluginThroughputPacketSendEventControl);
  testParams.messageTotalCount = 0;
  isRunning = 0;
  emberAfCorePrintln("Test Aborted");
}

void emAfPluginThroughputCliStartTest(void)
{
  uint8_t i;

  if (isRunning) {
    emberAfCorePrintln("Test already in progress");
    return;
  }

  if (testParams.packetLength <= 50) {
    emberAfCorePrintln("Err: Invalid Packet Size");
    emAfPluginThroughputCliPrintParameters();
    return;
  }

  if (testParams.messageTotalCount == 0) {
    emberAfCorePrintln("Err: Invalid Count");
    emAfPluginThroughputCliPrintParameters();
    return;
  }

  isRunning = 1;
  testParams.currentInFlight = 0;
  testParams.messageRunningCount = 0;
  testParams.messageSuccessCount = 0;
  testParams.startTime = halCommonGetInt32uMillisecondTick();
  testParams.runTime = 0;
  testParams.minSendTimeMs = 0xFFFFFFFF;
  testParams.maxSendTimeMs = 0;
  testParams.sumSendTimeMs = 0;
  testParams.varSendTimeMs = 0;
  for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
    testParams.inflightInfoTable[i].inUse = false;
  }
  emAfPluginThroughputCliClearCounters();
  emberAfCorePrintln("Starting Test");
  emberAfPluginThroughputPacketSendEventHandler();
}

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
bool emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                uint16_t indexOrDestination,
                                EmberApsFrame* apsFrame,
                                uint16_t msgLen,
                                uint8_t* message,
                                EmberStatus status)
{
  // Is this is a message sent out as part of the throughput test?
  if (apsFrame->profileId == 0x7F01 && apsFrame->clusterId == 0x0001) {
    uint32_t packetSendTimeMs = 0xFFFFFFFF;
    uint8_t i;

    testParams.currentInFlight--;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (testParams.inflightInfoTable[i].seqn == apsFrame->sequence) {
        testParams.inflightInfoTable[i].inUse = false;
        packetSendTimeMs = elapsedTimeInt32u(testParams.inflightInfoTable[i].startTime, halCommonGetInt32uMillisecondTick());
        break;
      }
    }

    assert(packetSendTimeMs != 0xFFFFFFFF);

    if (status == EMBER_SUCCESS) {
      testParams.messageSuccessCount++;
      testParams.sumSendTimeMs += packetSendTimeMs;

      if (testParams.minSendTimeMs > packetSendTimeMs) {
        testParams.minSendTimeMs = packetSendTimeMs;
      }
      if (testParams.maxSendTimeMs < packetSendTimeMs) {
        testParams.maxSendTimeMs = packetSendTimeMs;
      }

      //10X scaling factor makes large number less likely to overflow
      testParams.varSendTimeMs += packetSendTimeMs * packetSendTimeMs / 10;
    }

    if (testParams.currentInFlight == 0
        && testParams.messageRunningCount
        == testParams.messageTotalCount) {
      emberAfCorePrintln("Test Complete");
      testParams.runTime = elapsedTimeInt32u(testParams.startTime, halCommonGetInt32uMillisecondTick());
      emAfPluginThroughputCliPrintResult();
    }
    return true;
  }
  return false;
}

void emberAfPluginThroughputPacketSendEventHandler(void)
{
  EmberApsFrame apsFrame;
  uint8_t messagePayload[MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH];
  uint8_t messageTag[1] = { 0 };
  uint8_t i;
  uint32_t txIntervalAdjustmentMs;
  uint32_t adjustedTxIntervalMs;

  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t totalRunTimeMs = elapsedTimeInt32u(testParams.startTime, currentTimeMs);
  if ((totalRunTimeMs >= testParams.testTimeout)
      && (testParams.testTimeout > 0)) {
    emberAfCorePrintln("Timeout Exceeded");
    emAfPluginThroughputCliStopTest();
    return;
  }
  messagePayload[0] = testParams.payloadLength;
  messagePayload[1] = LOW_BYTE(testParams.messageRunningCount);
  messagePayload[2] = HIGH_BYTE(testParams.messageRunningCount);
  for (i = 3; i < testParams.payloadLength; i++) {
    messagePayload[i] = i - 3;
  }
  if (testParams.maxInFlight > 0
      && testParams.currentInFlight >= testParams.maxInFlight) {
    emberEventControlSetDelayMS(emberAfPluginThroughputPacketSendEventControl, 1);
    return;
  }
  apsFrame.sourceEndpoint = 0xFF;
  apsFrame.destinationEndpoint = 0xFF;
  apsFrame.options = testParams.apsOptions;
  apsFrame.profileId = 0x7F01; // test profile ID
  apsFrame.clusterId = 0x0001; // counted packets cluster

  if (emAfSend(EMBER_OUTGOING_DIRECT,
               testParams.destination,
               &apsFrame,
               testParams.payloadLength,
               messagePayload,
               messageTag,
               0xFFFF,
               0) == EMBER_SUCCESS) {
    testParams.messageRunningCount++;
    testParams.currentInFlight++;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (!testParams.inflightInfoTable[i].inUse) {
        testParams.inflightInfoTable[i].inUse = true;
        testParams.inflightInfoTable[i].seqn = apsFrame.sequence;
        testParams.inflightInfoTable[i].startTime =
          halCommonGetInt32uMillisecondTick();
        break;
      }
    }
    assert(i < ZIGBEE_TX_TEST_MAX_INFLIGHT);
  }

  if (testParams.messageRunningCount
      >= testParams.messageTotalCount) {
    emberEventControlSetInactive(emberAfPluginThroughputPacketSendEventControl);
    isRunning = 0;
  } else {
    // txIntervalAdjustment subtracts out time spent in this function from the send loop timer,
    // which makes a significant difference for host applications using ncp-uart
    txIntervalAdjustmentMs = elapsedTimeInt32u(currentTimeMs, halCommonGetInt32uMillisecondTick());
    if (txIntervalAdjustmentMs >= testParams.txIntervalMs) {
      adjustedTxIntervalMs = 0;
    } else {
      adjustedTxIntervalMs = testParams.txIntervalMs - txIntervalAdjustmentMs;
    }
    emberEventControlSetDelayMS(emberAfPluginThroughputPacketSendEventControl,
                                adjustedTxIntervalMs);
  }
}

void emAfPluginThroughputCliPrintResult(void)
{
  uint32_t mean, var, std;

  if (isRunning) {
    emberAfCorePrintln("Test still in progress");
    testParams.runTime = elapsedTimeInt32u(testParams.startTime, halCommonGetInt32uMillisecondTick());
  }
  uint64_t appThroughput = (testParams.messageSuccessCount
                            * testParams.payloadLength);
  appThroughput = appThroughput * 8 * 1000 / testParams.runTime;

  uint64_t phyThroughput = (testParams.messageSuccessCount
                            * testParams.packetLength);
  phyThroughput = phyThroughput * 8 * 1000 / testParams.runTime;

  emberAfCorePrintln(" ");
  emberAfCorePrintln("THROUGHPUT RESULTS");
  emberAfCorePrintln("Total time %u ms", testParams.runTime);
  emberAfCorePrintln("Success messages: %d out of %d",
                     testParams.messageSuccessCount,
                     testParams.messageTotalCount);
  emberAfCorePrintln("Payload Throughput: %u bits/s", appThroughput);
  emberAfCorePrintln("Phy Throughput: %u bits/s", phyThroughput);
  if (testParams.messageSuccessCount > 0) {
    mean = testParams.sumSendTimeMs / testParams.messageSuccessCount;
    //10X scaling factor makes large number less likely to overflow
    var = testParams.varSendTimeMs * 10 / testParams.messageSuccessCount - mean * mean;
    if (var > 0) {
      std = sqrt(fabs(var));
    } else {
      std = 0;
    }

    emberAfCorePrintln("Min packet send time: %u ms",
                       testParams.minSendTimeMs);
    emberAfCorePrintln("Max packet send time: %u ms",
                       testParams.maxSendTimeMs);
    emberAfCorePrintln("Avg packet send time: %u ms", mean);
    emberAfCorePrintln("STD packet send time: %u ms", std);
  }
}

static void printCounter(uint8_t id)
{
  emberAfCorePrintln("%p: %u ", titleStrings[id], emberCounters[id]);
}

void emAfPluginThroughputCliPrintCounters(void)
{
  #ifdef EZSP_HOST
  ezspReadCounters(emberCounters);
  #endif // EZSP_HOST
  emberAfCorePrintln(" ");
  emberAfCorePrintln("COUNTERS");
  printCounter(EMBER_COUNTER_PHY_CCA_FAIL_COUNT);
  printCounter(EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS);
  printCounter(EMBER_COUNTER_MAC_TX_UNICAST_RETRY);
  printCounter(EMBER_COUNTER_MAC_TX_UNICAST_FAILED);
  printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS);
  printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY);
  printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED);
}
