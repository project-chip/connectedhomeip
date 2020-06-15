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
 * @brief Test code for the Simple Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../include/af.h"
#include "../../util/common.h"
#include "../../util/af-main.h"
#include "../../util/client-api.h"
#include "enums.h"
#include "simple-metering-test.h"

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE

// Test mode:
//   bit 0: 0 - electric, 1 - gas
//   bit 1: reserved for future use
//   bit 2: 0 - no profiles, 1 - profiles
//   bit 3: 0 - no tick, 1 = tick
uint8_t testMode = 0;
uint8_t errorChance = 0;
uint16_t meterConsumptionRate = 0;
uint16_t meterConsumptionVariance = 0;
uint16_t batteryRate = 1;
uint32_t nextGetProfileInterval;
static uint32_t hourCounterTable[EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT];

#define emAfContainsSimpleMeterServerAttribute(endpoint, attribute) \
  emberAfContainsAttribute((endpoint), ZCL_SIMPLE_METERING_CLUSTER_ID, (attribute), CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE)

static void hourCounterTableInit(void);

static void hourCounterTableInit(void)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
    hourCounterTable[i] = 0;
  }
}

static void addToByteArray(uint8_t *data,
                           uint8_t len,
                           uint32_t toAdd,
                           bool lowHigh)
{
  uint16_t sum = 0;
  int8_t loc, end, incr;
  if (lowHigh) {
    loc  = 0;
    end  = len;
    incr = 1;
  } else {
    loc  = len - 1;
    end  = -1;
    incr = -1;
  }

  while ( loc != end ) {
    uint8_t t, s;
    t = data[loc];
    s = toAdd & 0xff;
    sum += t + s;
    data[loc] = sum & 0xff;
    sum >>= 8;
    toAdd >>= 8;
    loc += incr;
  }
}
static void addToByteArrayLowHigh(uint8_t *data,
                                  uint8_t len,
                                  uint32_t toAdd)
{
  addToByteArray(data, len, toAdd, true);
}
static void addToByteArrayHighLow(uint8_t *data,
                                  uint8_t len,
                                  uint32_t toAdd)
{
  addToByteArray(data, len, toAdd, false);
}

static uint32_t getMeteringConsumptionRate(uint8_t endpoint)
{
  uint8_t multiplierArray[] = { 0, 0, 0 };
  uint8_t divisorArray[] = { 0, 0, 0 };
  EmberAfAttributeType dataType;
  EmberStatus status;
  uint32_t multiplier;
  uint32_t divisor;
  uint32_t returnValue;

  // Now let's adjust the summation
  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_MULTIPLIER_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                multiplierArray,
                                3,
                                &dataType);

  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_DIVISOR_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                divisorArray,
                                3,
                                &dataType);

  multiplier = multiplierArray[2] << 16 | multiplierArray[1] << 8 | multiplierArray[0];
  divisor = divisorArray[2] << 16 | divisorArray[1] << 8 | divisorArray[0];
  returnValue = divisor > 0 ? multiplier / divisor : 0;
  return returnValue;
}

#if 0
//TODO: Review why this code is unused
static void setMeteringConsumptionRate(uint16_t rate, uint8_t endpoint)
{
  uint8_t dataType;
  EmberStatus status;
  uint32_t multiplier;
  uint32_t divisor;

  uint8_t multiplierArray[] = { 0, 0, 0 };
  uint8_t divisorArray[] = { 1, 0, 0 };
  multiplierArray[0] = rate & 0x00FF;
  multiplierArray[1] = rate & 0xFF00 >> 8;

  // Now let's adjust the summation
  status = emberAfWriteAttribute(endpoint,
                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                 ZCL_MULTIPLIER_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 multiplierArray,
                                 ZCL_INT24U_ATTRIBUTE_TYPE);

  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_MULTIPLIER_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                multiplierArray,
                                3,
                                &dataType);

  multiplier = multiplierArray[2] << 16 | multiplierArray[1] << 8 | multiplierArray[0];

  status = emberAfWriteAttribute(endpoint,
                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                 ZCL_DIVISOR_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 divisorArray,
                                 ZCL_INT24U_ATTRIBUTE_TYPE);

  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_DIVISOR_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                divisorArray,
                                3,
                                &dataType);
  divisor = divisorArray[2] << 16 | divisorArray[1] << 8 | divisorArray[0];
  emberAfCorePrintln("divisor %u", divisor);
}
#endif

