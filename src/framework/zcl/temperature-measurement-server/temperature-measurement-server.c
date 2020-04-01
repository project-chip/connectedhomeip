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
#include CHIP_AF_API_TEMPERATURE

#define MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS                               \
  (CHIP_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * 1000)

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeTemperatureAttributes(int32_t temperatureMilliC);
//static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
ChipEventControl chZclTemperatureMeasurementServerReadEventControl;

static uint32_t temperatureMeasurementIntervalMs =
  MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS;

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void chZclTemperatureMeasurementServerInitHandler(void)
{
  int16_t maxTempAttDeciC;
  int16_t minTempAttDeciC;
  int32_t maxTempMilliC;
  int32_t minTempMilliC;

  maxTempMilliC = halTemperatureGetMaxMeasureableTemperatureMilliC();
  minTempMilliC = halTemperatureGetMinMeasureableTemperatureMilliC();
  maxTempAttDeciC = maxTempMilliC / 10;
  minTempAttDeciC = minTempMilliC / 10;

  chipZclWriteAttribute(1,
                         &chipZclClusterTempMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MAX_MEASURED_VALUE,
                         (uint8_t *) &maxTempAttDeciC,
                         sizeof(maxTempAttDeciC));

  chipZclWriteAttribute(1,
                         &chipZclClusterTempMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MIN_MEASURED_VALUE,
                         (uint8_t *) &minTempAttDeciC,
                         sizeof(minTempAttDeciC));
  // Start the ReadEvent, which will re-activate itself perpetually
  chipEventControlSetActive(
    chZclTemperatureMeasurementServerReadEventControl);
}

void chZclTemperatureMeasurementServerNetworkStatusHandler(
  ChipStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == CHIP_NETWORK_UP) {
    //checkForReportingConfig();
    chipEventControlSetActive(
      chZclTemperatureMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new temperature value
//******************************************************************************
void chZclTemperatureMeasurementServerReadEventHandler(void)
{
  halTemperatureStartRead();
  chipEventControlSetInactive(
    chZclTemperatureMeasurementServerReadEventControl);
}

void halTemperatureReadingCompleteCallback(int32_t temperatureMilliC,
                                           bool readSuccess)
{
  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    chipAfAppPrintln("Temperature: %d milliC", temperatureMilliC);
    writeTemperatureAttributes(temperatureMilliC);
  } else {
    chipAfAppPrintln("Error reading temperature from HW");
  }

  chipEventControlSetDelayMS(
    chZclTemperatureMeasurementServerReadEventControl,
    temperatureMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin CLI functions
void temperatureSetCommand(void)
{
  int32_t temperatureMilliC;

  if (chipCommandArgumentCount() != 1) {
    chipAfAppPrintln("ERR: expected %s <temperature>",
                      chipStringCommandArgument(-1, NULL));
    return;
  }
  temperatureMilliC = chipSignedCommandArgument(0);

  chipAfAppPrintln("setting temperature to: %d", temperatureMilliC);
  halTemperatureReadingCompleteCallback(temperatureMilliC, true);
}

//------------------------------------------------------------------------------
// Plugin public functions

void chipAfPluginTemperatureMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > CHIP_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    temperatureMeasurementIntervalMs = MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS;
  } else {
    temperatureMeasurementIntervalMs
      = measurementIntervalS * 1000;
  }
  chipEventControlSetDelayMS(
    chZclTemperatureMeasurementServerReadEventControl,
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
  int16_t temperatureCentiC;

  //uint8_t i;
  uint8_t endpoint = 1;

  temperatureCentiC = temperatureMilliC / 10;

  //TODO: add multi-endpoint support here, once multiple endpoint are support in
  // zoip

  // Write the current temperature attribute
  chipZclWriteAttribute(endpoint,
                         &chipZclClusterTempMeasurementServerSpec,
                         CHIP_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MEASURED_VALUE,
                         (uint8_t *) &temperatureCentiC,
                         sizeof(temperatureCentiC));
}
