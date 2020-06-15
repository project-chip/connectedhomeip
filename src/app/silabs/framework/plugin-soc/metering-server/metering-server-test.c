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
 * @brief Test code for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/test/test-framework.h"
#include "metering-server.h"
#include "metering-interface.h"
#include "metering-server-test.h"

#define INIT_POWER_PARAMETER_TEST       0
#define REPORT_SUMMATION_DELIVERED_TEST 1
//------------------------------------------------------------------------------
// Global variables
extern EmberEventControl emberAfPluginElectricalMeasurementServerReadEventControl;

//------------------------------------------------------------------------------
// protoypes of event handlers and callbacks implemented by unit tested file
// that we'll need to directly call
//extern void emberAfPluginElectricalMeasurementServerInitCallback(void);
extern void emberAfSimpleMeteringClusterServerInitCallback(uint8_t endpoint);
//extern void emberAfPluginElectricalMeasurementServerReadEventHandler(void);
extern void emberAfPluginMeteringServerSamplingEventHandler(void);
extern void emberAfPluginMeteringServerSupplyEventHandler(void);

EmberAfStatus attributeWriteCallback(AttributeCallbackData * data);
static uint32_t fakeVoltageReadingMilliV;
static uint32_t fakeCurrentReadingMilliA;
static uint32_t fakePowerReadingMilliW;
static uint8_t  fakePowerMeterStatus;
static uint8_t  fakePowerFactor;
bool attributeWritten0;
bool attributeWritten1;
bool attributeWritten2;
bool attributeWritten3;
bool attributeWritten4;
bool attributeWritten5;
static uint8_t testItems;
uint8_t summation_start[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };//
uint8_t summation_stop[] = { 0x76, 0x34, 0x56, 0x78, 0x9a, 0xbc };//

extern AttributeCallbackFunction * attributeCallbackFunction;

EmberApsFrame apsFrame = {
  .profileId           = 0,
  .clusterId           = 0,
  .sourceEndpoint      = 0,
  .destinationEndpoint = 0,
};
EmberAfClusterCommand clusterCommand = {
  .apsFrame        = &apsFrame,
  .type            = 0,
  .source          = 0,
  .clusterSpecific = true,
  .commandId       = 0,
  .direction       = 0,
};

EmberAfClusterCommand * emAfCurrentCommand = &clusterCommand;

bool emberAfIsThisDataTypeAStringType(EmberAfAttributeType dataType)
{
  return (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType)
{
  return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

void emberAfPluginMeteringServerProcessNotificationFlagsCallback(
  uint16_t attributeId,
  uint32_t attributeValue)
{
}

uint8_t emberAfFindClusterServerEndpointIndex(uint8_t endpoint, EmberAfClusterId clusterId)
{
  return 0;
}

bool emberAfPluginMeteringServerContainsAttribute(uint8_t endpoint, EmberAfAttributeId attribute)
{
  return true;
}

void emberAfCopyInt24u(uint8_t *data, uint16_t index, uint32_t x)
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFF);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFF);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFF);
}

uint8_t emberAfEndpointFromIndex(uint8_t index)
{
  return 1;
}