void afTestMeterPrint(void)
{
  emberAfSimpleMeteringClusterPrintln("TM:\r\n"
                                      "mode:%x\r\n"
                                      "meterConsumptionRate:%2x\r\n"
                                      "meterConsumptionVariance:%2x\r\n",
                                      testMode,
                                      meterConsumptionRate,
                                      meterConsumptionVariance);
}

void afTestMeterSetConsumptionRate(uint16_t rate, uint8_t endpoint)
{
  emberAfSimpleMeteringClusterPrintln("TM: set consumption rate %2x", rate);
  meterConsumptionRate = rate;
  //setMeteringConsumptionRate(rate,endpoint);
}

void afTestMeterSetConsumptionVariance(uint16_t variance)
{
  emberAfSimpleMeteringClusterPrintln("TM: set consumption variance %2x", variance);
  meterConsumptionVariance = variance;
}
void afTestMeterAdjust(uint8_t endpoint)
{
  uint8_t summation[] = { 0, 0, 0, 0, 0, 0 };

  // Seconds in day
  uint32_t ct = emberAfGetCurrentTime() % 86400;
  uint32_t diff;

  emberAfSimpleMeteringClusterPrintln("TM: adjust");

  diff = (uint32_t)meterConsumptionRate * ct;

  if ( meterConsumptionVariance > 0 ) {
    diff += ct * (uint32_t)(emberGetPseudoRandomNumber() % meterConsumptionVariance);
  }

  if (BIGENDIAN_CPU) {
    addToByteArrayHighLow(summation, 6, diff);
  } else {
    addToByteArrayLowHigh(summation, 6, diff);
  }

  emberAfSimpleMeteringClusterPrintln("Summation:%x %x %x %x %x %x",
                                      summation[0],
                                      summation[1],
                                      summation[2],
                                      summation[3],
                                      summation[4],
                                      summation[5]);

  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        summation,
                        ZCL_INT48U_ATTRIBUTE_TYPE);
}

// 0 off, 1 if electric, 2 if gas
void afTestMeterMode(uint8_t endpoint, uint8_t mode)
{
  uint8_t status = 0;
  uint8_t unitOfMeasure;
  uint8_t deviceType;
  uint8_t summationFormatting = 0x2C; // 00101100

  if ( mode == 0 ) {
    testMode &= (~0x08);
  } else if ( mode == 1 ) {
    testMode &= (~0x01);
    testMode |= 0x08;
    unitOfMeasure = EMBER_ZCL_AMI_UNIT_OF_MEASURE_KILO_WATT_HOURS;
    deviceType = EMBER_ZCL_METERING_DEVICE_TYPE_ELECTRIC_METERING;
  } else if ( mode == 2 ) {
    testMode |= (0x01 | 0x08);
    unitOfMeasure = EMBER_ZCL_AMI_UNIT_OF_MEASURE_BT_US_OR_BTU_PER_HOUR;
    deviceType = EMBER_ZCL_METERING_DEVICE_TYPE_GAS_METERING;
  }
  emberAfSimpleMeteringClusterPrintln("TM: mode %x", testMode);

  // Set attributes...

  // Status = OK
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_STATUS_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &status,
                        ZCL_BITMAP8_ATTRIBUTE_TYPE);

  // Device type is either gas or electric
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &deviceType,
                        ZCL_BITMAP8_ATTRIBUTE_TYPE);

  // Unit of measure is either KWH or BTU
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_UNIT_OF_MEASURE_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &unitOfMeasure,
                        ZCL_ENUM8_ATTRIBUTE_TYPE);

  // Summation formatting is 0x2C
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_SUMMATION_FORMATTING_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &summationFormatting,
                        ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
