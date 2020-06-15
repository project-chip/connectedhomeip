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
 * @brief Interface code for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "metering-server-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/client-api.h"
#include "metering-interface.h"
#include EMBER_AF_API_POWER_METER

#ifndef EMBER_SCRIPTED_TEST
#define emberAfPluginMeteringServerContainsAttribute(endpoint, attribute) \
  emberAfContainsAttribute((endpoint),                                    \
                           ZCL_SIMPLE_METERING_CLUSTER_ID,                \
                           (attribute),                                   \
                           CLUSTER_MASK_SERVER,                           \
                           EMBER_AF_NULL_MANUFACTURER_CODE)
#else
bool emberAfPluginMeteringServerContainsAttribute(uint8_t endpoint,
                                                  EmberAfAttributeId attribute);
#endif

// The profile interval period timeframe enum value
// is 3 which according to the SE spec is 15 minutes
#define METERING_PROFILE_INTERVAL_PERIOD_TIMEFRAME            3
#define METERING_PROFILE_INTERVAL_PERIOD_IN_MINUTES          15
#define METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS \
  (METERING_PROFILE_INTERVAL_PERIOD_IN_MINUTES * 60)
#define METERING_PROFILE_INTERVAL_PERIOD_IN_MILLISECONDS \
  ((METERING_PROFILE_INTERVAL_PERIOD_IN_MINUTES * 60) * 1000)

#define METERING_SERVER_METER_STATUS_ERROR_NONE 0

#define METERING_MILLIWATTS_IN_WATTS                                        1000
#define METERING_MULTIPLIER                                                    1
#define METERING_DIVISOR        (METERING_MILLIWATTS_IN_WATTS * SECONDS_IN_HOUR)
#define METERING_METER_PROFILES_SIZE                                           3
#define METERING_START_BYTE_IN_RESPONSE_DATA                                  10

#define METERING_GET_PROFILE_RESPONSE_STATUS_SUCCESS                           0
#define METERING_GET_PROFILE_RESPONSE_STATUS_UNDEFINED_INTERVAL                1
#define METERING_GET_PROFILE_RESPONSE_STATUS_INTERVAL_NOT_SUPPORTED            2
#define METERING_GET_PROFILE_RESPONSE_STATUS_INVALID_ENDTIME                   3
#define METERING_GET_PROFILE_RESPONSE_STATUS_PERIOD_REQUESTED_ERROR            4
#define METERING_GET_PROFILE_RESPONSE_STATUS_INTERVAL_UNAVAILABLE              5

#define METERING_MULTIPLIER_ARRAY_SIZE                                         3
#define METERING_DIVISOR_ARRAY_SIZE                                            3

// Bits 0 to 2: Number of Digits to the right of the Decimal Point.
// Bits 3 to 6: Number of Digits to the left of the Decimal Point.
// Bit 7: If set, suppress leading zeros.
// for 0x33, it should be shown as 012345.123
#define METERING_SUMMATION_FORMAT                                           0x33
#define METERING_INIT_ATTRIBUTE_SIZE                                           4

static bool profileEnabled = false;
static uint32_t nextGetProfileInterval;
static uint32_t secondCounterTable[EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT];
static uint8_t meteringProfiles[EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES][METERING_METER_PROFILES_SIZE];
static uint32_t meteringIntervalEndTimes[EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES];

// this function add byte to an array as a summation.
// the endianness can the summation array can be determined
// the isLittleEndian parameter.
static void addToByteArrayEndian(uint8_t *data,
                                 uint8_t len,
                                 uint16_t toAdd,
                                 bool isLittleEndian)
{
  uint16_t temporarySummation = 0;
  int8_t location, end, step;
  if (isLittleEndian) {
    location = 0;
    end = len;
    step = 1;
  } else {
    location = len - 1;
    end = -1;
    step = -1;
  }

  while (location != end) {
    uint8_t temporaryArrayByte, temporaryByteToAdd;
    temporaryArrayByte = data[location];
    temporaryByteToAdd = toAdd & 0xff;
    temporarySummation += temporaryArrayByte + temporaryByteToAdd;
    data[location] = temporarySummation & 0xff;
    temporarySummation >>= 8;
    toAdd >>= 8;
    location += step;
  }
}

