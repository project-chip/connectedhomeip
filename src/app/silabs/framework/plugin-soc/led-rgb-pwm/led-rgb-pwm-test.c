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
static int8u onOff = 1;
static int8u expectedLevel = 0xff;
static int16u expectedWhite;
static int16u expectedRed;
static int16u expectedGreen;
static int16u expectedBlue;

static int16u minColor, maxColor, colorTemp;
static int8u  hue, saturation, colorMode;
static int16u currentX, currentY;

EmberAfClusterCommand *emAfCurrentCommand;

//------------------------------------------------------------------------------
// External declarations and events.
#define COLOR_MODE_HSV  EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION
#define COLOR_MODE_XY   EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y
#define COLOR_MODE_TEMP EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE

//============================================================================
// Forward Declarations

void emberAfPluginNetworkSteeringStackStatusCallback(EmberStatus status);
void emberAfPluginLedRgbPwmInitCallback(void);

//============================================================================

typedef struct {
  int8u level;
  int8u onOff;
  int8u colorMode;
  int16u colorX;
  int16u colorY;
  int8u hue;
  int8u saturation;
  int16u colorTemperature;
} RgbInputs;

typedef struct {
  int16u white;
  int16u red;
  int16u green;
  int16u blue;
} RgbOutputs;

RgbInputs rgbInputs[] = {
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 0, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 30, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 60, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 90, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 120, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 150, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 180, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 210, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 240, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 254, 254, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 0, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 30, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 60, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 90, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 120, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 150, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 180, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 210, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 240, 50, 0 },
  { 0xff, 1, COLOR_MODE_HSV, 0, 0, 254, 50, 0 },
  { 0xff, 1, COLOR_MODE_XY, 20316, 21627, 0, 0, 0 },  // white
  { 0xff, 1, COLOR_MODE_XY, 26870, 32768, 0, 0, 0 },  // yellow
  { 0xff, 1, COLOR_MODE_XY, 14418, 21627, 0, 0, 0 },  // cyan
  { 0xff, 1, COLOR_MODE_XY, 19005, 39322, 0, 0, 0 },  // green
  { 0xff, 1, COLOR_MODE_XY, 20972, 9830, 0, 0, 0 },  // purple
  { 0xff, 1, COLOR_MODE_XY, 41943, 21627, 0, 0, 0 },  // red
  { 0xff, 1, COLOR_MODE_XY, 9830, 3932, 0, 0, 0 },  // blue
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 100 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 150 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 200 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 250 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 300 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 350 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 400 },
  { 0xff, 1, COLOR_MODE_TEMP, 0, 0, 0, 0, 450 }
};

RgbOutputs rgbOutputs[] = {
  { 0, 6023, 0, 0 },
  { 0, 6023, 4299, 0 },
  { 0, 3425, 6023, 0 },
  { 0, 0, 6023, 826 },
  { 0, 0, 6023, 5031 },
  { 0, 0, 2716, 6023 },
  { 0, 1559, 0, 6023 },
  { 0, 5858, 0, 6023 },
  { 0, 6023, 0, 2007 },
  { 0, 6023, 0, 0 },
  { 0, 6023, 4818, 4818 },
  { 0, 6023, 5669, 4818 },
  { 0, 5503, 6023, 4818 },
  { 0, 4818, 6023, 4984 },
  { 0, 4818, 6023, 5811 },
  { 0, 4818, 5362, 6023 },
  { 0, 5125, 4818, 6023 },
  { 0, 5976, 4818, 6023 },
  { 0, 6023, 4818, 5220 },
  { 0, 6023, 4818, 4818 },
  { 0, 1601, 2043, 2151 },
  { 0, 2862, 3042, 508 },
  { 0, 77, 2372, 2689 },
  { 0, 573, 4279, 621 },
  { 0, 2235, 573, 3191 },
  { 0, 6000, 854, 167 },
  { 0, 0, 466, 4763 },
  { 0, 4752, 5129, 6000 },
  { 0, 6000, 5882, 6000 },
  { 0, 6000, 5364, 4847 },
  { 0, 6000, 4847, 3905 },
  { 0, 6000, 4423, 3082 },
  { 0, 6000, 4047, 2352 },
  { 0, 6000, 3741, 1647 },
  { 0, 6000, 3458, 988 }
};

static void setExpectedWRGB(int16u white, int16u red, int16u green, int16u blue)
{
  expectedWhite =  white;
  expectedRed = red;
  expectedGreen = green;
  expectedBlue = blue;
}

