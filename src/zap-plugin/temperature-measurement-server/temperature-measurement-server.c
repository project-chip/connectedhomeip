/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_COMMAND_INTERPRETER2
  #include EMBER_AF_API_COMMAND_INTERPRETER2
#endif
#include EMBER_AF_API_TEMPERATURE

#define MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS                               \
  (EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * 1000)

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeTemperatureAttributes(int32_t temperatureMilliC);
//static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emZclTemperatureMeasurementServerReadEventControl;

static uint32_t temperatureMeasurementIntervalMs =
  MAX_TEMPERATURE_MEASUREMENT_INTERVAL_MS;

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emZclTemperatureMeasurementServerInitHandler(void)
{
  int16_t maxTempAttDeciC;
  int16_t minTempAttDeciC;
  int32_t maxTempMilliC;
  int32_t minTempMilliC;

  maxTempMilliC = halTemperatureGetMaxMeasureableTemperatureMilliC();
  minTempMilliC = halTemperatureGetMinMeasureableTemperatureMilliC();
  maxTempAttDeciC = maxTempMilliC / 10;
  minTempAttDeciC = minTempMilliC / 10;

  emberZclWriteAttribute(1,
                         &emberZclClusterTempMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MAX_MEASURED_VALUE,
                         (uint8_t *) &maxTempAttDeciC,
                         sizeof(maxTempAttDeciC));

  emberZclWriteAttribute(1,
                         &emberZclClusterTempMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MIN_MEASURED_VALUE,
                         (uint8_t *) &minTempAttDeciC,
                         sizeof(minTempAttDeciC));
  // Start the ReadEvent, which will re-activate itself perpetually
  emberEventControlSetActive(
    emZclTemperatureMeasurementServerReadEventControl);
}

void emZclTemperatureMeasurementServerNetworkStatusHandler(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    //checkForReportingConfig();
    emberEventControlSetActive(
      emZclTemperatureMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new temperature value
//******************************************************************************
void emZclTemperatureMeasurementServerReadEventHandler(void)
{
  halTemperatureStartRead();
  emberEventControlSetInactive(
    emZclTemperatureMeasurementServerReadEventControl);
}

void halTemperatureReadingCompleteCallback(int32_t temperatureMilliC,
                                           bool readSuccess)
{
  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    emberAfAppPrintln("Temperature: %d milliC", temperatureMilliC);
    writeTemperatureAttributes(temperatureMilliC);
  } else {
    emberAfAppPrintln("Error reading temperature from HW");
  }

  emberEventControlSetDelayMS(
    emZclTemperatureMeasurementServerReadEventControl,
    temperatureMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin CLI functions
void temperatureSetCommand(void)
{
  int32_t temperatureMilliC;

  if (emberCommandArgumentCount() != 1) {
    emberAfAppPrintln("ERR: expected %s <temperature>",
                      emberStringCommandArgument(-1, NULL));
    return;
  }
  temperatureMilliC = emberSignedCommandArgument(0);

  emberAfAppPrintln("setting temperature to: %d", temperatureMilliC);
  halTemperatureReadingCompleteCallback(temperatureMilliC, true);
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
      = measurementIntervalS * 1000;
  }
  emberEventControlSetDelayMS(
    emZclTemperatureMeasurementServerReadEventControl,
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
  emberZclWriteAttribute(endpoint,
                         &emberZclClusterTempMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_TEMP_MEASUREMENT_SERVER_ATTRIBUTE_TEMP_MEASURED_VALUE,
                         (uint8_t *) &temperatureCentiC,
                         sizeof(temperatureCentiC));
}
