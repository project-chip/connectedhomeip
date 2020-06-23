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
 * @brief Test for scan dispatch plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/test/script/afv2-scripted.h"

#include "scan-dispatch.h"

void emberAfPluginScanDispatchTickCallback(void);

// -----------------------------------------------------------------------------
// CONSTANTS.

// See scan-dispatch.h mask definitions.
// I am lazy and don't want to have to type that long mask #define
// for the failure bit...
#define SUCCESS (0)
#define FAILURE (2)

#define DURATION (10)
#define RSSI (-30)
#define LQI (80)

// Channels: single
//           mask
#define CHANNEL_MASK_COUNT (2)
#define CHANNEL_BIT  (BIT32(11))
#define CHANNEL_MASK (BIT32(11) | BIT32(15) | BIT32(20) | BIT32(25))
static uint8_t channelNumsCounts[] = { 1, 4 };
static uint32_t channelMasks[] = { CHANNEL_BIT, CHANNEL_MASK };
static uint8_t channelNums[][4] = {
  { 11, },
  { 11, 15, 20, 25, },
};
static uint8_t channelNumsCount;
static uint8_t *channelNumsCurrent, channelNumCurrent;
static uint32_t channelMaskCurrent;
static uint8_t channelsIndex;

// Scan type: energy
//            active
#define SCAN_TYPE_COUNT (2)
static EmberNetworkScanType scanTypes[] = { EMBER_ENERGY_SCAN, EMBER_ACTIVE_SCAN };
static EmberNetworkScanType scanType;
static uint8_t scanTypeIndex;

// Consumers: single
//            double
#define CONSUMER_COUNT (2)
static uint8_t consumer;

// -----------------------------------------------------------------------------
// GLOBALS.

static bool expectingScanCallback = false;
static EmberZigbeeNetwork NETWORK;

// -----------------------------------------------------------------------------
// HANDLERS.

void emberAfEnergyScanResultCallback(uint8_t channel, int8_t rssi);
void emberScanCompleteHandler(uint8_t channel, EmberStatus status);

static void scanHandler0(EmberAfPluginScanDispatchScanResults *results)
{
  functionCallCheck("scanHandler",
                    "iiiiii",
                    results->status,
                    results->channel,
                    emberAfPluginScanDispatchScanResultsGetScanType(results),
                    emberAfPluginScanDispatchScanResultsAreComplete(results),
                    emberAfPluginScanDispatchScanResultsAreFailure(results),
                    0); // consumer 0
}

static void scanHandler1(EmberAfPluginScanDispatchScanResults *results)
{
  functionCallCheck("scanHandler",
                    "iiiiii",
                    results->status,
                    results->channel,
                    emberAfPluginScanDispatchScanResultsGetScanType(results),
                    emberAfPluginScanDispatchScanResultsAreComplete(results),
                    emberAfPluginScanDispatchScanResultsAreFailure(results),
                    1); // consumer 1
}

static void(*scanHandlers[])(EmberAfPluginScanDispatchScanResults * results) = {
  scanHandler0,
  scanHandler1,
};

#define addScanHandlerCheck(statusOrRssi, \
                            channelOrLqi, \
                            scanType,     \
                            complete,     \
                            failure,      \
                            consumer)     \
  addSimpleCheck("scanHandler",           \
                 "iiiiii",                \
                 (statusOrRssi),          \
                 (channelOrLqi),          \
                 (scanType),              \
                 (complete),              \
                 (failure),               \
                 (consumer))

// -----------------------------------------------------------------------------
// SCAN COMPLETE CALLBACK ACTION.

void emberAfScanCompleteCallback(uint8_t channel, EmberStatus status);

PRINTER(scanCompleteCallback)
{
  fprintf(stderr,
          " scan complete callback with status 0x%02X",
          (EmberStatus)action->contents[1]);
}
PERFORMER(scanCompleteCallback)
{
  uint8_t channel = (uint8_t)action->contents[0];
  EmberStatus status = (int8_t)action->contents[1];
  emberAfScanCompleteCallback(channel, status);
  expectingScanCallback = false;
}

ACTION(scanCompleteCallback, ii);

#define addScanCompleteCallbackAction(channel, status) \
  addAction(&scanCompleteCallbackActionType,           \
            (channel),                                 \
            (status))

// -----------------------------------------------------------------------------
// ACTIVE SCAN CALLBACK ACTION.