void afTestMeterSetError(uint8_t endpoint,
                         uint8_t error)
{
  emberAfSimpleMeteringClusterPrintln("TM: set error %x", error);
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_STATUS_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &error,
                        ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

void afTestMeterRandomError(uint8_t chanceIn256)
{
  emberAfSimpleMeteringClusterPrintln("TM: random error %x", chanceIn256);
  errorChance = chanceIn256;
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)

uint8_t testMeterProfiles[EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES][3];
uint32_t testMeterIntervalEndTimes[EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES];
void afTestMeterEnableProfiles(uint8_t enable)
{
  uint8_t i;
  emberAfSimpleMeteringClusterPrintln("TM: profiles %x", enable);
  switch (enable) {
    case 0:
      testMode &= (~0x04);
      break;
    case 1:
      testMode |= 0x04;
      break;
    case 2:
      for ( i = 0; i < EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++ ) {
        testMeterProfiles[i][0]
          = testMeterProfiles[i][1]
              = testMeterProfiles[i][2]
                  = 0x00;
      }
    case 3:
      for ( i = 0; i < EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++ ) {
        emberAfSimpleMeteringClusterPrintln("P %x: %x%x%x",
                                            i,
                                            testMeterProfiles[i][0],
                                            testMeterProfiles[i][1],
                                            testMeterProfiles[i][2]);
      }
      break;
  }
}

#endif // EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0

void emAfTestMeterInit(uint8_t endpoint)
{
  MEMSET(testMeterProfiles,
         0,
         (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES * 3));
  MEMSET(testMeterIntervalEndTimes,
         0,
         (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) * 4);
  // battery life remaining (0x0201), begin at 255, and decrement every minute. INT8U.
  if ( emAfContainsSimpleMeterServerAttribute(endpoint,
                                              ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID) ) {
    uint8_t batteryLife = 100; // 100% to begin. 0xff is reserved.
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          &batteryLife,
                          ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  hourCounterTableInit();

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE != 0)
  batteryRate = EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_BATTERY_RATE;
#else
  batteryRate = 1;
#endif
#endif
  if (batteryRate > 100) {
    batteryRate = 100;
  }

  #if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  nextGetProfileInterval = PROFILE_INTERVAL_PERIOD_IN_SECONDS;
  #endif
}

// Note: In the final implementation, according to the SE spec following must
//  happen:
//   - ReadingSnapShotTime attribute must be set to the actual UTC time of
//        when the CurrentSummationDelivered, CurrentSummationReceived,
//        CurrentMaxDemandDelievered and CurrentMaxDemandReceived were measured.
//   - CurrentMaxDemandDeliveredTime must be set to UTC time of when the
//        CurrentMaxDelivered was read
//   - CurrentMaxDemandReceivedTime must be set to UTC time of when the
//        CurrentMaxDemandReceived was read
void emAfTestMeterTick(uint8_t endpoint)
{
  uint32_t *hourCounter;
  uint32_t diff, currentTime;
  uint8_t status, dataType;
  uint8_t summation[] = { 0, 0, 0, 0, 0, 0 };
  uint32_t formatting;

#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  uint8_t intervalSummation[] = { 0, 0, 0 };
#endif

  // random counters for keeping track of minutes/seconds locally
  // should probably use local time but I'm worried what happens when
  // that changes through the CLI, so I'm doing this for now.
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint,
                                                     ZCL_SIMPLE_METERING_CLUSTER_ID);
  if (ep == 0xFF) {
    emberAfSimpleMeteringClusterPrintln("Invalid endpoint %x", endpoint);
    return;
  }

  hourCounter = &hourCounterTable[ep];
  //static uint16_t batteryLifeLastUpdateTime = 0; // would be used for local time
  //static uint16_t hoursInOperationLastUpdateTime = 0;

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
  uint8_t meterStatus;
#endif // EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

  // Now let's adjust the summation
  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                summation,
                                6,
                                &dataType);

  if ( status != EMBER_ZCL_STATUS_SUCCESS ) {
    emberAfSimpleMeteringClusterPrintln("ERR: can't read summation status 0x%x", status);
    return;
  }

  //diff = getMeteringConsumptionRate(endpoint);
  diff = (uint32_t)meterConsumptionRate;
  if ( meterConsumptionVariance > 0 ) {
    diff += (uint32_t)(emberGetPseudoRandomNumber() % meterConsumptionVariance);
  }

  if (BIGENDIAN_CPU) {
    addToByteArrayHighLow(summation, 6, diff);
  } else {
    addToByteArrayLowHigh(summation, 6, diff);
  }

  if ((*hourCounter % 60) == 0) {
    // emberAfSimpleMeteringClusterPrintln("TM summation:%x %x %x %x %x %x",
    //                                     summation[0],
    //                                     summation[1],
    //                                     summation[2],
    //                                     summation[3],
    //                                     summation[4],
    //                                     summation[5]);
  }

  formatting = getMeteringConsumptionRate(endpoint);

  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        summation,
                        ZCL_INT48U_ATTRIBUTE_TYPE);

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
  if ( errorChance > 0 ) {
    if ( (emberGetPseudoRandomNumber() % 256) < errorChance ) {
      emberAfReadAttribute(endpoint,
                           ZCL_SIMPLE_METERING_CLUSTER_ID,
                           ZCL_STATUS_ATTRIBUTE_ID,
                           CLUSTER_MASK_SERVER,
                           &meterStatus,
                           1,
                           &dataType);
      if ( meterStatus == 0 ) {
        emberAfSimpleMeteringClusterPrintln("TM: random error set");
        meterStatus = 1;
        emberAfWriteAttribute(endpoint,
                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                              ZCL_STATUS_ATTRIBUTE_ID,
                              CLUSTER_MASK_SERVER,
                              &meterStatus,
                              ZCL_BITMAP8_ATTRIBUTE_TYPE);
      }
    }
  }
