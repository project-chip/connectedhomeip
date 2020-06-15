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
 * @brief Definitions for the Illuminance Measurement Server plugin, which
 *        implements the server side of the Illuminance Measurement cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "illuminance-measurement-server.h"
#include EMBER_AF_API_ILLUMINANCE

#ifdef EMBER_AF_PLUGIN_REPORTING
#include "app/framework/plugin/reporting/reporting.h"
#endif

//------------------------------------------------------------------------------
// Plugin private macros

// Shorter macros for plugin options
#define MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS                               \
  (EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * MILLISECOND_TICKS_PER_SECOND)

#define ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MAX   200
#define ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MIN   2
//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeIlluminanceAttributes(uint16_t illuminanceLogLx);
static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emberAfPluginIlluminanceMeasurementServerReadEventControl;
static uint32_t illuminanceMeasurementIntervalMS =
  MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS;

// The default configuration to be used if no reporting has been set up
static EmberAfPluginReportingEntry defaultConfiguration = {
  EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
  0, //endpoint, which will be set on a per-use basis
  ZCL_ILLUM_MEASUREMENT_CLUSTER_ID, //clusterId
  ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID, //attributeId
  CLUSTER_MASK_SERVER, //mask
  EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
  .data.reported = {
    1, //minInterval
    EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S, //maxInterval
    EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_CHANGE //reportableChange
  }
};

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfPluginIlluminanceMeasurementServerInitCallback(void)
{
  checkForReportingConfig();
  // Start the ReadEvent, which will re-activate itself perpetually
  emberEventControlSetActive(
    emberAfPluginIlluminanceMeasurementServerReadEventControl);
}

void emberAfPluginIlluminanceMeasurementServerStackStatusCallback(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the illuminance for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    checkForReportingConfig();
    emberEventControlSetActive(
      emberAfPluginIlluminanceMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new illuminance value
//******************************************************************************
void emberAfPluginIlluminanceMeasurementServerReadEventHandler(void)
{
  uint8_t multiplier;

  halCommonGetToken(&multiplier, TOKEN_SI1141_MULTIPLIER);

  // sanity check for mulitplier
  if ((multiplier < ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MIN)
      || (multiplier > ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MAX)) {
    multiplier = 0; // use default value instead
  }
  halIlluminanceStartRead(multiplier);
  emberEventControlSetInactive(
    emberAfPluginIlluminanceMeasurementServerReadEventControl);
}

void halIlluminanceReadingCompleteCallback(uint16_t logLux)
{
  writeIlluminanceAttributes(logLux);
  emberEventControlSetDelayMS(
    emberAfPluginIlluminanceMeasurementServerReadEventControl,
    illuminanceMeasurementIntervalMS);
}
//------------------------------------------------------------------------------
// Plugin public functions

void emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    illuminanceMeasurementIntervalMS = MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS;
  } else {
    illuminanceMeasurementIntervalMS = (measurementIntervalS
                                        * MILLISECOND_TICKS_PER_SECOND);
  }
  emberEventControlSetDelayMS(
    emberAfPluginIlluminanceMeasurementServerReadEventControl,
    illuminanceMeasurementIntervalMS);
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
    if ((entry.clusterId == ZCL_ILLUM_MEASUREMENT_CLUSTER_ID)
        && (entry.attributeId == ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID)
        && (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        && (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        && (entry.manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)) {
      // Max interval is set in seconds, which is the same unit of time the
      // emberAfPluginIlluminanceMeasurementServerSetMeasurementRate expects in
      // this API.
      emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(
        entry.data.reported.maxInterval);
      existingEntry = true;
    }
  }

  // If no entry is found for the illuminance measurement server, a default
  // reporting configuration should be created using the plugin defined options.
  // This needs to be done for all endpoints that support a illuminance
  // measurement server.
  if (!existingEntry) {
    for (i = 0; i < emberAfEndpointCount(); i++) {
      endpoint = emberAfEndpointFromIndex(i);
      defaultConfiguration.endpoint = endpoint;
      if (emberAfContainsServer(endpoint, ZCL_ILLUM_MEASUREMENT_CLUSTER_ID)) {
        emAfPluginReportingAddEntry(&defaultConfiguration);
      }
    }
  }
#endif
}

//******************************************************************************
// Update the illuminance attribute of the illuminance measurement cluster to
// be the illuminance value given by the HAL layer. This function will also
// query the current max and min read values, and update them if the given
// values is higher (or lower) than the previous records.
//******************************************************************************
static void writeIlluminanceAttributes(uint16_t illuminanceLogLx)
{
  uint16_t illumLimitLogLx;

  uint8_t i;
  uint8_t endpoint;

  // Cycle through all endpoints, check to see if the endpoint has a illuminance
  // server, and if so update the illuminance attributes of that endpoint
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint, ZCL_ILLUM_MEASUREMENT_CLUSTER_ID)) {
      emberAfIllumMeasurementClusterPrintln(
        "Illuminance Measurement(LogLux):%d",
        illuminanceLogLx);
      //Write the current illuminance attribute
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                  ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
                                  (uint8_t *) &illuminanceLogLx,
                                  ZCL_INT16U_ATTRIBUTE_TYPE);

      // Determine if this is a new minimum measured illuminance, and update the
      // ILLUM_MIN_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(endpoint,
                                 ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                 ZCL_ILLUM_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                 (uint8_t *) (&illumLimitLogLx),
                                 sizeof(uint16_t));
      if (illumLimitLogLx > illuminanceLogLx) {
        emberAfWriteServerAttribute(endpoint,
                                    ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                    ZCL_ILLUM_MIN_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &illuminanceLogLx,
                                    ZCL_INT16U_ATTRIBUTE_TYPE);
      }

      // Determine if this is a new maximum measured illuminance, and update the
      // ILLUM_MAX_MEASURED attribute if that is the case.
      emberAfReadServerAttribute(endpoint,
                                 ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                 ZCL_ILLUM_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                 (uint8_t *)(&illumLimitLogLx),
                                 sizeof(uint16_t));
      if (illumLimitLogLx < illuminanceLogLx) {
        emberAfWriteServerAttribute(endpoint,
                                    ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
                                    ZCL_ILLUM_MAX_MEASURED_VALUE_ATTRIBUTE_ID,
                                    (uint8_t *) &illuminanceLogLx,
                                    ZCL_INT16U_ATTRIBUTE_TYPE);
      }
    }
  }
}