static void setExpectedRgbOut(RgbOutputs rgbOutputs)
{
  setExpectedWRGB(rgbOutputs.white, rgbOutputs.red, rgbOutputs.green, rgbOutputs.blue);
}

static void setExpectedRgbInputs(RgbInputs rgbInputs)
{
  expectedLevel = rgbInputs.level;
  onOff         = rgbInputs.onOff;
  colorMode     = rgbInputs.colorMode;
  currentX      = rgbInputs.colorX;
  currentY      = rgbInputs.colorY;
  hue           = rgbInputs.hue;
  saturation    = rgbInputs.saturation;
  colorTemp     = rgbInputs.colorTemperature;
}

static void initTest(void)
{
  int i;
  int numValues = sizeof(rgbInputs) / sizeof(RgbInputs);

  for (i = 0; i < numValues; i++) {
    setExpectedRgbInputs(rgbInputs[i]);
    setExpectedRgbOut(rgbOutputs[i]);

    emberAfPluginLedRgbPwmInitCallback();
  }
}

void emberAfLevelControlClusterServerAttributeChangedCallback(int8u endpoint,
                                                              EmberAfAttributeId attributeId);
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId);
void emberAfColorControlClusterServerAttributeChangedCallback(int8u endpoint,
                                                              EmberAfAttributeId attributeId);
void halBulbPwmDriverBlinkStopCallback(void);
//void emberAfPluginBulbPwmDriverBlinkStopCallback( int8u endpoint );

static void driveTest(void)
{
  setExpectedRgbInputs(rgbInputs[1]);
  setExpectedRgbOut(rgbOutputs[1]);
  emberAfPluginLedRgbPwmInitCallback();

  setExpectedRgbInputs(rgbInputs[2]);
  setExpectedRgbOut(rgbOutputs[2]);
  emberAfLevelControlClusterServerAttributeChangedCallback(1, 0x0000);

  setExpectedRgbInputs(rgbInputs[3]);
  setExpectedRgbOut(rgbOutputs[3]);
  emberAfOnOffClusterServerAttributeChangedCallback(1, 0x0000);

  setExpectedRgbInputs(rgbInputs[4]);
  setExpectedRgbOut(rgbOutputs[4]);
  emberAfColorControlClusterServerAttributeChangedCallback(1, 0x0000);

  setExpectedRgbInputs(rgbInputs[5]);
  setExpectedRgbOut(rgbOutputs[5]);
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
  switch (pwmChannel) {
    case 1:
      expectComparisonDecimal(pwmValue,
                              expectedWhite,
                              "white",
                              "expectedWhite");
      break;
    case 2:
      expectComparisonDecimal(pwmValue,
                              expectedRed,
                              "red",
                              "expectedRed");
      break;
    case 3:
      expectComparisonDecimal(pwmValue,
                              expectedGreen,
                              "green",
                              "expectedGreen");
      break;
    case 4:
      expectComparisonDecimal(pwmValue,
                              expectedBlue,
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

#if 1
  expectComparisonDecimal(white,
                          expectedWhite,
                          "white",
                          "expectedWhite");
  expectComparisonDecimal(red,
                          expectedRed,
                          "red",
                          "expectedRed");
  expectComparisonDecimal(green,
                          expectedGreen,
                          "green",
                          "expectedGreen");
  expectComparisonDecimal(blue,
                          expectedBlue,
                          "blue",
                          "expectedBlue");
#endif
}

int16u emberAfPluginBulbPwmDriverMinDriveValue(void)
{
  return 0;
}

int16u emberAfPluginBulbPwmDriverMaxDriveValue(void)
{
  return 6000;
}

static void copyInt16uToDataPointer(int16u value, int8u *dataPtr)
{
  *dataPtr = (int8u) value;
  dataPtr++;
  *dataPtr = (int8u) (value >> 8);
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
        case ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID:
          *dataPtr = hue;
          break;
        case ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID:
          *dataPtr = saturation;
          break;
        case ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID:
          copyInt16uToDataPointer(currentX, dataPtr);
          break;
        case ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID:
          copyInt16uToDataPointer(currentY, dataPtr);
          break;
        case ZCL_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID:
          *dataPtr = colorMode;
          break;
        case ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_ATTRIBUTE_ID:
          copyInt16uToDataPointer(minColor, dataPtr);
          break;
        case ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_ATTRIBUTE_ID:
          copyInt16uToDataPointer(maxColor, dataPtr);
          break;
        case ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID:
          copyInt16uToDataPointer(colorTemp, dataPtr);
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