void emberAfNetworkFoundCallback(EmberZigbeeNetwork *networkFound,
                                 uint8_t lqi,
                                 int8_t rssi);

PRINTER(activeScanCallback)
{
}
PERFORMER(activeScanCallback)
{
  uint8_t lqi = (uint8_t)action->contents[0];
  int8_t rssi = (int8_t)action->contents[1];
  emberAfNetworkFoundCallback(&NETWORK, lqi, rssi);
}

ACTION(activeScanCallback, ii);

#define addActiveScanCallbackAction(lqi, rssi) \
  addAction(&activeScanCallbackActionType,     \
            (lqi),                             \
            (rssi))

// -----------------------------------------------------------------------------
// ENERGY SCAN CALLBACK ACTION.

void emberAfEnergyScanResultCallback(uint8_t channel, int8_t rssi);

PRINTER(energyScanCallback)
{
  fprintf(stderr,
          " energy scan callback for channel %d",
          (uint8_t)action->contents[0]);
}
PERFORMER(energyScanCallback)
{
  uint8_t channel = (uint8_t)action->contents[0];
  int8_t rssi = (int8_t)action->contents[1];
  emberAfEnergyScanResultCallback(channel, rssi);
}

ACTION(energyScanCallback, ii);

#define addEnergyScanCallbackAction(channel, rssi) \
  addAction(&energyScanCallbackActionType,         \
            (channel),                             \
            (rssi))

// -----------------------------------------------------------------------------
// SCAN ACTION.

EmberStatus emberStartScan(EmberNetworkScanType scanType,
                           uint32_t channelMask,
                           uint8_t duration)
{
  long *values = functionCallCheck("emberStartScan",
                                   "iii",
                                   scanType,
                                   channelMask,
                                   duration);
  bool failure = (bool)values[3];

  if (failure) {
    return EMBER_ERR_FATAL;
  } else if (expectingScanCallback) {
    return EMBER_MAC_SCANNING;
  } else {
    expectingScanCallback = true;
    return EMBER_SUCCESS;
  }
}

#define addStartScanCheck(type, mask, duration, failure) \
  addSimpleCheck("emberStartScan",                       \
                 "iii!",                                 \
                 (type),                                 \
                 (mask),                                 \
                 (duration),                             \
                 (failure))

// -----------------------------------------------------------------------------
// STOP ACTION.

// If we want to add functionality to stop specific scans,
// then we can add actions here.
EmberStatus emberStopScan(void)
{
  return EMBER_SUCCESS;
}

// -----------------------------------------------------------------------------
// SCHEDULE ACTION.

PRINTER(schedule)
{
  EmberNetworkScanType type = (EmberNetworkScanType)action->contents[0];
  uint32_t mask = (uint32_t)action->contents[1];
  bool failure = (bool)action->contents[3];
  uint8_t consumer = (uint8_t)action->contents[4];

  fprintf(stderr,
          " scheduled scan for consumer %d, mask 0x%08X, type %s, (should %s)",
          consumer,
          mask,
          (type == EMBER_ENERGY_SCAN ? "energy" : "active"),
          (failure ? "fail" : "pass"));
}
PERFORMER(schedule)
{
  EmberNetworkScanType type = (EmberNetworkScanType)action->contents[0];
  uint32_t mask = (uint32_t)action->contents[1];
  uint8_t duration = (uint8_t)action->contents[2];
  bool failure = (bool)action->contents[3];
  uint8_t consumer = (uint8_t)action->contents[4];
  EmberAfPluginScanDispatchScanData data = {
    .scanType = type,
    .channelMask = mask,
    .duration = duration,
    .handler = scanHandlers[consumer],
  };

  scriptAssert(action,
               ((emberAfPluginScanDispatchScheduleScan(&data) == EMBER_SUCCESS)
                || failure));
}

ACTION(schedule, iiiii);

#define addScheduleAction(type, mask, duration, failure, consumer) \
  addAction(&scheduleActionType,                                   \
            (type),                                                \
            (mask),                                                \
            (duration),                                            \
            (failure),                                             \
            (consumer))

// -----------------------------------------------------------------------------
// SINGLE.