static void addToByteArray(uint8_t *data,
                           uint8_t len,
                           uint16_t toAdd)
{
  addToByteArrayEndian(data, len, toAdd, !(BIGENDIAN_CPU));
}

static uint16_t milliWsToWs(int32_t data)
{
  uint32_t retval;

  //we dont handle negative power here.
  if (data < 0) {
    data = 0;
  }
  retval = data / METERING_MILLIWATTS_IN_WATTS; // integer division to change
  // the milliWatt to Watt

  // minimize the flooring error by round off)
  if ((data % METERING_MILLIWATTS_IN_WATTS)
      > (METERING_MILLIWATTS_IN_WATTS / 2)) {
    retval = retval + 1;
  }
  return retval;
}

void emberAfPluginMeteringServerAttributeInit(uint8_t endpoint)
{
  uint8_t attributeIndex;

  uint8_t multiplierArray[METERING_MULTIPLIER_ARRAY_SIZE] = {
    BYTE_0(METERING_MULTIPLIER),
    BYTE_1(METERING_MULTIPLIER),
    BYTE_2(METERING_MULTIPLIER)
  };

  uint8_t divisorArray[METERING_DIVISOR_ARRAY_SIZE] = {
    BYTE_0(METERING_DIVISOR),
    BYTE_1(METERING_DIVISOR),
    BYTE_2(METERING_DIVISOR)
  };

  uint8_t attributeValue[METERING_INIT_ATTRIBUTE_SIZE] = {
    METERING_SERVER_METER_STATUS_ERROR_NONE,
    EMBER_ZCL_METERING_DEVICE_TYPE_ELECTRIC_METERING,
    EMBER_ZCL_AMI_UNIT_OF_MEASURE_KILO_WATT_HOURS,
    METERING_SUMMATION_FORMAT
  };
  uint8_t attributeType[METERING_INIT_ATTRIBUTE_SIZE] = {
    ZCL_BITMAP8_ATTRIBUTE_TYPE,
    ZCL_BITMAP8_ATTRIBUTE_TYPE,
    ZCL_ENUM8_ATTRIBUTE_TYPE,
    ZCL_BITMAP8_ATTRIBUTE_TYPE
  };
  uint16_t attributeIds[METERING_INIT_ATTRIBUTE_SIZE] = {
    ZCL_STATUS_ATTRIBUTE_ID,
    ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
    ZCL_UNIT_OF_MEASURE_ATTRIBUTE_ID,
    ZCL_SUMMATION_FORMATTING_ATTRIBUTE_ID
  };

  // write attributes
  for (attributeIndex = 0;
       attributeIndex < METERING_INIT_ATTRIBUTE_SIZE;
       attributeIndex++) {
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          attributeIds[attributeIndex],
                          CLUSTER_MASK_SERVER,
                          &attributeValue[attributeIndex],
                          attributeType[attributeIndex]);
  }

  // Now let's adjust the mulitplier and divisor
  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_MULTIPLIER_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        multiplierArray,
                        ZCL_INT24U_ATTRIBUTE_TYPE);

  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_DIVISOR_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        divisorArray,
                        ZCL_INT24U_ATTRIBUTE_TYPE);
}

bool emberAfPluginMeteringServerInterfaceEnableProfiles(uint8_t enable)
{
#if (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  uint8_t i;
  switch (enable) {
    case EMBER_AF_PLUGIN_METERING_SERVER_DISABLE_PROFILING:
      profileEnabled = false;
      break;
    case EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING:
      profileEnabled = true;
      break;
    case EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES:
      for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES; i++) {
        meteringProfiles[i][0]
          = meteringProfiles[i][1]
              = meteringProfiles[i][2]
                  = 0x00;
      }
      break;
    default:
      break;
  }
  return true;
#else
  return false;
#endif // EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0
}