EmberAfStatus attributeWriteCallback(AttributeCallbackData * data)
{
  uint32_t multiplier, divisor;
  uint8_t index;
  if (data->read) {
    if (data->attributeId == ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID) {
      for (index = 0; index < 6; index++) {
        data->dataPtr[index] = summation_start[index];
      }
    } else if (data->attributeId == ZCL_STATUS_ATTRIBUTE_ID) {
      *(uint8_t *)(data->dataPtr) = fakePowerMeterStatus;
    }
    return 0;
  } else if (testItems == INIT_POWER_PARAMETER_TEST) {
    switch (data->attributeId ) {
      case ZCL_STATUS_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          TEST_METERING_SERVER_METER_STATUS_ERROR_NONE,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten0 = true;
        break;
      case ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          EMBER_ZCL_METERING_DEVICE_TYPE_ELECTRIC_METERING,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten1 = true;
        break;
      case ZCL_UNIT_OF_MEASURE_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          EMBER_ZCL_AMI_UNIT_OF_MEASURE_KILO_WATT_HOURS,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten2 = true;
        break;
      case ZCL_SUMMATION_FORMATTING_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          TEST_METERING_SUMMATION_FORMAT,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten3 = true;
        break;
      case ZCL_MULTIPLIER_ATTRIBUTE_ID:
        multiplier = 0;
        multiplier = *(uint8_t *)(data->dataPtr++);
        multiplier = multiplier + ((*(uint8_t *)(data->dataPtr++)) << 8);
        multiplier = multiplier + ((*(uint8_t *)(data->dataPtr)) << 16);
        expectComparisonDecimal(
          multiplier,
          TEST_METERING_MULTIPLIER,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten4 = true;
        break;
      case ZCL_DIVISOR_ATTRIBUTE_ID:
        divisor = 0;
        divisor = *(uint8_t *)(data->dataPtr++);
        divisor = divisor + ((*(uint8_t *)(data->dataPtr++)) << 8);
        divisor = divisor + ((*(uint8_t *)(data->dataPtr)) << 16);
        expectComparisonDecimal(
          divisor,
          TEST_METERING_DIVISOR,
          "Written Attribute",
          "Expected Attribute");
        attributeWritten5 = true;
        break;
      default:
        break;
    }
  } else if (testItems == REPORT_SUMMATION_DELIVERED_TEST) {
    switch (data->attributeId ) {
      case ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          summation_stop[0],
          "Written summation 0",
          "Expected Attribute");
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr + 1),
          summation_stop[1],
          "Written summation 1",
          "Expected Attribute");
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr + 2),
          summation_stop[2],
          "Written summation 2",
          "Expected Attribute");
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr + 3),
          summation_stop[3],
          "Written summation 3",
          "Expected Attribute");
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr + 4),
          summation_stop[4],
          "Written summation 4",
          "Expected Attribute");
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr + 5),
          summation_stop[5],
          "Written summation 5",
          "Expected Attribute");
        attributeWritten0 = true;
        break;
      case ZCL_POWER_FACTOR_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          fakePowerFactor,
          "Written Power Factor",
          "Expected Attribute");
        attributeWritten1 = true;
        break;
      case ZCL_STATUS_ATTRIBUTE_ID:
        expectComparisonDecimal(
          *(uint8_t *)(data->dataPtr),
          TEST_METERING_SERVER_METER_STATUS_ERROR_NONE,
          "Written Status",
          "Expected Attribute");
        attributeWritten2 = true;
        break;
      default:
        break;
    }
    return 0;
  }
  return 123;
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

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
  return EMBER_SUCCESS;
}

bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
  if ((endpoint == 1) && (clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)) {
    return true;
  }
  return false;
}

uint32_t emberAfGetCurrentTime(void)
{
  return 0;
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
  return fakePowerFactor;
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
  return fakePowerMeterStatus;
}

static void InitPowerParameterTest(void)
{
  attributeCallbackFunction = attributeWriteCallback;
  testItems = INIT_POWER_PARAMETER_TEST;
  // Normal condition, no failure reported.
  fakePowerMeterStatus = TEST_METERING_SERVER_METER_STATUS_ERROR_NONE;
  // The read event handler should kick off the call to
  // emberAfWriteAttribute, where the comparison based on fake read data
  // was set
  //we have 6 attributes to be written during initilization
  attributeWritten0 = false;
  attributeWritten1 = false;
  attributeWritten2 = false;
  attributeWritten3 = false;
  attributeWritten4 = false;
  attributeWritten5 = false;
  emberAfSimpleMeteringClusterServerInitCallback(0);
  expectComparisonDecimal(
    attributeWritten0,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten1,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten2,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten3,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten4,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten5,
    true,
    "Attribute written state variable",
    "true");
}
static void reportSummationDeliveredTest(void)
{
  attributeCallbackFunction = attributeWriteCallback;
  testItems = REPORT_SUMMATION_DELIVERED_TEST;
  // Normal condition, assume no failure reported.
  fakePowerMeterStatus = TEST_METERING_SERVER_METER_STATUS_ERROR_NONE;
  fakePowerReadingMilliW = 100 * 1000; //we assume 100W load.
  fakePowerFactor = 100; //assume a 100% power factor.
  //we have 3 attribute to be written here
  attributeWritten0 = false;
  attributeWritten1 = false;
  attributeWritten2 = false;

  emberAfSimpleMeteringClusterServerTickCallback(0);
  expectComparisonDecimal(
    attributeWritten0,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten1,
    true,
    "Attribute written state variable",
    "true");
  expectComparisonDecimal(
    attributeWritten2,
    true,
    "Attribute written state variable",
    "true");
}
int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "init-power-parameter-test", InitPowerParameterTest },
    { "report-summation-delivered-test", reportSummationDeliveredTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Security Sensor", tests);
}
