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
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_BULB_PWM_DRIVER

// Macros for zcl color modes
#define COLOR_MODE_HSV  0//COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION
#define COLOR_MODE_XY   1//COLOR_MODE_CURRENT_X_AND_CURRENT_Y
#define COLOR_MODE_TEMP 2//COLOR_MODE_COLOR_TEMPERATURE

// attributes
static bool onOff = 1;
static uint8_t expectedLevel = 0xff;
static uint16_t expectedWhite;
static uint16_t expectedRed;
static uint16_t expectedGreen;
static uint16_t expectedBlue;

static uint16_t minColor, maxColor, colorTemp;
static uint8_t  hue, saturation, colorMode;
static uint16_t currentX, currentY;

const EmberZclClusterSpec_t emberZclClusterLevelControlServerSpec = {
  EMBER_ZCL_ROLE_CLIENT,
  EMBER_ZCL_MANUFACTURER_CODE_NULL,
  EMBER_ZCL_CLUSTER_LEVEL_CONTROL,
};

const EmberZclClusterSpec_t emberZclClusterOnOffServerSpec = {
  EMBER_ZCL_ROLE_CLIENT,
  EMBER_ZCL_MANUFACTURER_CODE_NULL,
  EMBER_ZCL_CLUSTER_ON_OFF,
};

const EmberZclClusterSpec_t emberZclClusterColorControlServerSpec = {
  EMBER_ZCL_ROLE_CLIENT,
  EMBER_ZCL_MANUFACTURER_CODE_NULL,
  EMBER_ZCL_CLUSTER_COLOR_CONTROL,
};

void emLedRgbPwmInitHandler(void);
void emLedRgbPwmPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclAttributeId_t attributeId,
                                           const void *buffer,
                                           size_t bufferLength);

//============================================================================

typedef struct {
  uint8_t level;
  uint8_t onOff;
  uint8_t colorMode;
  uint16_t colorX;
  uint16_t colorY;
  uint8_t hue;
  uint8_t saturation;
  uint16_t colorTemperature;
} RgbInputs;

typedef struct {
  uint16_t white;
  uint16_t red;
  uint16_t green;
  uint16_t blue;
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

EmberZclEndpointId_t emberZclEndpointIndexToId(EmberZclEndpointIndex_t index,
                                               const EmberZclClusterSpec_t *clusterSpec)
{
  return 1;
}

bool emberZclAreClusterSpecsEqual(const EmberZclClusterSpec_t *s1,
                                  const EmberZclClusterSpec_t *s2)
{
  return (s1->role == s2->role
          && s1->manufacturerCode == s2->manufacturerCode
          && s1->id == s2->id);
}

static void setExpectedWrgb(uint16_t white, uint16_t red, uint16_t green, uint16_t blue)
{
  expectedWhite =  white;
  expectedRed = red;
  expectedGreen = green;
  expectedBlue = blue;
}

static void setExpectedRgbOut(RgbOutputs rgbOutputs)
{
  setExpectedWrgb(rgbOutputs.white, rgbOutputs.red, rgbOutputs.green, rgbOutputs.blue);
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
  int numValues = COUNTOF(rgbInputs);

  for (i = 0; i < numValues; i++) {
    setExpectedRgbInputs(rgbInputs[i]);
    setExpectedRgbOut(rgbOutputs[i]);

    emLedRgbPwmInitHandler();
  }
}

static void driveTest(void)
{
  void *dummy = NULL;
  size_t dummyLength = 0;

  setExpectedRgbInputs(rgbInputs[1]);
  setExpectedRgbOut(rgbOutputs[1]);
  emLedRgbPwmInitHandler();

  setExpectedRgbInputs(rgbInputs[2]);
  setExpectedRgbOut(rgbOutputs[2]);
  emLedRgbPwmPostAttributeChangeHandler(1,
                                        &emberZclClusterLevelControlServerSpec,
                                        0x0000,
                                        dummy,
                                        dummyLength);

  setExpectedRgbInputs(rgbInputs[3]);
  setExpectedRgbOut(rgbOutputs[3]);
  emLedRgbPwmPostAttributeChangeHandler(1,
                                        &emberZclClusterOnOffServerSpec,
                                        0x0000,
                                        dummy,
                                        dummyLength);

  setExpectedRgbInputs(rgbInputs[4]);
  setExpectedRgbOut(rgbOutputs[4]);
  emLedRgbPwmPostAttributeChangeHandler(1,
                                        &emberZclClusterColorControlServerSpec,
                                        0x0000,
                                        dummy,
                                        dummyLength);

  setExpectedRgbInputs(rgbInputs[5]);
  setExpectedRgbOut(rgbOutputs[5]);
  halBulbPwmDriverBlinkStopCallback();
}

int main(int argc, char* argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  fputc('.', stderr);
  initTest();
  fputc('.', stderr);
  driveTest();
  fputc('.', stderr);

  fprintf(stderr, " done]\n");

  return 0;
}

void halBulbPwmDriverSetPwmLevel(uint16_t pwmValue, uint8_t pwmChannel)
{
  switch (pwmChannel) {
    case 1:
      assert(pwmValue == expectedWhite);
      break;
    case 2:
      assert(pwmValue == expectedRed);
      break;
    case 3:
      assert(pwmValue == expectedGreen);
      break;
    case 4:
      assert(pwmValue == expectedBlue);
      break;
    default:
      assert(false);
  }
}

uint16_t halBulbPwmDriverTicksPerMicrosecond(void)
{
  return 6;
}

uint16_t halBulbPwmDriverTicksPerPeriod(void)
{
  return 6000;
}

void emberAfPluginBulbPwmDriverDriveWrgb(uint16_t white, uint16_t red, uint16_t green, uint16_t blue)
{
  //  printf("%d %d %d %d\r\n", white, red, green, blue);
  assert(white == expectedWhite);
  assert(red == expectedRed);
  assert(green == expectedGreen);
  assert(blue == expectedBlue);
}

uint16_t emberAfPluginBulbPwmDriverMinDriveValue(void)
{
  return 0;
}

uint16_t emberAfPluginBulbPwmDriverMaxDriveValue(void)
{
  return 6000;
}

EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterColorControlServerSpec,
                                   clusterSpec)) {
    switch (attributeId) {
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_HUE:
        *(uint8_t *)buffer = hue;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION:
        *(uint8_t *)buffer = saturation;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X:
        *(uint16_t *)buffer = currentX;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y:
        *(uint16_t *)buffer = currentY;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_MODE:
        *(uint8_t *)buffer = colorMode;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN:
        *(uint16_t *)buffer = minColor;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX:
        *(uint16_t *)buffer = maxColor;
        break;
      case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMPERATURE:
        *(uint16_t *)buffer = colorTemp;
        break;
      default:
        printf("Color Control attribute 0x%0x read unexpected\n", attributeId);
        assert(false);
    }
  } else if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec,
                                          clusterSpec)) {
    switch (attributeId) {
      case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF:
        *(bool *)buffer = onOff;
        break;
      default:
        printf("ON OFF attribute 0x%0x read unexpected\n", attributeId);
        assert(false);
        break;
    }
  } else if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec,
                                          clusterSpec)) {
    switch (attributeId) {
      case EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL:
        *(uint8_t *)buffer = expectedLevel;
        break;
      default:
        printf("LEVEL attribute 0x%0x read unexpected\n", attributeId);
        assert(false);
        break;
    }
  } else {
    printf("Unexpected read of cluster 0x%0x", clusterSpec->id);
    assert(false);
  }

  return EMBER_ZCL_STATUS_SUCCESS;
}
