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
 * @brief Definitions for the Temperature Measurement Server plugin, which
 *        implement the server side of the Temperature Measurement cluster.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "temperature-measurement-server-test.h"
#endif

#include "app/framework/include/af.h"
#include "temperature-measurement-server.h"
#include EMBER_AF_API_TEMPERATURE

#ifdef EMBER_AF_PLUGIN_REPORTING
#include "app/framework/plugin/reporting/reporting.h"
#endif

//------------------------------------------------------------------------------
// Plugin private macros

// Shorter macros for plugin options
#define MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS                               \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * MILLISECOND_TICKS_PER_SECOND)

#define CRITICAL_TEMPERATURE_ASSERT_THRESHOLD_MC                                             \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_ASSERT_CRITICAL_THRESHOLD \
   * 1000)

#define CRITICAL_TEMPERATURE_DEASSERT_THRESHOLD_MC                                             \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_DEASSERT_CRITICAL_THRESHOLD \
   * 1000)

#define WARNING_TEMPERATURE_ASSERT_THRESHOLD_MC                                             \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_ASSERT_WARNING_THRESHOLD \
   * 1000)

#define WARNING_TEMPERATURE_DEASSERT_THRESHOLD_MC                                             \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_DEASSERT_WARNING_THRESHOLD \
   * 1000)

//Macro used to ensure sane temperature max/min values are stored
#define TEMPERATURE_SANITY_CHECK (-30000)

//shorter defines for local use
#define OVER_TEMPERATURE_NORMAL \
  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_NORMAL
#define OVER_TEMPERATURE_WARNING \
  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_WARNING
#define OVER_TEMPERATURE_CRITICAL \
  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_CRITICAL

#define OVERTEMPERATURE_STATE_NUMBER                  3
#define OVERTEMPERATURE_STATE_STRING_CHAR_MAX         9

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeTemperatureAttributes(int32_t temperatureMilliC);
static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emberAfPluginTemperatureMeasurementServerReadEventControl;
static uint32_t temperatureMeasurementIntervalMs =
  MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS;

#if defined(EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE)
static uint8_t overTemperatureCurrentState;
static const uint8_t overTemperaturePrint[OVERTEMPERATURE_STATE_NUMBER][OVERTEMPERATURE_STATE_STRING_CHAR_MAX] = {
  { "NORMAL" },
  { "WARNING" },
  { "CRITICAL" }
};
#endif

// The default configuration to be used if no reporting has been set up
#ifdef EMBER_AF_PLUGIN_REPORTING
static EmberAfPluginReportingEntry defaultConfiguration = {
  EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
  0, //endpoint, which will be set on a per-use basis
  ZCL_TEMP_MEASUREMENT_CLUSTER_ID, //clusterId
  ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID, //attributeId
  CLUSTER_MASK_SERVER, //mask
  EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
  .data.reported = {
    1, //minInterval
    EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S, //maxInterval
    EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_TEMPERATURE_CHANGE_M_C //reportableChange
  }
};
#endif

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

