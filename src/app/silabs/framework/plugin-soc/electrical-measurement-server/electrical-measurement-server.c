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
 * @brief Definitions for the Electrical Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "electrical-measurement-server-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "electrical-measurement-server.h"
#include EMBER_AF_API_POWER_METER

#define ELECTRICAL_MEASUREMENT_TYPE_BIT_ACTIVE         0
#define ELECTRICAL_MEASUREMENT_TYPE_BIT_PHASE_A        3

#define ELECTRICAL_AC_ALARM_MASK_BIT_VOLTAGE_OVERLOAD        0
#define ELECTRICAL_AC_ALARM_MASK_BIT_CURRENT_OVERLOAD        1
#define ELECTRICAL_AC_ALARM_MASK_BIT_ACTIVE_POWER_OVERLOAD   2

#define ELECTRICAL_VOLTAGE_MULTIPLIER                 1
#define ELECTRICAL_VOLTAGE_DIVISOR                   10 // accuracy to 0.1V
#define ELECTRICAL_CURRENT_MULTIPLIER                 1
#define ELECTRICAL_CURRENT_DIVISOR                 1000 // accuracy to mA
#define ELECTRICAL_POWER_MULTIPLIER                   1
#define ELECTRICAL_POWER_DIVISOR                     10 // accuracy to 0.1W

#define ELECTRICAL_MILLIUNIT_IN_UNIT               1000

#define BIT16(x) (((uint16_t) 1) << (x))

typedef struct {
  uint16_t rmsVoltage;
  uint16_t rmsCurrent;
  int16_t activePower;
  int8_t  powerFactor;
} ElectricalData;

//------------------------------------------------------------------------------
// Plugin private macros

// Shorter macros for plugin options
#define ELECTRICAL_MAX_MEASUREMENT_INTERVAL_MS                               \
  (EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * MILLISECOND_TICKS_PER_SECOND)

#define ELECTRICAL_MAX_VOLTAGE_OVERLOAD_VOLT \
  EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_VOLTAGE_RATING_V

#define ELECTRICAL_MAX_CURRENT_OVERLOAD_AMP \
  EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_CURRENT_RATING_A

#define ELECTRICAL_MAX_ACTIVE_POWER_OVERLOAD_WATT \
  EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_POWER_RATING_W

#define ELECTRICAL_OVERLOAD_ALARMS_MASK                   \
  (BIT16(ELECTRICAL_AC_ALARM_MASK_BIT_VOLTAGE_OVERLOAD)   \
   | BIT16(ELECTRICAL_AC_ALARM_MASK_BIT_CURRENT_OVERLOAD) \
   | BIT16(ELECTRICAL_AC_ALARM_MASK_BIT_ACTIVE_POWER_OVERLOAD))

#define ELECTRICAL_MEASUREMENT_TYPE              \
  (BIT32(ELECTRICAL_MEASUREMENT_TYPE_BIT_ACTIVE) \
   | BIT32(ELECTRICAL_MEASUREMENT_TYPE_BIT_PHASE_A))

#define ELECTRICAL_REPORT_CHANGE_VOLTAGE                                                   \
  (EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_RMS_VOLTAGE_CHANGE_M_C \
   * ELECTRICAL_VOLTAGE_DIVISOR)                                                           \
  / (ELECTRICAL_MILLIUNIT_IN_UNIT                                                          \
     * ELECTRICAL_VOLTAGE_MULTIPLIER)

#define ELECTRICAL_REPORT_CHANGE_CURRENT                                                   \
  (EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_RMS_CURRENT_CHANGE_M_C \
   * ELECTRICAL_CURRENT_DIVISOR)                                                           \
  / (ELECTRICAL_MILLIUNIT_IN_UNIT                                                          \
     * ELECTRICAL_CURRENT_MULTIPLIER)

#define ELECTRICAL_REPORT_CHANGE_POWER                                                      \
  (EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_ACTIVE_POWER_CHANGE_M_C \
   * ELECTRICAL_POWER_DIVISOR)                                                              \
  / (ELECTRICAL_MILLIUNIT_IN_UNIT                                                           \
     * ELECTRICAL_POWER_MULTIPLIER)

#define ELECTTRICAL_INIT_ATTRIBUTES_ARRAY_SIZE 11

const static uint16_t attributeValues[ELECTTRICAL_INIT_ATTRIBUTES_ARRAY_SIZE] = {
  ELECTRICAL_MEASUREMENT_TYPE,
  ELECTRICAL_VOLTAGE_MULTIPLIER,
  ELECTRICAL_VOLTAGE_DIVISOR,
  ELECTRICAL_CURRENT_MULTIPLIER,
  ELECTRICAL_CURRENT_DIVISOR,
  ELECTRICAL_POWER_MULTIPLIER,
  ELECTRICAL_POWER_DIVISOR,
  ELECTRICAL_OVERLOAD_ALARMS_MASK,
  ELECTRICAL_MAX_VOLTAGE_OVERLOAD_VOLT,
  ELECTRICAL_MAX_CURRENT_OVERLOAD_AMP,
  ELECTRICAL_MAX_ACTIVE_POWER_OVERLOAD_WATT
};