void emberAfPluginMeteringServerInterfaceInit(uint8_t endpoint)
{
  MEMSET(meteringProfiles,
         0,
         sizeof(meteringProfiles));
  MEMSET(meteringIntervalEndTimes,
         0,
         sizeof(meteringIntervalEndTimes));
  MEMSET(secondCounterTable,
         0,
         EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT
         * sizeof(secondCounterTable[0]));

  emberAfPluginMeteringServerAttributeInit(endpoint);
  // clear the profile data
  emberAfPluginMeteringServerInterfaceEnableProfiles(
    EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES);
  // enable the profiling
  emberAfPluginMeteringServerInterfaceEnableProfiles(
    EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING);

  #if (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  nextGetProfileInterval = METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS;
  #endif
}

// this tick function will be called every seconds, and we take measurements
// here then we calcaulate the energy consummed in mWs
void emberAfPluginMeteringServerInterfaceTick(uint8_t endpoint)
{
  uint32_t *secondCounter;
  uint32_t currentTime;
  uint8_t status, dataType;
  uint8_t summation[] = { 0, 0, 0, 0, 0, 0 };
  uint8_t meterStatus;
  int16_t temperature;
  uint16_t diff;

#if (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  uint8_t intervalSummation[] = { 0, 0, 0 };
#endif

  // random counters for keeping track of minutes/seconds locally
  uint8_t endpointIndex = emberAfFindClusterServerEndpointIndex(endpoint,
                                                                ZCL_SIMPLE_METERING_CLUSTER_ID);
  if (endpointIndex == MAX_INT8U_VALUE) {
    emberAfSimpleMeteringClusterPrintln(
      "Simple Metering Cluster cannot be found in endpoint 0x%x",
      endpoint);
    return;
  }

  secondCounter = &secondCounterTable[endpointIndex];

  // Now let's adjust the summation
  status = emberAfReadAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                summation,
                                sizeof(summation),
                                &dataType);

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSimpleMeteringClusterPrintln(
      "Metering: can't read summation status 0x%x",
      status);
    return;
  }

  diff = milliWsToWs(halGetActivePowerMilliW());
  addToByteArray(summation, sizeof(summation), diff);
  if ((*secondCounter % SECONDS_IN_MINUTE) == 1) {
    temperature = halGetPowerMeterTempCentiC();
    emberAfSimpleMeteringClusterPrintln(
      "Metering: OnChip temperature: %d CentiC",
      temperature);
  }

  emberAfWriteAttribute(endpoint,
                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                        ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        summation,
                        ZCL_INT48U_ATTRIBUTE_TYPE);

  // power factor
  if (emberAfPluginMeteringServerContainsAttribute(
        endpoint,
        ZCL_POWER_FACTOR_ATTRIBUTE_ID)) {
    uint8_t powerFactorPositive;
    int8_t powerFactor;
    powerFactor = halGetPowerFactor();
    emberAfSimpleMeteringClusterPrintln("Metering: Power Factor:%d",
                                        powerFactor);
    if (powerFactor < 0) {
      powerFactorPositive = 0;
    } else {
      powerFactorPositive = (uint8_t)powerFactor;
    }
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_POWER_FACTOR_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          &powerFactorPositive,
                          ZCL_INT8S_ATTRIBUTE_TYPE);
  }
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       ZCL_STATUS_ATTRIBUTE_ID,
                       CLUSTER_MASK_SERVER,
                       &meterStatus,
                       sizeof(meterStatus),
                       &dataType);
  if (meterStatus == 0) {
    meterStatus = halGetPowerMeterStatus();
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_STATUS_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          &meterStatus,
                          ZCL_BITMAP8_ATTRIBUTE_TYPE);
  }

