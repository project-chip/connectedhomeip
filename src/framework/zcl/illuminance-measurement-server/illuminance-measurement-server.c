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
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_COMMAND_INTERPRETER2
  #include EMBER_AF_API_COMMAND_INTERPRETER2
#endif
#include EMBER_AF_API_ILLUMINANCE

#define MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS                               \
  (EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S \
   * 1000)

#define ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MAX   200
#define ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MIN   2

//------------------------------------------------------------------------------
// Forward Declaration of private functions
static void writeIlluminanceAttributes(uint16_t illuminanceLogLx);
//static void checkForReportingConfig(void);

//------------------------------------------------------------------------------
// Global variables
EmberEventControl emZclIlluminanceMeasurementServerReadEventControl;

static uint32_t illuminanceMeasurementIntervalMs =
  MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS;

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Plugin init function
//******************************************************************************
void emZclIlluminanceMeasurementServerInitHandler(void)
{
  uint16_t maxIlluminanceLogLux;
  uint16_t minIlluminanceLogLux;

  maxIlluminanceLogLux = halIlluminanceGetMaxMeasureableIlluminanceLogLux();
  minIlluminanceLogLux = halIlluminanceGetMinMeasureableIlluminanceLogLux();

  emberZclWriteAttribute(1,
                         &emberZclClusterIllumMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_ILLUM_MEASUREMENT_SERVER_ATTRIBUTE_ILLUM_MAX_MEASURED_VALUE,
                         (uint8_t *) &maxIlluminanceLogLux,
                         sizeof(maxIlluminanceLogLux));

  emberZclWriteAttribute(1,
                         &emberZclClusterIllumMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_ILLUM_MEASUREMENT_SERVER_ATTRIBUTE_ILLUM_MIN_MEASURED_VALUE,
                         (uint8_t *) &minIlluminanceLogLux,
                         sizeof(minIlluminanceLogLux));
  // Start the ReadEvent, which will re-activate itself perpetually
  emberEventControlSetActive(
    emZclIlluminanceMeasurementServerReadEventControl);
}

void emZclIlluminanceMeasurementServerNetworkStatusHandler(
  EmberStatus status)
{
  // On network connect, chances are very good that someone (coordinator,
  // gateway, etc) will poll the illuminance for an initial status.  As such,
  // it is useful to have fresh data to be polled.
  if (status == EMBER_NETWORK_UP) {
    //checkForReportingConfig();
    emberEventControlSetActive(
      emZclIlluminanceMeasurementServerReadEventControl);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Event used to generate a read of a new illuminance value
//******************************************************************************
void emZclIlluminanceMeasurementServerReadEventHandler(void)
{
  uint8_t multiplier;

  //TODO: figure out how to do tokens in AFv6
  //halCommonGetToken(&multiplier, TOKEN_SI1141_MULTIPLIER);
  multiplier = 4;

  // sanity check for mulitplier
  if ((multiplier < ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MIN)
      || (multiplier > ILLUMINANCE_MEASUREMENT_SERVER_MULTIPLIER_MAX)) {
    multiplier = 0; // use default value instead
  }

  emberAfAppPrintln("I can read!! (eventhandler)");
  halIlluminanceStartRead(multiplier);
  emberEventControlSetInactive(
    emZclIlluminanceMeasurementServerReadEventControl);
}

void halIlluminanceReadingCompleteCallback(uint16_t illuminanceLogLx)
{
  emberAfAppPrintln("Illuminance: %d log lux", illuminanceLogLx);
  writeIlluminanceAttributes(illuminanceLogLx);

  emberEventControlSetDelayMS(
    emZclIlluminanceMeasurementServerReadEventControl,
    illuminanceMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin CLI functions
void illuminanceSetCommand(void)
{
  int16_t illuminanceLogLx;

  if (emberCommandArgumentCount() != 1) {
    emberAfAppPrintln("ERR: expected %s <illuminance>",
                      emberStringCommandArgument(-1, NULL));
    return;
  }
  illuminanceLogLx = (uint16_t)emberUnsignedCommandArgument(0);

  emberAfAppPrintln("setting illuminance to: %d", illuminanceLogLx);
  halIlluminanceReadingCompleteCallback(illuminanceLogLx);
}

//------------------------------------------------------------------------------
// Plugin public functions

void emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(
  uint32_t measurementIntervalS)
{
  if ((measurementIntervalS == 0)
      || (measurementIntervalS
          > EMBER_AF_PLUGIN_ILLUMINANCE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S)) {
    illuminanceMeasurementIntervalMs = MAX_ILLUMINANCE_MEASUREMENT_INTERVAL_MS;
  } else {
    illuminanceMeasurementIntervalMs
      = measurementIntervalS * 1000;
  }
  emberEventControlSetDelayMS(
    emZclIlluminanceMeasurementServerReadEventControl,
    illuminanceMeasurementIntervalMs);
}

//------------------------------------------------------------------------------
// Plugin private functions

//******************************************************************************
// Update the illuminance attribute of the illuminance measurement cluster to
// be the illuminance value given by the function's parameter.  This function
// will also query the current max and min read values, and update them if the
// given values is higher (or lower) than the previous records.
//******************************************************************************
static void writeIlluminanceAttributes(uint16_t illuminanceLogLx)
{
  //uint8_t i;
  uint8_t endpoint = 1;

  //TODO: add multi-endpoint support here, once multiple endpoint are support in
  // zoip

  // Write the current illuminance attribute
  emberZclWriteAttribute(endpoint,
                         &emberZclClusterIllumMeasurementServerSpec,
                         EMBER_ZCL_CLUSTER_ILLUM_MEASUREMENT_SERVER_ATTRIBUTE_ILLUM_MEASURED_VALUE,
                         (uint8_t *) &illuminanceLogLx,
                         sizeof(illuminanceLogLx));
}