static void checkForReportingConfig(void)
{
#ifdef EMBER_AF_PLUGIN_REPORTING
  uint8_t i;
  EmberAfPluginReportingEntry entry;
  uint8_t endpoint;
  bool existingEntry = false;

  // Cycle through the reporting table to determine if an entry has been created
  // for the device before.  If so, update the sensor's hardware polling rate to
  // match the attribute defined maxInterval.  Otherwise, the plugin will use
  // the plugin's option defined default hardware polling interval.
  for (i = 0; i < REPORT_TABLE_SIZE; i++) {
    emAfPluginReportingGetEntry(i, &entry);
    if ((entry.clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)
        && (entry.attributeId == ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID)
        && (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        && (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        && (entry.manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)) {
      // Max interval is set in seconds, which is the same unit of time the
      // emberAfPluginTemperatureMeasurementServerSetMeasurementInterval
      // expects in this API.
      emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(
        entry.data.reported.maxInterval);
      existingEntry = true;
    }
  }

  // If no entry is found for the temperature measurement server, a default
  // reporting configuration should be created using the plugin defined options.
  // This needs to be done for all endpoints that support a temperature
  // measurement server.
  if (!existingEntry) {
    for (i = 0; i < emberAfEndpointCount(); i++) {
      endpoint = emberAfEndpointFromIndex(i);
      defaultConfiguration.endpoint = endpoint;
      if (emberAfContainsServer(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID)) {
        emAfPluginReportingAddEntry(&defaultConfiguration);
      }
    }
  }
#endif
}

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfPluginTemperatureMeasurementServerInitCallback(void)
{
  checkForReportingConfig();

  // Start the ReadEvent, which will re-activate itself perpetually
  emberEventControlSetActive(
    emberAfPluginTemperatureMeasurementServerReadEventControl);
}

void emberAfPluginTemperatureMeasurementServerStackStatusCallback(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    checkForReportingConfig();
    emberEventControlSetActive(
      emberAfPluginTemperatureMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new temperature value
//******************************************************************************
void emberAfPluginTemperatureMeasurementServerReadEventHandler(void)
{
  halTemperatureStartRead();
  emberEventControlSetInactive(
    emberAfPluginTemperatureMeasurementServerReadEventControl);
}

void halTemperatureReadingCompleteCallback(int32_t temperatureMilliC,
                                           bool readSuccess)
{
#if defined(EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE)
  uint8_t overTemperatureNewState;
#endif

  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    emberAfTempMeasurementClusterPrintln("Temperature: %d milliC",
                                         temperatureMilliC);
    writeTemperatureAttributes(temperatureMilliC);
  } else {
    emberAfTempMeasurementClusterPrintln("Error reading temperature from HW");
  }
#if defined(EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE)
  overTemperatureNewState = overTemperatureCurrentState;
  switch (overTemperatureCurrentState) {
    case OVER_TEMPERATURE_NORMAL:
      if (temperatureMilliC > WARNING_TEMPERATURE_ASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_WARNING;
      } else if (temperatureMilliC > CRITICAL_TEMPERATURE_ASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_CRITICAL;
      }
      break;
    case OVER_TEMPERATURE_WARNING:
      if (temperatureMilliC < WARNING_TEMPERATURE_DEASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_NORMAL;
      } else if (temperatureMilliC > CRITICAL_TEMPERATURE_ASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_CRITICAL;
      }
      break;
    case OVER_TEMPERATURE_CRITICAL:
      if (temperatureMilliC < CRITICAL_TEMPERATURE_DEASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_WARNING;
      } else if (temperatureMilliC < WARNING_TEMPERATURE_DEASSERT_THRESHOLD_MC) {
        overTemperatureNewState = OVER_TEMPERATURE_NORMAL;
      }
      break;
    default:
      break;
  }
  if (overTemperatureNewState != overTemperatureCurrentState) {
    emberAfTempMeasurementClusterPrintln(
      "Over Temperature previous state: %s new state: %s",
      overTemperaturePrint[overTemperatureCurrentState],
      overTemperaturePrint[overTemperatureNewState]);
    emberAfPluginTemperatureMeasurementServerOverTemperatureCallback(
      overTemperatureCurrentState,
      overTemperatureNewState);
    overTemperatureCurrentState = overTemperatureNewState;
  }
#endif
  emberEventControlSetDelayMS(
    emberAfPluginTemperatureMeasurementServerReadEventControl,
    temperatureMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin public functions

void emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    temperatureMeasurementIntervalMs = MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS;
  } else {
    temperatureMeasurementIntervalMs
      = measurementIntervalS * MILLISECOND_TICKS_PER_SECOND;
  }
  emberEventControlSetDelayMS(
    emberAfPluginTemperatureMeasurementServerReadEventControl,
    temperatureMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin private functions

//******************************************************************************
// Update the temperature attribute of the temperature measurement cluster to
// be the temperature value given by the function's parameter.  This function
// will also query the current max and min read values, and update them if the
// given values is higher (or lower) than the previous records.
//******************************************************************************
static void writeTemperatureAttributes(int32_t temperatureMilliC)
{
  int16_t tempLimitCentiC;
  int16_t temperatureCentiC;

  uint8_t i;
  uint8_t endpoint;

  temperatureCentiC = temperatureMilliC / 10;

  // Cycle through all endpoints, check to see if the endpoint has a temperature
  // server, and if so update the temperature attributes of that endpoint
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint, ZCL_TEMP_MEASUREMENT_CLUSTER_ID)) {
      // Write the current temperature attribute
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                  ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
                                  (uint8_t *) &temperatureCentiC,
                                  ZCL_INT16S_ATTRIBUTE_TYPE);

      // Determine if this is a new minimum measured temperature, and update the
      // TEMP_MIN_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(endpoint,
                                 ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                 ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                 (uint8_t *) (&tempLimitCentiC),
                                 sizeof(int16_t));
      if ((tempLimitCentiC < TEMPERATURE_SANITY_CHECK)
          || (tempLimitCentiC > temperatureCentiC)) {
        emberAfWriteServerAttribute(endpoint,
                                    ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                    ZCL_TEMP_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &temperatureCentiC,
                                    ZCL_INT16S_ATTRIBUTE_TYPE);
      }

      // Determine if this is a new maximum measured temperature, and update the
      // TEMP_MAX_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(endpoint,
                                 ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                 ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                 (uint8_t *) (&tempLimitCentiC),
                                 sizeof(int16_t));
      if ((tempLimitCentiC < TEMPERATURE_SANITY_CHECK)
          || (tempLimitCentiC < temperatureCentiC)) {
        emberAfWriteServerAttribute(endpoint,
                                    ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
                                    ZCL_TEMP_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &temperatureCentiC,
                                    ZCL_INT16S_ATTRIBUTE_TYPE);
      }
    }
  }
}