#if (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  if (profileEnabled) { // Profiles are enabled
    currentTime = emberAfGetCurrentTime();
    if (currentTime >= nextGetProfileInterval) {
      status = emberAfReadAttribute(
        endpoint,
        ZCL_SIMPLE_METERING_CLUSTER_ID,
        ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
        CLUSTER_MASK_SERVER,
        intervalSummation,
        sizeof(intervalSummation),
        &dataType);
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSimpleMeteringClusterPrintln(
          "Metering: can't read interval summation status 0x%x",
          status);
        return;
      }

      addToByteArray(intervalSummation,
                     sizeof(intervalSummation),
                     diff);
      emberAfWriteAttribute(
        endpoint,
        ZCL_SIMPLE_METERING_CLUSTER_ID,
        ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
        CLUSTER_MASK_SERVER,
        intervalSummation,
        ZCL_INT24U_ATTRIBUTE_TYPE);

      // Profile swap
      for (status = EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES - 1;
           status > 0;
           status--) {
        MEMMOVE(meteringProfiles[status],
                meteringProfiles[status - 1],
                METERING_METER_PROFILES_SIZE);
        MEMMOVE(&meteringIntervalEndTimes[status],
                &meteringIntervalEndTimes[status - 1],
                sizeof(uint32_t));
      }
      MEMMOVE(meteringProfiles[0],
              intervalSummation,
              sizeof(intervalSummation));
      meteringIntervalEndTimes[0] = nextGetProfileInterval;

      MEMSET(intervalSummation, 0, sizeof(intervalSummation));
      nextGetProfileInterval += METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS;
      // Reset summation
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_VALUE_DELIVERED_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            intervalSummation,
                            ZCL_INT24U_ATTRIBUTE_TYPE);

      // Set interval time
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            (uint8_t*)&nextGetProfileInterval,
                            ZCL_UTC_TIME_ATTRIBUTE_TYPE);
    } // 2nd if
  } // profileEnabled
#endif // (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)

  // adjust the optional attributes, if they were selected in the cluster
  // configuration window current tier 1 summation delivered (0x0100),
  // increment with same values from current summation delivered.
  // Type is INT48U so same as current samation delivered.
  if ( emberAfPluginMeteringServerContainsAttribute(endpoint,
                                                    ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID) ) {
    emberAfWriteAttribute(endpoint,
                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                          ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          summation,
                          ZCL_INT48U_ATTRIBUTE_TYPE);
  }
  // hours in operation (0x0202), increment every 60 minutes. INT24U.
  if ( emberAfPluginMeteringServerContainsAttribute(endpoint,
                                                    ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID) ) {
    // update the attribute every hr, but skip hour zero
    if ((*secondCounter) && ((*secondCounter % SECONDS_IN_HOUR) == 0)) {
      uint8_t hoursInOperation[] = { 0, 0, 0 };
      status = emberAfReadAttribute(endpoint,
                                    ZCL_SIMPLE_METERING_CLUSTER_ID,
                                    ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                                    CLUSTER_MASK_SERVER,
                                    hoursInOperation,
                                    sizeof(hoursInOperation),
                                    &dataType);
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSimpleMeteringClusterPrintln(
          "Metering: can't read hours in operation");
        return;
      }
      // increment every hour. no clue what happens on overflow
      addToByteArray(hoursInOperation, sizeof(hoursInOperation), 1);
      emberAfWriteAttribute(endpoint,
                            ZCL_SIMPLE_METERING_CLUSTER_ID,
                            ZCL_HOURS_IN_OPERATION_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            hoursInOperation,
                            ZCL_INT24U_ATTRIBUTE_TYPE);
    } // end if secondCounter is at hour else do nothing
  } // end if contains attribute hours in operation
    // need to do this whether or not hours in operation is selected, so
    // battery life counter still works properly
  (*secondCounter)++; // this function called every second
}