static void singleSuccessTest(void)
{
  uint8_t i;

  for (scanTypeIndex = 0; scanTypeIndex < SCAN_TYPE_COUNT; scanTypeIndex++) {
    scanType = scanTypes[scanTypeIndex];
    for (channelsIndex = 0; channelsIndex < CHANNEL_MASK_COUNT; channelsIndex++) {
      channelNumsCurrent = channelNums[channelsIndex];
      channelMaskCurrent = channelMasks[channelsIndex];
      channelNumsCount   = channelNumsCounts[channelsIndex];

      // After we schedule a scan...
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false,  // failure?
                        0);     // consumer 0

      // ...a scan should start in the MAC...
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?
      // ...and the associated callbacks should eventually be called.
      for (i = 0; i < channelNumsCount; i++) {
        addRunAction(10); // some time passes between callbacks
        channelNumCurrent = channelNumsCurrent[i];

        if (scanType == EMBER_ENERGY_SCAN) {
          addEnergyScanCallbackAction(channelNumCurrent,
                                      RSSI);
          addScanHandlerCheck((uint8_t)RSSI,
                              channelNumCurrent,
                              scanType,
                              false, // complete?
                              SUCCESS,
                              0);    // consumer 0
        } else if (scanType == EMBER_ACTIVE_SCAN) {
          addActiveScanCallbackAction(LQI,
                                      RSSI);
          addScanHandlerCheck((uint8_t)RSSI,
                              LQI,
                              scanType,
                              false, // complete?
                              SUCCESS,
                              0);    // consumer 0
        } else {
          assert(0);
        }
      } // for channelNumCurrent in channelNumsCurrent

      // Finally, the scan complete handler should be called...
      addScanCompleteCallbackAction(0, // scan complete
                                    EMBER_SUCCESS);
      // ...and again we notify the consumer.
      addScanHandlerCheck(EMBER_SUCCESS,
                          0,     // scan complete
                          scanType,
                          true,  // complete?
                          SUCCESS,
                          0);    // consumer 0
    }
  }

  runScript();
}

static void singleFailureTest(void)
{
  uint8_t i;
  //state machine for Active scan has changed
  for (scanTypeIndex = 0; scanTypeIndex < SCAN_TYPE_COUNT - 1; scanTypeIndex++) {
    scanType = scanTypes[scanTypeIndex];
    for (channelsIndex = 0; channelsIndex < CHANNEL_MASK_COUNT; channelsIndex++) {
      channelNumsCurrent = channelNums[channelsIndex];
      channelMaskCurrent = channelMasks[channelsIndex];
      channelNumsCount   = channelNumsCounts[channelsIndex];

      // After we schedule a scan...
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false,  // failure?
                        0);     // consumer 0

      // ...what if the start scan passes...
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?
      // ...but the scan itself fails?
      for (i = 0; i < channelNumsCount; i++) {
        addRunAction(10); // some time passes before failure
        // The consumer should receives handlers for the failures.
        channelNumCurrent = channelNumsCurrent[i];

        addScanCompleteCallbackAction(channelNumCurrent,
                                      EMBER_ERR_FATAL);
        addScanHandlerCheck(EMBER_ERR_FATAL,
                            channelNumCurrent,
                            scanType,
                            true,  // complete?
                            SUCCESS,
                            0);    // consumer 0
      } // for channelNumCurrent in channelNumsCurrent

      // Then the scan is done.
      addScanCompleteCallbackAction(0, // scan complete
                                    EMBER_SUCCESS);
      addScanHandlerCheck(EMBER_SUCCESS,
                          0,     // scan complete
                          scanType,
                          true,  // complete?
                          SUCCESS,
                          0);    // consumer

      // After we schedule a scan...
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false,  // failure?
                        0);      // consumer 0
      // ...what if the start scan itself fails?
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        true); // failure
      // Then the consumer just gets notified that the scan failed.
      addScanHandlerCheck(EMBER_ERR_FATAL,
                          0,     // scan complete
                          scanType,
                          true,  // complete?
                          FAILURE,
                          0);    // consumer 0
    } // for channel in channels
  } // for scanType in scanTypes

  runScript();
}

// -----------------------------------------------------------------------------
// DOUBLE.

