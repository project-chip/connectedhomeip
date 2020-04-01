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
#include EMBER_AF_API_HUMIDITY

#define MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS                                        \
  (EMBER_AF_PLUGIN_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * 1000)

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeHumidityAttributes(uint16_t humidityPercentage);
//static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emZclRelativeHumidityMeasurementServerReadEventControl;

static uint32_t humidityMeasurementIntervalMs =
  MAX_HUMIDITY_MEASUREMENT_INTERVAL_MS;

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emZclRelativeHumidityeMeasurementServerInitHandler(void)
{
  uint16_t maxHumidityDeciPercent;
  uint16_t minHumidityDeciPercent;

  maxHumidityDeciPercent = halHumidityGetMaxMeasureableHumidityDeciPercent();
  minHumidityDeciPercent = halHumidityGetMinMeasureableHumidityDeciPercent();

  emberZclWriteAttribute(1,
                         &emberZclClusterRelativeHumidityMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE,
                         (uint8_t *) &maxHumidityDeciPercent,
                         sizeof(maxHumidityDeciPercent));

  emberZclWriteAttribute(1,
                         &emberZclClusterRelativeHumidityMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE,
                         (uint8_t *) &minHumidityDeciPercent,
                         sizeof(minHumidityDeciPercent));
  // Start the ReadEvent, which will re-activate itself perpetually
  emberEventControlSetActive(
    emZclRelativeHumidityMeasurementServerReadEventControl);
}

void emZclRelativeHumidityMeasurementServerNetworkStatusHandler(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the temperature for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    //checkForReportingConfig();
    emberEventControlSetActive(
      emZclRelativeHumidityMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new temperature value
//******************************************************************************
void emZclRelativeHumidityMeasurementServerReadEventHandler(void)
{
  halHumidityStartRead();
  emberEventControlSetInactive(
    emZclRelativeHumidityMeasurementServerReadEventControl);
}

void halHumidityReadingCompleteCallback(uint16_t humidityDeciPercent,
                                        bool readSuccess)
{
  // If the read was successful, post the results to the cluster
  if (readSuccess) {
    emberAfAppPrintln("Humidity: %d deciPercent", humidityDeciPercent);
    writeHumidityAttributes(humidityDeciPercent);
  } else {
    emberAfAppPrintln("Error reading humidity from HW");
  }

  emberEventControlSetDelayMS(
    emZclRelativeHumidityMeasurementServerReadEventControl,
    humidityMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin CLI functions
void humiditySetCommand(void)
{
  uint16_t humidityDeciPercent;

  if (emberCommandArgumentCount() != 1) {
    emberAfAppPrintln("ERR: expected %s <humidity>",
                      emberStringCommandArgument(-1, NULL));
    return;
  }
  humidityDeciPercent = (uint16_t)emberUnsignedCommandArgument(0);

  emberAfAppPrintln("setting humidity to: %d", humidityDeciPercent);
  halHumidityReadingCompleteCallback(humidityDeciPercent, true);
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
      = measurementIntervalS * 1000;
  }
  emberEventControlSetDelayMS(
    emZclRelativeHumidityMeasurementServerReadEventControl,
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
  //EmberZclEndpointId_t i;
  EmberZclEndpointId_t endpoint = 1;

  //TODO: add multi-endpoint support here, once multiple endpoint are support in
  // zoip

  // Write the current temperature attribute
  emberZclWriteAttribute(endpoint,
                         &emberZclClusterRelativeHumidityMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT_SERVER_ATTRIBUTE_RELATIVE_HUMIDITY_MEASURED_VALUE,
                         (uint8_t *) &humidityDeciPercent,
                         sizeof(humidityDeciPercent));
}