#endif // EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  if ( testMode & 0x04 ) { // Profiles are enabled
    currentTime = emberAfGetCurrentTime();
    if (currentTime >= nextGetProfileInterval) {
      emberAfSimpleMeteringClusterPrintln("Current Time %u", currentTime);

      //emberAfSimpleMeteringClusterPrintln("currentTime %u nextGetProfileInterval %u",currentTime,nextGetProfileInterval);
      status = emberAfReadAttribute(endpoint,
                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                    ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                                    CLUSTER_MASK_SERVER,
                                    intervalSummation,
                                    3,
                                    &dataType);
      if ( status != EMBER_ZCL_STATUS_SUCCESS ) {
        emberAfSimpleMeteringClusterPrintln("ERR: can't read interval summation status 0x%x", status);
        return;
      }

      if (BIGENDIAN_CPU) {
        addToByteArrayHighLow(intervalSummation, 3, diff);
      } else {
        addToByteArrayLowHigh(intervalSummation, 3, diff);
      }

      emberAfSimpleMeteringClusterPrintln("diff %u", diff);

      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            intervalSummation,
                            ZCL_INT24U_ATTRIBUTE_TYPE);

      // Profile swap...
      //emberAfSimpleMeteringClusterPrintln("TM: swapping profile");

      for ( status = EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES - 1;
            status > 0;
            status-- ) {
        //emberAfSimpleMeteringClusterPrintln("Copying from %u:%x%x%x to %u:%x%x%x", status, testMeterProfiles[status][0],testMeterProfiles[status][1],testMeterProfiles[status][2],status-1,testMeterProfiles[status-1][0],testMeterProfiles[status-1][1],testMeterProfiles[status-1][2]);
        MEMCOPY(testMeterProfiles[status],
                testMeterProfiles[status - 1],
                3);
        MEMCOPY(&testMeterIntervalEndTimes[status],
                &testMeterIntervalEndTimes[status - 1],
                4);
      }
      MEMCOPY(testMeterProfiles[0],
              intervalSummation,
              3);
      testMeterIntervalEndTimes[0] = nextGetProfileInterval;

      for ( status = 0;
            status < EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
            status++ ) {
        emberAfSimpleMeteringClusterPrintln("i %u profile %x%x%x time %u", status, testMeterProfiles[status][0], testMeterProfiles[status][1], testMeterProfiles[status][2], testMeterIntervalEndTimes[status]);
      }
      //emberAfSimpleMeteringClusterPrintln("Copying Interval Summation %x%x%x",testMeterProfiles[0][0],testMeterProfiles[0][1],testMeterProfiles[0][2]);
      intervalSummation[0]
        = intervalSummation[1]
            = intervalSummation[2] = 0;
      nextGetProfileInterval += PROFILE_INTERVAL_PERIOD_IN_SECONDS;
      // Reset summation
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            intervalSummation,
                            ZCL_INT24U_ATTRIBUTE_TYPE);

      // Set interval time
      currentTime = emberAfGetCurrentTime();
      //emberAfSimpleMeteringClusterPrintln("WRITING CURRENT TIME %u",currentTime);
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            (uint8_t*)&nextGetProfileInterval,
                            ZCL_UTC_TIME_ATTRIBUTE_TYPE);

      for ( status = 0;
            status < EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
            status++ ) {
        // emberAfSimpleMeteringClusterPrintln("TM: Pr %x: %x%x%x",
        //  status,
        //  testMeterProfiles[status][0],
        //  testMeterProfiles[status][1],
        //  testMeterProfiles[status][2]);
      }
    } // 2nd if
  } // testMode & 0x04