bool emberAfPluginMeteringServerInterfaceGetProfiles(uint8_t intervalChannel,
                                                     uint32_t endTime,
                                                     uint8_t numberOfPeriods)
{
#if (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  uint32_t intervalEndTime;
  uint32_t currentTime;
  uint8_t start = 0, stop, dataType, i, profilesReturned;
  uint8_t profileIntervalPeriod = METERING_PROFILE_INTERVAL_PERIOD_TIMEFRAME;
  uint8_t endpoint = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
  // Get the current time
  EmberAfStatus status =
    emberAfReadAttribute(endpoint,
                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                         ZCL_CURRENT_PARTIAL_PROFILE_INTERVAL_START_TIME_DELIVERED_ATTRIBUTE_ID,
                         CLUSTER_MASK_SERVER,
                         (uint8_t*)&currentTime,
                         sizeof(currentTime),
                         &dataType);
  EmberAfAmiGetProfileStatus returnStatus;
  // If we can read the last profile time, AND either the endTime requested
  // is 0 OR it is greater than the oldest read time, we can handle this -
  // otherwise we return failure 0x05.
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    uint32_t lastValidInterval =
      (currentTime / METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS)
      * METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS;
    uint32_t lastEndTime = ((lastValidInterval
                             >= (METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS
                                 * EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES))
                            ? (lastValidInterval
                               - (METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS
                                  * EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES))
                            : METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS);
    if (endTime >= lastEndTime || endTime == 0) {
      for (i = 0; i < EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES; i++) {
        if (endTime >= meteringIntervalEndTimes[i]) {
          start = i;
          break;
        }
      }

      if (endTime == 0) {
        start = 0;
      }

      if (lastValidInterval < METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS
          && start == 0
          && endTime > 0) {
        stop = start;
      } else {
        stop = (lastValidInterval
                / METERING_PROFILE_INTERVAL_PERIOD_IN_SECONDS)
               - 1;
      }

      if (stop > EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES) {
        stop = EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES;
      }
      profilesReturned = (stop - start);

      if (profilesReturned > numberOfPeriods) {
        profilesReturned = numberOfPeriods;
      }
      intervalEndTime = meteringIntervalEndTimes[start];

      // Set the return Status
      returnStatus = EMBER_ZCL_AMI_GET_PROFILE_STATUS_SUCCESS;
      if (intervalChannel > EMBER_ZCL_AMI_INTERVAL_CHANNEL_CONSUMPTION_RECEIVED) {
        returnStatus =
          EMBER_ZCL_AMI_GET_PROFILE_STATUS_UNDEFINED_INTERVAL_CHANNEL_REQUESTED;
      }
      if (profilesReturned == 0) {
        returnStatus =
          EMBER_ZCL_AMI_GET_PROFILE_STATUS_NO_INTERVALS_AVAILABLE_FOR_THE_REQUESTED_TIME;
        profileIntervalPeriod = 0;
        intervalEndTime = 0;
      }
      emberAfFillCommandSimpleMeteringClusterGetProfileResponse(intervalEndTime,
                                                                returnStatus,
                                                                profileIntervalPeriod,
                                                                profilesReturned,
                                                                appResponseData,
                                                                profilesReturned * METERING_METER_PROFILES_SIZE);

      for (i = 0; i < profilesReturned; i++) {
        uint8_t profileIndex = start + i;
        uint32_t data = INT8U_TO_INT32U(0,
                                        meteringProfiles[profileIndex][0],
                                        meteringProfiles[profileIndex][1],
                                        meteringProfiles[profileIndex][2]);
        emberAfCopyInt24u(
          appResponseData,
          (METERING_START_BYTE_IN_RESPONSE_DATA
           + (i
              * METERING_METER_PROFILES_SIZE)),
          data);
      }
      appResponseLength = METERING_START_BYTE_IN_RESPONSE_DATA
                          + (METERING_METER_PROFILES_SIZE
                             * profilesReturned);

      emberAfSendResponse();
      return true;
    }
  }
#endif // (EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES != 0)
  // Otherwise we send back a failure
  emberAfFillCommandSimpleMeteringClusterGetProfileResponse(
    0,                    // endTime
    METERING_GET_PROFILE_RESPONSE_STATUS_INTERVAL_UNAVAILABLE,
    0,                    // profileIntervalPeriod
    0,                    // numberOfPeriodsDelivered
    appResponseData,
    0);                   // intervalsLen
  appResponseLength = METERING_START_BYTE_IN_RESPONSE_DATA;
  emberAfSendResponse();
  return true;
}