const static uint16_t attributeValuesIds[ELECTTRICAL_INIT_ATTRIBUTES_ARRAY_SIZE] = {
  ZCL_MEASUREMENT_TYPE_ATTRIBUTE_ID,
  ZCL_AC_VOLTAGE_MULTIPLIER_ATTRIBUTE_ID,
  ZCL_AC_VOLTAGE_DIVISOR_ATTRIBUTE_ID,
  ZCL_AC_CURRENT_MULTIPLIER_ATTRIBUTE_ID,
  ZCL_AC_CURRENT_DIVISOR_ATTRIBUTE_ID,
  ZCL_AC_POWER_MULTIPLIER_ATTRIBUTE_ID,
  ZCL_AC_POWER_DIVISOR_ATTRIBUTE_ID,
  ZCL_AC_OVERLOAD_ALARMS_MASK_ATTRIBUTE_ID,
  ZCL_AC_VOLTAGE_OVERLOAD_ATTRIBUTE_ID,
  ZCL_AC_CURRENT_OVERLOAD_ATTRIBUTE_ID,
  ZCL_AC_POWER_OVERLOAD_ATTRIBUTE_ID
};

const static uint8_t attributeType[ELECTTRICAL_INIT_ATTRIBUTES_ARRAY_SIZE] = {
  ZCL_INT32U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_INT16U_ATTRIBUTE_TYPE,
  ZCL_BITMAP16_ATTRIBUTE_TYPE,
  ZCL_INT16S_ATTRIBUTE_TYPE,
  ZCL_INT16S_ATTRIBUTE_TYPE,
  ZCL_INT16S_ATTRIBUTE_TYPE
};
//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeElectricalAttributes(ElectricalData * pElectricalData);
static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emberAfPluginElectricalMeasurementServerReadEventControl;
static uint32_t electricalMeasurementRateMS =
  ELECTRICAL_MAX_MEASUREMENT_INTERVAL_MS;
static ElectricalData electricalReadings;