#endif // (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)

  // adjust the optional attributes, if they were selected in the cluster configuration window
  // current tier 1 summation delivered (0x0100), increment with same values from current summation
  // delivered. Type is INT48U so same as current samation delivered.
  if ( emAfContainsSimpleMeterServerAttribute(endpoint,
                                              ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID) ) {
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          summation,
                          ZCL_INT48U_ATTRIBUTE_TYPE);

    if ((*hourCounter % 60) == 0) {
      //emberAfSimpleMeteringClusterPrintln("TM tier1 updated too");
    }
  }

  // battery life remaining (0x0201), begin at 255, and decrement every minute. INT8U.
  if ( emAfContainsSimpleMeterServerAttribute(endpoint,
                                              ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID) ) {
    if ((*hourCounter) && ((*hourCounter % (60 * batteryRate)) == 0) ) { // every minute
      uint8_t batteryLife;
      status = emberAfReadAttribute(endpoint,
                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                    ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                                    CLUSTER_MASK_SERVER,
                                    &batteryLife,
                                    1,
                                    &dataType);
      if ( status != EMBER_ZCL_STATUS_SUCCESS ) {
        emberAfSimpleMeteringClusterPrintln("ERR: can't read battery life");
        return;
      }
      if (batteryLife) {
        batteryLife--; // decrement every minute. stop at zero.
      }
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_REMAINING_BATTERY_LIFE_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            &batteryLife,
                            ZCL_INT8U_ATTRIBUTE_TYPE);
      emberAfSimpleMeteringClusterPrintln("TM battery life: %x",
                                          batteryLife);
    } // end if hourCounter is at minute else do nothing
  } // end if contains attribute battery life

  // hours in operation (0x0202), increment every 60 minutes. INT24U.
  if ( emAfContainsSimpleMeterServerAttribute(endpoint,
                                              ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID) ) {
    if ((*hourCounter) && ((*hourCounter % 3600) == 0)) {  // every hour, but skip 0
      uint8_t hoursInOperation[] = { 0, 0, 0 };
      status = emberAfReadAttribute(endpoint,
                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                    ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                                    CLUSTER_MASK_SERVER,
                                    hoursInOperation,
                                    3,
                                    &dataType);
      if ( status != EMBER_ZCL_STATUS_SUCCESS ) {
        emberAfSimpleMeteringClusterPrintln("ERR: can't read hours in operation");
        return;
      }
      // increment every hour. no clue what happens on overflow
      if (BIGENDIAN_CPU) {
        addToByteArrayHighLow(hoursInOperation, 3, 1);
      } else {
        addToByteArrayLowHigh(hoursInOperation, 3, 1);
      }
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            hoursInOperation,
                            ZCL_INT24U_ATTRIBUTE_TYPE);
      emberAfSimpleMeteringClusterPrintln("TM hours in operation:%x %x %x",
                                          hoursInOperation[0],
                                          hoursInOperation[1],
                                          hoursInOperation[2]);
    } // end if hourCounter is at hour else do nothing
  } // end if contains attribute hours in operation

  // instantaneous demand (0x0400), increment with same values from current summation
  // delivered, namely the difference from the last second (the rate +/- the variance
  // applied this current second. this is a INT24S.
  if ( emAfContainsSimpleMeterServerAttribute(endpoint,
                                              ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID) ) {
    // how do you do signed intergers? I think ZCL uses two's complement, but
    // I can't find this in the document anywhere. In this implementation,
    // the demand will always be positive, so note that this code will not
    // support a negative demand as it is.
    uint8_t instantaneousDemand[] = { 0, 0, 0 };
    if (BIGENDIAN_CPU) {
      addToByteArrayHighLow(instantaneousDemand, 3, diff);
    } else {
      addToByteArrayLowHigh(instantaneousDemand, 3, diff);
    }
    // uncomment this to test. it's too noisy to do each time and
    // not useful to do once in a while, since it's random.
    //emberAfSimpleMeteringClusterPrintln("TM instantaneous demand:%x %x %x",
    //                                    instantaneousDemand[0],
    //                                    instantaneousDemand[1],
    //                                    instantaneousDemand[2]);
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          instantaneousDemand,
                          ZCL_INT24S_ATTRIBUTE_TYPE);
  }
  // need to do this whether or not hours in operation is selected, so
  // battery life counter still works properly
  (*hourCounter)++; // this function called every second
}

