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
 * @brief Test code for the LED Dim PWM plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "app/framework/test/test-framework.h"

#include "app/framework/plugin-soc/led-temp-pwm/led-temp-pwm.h"

//============================================================================
// Globals

// attributes
static int8u onOff = 1;
static int8u expectedLevel = 0xff;
static int16u expectedPwmOutput;

EmberAfClusterCommand *emAfCurrentCommand;

//------------------------------------------------------------------------------
// External declarations and events.
void emberAfPluginLedDimPwmInitCallback(void);

//============================================================================
// Forward Declarations

void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status);

//============================================================================

int8u levelValues[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130,
                        140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240,
                        250, 255 };
int16u pwmValues1[] = { 0, 285, 501, 718, 936, 1152, 1369, 1586, 1802, 2019, 2237, 2454,
                        2670, 2887, 3104, 3320, 3538, 3755, 3971, 4188, 4405, 4621,
                        4838, 5056, 5273, 5489, 6000 };

static void initTest(void)
{
  int i;
  int numValues = sizeof(levelValues) / sizeof(int8u);

  onOff = 1;

  for (i = 0; i < numValues; i++) {
    expectedLevel = levelValues[i];
    expectedPwmOutput = pwmValues1[i];
    emberAfPluginLedDimPwmInitCallback();
  }

  onOff = 0;
  expectedLevel = 0x80;
  expectedPwmOutput = 0;
  emberAfPluginLedDimPwmInitCallback();
}

void emberAfLevelControlClusterServerAttributeChangedCallback(int8u endpoint,
                                                              EmberAfAttributeId attributeId);
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId);
void halBulbPwmDriverBlinkStopCallback(void);
//void emberAfPluginBulbPwmDriverBlinkStopCallback( int8u endpoint );

static void driveTest(void)
{
  onOff = 1;
  expectedLevel = 0xff;
  expectedPwmOutput = 6000;
  emberAfPluginLedDimPwmInitCallback();

  onOff = 1;
  expectedLevel = 20;
  expectedPwmOutput = 501;
  emberAfLevelControlClusterServerAttributeChangedCallback(1, 0x0000);

  expectedLevel = 30;
  expectedPwmOutput = 718;
  emberAfOnOffClusterServerAttributeChangedCallback(1, 0x0000);

  expectedLevel = 40;
  expectedPwmOutput = 936;
  halBulbPwmDriverBlinkStopCallback();
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "init", initTest    },
    { "drive", driveTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc, argv, "Led Temperature PWM", tests);
}

// New DC code
int8u emberAfEndpointCount(void)
{
  return 1;
}

int8u emberAfEndpointFromIndex(int8u index)
{
  assert(index == 0);
  return 1;
}

void halBulbPwmDriverSetPwmLevel(int16u pwmValue, int8u pwmChannel)
{
  assert(pwmChannel == 1);

  expectComparisonDecimal(pwmValue,
                          expectedPwmOutput,
                          "pwmOutput",
                          "expectedPwmOutput");
}

int16u halBulbPwmDriverTicksPerMicrosecond(void)
{
  return 6;
}

int16u halBulbPwmDriverTicksPerPeriod(void)
{
  return 6000;
}

EmberAfStatus emberAfReadServerAttribute(int8u endpoint,
                                         EmberAfClusterId cluster,
                                         EmberAfAttributeId attributeID,
                                         int8u* dataPtr,
                                         int8u readLength)
{
  switch (cluster) {
    case ZCL_ON_OFF_CLUSTER_ID:
      switch (attributeID) {
        case ZCL_ON_OFF_ATTRIBUTE_ID:
          *dataPtr = onOff;
          break;
        default:
          printf("ON OFF attribute %x undefined\r\n", attributeID);
          //assert(0);
      }
      break;
    case ZCL_LEVEL_CONTROL_CLUSTER_ID:
      switch (attributeID) {
        case ZCL_CURRENT_LEVEL_ATTRIBUTE_ID:
          *dataPtr = expectedLevel;
          break;
        default:
          printf("LEVEL attribute %x undefined\r\n", attributeID);
          //assert(0);
      }
      break;
    default:
      printf("Read attributes undefined cluster %x\r\n", cluster);
      //assert(0);
      break;
  }

  return EMBER_SUCCESS;
}

void emberAfPrintln(int16u functionality, const char * formatString, ...)
{
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
  return EMBER_SUCCESS;
}

EmberAfStatus emberAfWriteAttribute(int8u endpoint,
                                    EmberAfClusterId cluster,
                                    EmberAfAttributeId attributeID,
                                    int8u mask,
                                    int8u* dataPtr,
                                    EmberAfAttributeType dataType)
{
  int16u value = (int16u) dataPtr[0];

  printf("write attributes:  %x %x %d\r\n", cluster, attributeID, value);

  return EMBER_SUCCESS;
}

EmberStatus halCommonIdleForMilliseconds(int32u *duration)
{
  return EMBER_SUCCESS;
}

// stale reference code
#if 0
expectComparisonHex(parameters->panId, expectedPanId, "parameters->panId", "expectedPanId");
expectComparisonDecimal(expectedCompleteCallbackStatusCode,
                        status,
                        "expectedCompleteCallbackStatusCode",
                        "status");
expect(duration >= 180);   // bdb spec min commission time

#endif
