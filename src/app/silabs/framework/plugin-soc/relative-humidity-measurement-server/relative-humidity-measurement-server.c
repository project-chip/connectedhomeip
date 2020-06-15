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
 * @brief Definitions for the Relative Humidity Measurement Server plugin, which
 *        implements the server side of the Relative Humidity Measurement
 *        cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "relative-humidity-measurement-server.h"
#include EMBER_AF_API_HUMIDITY

#ifdef EMBER_AF_PLUGIN_REPORTING
#include "app/framework/plugin/reporting/reporting.h"
#endif

//------------------------------------------------------------------------------
// Plugin private macros

// Shorter macros for plugin options
#define MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS                                        \
  (EMBER_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * MILLISECOND_TICKS_PER_SECOND)

// Macro used to ensure sane humidity max/min values are stored
#define HUMIDITY_SANITY_CHECK 10000 //100.00%, in 0.01% steps = 10000

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeHumidityAttributes(uint16_t humidityPercentage);
static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emberAfPluginRelativeHumidityMeasurementServerReadEventControl;
static uint32_t humidityMeasurementIntervalMs
  = MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS;

// The default configuration to be used if no reporting has been set up
static EmberAfPluginReportingEntry defaultConfiguration = {
  EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
  0, //endpoint, which will be set on a per-use basis
  ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID, //clusterId
  ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID, //attributeId
  CLUSTER_MASK_SERVER, //mask
  EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
  .data.reported = {
    1, //minInterval
    EMBER_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S, //maxInterval
    EMBER_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_HUMIDITY_CHANGE_D_P //reportableChange
  }
};

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfPluginRelativeHumidityMeasurementServerInitCallback(void)
{
  checkForReportingConfig();
  emberEventControlSetActive(
    emberAfPluginRelativeHumidityMeasurementServerReadEventControl);
}

void emberAfPluginRelativeHumidityMeasurementServerStackStatusCallback(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    checkForReportingConfig();
    emberEventControlSetActive(
      emberAfPluginRelativeHumidityMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new humidity value
//******************************************************************************
void emberAfPluginRelativeHumidityMeasurementServerReadEventHandler(void)
{
  halHumidityStartRead();
  emberEventControlSetInactive(
    emberAfPluginRelativeHumidityMeasurementServerReadEventControl);
}

void halHumidityReadingCompleteCallback(uint16_t humidityCentiPercent,
                                        bool readSuccess)
{
  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    emberAfRelativeHumidityMeasurementClusterPrintln(
      "Humidity Measurement: %2d.%2d%%",
      (humidityCentiPercent / 100),
      (humidityCentiPercent % 100));
    writeHumidityAttributes(humidityCentiPercent);
  } else {
    emberAfRelativeHumidityMeasurementClusterPrintln(
      "Error reading humidity from HW");
  }

  emberEventControlSetDelayMS(
    emberAfPluginRelativeHumidityMeasurementServerReadEventControl,
    humidityMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin public functions

void emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > EMBER_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    humidityMeasurementIntervalMs = MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS;
  } else {
    humidityMeasurementIntervalMs
      = measurementIntervalS * MILLISECOND_TICKS_PER_SECOND;
  }
  emberEventControlSetDelayMS(
    emberAfPluginRelativeHumidityMeasurementServerReadEventControl,
    humidityMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin private functions

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
    if ((entry.clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)
        && (entry.attributeId
            == ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID)
        && (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        && (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        && (entry.manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)) {
      // Max interval is set in seconds, which is the same unit of time the
      // emberAfPluginRelativeHumidityMeasurementServerSetMeasurementRate
      // expects in this API.
      emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(
        entry.data.reported.maxInterval);
      existingEntry = true;
    }
  }

  // If no entry is found for the humidity measurement server, a default
  // reporting configuration should be created using the plugin defined options.
  // This needs to be done for all endpoints that support a humidity
  // measurement server.
  if (!existingEntry) {
    for (i = 0; i < emberAfEndpointCount(); i++) {
      endpoint = emberAfEndpointFromIndex(i);
      defaultConfiguration.endpoint = endpoint;
      if (emberAfContainsServer(endpoint, ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)) {
        emAfPluginReportingAddEntry(&defaultConfiguration);
      }
    }
  }
#endif
}

//******************************************************************************
// Update the humidity attribute of the humidity measurement cluster to
// be the humidity value given by the function's parameter.  This function
// will also query the current max and min read values, and update them if the
// given values is higher (or lower) than the previous records.
//******************************************************************************
static void writeHumidityAttributes(uint16_t humidityPercentage)
{
  uint8_t i;
  uint16_t humidityLimitPercentage;
  uint8_t endpoint;

  //we should not do anything if the humidityPercentage > 100%
  if (humidityPercentage > HUMIDITY_SANITY_CHECK) {
    return;
  }

  // Cycle through all endpoints, check to see if the endpoint has a humidity
  // server, and if so update the humidity attributes of that endpoint
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint,
                              ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)) {
      // Write the current humidity attribute
      emberAfWriteServerAttribute(
        endpoint,
        ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
        ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID,
        (uint8_t *) &humidityPercentage,
        ZCL_INT16U_ATTRIBUTE_TYPE);

      // Determine if this is a new minimum measured humidity, and update the
      // HUMIDITY_MIN_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(
        endpoint,
        ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
        ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
        (uint8_t *) (&humidityLimitPercentage),
        sizeof(uint16_t));
      if ((humidityLimitPercentage > HUMIDITY_SANITY_CHECK)
          || (humidityLimitPercentage > humidityPercentage)) {
        emberAfWriteServerAttribute(
          endpoint,
          ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
          ZCL_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
          (uint8_t *) &humidityPercentage,
          ZCL_INT16U_ATTRIBUTE_TYPE);
      }

      // Determine if this is a new maximum measured humidity, and update the
      // HUMIDITY_MAX_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(
        endpoint,
        ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
        ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
        (uint8_t *) (&humidityLimitPercentage),
        sizeof(uint16_t));
      if ((humidityLimitPercentage > HUMIDITY_SANITY_CHECK)
          || (humidityLimitPercentage < humidityPercentage)) {
        emberAfWriteServerAttribute(
          endpoint,
          ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
          ZCL_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
          (uint8_t *) &humidityPercentage,
          ZCL_INT16U_ATTRIBUTE_TYPE);
      }
    }
  }
}