static void doubleSuccessTest(void)
{
  uint8_t i;

  for (scanTypeIndex = 0; scanTypeIndex < SCAN_TYPE_COUNT; scanTypeIndex++) {
    scanType = scanTypes[scanTypeIndex];
    for (channelsIndex = 0; channelsIndex < CHANNEL_MASK_COUNT; channelsIndex++) {
      channelNumsCurrent = channelNums[channelsIndex];
      channelMaskCurrent = channelMasks[channelsIndex];
      channelNumsCount   = channelNumsCounts[channelsIndex];

      //
      // First, the two consumers schedule scans synchronously.
      //

      for (consumer = 0; consumer < CONSUMER_COUNT; consumer++) {
        addScheduleAction(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false,    // failure?
                          consumer);
        addStartScanCheck(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false); // failure?
        for (i = 0; i < channelNumsCount; i++) {
          addRunAction(10); // some time passes between callbacks
          channelNumCurrent = channelNumsCurrent[i];

          if (scanType == EMBER_ENERGY_SCAN) {
            addEnergyScanCallbackAction(channelNumCurrent,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                channelNumCurrent,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                consumer);
          } else if (scanType == EMBER_ACTIVE_SCAN) {
            addActiveScanCallbackAction(LQI,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                LQI,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                consumer);
          } else {
            assert(0);
          }
        } // for channelNumCurrent in channelNumsCurrent

        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        addScanHandlerCheck(EMBER_SUCCESS,
                            0, // scan complete
                            scanType,
                            true, // complete?
                            SUCCESS,
                            consumer);
      } // for consumer in consumers

      //
      // Now, the first consumer successfully starts a scan,
      // and the second consumer schedules one right after. The behavior
      // is the same for both of the two scenarios.
      //   1. The second consumer schedules a scan right after the
      //      first consumer does.
      //   2. The second consumer schedules a scan during the scan
      //      that the first consumer schueduled.
      //

      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false,    // failure?
                        0);       // consumer 0
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?

      // When the second consumer schedules a scan, we will kick the dispatch
      // tick, but the call to emberStartScan will return EMBER_MAC_SCANNING.
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false,    // failure?
                        1);       // consumer 1
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?
      for (consumer = 0; consumer < CONSUMER_COUNT; consumer++) {
        if (consumer == 1) {
          addStartScanCheck(scanType,
                            channelMaskCurrent,
                            DURATION,
                            false); // failure?
        }
        for (i = 0; i < channelNumsCount; i++) {
          addRunAction(10); // some time passes between callbacks
          channelNumCurrent = channelNumsCurrent[i];

          if (scanType == EMBER_ENERGY_SCAN) {
            addEnergyScanCallbackAction(channelNumCurrent,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                channelNumCurrent,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                consumer);
          } else if (scanType == EMBER_ACTIVE_SCAN) {
            addActiveScanCallbackAction(LQI,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                LQI,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                consumer);
          } else {
            assert(0);
          }
        } // for channelNumCurrent in channelNumsCurrent

        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        addScanHandlerCheck(EMBER_SUCCESS,
                            0, // scan complete
                            scanType,
                            true, // complete?
                            SUCCESS,
                            consumer);
      } // for consumer in consumers
    } // for channel in channels
  } // for scanType in scanTypes

  runScript();
}

