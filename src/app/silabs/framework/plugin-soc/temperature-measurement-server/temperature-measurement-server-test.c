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
 * @brief Test code for the Temperature Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/test/test-framework.h"
#include "temperature-measurement-server.h"
#include "temperature-measurement-server-test.h"

//------------------------------------------------------------------------------
// Global variables
extern EmberEventControl emberAfPluginTemperatureMeasurementServerReadEventControl;

//------------------------------------------------------------------------------
// protoypes of event handlers and callbacks implemented by unit tested file
// that we'll need to directly call
extern void emberAfPluginTemperatureMeasurementServerInitCallback(void);
extern void emberAfPluginTemperatureMeasurementServerReadEventHandler(void);
extern void halTemperatureReadingCompleteCallback(int32_t temperature,
                                                  bool readSuccess);

int32_t fakeTemperatureReadingMilliC;
bool attributeWritten;
bool needCallback = false;

//******************************************************************************
// Plugin init function
//******************************************************************************

void halTemperatureStartRead(void)
{
  needCallback = true;
}

uint8_t emberAfEndpointFromIndex(uint8_t index)
{
  return 1;
}

EmberAfStatus emberAfWriteServerAttribute(uint8_t endpoint,
                                          EmberAfClusterId cluster,
                                          EmberAfAttributeId attributeID,
                                          uint8_t *dataPtr,
                                          uint8_t dataType)
{
  expectComparisonDecimal(
    *(int16_t *)dataPtr,
    fakeTemperatureReadingMilliC / 10,
    "Written Attribute",
    "Expected Attribute");
  attributeWritten = true;
  return 0;
}

EmberAfStatus emberAfReadServerAttribute(uint8_t endpoint,
                                         EmberAfClusterId cluster,
                                         EmberAfAttributeId attributeID,
                                         uint8_t *dataPtr,
                                         uint8_t readLength)
{
  return 0;
}

uint8_t emberAfEndpointCount(void)
{
  return 1;
}

bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
  if ((endpoint == 1) && (clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)) {
    return true;
  }
  return false;
}

static void reportTempTest(void)
{
  // First test a normal looking value, 28 degrees celsius
  fakeTemperatureReadingMilliC = 28000;

  // The read event handler should kick off the call to
  // emberAfWriteServerAttribute, where the comparison based on fake read data
  // was set
  attributeWritten = false;
  needCallback = false;
  emberAfPluginTemperatureMeasurementServerReadEventHandler();
  if (needCallback) {
    halTemperatureReadingCompleteCallback(
      fakeTemperatureReadingMilliC, true);
  }
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // Big negative number
  fakeTemperatureReadingMilliC = -30000;
  attributeWritten = false;
  needCallback = false;
  emberAfPluginTemperatureMeasurementServerReadEventHandler();
  if (needCallback) {
    halTemperatureReadingCompleteCallback(
      fakeTemperatureReadingMilliC, true);
  }
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // Bit positive number
  fakeTemperatureReadingMilliC = 30000;
  attributeWritten = false;
  needCallback = false;
  emberAfPluginTemperatureMeasurementServerReadEventHandler();
  if (needCallback) {
    halTemperatureReadingCompleteCallback(
      fakeTemperatureReadingMilliC, true);
  }
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");
}

static void initIntervalTest(void)
{
  emberAfPluginTemperatureMeasurementServerInitCallback();

  expectComparisonDecimal(
    emberAfPluginTemperatureMeasurementServerReadEventControl.status,
    4,
    "read event status",
    "not 0 (inactive)");
  needCallback = false;
  emberAfPluginTemperatureMeasurementServerReadEventHandler();
  if (needCallback) {
    halTemperatureReadingCompleteCallback(
      fakeTemperatureReadingMilliC, true);
  }

  expectComparisonDecimal(
    emberAfPluginTemperatureMeasurementServerReadEventControl.timeToExecute,
    EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    * MILLISECOND_TICKS_PER_SECOND,
    "next read event",
    "30720");
}

static void bigIntervalTest(void)
{
  emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(
    EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    + 20);
  expectComparisonDecimal(
    emberAfPluginTemperatureMeasurementServerReadEventControl.timeToExecute,
    EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    * MILLISECOND_TICKS_PER_SECOND,
    "next read event",
    "30720");
}

static void smallIntervalTest(void)
{
  emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(1);
  expectComparisonDecimal(
    emberAfPluginTemperatureMeasurementServerReadEventControl.timeToExecute,
    1 * MILLISECOND_TICKS_PER_SECOND,
    "next read event",
    "1024");
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "init-interval", initIntervalTest },
    { "small-interval", smallIntervalTest },
    { "big-interval", bigIntervalTest },
    { "read-and-write-temp", reportTempTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Security Sensor", tests);
}
