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
 * @brief Test code for the LED RGB PWM plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "app/framework/test/test-framework.h"

#include "app/framework/plugin-soc/led-temp-pwm/led-temp-pwm.h"

//============================================================================
// Globals

// attributes
static int16u minColor = 200;
static int16u maxColor = 450;
static int16u colorTemp = 150;
static int8u onOff = 1;
static int8u level = 0xff;
static int16u expectedWhite;
static int16u expectedRed;

static int16u expectedTemp;

static int32u scratch;

//============================================================================
// Forward Declarations

void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status);
void emberAfPluginLedTempPwmInitCallback(void);

//============================================================================

int16u colorTemps[] = { 190, 200, 220, 240, 260, 280, 300, 320, 340, 360, 380,
                        400, 420, 440, 460, 480, 1, 1 };
int16u rValues1[] = { 6000, 6000, 5017, 4198, 3507, 2913, 2399, 1950, 1552, 1198,
                      883, 600, 341, 107, 0, 0, 6000, 6000 };
int16u wValues1[] = { 0, 0, 982, 1801, 2492, 3086, 3600, 4049, 4447, 4801, 5116,
                      5399, 5658, 5892, 6000, 6000, 0, 0 };

static void initTest(void)
{
  int i;
  int numValues = sizeof(colorTemps) / sizeof(int16u);

  onOff = 1;
  level = 0xfe;

  for (i = 0; i < numValues; i++) {
    colorTemp = colorTemps[i];
    expectedWhite = wValues1[i];
    expectedRed = rValues1[i];
    emberAfPluginLedTempPwmInitCallback();
  }

  onOff = 0;
  expectedWhite = 0;
  expectedRed = 0;
  emberAfPluginLedTempPwmInitCallback();

  onOff = 1;
  level = 0x80;
  for (i = 0; i < numValues; i++) {
    colorTemp = colorTemps[i];
    scratch = wValues1[i];
    scratch *= level;
    scratch /= 254;
    expectedWhite = (uint16_t) scratch;

    scratch = rValues1[i];
    scratch *= level;
    scratch /= 254;
    expectedRed = (uint16_t) scratch;
    emberAfPluginLedTempPwmInitCallback();
  }
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "init", initTest    },
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
  switch (pwmChannel) {
    case 1:
      expectComparisonDecimal(pwmValue,
                              expectedRed,
                              "red",
                              "expectedRed");
      break;
    case 2:
      expectComparisonDecimal(pwmValue,
                              expectedWhite,
                              "white",
                              "expectedWhite");
      break;
    case 3:
      expectComparisonDecimal(pwmValue,
                              0,
                              "green",
                              "expectedGreen");
    case 4:
      expectComparisonDecimal(pwmValue,
                              0,
                              "blue",
                              "expectedBlue");
      break;
  }
}

int16u halBulbPwmDriverTicksPerMicrosecond(void)
{
  return 6;
}

int16u halBulbPwmDriverTicksPerPeriod(void)
{
  return 6000;
}

void emberAfPluginBulbPwmDriverDriveWRGB(int16u white, int16u red, int16u green, int16u blue)
{
  //  printf("%d %d %d %d\r\n", white, red, green, blue);

  expectComparisonDecimal(white,
                          expectedWhite,
                          "white",
                          "expectedWhite");
  expectComparisonDecimal(red,
                          expectedRed,
                          "red",
                          "expectedRed");
}

int16u emberAfPluginBulbPwmDriverMinDriveValue(void)
{
  return 0;
}

int16u emberAfPluginBulbPwmDriverMaxDriveValue(void)
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
    case ZCL_COLOR_CONTROL_CLUSTER_ID:
      switch (attributeID) {
        case ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_ATTRIBUTE_ID:
          *dataPtr = (int8u) minColor;
          dataPtr++;
          *dataPtr = (int8u) (minColor >> 8);;
          break;
        case ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_ATTRIBUTE_ID:
          *dataPtr = (int8u) maxColor;
          dataPtr++;
          *dataPtr = (int8u) (maxColor >> 8);;
          break;
        case ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID:
          *dataPtr = (int8u) colorTemp;
          dataPtr++;
          *dataPtr = (int8u) (colorTemp >> 8);;
          break;
        default:
          printf("Color Control attribute %x undefined\r\n", attributeID);
          //assert(0);
      }
      break;
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
          *dataPtr = level;
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

  if (attributeID == 7) {
    value += ((int16u) dataPtr[1]) << 8;
    expectComparisonDecimal(value, expectedTemp, "Temperature", "expectedTemperature");
  }

  //printf("write attributes:  %x %x %d\r\n", cluster, attributeID, value);

  return EMBER_SUCCESS;
}

EmberStatus halCommonIdleForMilliseconds(int32u *duration)
{
  return EMBER_SUCCESS;
}