static void doubleFailureTest(void)
{
  uint8_t i;
  //state machine for Active scan has changed
  for (scanTypeIndex = 0; scanTypeIndex < SCAN_TYPE_COUNT - 1; scanTypeIndex++) {
    scanType = scanTypes[scanTypeIndex];
    for (channelsIndex = 0; channelsIndex < CHANNEL_MASK_COUNT; channelsIndex++) {
      channelNumsCurrent = channelNums[channelsIndex];
      channelMaskCurrent = channelMasks[channelsIndex];
      channelNumsCount   = channelNumsCounts[channelsIndex];

      //
      // First, the two consumers fail synchronously.
      //

      for (consumer = 0; consumer < CONSUMER_COUNT; consumer++) {
        // After we schedule a scan...
        addScheduleAction(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false,    // failure?
                          consumer);
        // ...what if the start scan passes...
        addStartScanCheck(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false); // failure?
        // ...but the scan itself fails?
        for (i = 0; i < channelNumsCount; i++) {
          addRunAction(10); // some time passes between callbacks
          // The consumer should receives handlers for the failures.
          channelNumCurrent = channelNumsCurrent[i];

          addScanCompleteCallbackAction(channelNumCurrent,
                                        EMBER_ERR_FATAL);
          addScanHandlerCheck(EMBER_ERR_FATAL,
                              channelNumCurrent,
                              scanType,
                              true,  // complete?
                              SUCCESS,
                              consumer);
        }

        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        addScanHandlerCheck(EMBER_SUCCESS,
                            0, // scan complete
                            scanType,
                            true, // complete?
                            SUCCESS,
                            consumer);
      } // for consumer in consumers

      //
      // Next, the first consumer fails the scans but the second passes.
      //

      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false, // failure?
                        0);    // consumer 0
      // The start scan for consumer 0 passes...
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?

      // ...and the start scan for consumer 1 returns EMBER_MAC_SCANNING...
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false, // failure?
                        1);    // consumer 1
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?

      for (consumer = 0; consumer < CONSUMER_COUNT; consumer++) {
        // ...so consumer 1 keeps trying...
        if (consumer == 1) {
          addStartScanCheck(scanType,
                            channelMaskCurrent,
                            DURATION,
                            false); // failure?
        }
        for (i = 0; i < channelNumsCount; i++) {
          addRunAction(10); // some time passes between callbacks
          channelNumCurrent = channelNumsCurrent[i];

          // ...but the scan fails for the first consumer.
          if (consumer == 0) {
            addScanCompleteCallbackAction(channelNumCurrent,
                                          EMBER_ERR_FATAL);
            addScanHandlerCheck(EMBER_ERR_FATAL,
                                channelNumCurrent,
                                scanType,
                                true,  // complete?
                                SUCCESS,
                                consumer);
          } else {
            if (scanType == EMBER_ENERGY_SCAN) {
              addEnergyScanCallbackAction(channelNumCurrent,
                                          RSSI);
              addScanHandlerCheck((uint8_t)RSSI,
                                  channelNumCurrent,
                                  scanType,
                                  false, // complete?
                                  SUCCESS,
                                  consumer);
            } else if (scanType == EMBER_ACTIVE_SCAN) {
              addActiveScanCallbackAction(LQI,
                                          RSSI);
              addScanHandlerCheck((uint8_t)RSSI,
                                  LQI,
                                  scanType,
                                  false, // complete?
                                  SUCCESS,
                                  consumer);
            } else {
              assert(0);
            }
          }
        } // for channelNumCurrent in channelNumsCurrent

        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        addScanHandlerCheck(EMBER_SUCCESS,
                            0, // scan complete
                            scanType,
                            true, // complete?
                            SUCCESS,
                            consumer);
      } // for consumer in consumers

      //
      // Finally, the first consumer fails to start the scan, but the
      // second consumer succeeds.
      //

      // The start scan fails for consumer 0 but passes for consumer 1...

      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false, // failure?
                        0);    // consumer 0
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        true); // failure?
      addScanHandlerCheck(EMBER_ERR_FATAL,
                          0,
                          scanType,
                          true, //complete?
                          FAILURE,
                          0); // consumer 0
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false, // failure?
                        1);    // consumer 1
      addStartScanCheck(scanType,
                        channelMaskCurrent,
                        DURATION,
                        false); // failure?

      // ...so consumer 1 does its thing.
      for (i = 0; i < channelNumsCount; i++) {
        addRunAction(10); // some time passes between callbacks
        channelNumCurrent = channelNumsCurrent[i];

        if (scanType == EMBER_ENERGY_SCAN) {
          addEnergyScanCallbackAction(channelNumCurrent,
                                      RSSI);
          addScanHandlerCheck((uint8_t)RSSI,
                              channelNumCurrent,
                              scanType,
                              false, // complete?
                              SUCCESS,
                              1);
        } else if (scanType == EMBER_ACTIVE_SCAN) {
          addActiveScanCallbackAction(LQI,
                                      RSSI);
          addScanHandlerCheck((uint8_t)RSSI,
                              LQI,
                              scanType,
                              false, // complete?
                              SUCCESS,
                              1);
        } else {
          assert(0);
        }
      } // for channelNumCurrent in channelNumsCurrent

      addScanCompleteCallbackAction(0, // scan complete
                                    EMBER_SUCCESS);
      addScanHandlerCheck(EMBER_SUCCESS,
                          0, // scan complete
                          scanType,
                          true,  // complete?
                          SUCCESS,
                          1); // consumer 1
    } // for channel in channels
  } // for scanType in scanTypes

  runScript();
}

// -----------------------------------------------------------------------------
// STRESS.

