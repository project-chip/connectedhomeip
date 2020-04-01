/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_COMMAND_INTERPRETER2
  #include CHIP_AF_API_COMMAND_INTERPRETER2
#endif
#include CHIP_AF_API_HUMIDITY

#define MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS                                        \
  (CHIP_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * 1000)

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeHumidityAttributes(uint16_t humidityPercentage);
//static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
ChipEventControl chZclRelativeHumidityMeasurementServerReadEventControl;

static uint32_t humidityMeasurementIntervalMs =
  MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS;

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void chZclRelativeHumidityeMeasurementServerInitHandler(void)
{
  uint16_t maxHumidityDeciPercent;
  uint16_t minHumidityDeciPercent;

  maxHumidityDeciPercent = halHumidityGetMaxMeasureableHumidityDeciPercent();
  minHumidityDeciPercent = halHumidityGetMinMeasureableHumidityDeciPercent();

  chipZclWriteAttribute(1,
                         &chipZclClusterRelativeHumidityMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE,
                         (uint8_t *) &maxHumidityDeciPercent,
                         sizeof(maxHumidityDeciPercent));

  chipZclWriteAttribute(1,
                         &chipZclClusterRelativeHumidityMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE,
                         (uint8_t *) &minHumidityDeciPercent,
                         sizeof(minHumidityDeciPercent));
  // Start the ReadEvent, which will re-activate itself perpetually
  chipEventControlSetActive(
    chZclRelativeHumidityMeasurementServerReadEventControl);
}

void chZclRelativeHumidityMeasurementServerNetworkStatusHandler(
  ChipStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == CHIP_NETWORK_UP) {
    //checkForReportingConfig();
    chipEventControlSetActive(
      chZclRelativeHumidityMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new temperature value
//******************************************************************************
void chZclRelativeHumidityMeasurementServerReadEventHandler(void)
{
  halHumidityStartRead();
  chipEventControlSetInactive(
    chZclRelativeHumidityMeasurementServerReadEventControl);
}

void halHumidityReadingCompleteCallback(uint16_t humidityDeciPercent,
                                        bool readSuccess)
{
  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    chipAfAppPrintln("Humidity: %d deciPercent", humidityDeciPercent);
    writeHumidityAttributes(humidityDeciPercent);
  } else {
    chipAfAppPrintln("Error reading humidity from HW");
  }

  chipEventControlSetDelayMS(
    chZclRelativeHumidityMeasurementServerReadEventControl,
    humidityMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin CLI functions
void humiditySetCommand(void)
{
  uint16_t humidityDeciPercent;

  if (chipCommandArgumentCount() != 1) {
    chipAfAppPrintln("ERR: expected %s <humidity>",
                      chipStringCommandArgument(-1, NULL));
    return;
  }
  humidityDeciPercent = (uint16_t)chipUnsignedCommandArgument(0);

  chipAfAppPrintln("setting humidity to: %d", humidityDeciPercent);
  halHumidityReadingCompleteCallback(humidityDeciPercent, true);
}

//------------------------------------------------------------------------------
// Plugin public functions

void chipAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > CHIP_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    humidityMeasurementIntervalMs = MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS;
  } else {
    humidityMeasurementIntervalMs
      = measurementIntervalS * 1000;
  }
  chipEventControlSetDelayMS(
    chZclRelativeHumidityMeasurementServerReadEventControl,
    humidityMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin private functions

//******************************************************************************
// Update the temperature attribute of the temperature measurement cluster to
// be the temperature value given by the function's parameter.  This function
// will also query the current max and min read values, and update them if the
// given values is higher (or lower) than the previous records.
//******************************************************************************
static void writeHumidityAttributes(uint16_t humidityDeciPercent)
{
  //ChipZclEndpointId_t i;
  ChipZclEndpointId_t endpoint = 1;

  //TODO: add multi-endpoint support here, once multiple endpoint are support in
  // zoip

  // Write the current temperature attribute
  chipZclWriteAttribute(endpoint,
                         &chipZclClusterRelativeHumidityMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MEASURED_VALUE,
                         (uint8_t *) &humidityDeciPercent,
                         sizeof(humidityDeciPercent));
}