bool emAfTestMeterGetProfiles(uint8_t intervalChannel,
                              uint32_t endTime,
                              uint8_t numberOfPeriods)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  // Get the current time
  uint32_t intervalEndTime;
  uint32_t ct;
  uint8_t start, stop, dt, profilesReturned;
  uint8_t profileIntervalPeriod = PROFILE_INTERVAL_PERIOD_TIMEFRAME;
  uint8_t ep = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
  EmberAfStatus status = emberAfReadAttribute(ep,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER,
                                              (uint8_t*)&ct,
                                              4,
                                              &dt);
  EmberAfAmiGetProfileStatus returnStatus;
  // If we can read the last profile time, AND either the endTime requested is 0 OR it is greater than
  // the oldest read time, we can handle this - otherwise we return failure 0x05.
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    uint32_t lastValidInterval = (ct / PROFILE_INTERVAL_PERIOD_IN_SECONDS) * PROFILE_INTERVAL_PERIOD_IN_SECONDS;
    uint32_t lastEndTime = (lastValidInterval >= (PROFILE_INTERVAL_PERIOD_IN_SECONDS * EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES)
                            ? lastValidInterval - (PROFILE_INTERVAL_PERIOD_IN_SECONDS * EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES)
                            : PROFILE_INTERVAL_PERIOD_IN_SECONDS);
    emberAfSimpleMeteringClusterPrintln("lastValidInterval %u lastEndTime %u", lastValidInterval, lastEndTime);
    if (endTime >= lastEndTime || endTime == 0) {
      uint8_t i;
      for (i = 0; i < EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES; i++) {
        if (endTime >= testMeterIntervalEndTimes[i]) {
          start = i;
          break;
        }
      }
      // start = ((endTime > 0 && endTime >lastEndTime ) ? EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES - ((endTime - lastEndTime) / PROFILE_INTERVAL_PERIOD_IN_SECONDS) -1 : 0);

      if (endTime == 0) {
        start = 0;
      }
      //If the end time is far ahead of the lastEndTime interval, we want to start at the latest interval.
      if (start > EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) {
        start = 0;
      }

      emberAfSimpleMeteringClusterPrintln("ct-endtime%u, start%u ", (ct - endTime), start);

      //stop = (((MAX_PROFILE_INDEX - start) > numberOfPeriods) ? (start + numberOfPeriods - 1) : MAX_PROFILE_INDEX);
      if (lastValidInterval < PROFILE_INTERVAL_PERIOD_IN_SECONDS && start == 0 && endTime > 0) {
        stop = start;
      } else {
        stop = (lastValidInterval / PROFILE_INTERVAL_PERIOD_IN_SECONDS) - 1;
      }

      if (stop > EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES) {
        stop = EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES;
      }

      emberAfSimpleMeteringClusterPrintln("end Time 0x%4x, lastValidInterval 0x%4x, start %u", endTime, lastValidInterval, start);
      profilesReturned = (stop - start);

      if (profilesReturned > numberOfPeriods) {
        profilesReturned = numberOfPeriods;
      }

      //DEBUG
      intervalEndTime = testMeterIntervalEndTimes[start];
      emberAfSimpleMeteringClusterPrintln("start: %x, stop: %x, preq: %x, pret, %x", start, stop, numberOfPeriods, profilesReturned);
      emberAfSimpleMeteringClusterFlush();

      //Set the return Status
      returnStatus = EMBER_ZCL_AMI_GET_PROFILE_STATUS_SUCCESS;

      if (intervalChannel > EMBER_ZCL_AMI_INTERVAL_CHANNEL_CONSUMPTION_RECEIVED) {
        returnStatus = EMBER_ZCL_AMI_GET_PROFILE_STATUS_UNDEFINED_INTERVAL_CHANNEL_REQUESTED;
      }

      if (profilesReturned == 0) {
        returnStatus = EMBER_ZCL_AMI_GET_PROFILE_STATUS_NO_INTERVALS_AVAILABLE_FOR_THE_REQUESTED_TIME;
        profileIntervalPeriod = 0;
        intervalEndTime = 0;
      }

      emberAfFillCommandSimpleMeteringClusterGetProfileResponse(intervalEndTime,
                                                                returnStatus,
                                                                profileIntervalPeriod,
                                                                profilesReturned,
                                                                appResponseData,
                                                                profilesReturned * 3);
      appResponseData[1] = emberAfIncomingZclSequenceNumber;

      for (i = 0; i < profilesReturned; i++) {
        uint32_t data = (0x00 << 24) | (testMeterProfiles[start + i][0] << 16) | (testMeterProfiles[start + i][1] << 8) | (testMeterProfiles[start + i][2]);
        emberAfCopyInt24u(appResponseData, 10 + (i * 3), data);
        emberAfSimpleMeteringClusterPrintln("i %u profile %x%x%x time %u", i, testMeterProfiles[start + i][0], testMeterProfiles[start + i][1], testMeterProfiles[start + i][2], testMeterIntervalEndTimes[start + i]);
      }
      appResponseLength = 10 + (3 * profilesReturned);

      emberAfSendResponse();

      emberAfSimpleMeteringClusterPrintln("get profile: 0x%x, 0x%4x, 0x%x",
                                          intervalChannel,
                                          endTime,
                                          numberOfPeriods);

      return true;
    }
  }
#endif //(EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  //Otherwise we send back a failure

  emberAfFillCommandSimpleMeteringClusterGetProfileResponse(0x00000000,
                                                            0x05,
                                                            0,
                                                            0,
                                                            appResponseData,
                                                            0);
  appResponseData[1] = emberAfIncomingZclSequenceNumber;
  appResponseLength = 10;
  emberAfSendResponse();
  return true;
}

#else
void emAfTestMeterInit(uint8_t endpoint)
{
}
void emAfTestMeterTick(uint8_t endpoint)
{
}
bool emAfTestMeterGetProfiles(uint8_t intervalChannel,
                              uint32_t endTime,
                              uint8_t numberOfPeriods)
{
  return false;
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