// The default configuration to be used if no reporting has been set up
#ifdef EMBER_AF_PLUGIN_REPORTING
static EmberAfPluginReportingEntry defaultConfiguration = {
  EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
  0, //endpoint, which will be set on a per-use basis
  ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID, //clusterId
  ZCL_RMS_VOLTAGE_ATTRIBUTE_ID, //attributeId
  CLUSTER_MASK_SERVER, //mask
  EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
  .data.reported = {
    1, //minInterval
    EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S, //maxInterval
    ELECTRICAL_REPORT_CHANGE_VOLTAGE //reportableChange
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
    if ((entry.clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)
        && (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        && (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)
        && (entry.manufacturerCode == EMBER_AF_NULL_MANUFACTURER_CODE)) {
      // Max interval is set in seconds, which is the same unit of time the
      // emberAfPluginElectricalMeasurementServerSetMeasurementInterval
      // expects in this API.
      emberAfPluginElectricalMeasurementServerSetMeasurementInterval(
        entry.data.reported.maxInterval);
      existingEntry = true;
    }
  }

  // If no entry is found for the electrical measurement server, a default
  // reporting configuration should be created using the plugin defined options.
  // This needs to be done for all endpoints that support a electrical
  // measurement server.
  if (!existingEntry) {
    for (i = 0; i < emberAfEndpointCount(); i++) {
      endpoint = emberAfEndpointFromIndex(i);
      defaultConfiguration.endpoint = endpoint;
      if (emberAfContainsServer(endpoint, ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)) {
        defaultConfiguration.attributeId = ZCL_RMS_VOLTAGE_ATTRIBUTE_ID;
        defaultConfiguration.data.reported.reportableChange =
          ELECTRICAL_REPORT_CHANGE_VOLTAGE;
        emAfPluginReportingAddEntry(&defaultConfiguration);
        defaultConfiguration.attributeId = ZCL_RMS_CURRENT_ATTRIBUTE_ID;
        defaultConfiguration.data.reported.reportableChange =
          ELECTRICAL_REPORT_CHANGE_CURRENT;
        emAfPluginReportingAddEntry(&defaultConfiguration);
        defaultConfiguration.attributeId = ZCL_ACTIVE_POWER_ATTRIBUTE_ID;
        defaultConfiguration.data.reported.reportableChange =
          ELECTRICAL_REPORT_CHANGE_POWER;
        emAfPluginReportingAddEntry(&defaultConfiguration);
      }
    }
  }
#endif
}

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfPluginElectricalMeasurementServerInitCallback(void)
{
  checkForReportingConfig();
  // Start the ReadEvent, which will
  // re-activate itself perpetually
  emberEventControlSetActive(
    emberAfPluginElectricalMeasurementServerReadEventControl);
}

void emberAfPluginElectricalMeasurementServerStackStatusCallback(
  EmberStatus status)
{
  uint8_t endpointIndex, endpoint, attributeIndex;
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the electrical for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    checkForReportingConfig();
    // Cycle through all endpoints, check to see if the endpoint has an
    // electrical server, and if so update the electrical attributes of
    // that endpoint
    for (endpointIndex = 0; endpointIndex < emberAfEndpointCount(); endpointIndex++) {
      endpoint = emberAfEndpointFromIndex(endpointIndex);
      if (emberAfContainsServer(endpoint,
                                ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)) {
        for (attributeIndex = 0;
             attributeIndex < ELECTTRICAL_INIT_ATTRIBUTES_ARRAY_SIZE;
             attributeIndex++) {
          emberAfWriteServerAttribute(endpoint,
                                      ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID,
                                      attributeValuesIds[attributeIndex],
                                      (uint8_t *)&attributeValues[attributeIndex],
                                      attributeType[attributeIndex]);
        }
      }
    }
    emberEventControlSetActive(
      emberAfPluginElectricalMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event handler to read new electrical values and write attributes
//******************************************************************************
void emberAfPluginElectricalMeasurementServerReadEventHandler(void)
{
  electricalReadings.activePower =
    (halGetActivePowerMilliW()
     * ELECTRICAL_POWER_DIVISOR)
    / (ELECTRICAL_MILLIUNIT_IN_UNIT
       * ELECTRICAL_POWER_MULTIPLIER);

  electricalReadings.rmsCurrent =
    (halGetCrmsMilliA()
     * ELECTRICAL_CURRENT_DIVISOR)
    / (ELECTRICAL_MILLIUNIT_IN_UNIT
       * ELECTRICAL_CURRENT_MULTIPLIER);

  electricalReadings.rmsVoltage =
    (halGetVrmsMilliV()
     * ELECTRICAL_VOLTAGE_DIVISOR)
    / (ELECTRICAL_MILLIUNIT_IN_UNIT
       * ELECTRICAL_VOLTAGE_MULTIPLIER);
  electricalReadings.powerFactor = halGetPowerFactor();
  writeElectricalAttributes(&electricalReadings);
  emberEventControlSetDelayMS(
    emberAfPluginElectricalMeasurementServerReadEventControl,
    electricalMeasurementRateMS);
}

//------------------------------------------------------------------------------
// Plugin public functions

void emberAfPluginElectricalMeasurementServerSetMeasurementInterval(
  uint32_t measurementRateS)
{
  if ((measurementRateS == 0)
      || (measurementRateS
          > EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    electricalMeasurementRateMS = ELECTRICAL_MAX_MEASUREMENT_INTERVAL_MS;
  } else {
    electricalMeasurementRateMS = measurementRateS
                                  * MILLISECOND_TICKS_PER_SECOND;
  }
  emberEventControlSetDelayMS(
    emberAfPluginElectricalMeasurementServerReadEventControl,
    electricalMeasurementRateMS);
}

//------------------------------------------------------------------------------
// Plugin private functions

//******************************************************************************
// Update the electrical attribute of the electrical measurement cluster to
// be the electrical value given by the function's parameasurement. This
// function will also query the current max and min read values, and update
// them if the given values is higher (or lower) than the previous records.
//******************************************************************************
static void writeElectricalAttributes(ElectricalData* pElectricalData)
{
  uint8_t i;
  uint8_t endpoint;

  // Cycle through all endpoints, check to see if the endpoint has a electrical
  // server, and if so update the electrical attributes of that endpoint
  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint,
                              ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)) {
      emberAfElectricalMeasurementClusterPrintln(
        "Electrical Measurement: Vrms:%d.%1dV, Arms:%dmA",
        pElectricalData->rmsVoltage / 10,
        pElectricalData->rmsVoltage % 10,
        pElectricalData->rmsCurrent
        );
      emberAfElectricalMeasurementClusterPrintln(
        "Electrical Measurement: Active Power:%d.%1dW, Power Factor:%d",
        pElectricalData->activePower / 10,
        pElectricalData->activePower % 10,
        pElectricalData->powerFactor
        );
      // Write the current electrical attribute
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID,
                                  ZCL_RMS_VOLTAGE_ATTRIBUTE_ID,
                                  (uint8_t *) &pElectricalData->rmsVoltage,
                                  ZCL_INT16U_ATTRIBUTE_TYPE);
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID,
                                  ZCL_RMS_CURRENT_ATTRIBUTE_ID,
                                  (uint8_t *) &pElectricalData->rmsCurrent,
                                  ZCL_INT16U_ATTRIBUTE_TYPE);
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID,
                                  ZCL_ACTIVE_POWER_ATTRIBUTE_ID,
                                  (uint8_t *) &pElectricalData->activePower,
                                  ZCL_INT16S_ATTRIBUTE_TYPE);
    }
  }
}
