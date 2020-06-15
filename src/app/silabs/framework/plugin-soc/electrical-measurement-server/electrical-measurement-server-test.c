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
 * @brief Test code for the Electrical Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/test/test-framework.h"
#include "electrical-measurement-server.h"
#include "electrical-measurement-server-test.h"

//------------------------------------------------------------------------------
// Global variables
extern EmberEventControl emberAfPluginElectricalMeasurementServerReadEventControl;

//------------------------------------------------------------------------------
// protoypes of event handlers and callbacks implemented by unit tested file
// that we'll need to directly call
extern void emberAfPluginElectricalMeasurementServerInitCallback(void);
extern void emberAfPluginElectricalMeasurementServerReadEventHandler(void);

static uint32_t fakeVoltageReadingMilliV;
static uint32_t fakeCurrentReadingMilliA;
static uint32_t fakePowerReadingMilliW;
bool attributeWritten;

//******************************************************************************
// Plugin init function
//******************************************************************************

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
  if (attributeID == ZCL_RMS_VOLTAGE_ATTRIBUTE_ID) {
    expectComparisonDecimal(
      *(int16_t *)dataPtr,
      fakeVoltageReadingMilliV / 100,
      "Written Attribute",
      "Expected Attribute");
    attributeWritten = true;
  } else if (attributeID == ZCL_RMS_CURRENT_ATTRIBUTE_ID) {
    expectComparisonDecimal(
      *(int16_t *)dataPtr,
      fakeCurrentReadingMilliA,
      "Written Attribute",
      "Expected Attribute");
    attributeWritten = true;
  } else if (attributeID == ZCL_ACTIVE_POWER_ATTRIBUTE_ID) {
    expectComparisonDecimal(
      *(uint16_t *)dataPtr,
      fakePowerReadingMilliW / 100,
      "Written Attribute",
      "Expected Attribute");
    attributeWritten = true;
  }
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
  if ((endpoint == 1) && (clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)) {
    return true;
  }
  return false;
}

// Emulated functions from HAL plugins
void halPowerMeterInit(void)
{
}

uint32_t halGetVrmsMilliV(void)
{
  return fakeVoltageReadingMilliV;
}

uint32_t halGetCrmsMilliA(void)
{
  return fakeCurrentReadingMilliA;
}

uint32_t halGetApparentPowerMilliW(void)
{
  return 0;
}

int32_t halGetActivePowerMilliW(void)
{
  return fakePowerReadingMilliW;
}

int8_t halGetPowerFactor(void)
{
  return 0;
}

void halPowerMeterCalibrateAll(int16_t temperature)
{
}

int16_t halGetPowerMeterTempCentiC(void)
{
  return 0;
}

uint8_t halGetPowerMeterStatus(void)
{
  return 0;
}

static void reportPowerTest(void)
{
  // First test a normal looking value, 200W
  fakePowerReadingMilliW = 200000;
  // The read event handler should kick off the call to
  // emberAfWriteServerAttribute, where the comparison based on fake read data
  // was set
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // Big number
  fakePowerReadingMilliW = 4000000;//4000W
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // small number
  fakePowerReadingMilliW = 1;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");
}

static void reportCurrentTest(void)
{
  // First test a normal looking value, 2A
  fakeCurrentReadingMilliA = 2000;
  // The read event handler should kick off the call to
  // emberAfWriteServerAttribute, where the comparison based on fake read data
  // was set
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // Big number
  fakeCurrentReadingMilliA = 14900;//14.9A
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // small number
  fakeCurrentReadingMilliA = 1;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");
}

static void reportVoltageTest(void)
{
  // First test a normal looking value, 110V
  fakeVoltageReadingMilliV = 110000;
  // The read event handler should kick off the call to
  // emberAfWriteServerAttribute, where the comparison based on fake read data
  // was set
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // Big number
  fakeVoltageReadingMilliV = 260000;
  attributeWritten = false;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");

  // small number
  fakeVoltageReadingMilliV = 80000;
  emberAfPluginElectricalMeasurementServerReadEventHandler();
  expectComparisonDecimal(
    attributeWritten,
    true,
    "Attribute written state variable",
    "true");
}

static void initIntervalTest(void)
{
  emberAfPluginElectricalMeasurementServerInitCallback();

  expectComparisonDecimal(
    emberAfPluginElectricalMeasurementServerReadEventControl.status,
    4,
    "read event status",
    "not 0 (inactive)");
  emberAfPluginElectricalMeasurementServerReadEventHandler();

  expectComparisonDecimal(
    emberAfPluginElectricalMeasurementServerReadEventControl.timeToExecute,
    EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    * MILLISECOND_TICKS_PER_SECOND,
    "next read event",
    "30720");
}

static void bigIntervalTest(void)
{
  emberAfPluginElectricalMeasurementServerSetMeasurementInterval(
    EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    + 20);
  expectComparisonDecimal(
    emberAfPluginElectricalMeasurementServerReadEventControl.timeToExecute,
    EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S
    * MILLISECOND_TICKS_PER_SECOND,
    "next read event",
    "30720");
}

static void smallIntervalTest(void)
{
  emberAfPluginElectricalMeasurementServerSetMeasurementInterval(1);
  expectComparisonDecimal(
    emberAfPluginElectricalMeasurementServerReadEventControl.timeToExecute,
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
    { "read-and-write-voltage", reportVoltageTest },
    { "read-and-write-current", reportCurrentTest },
    { "read-and-write-power", reportPowerTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Security Sensor", tests);
}