static void stressTest(void)
{
  uint8_t i, j;

  for (scanTypeIndex = 0; scanTypeIndex < SCAN_TYPE_COUNT; scanTypeIndex++) {
    scanType = scanTypes[scanTypeIndex];
    for (channelsIndex = 0; channelsIndex < CHANNEL_MASK_COUNT; channelsIndex++) {
      channelNumsCurrent = channelNums[channelsIndex];
      channelMaskCurrent = channelMasks[channelsIndex];
      channelNumsCount   = channelNumsCounts[channelsIndex];

      //
      // Synchronus (breadth) stress testing.
      //

      for (i = 0; i < EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE; i++) {
        // After we schedule a scan...
        addScheduleAction(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false,    // failure?
                          0);       // consumer 0
        // ...a scan should start in the MAC...
        addStartScanCheck(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false); // failure?
        // ...and the associated callbacks should eventually be called.
        for (j = 0; j < channelNumsCount; j++) {
          addRunAction(10); // some time passes between callbacks
          channelNumCurrent = channelNumsCurrent[j];

          if (scanType == EMBER_ENERGY_SCAN) {
            addEnergyScanCallbackAction(channelNumCurrent,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                channelNumCurrent,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                0);    // consumer 0
          } else if (scanType == EMBER_ACTIVE_SCAN) {
            addActiveScanCallbackAction(LQI,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                LQI,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                0);    // consumer 0
          } else {
            assert(0);
          }
        }

        // Finally, the scan complete handler should be called...
        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        // ...and again we notify the consumer.
        addScanHandlerCheck(EMBER_SUCCESS,
                            0,     // scan complete
                            scanType,
                            true,  // complete?
                            SUCCESS,
                            0);    // consumer 0
      }

      //
      // Asynchronus (depth) stress testing.
      //

      for (i = 0; i < EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE; i++) {
        addScheduleAction(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false,    // failure?
                          0);       // consumer 0
        addStartScanCheck(scanType,
                          channelMaskCurrent,
                          DURATION,
                          false); // failure?
      }

      // Make sure we are at capacity.
      addScheduleAction(scanType,
                        channelMaskCurrent,
                        DURATION,
                        true,    // failure?
                        0);      // consumer 0

      for (i = 0; i < EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE; i++) {
        if (i != 0) {
          addStartScanCheck(scanType,
                            channelMaskCurrent,
                            DURATION,
                            false); // failure?
        }
        for (j = 0; j < channelNumsCount; j++) {
          addRunAction(10); // some time passes between callbacks
          channelNumCurrent = channelNumsCurrent[j];

          if (scanType == EMBER_ENERGY_SCAN) {
            addEnergyScanCallbackAction(channelNumCurrent,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                channelNumCurrent,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                0);    // consumer 0
          } else if (scanType == EMBER_ACTIVE_SCAN) {
            addActiveScanCallbackAction(LQI,
                                        RSSI);
            addScanHandlerCheck((uint8_t)RSSI,
                                LQI,
                                scanType,
                                false, // complete?
                                SUCCESS,
                                0);    // consumer 0
          } else {
            assert(0);
          }
        }
        addScanCompleteCallbackAction(0, // scan complete
                                      EMBER_SUCCESS);
        addScanHandlerCheck(EMBER_SUCCESS,
                            0,     // scan complete
                            scanType,
                            true,  // complete?
                            SUCCESS,
                            0);    // consumer 0
      }
    } // for channel in channels
  } // for scanType in scanTypes

  runScript();
}

// -----------------------------------------------------------------------------
// MAIN.

static Test tests[] = {
  { "single-consumer-success-test", singleSuccessTest },
  { "single-consumer-failure-test", singleFailureTest },

  { "double-consumer-success-test", doubleSuccessTest },
  { "double-consumer-failure-test", doubleFailureTest },

  { "stress-test", stressTest        },

  { NULL, NULL              },
};

int main(int argc, char *argv[])
{
  Thunk test = parseTestArgument(argc, argv, tests);
  test();
  printf(" done ]\n");
  return 0;
}

// -----------------------------------------------------------------------------
// CALLBACKS.

void scriptTickCallback(void)
{
  // These guys are generated from AppBuilder.
  extern EmberEventControl emberAfPluginScanDispatchScanEventControl;
  extern void emberAfPluginScanDispatchScanEventHandler(void);
  const EmberEventData data[] = {
    { &emberAfPluginScanDispatchScanEventControl,
      emberAfPluginScanDispatchScanEventHandler, },
    { NULL, NULL, },
  };
  emberRunEvents(data);
}
