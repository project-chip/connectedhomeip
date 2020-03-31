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

// attributes
static uint16_t minColor = 150;
static uint16_t maxColor = 300;
static uint16_t colorTemp = 150;
static uint8_t onOff = 1;
static uint8_t level = 0xff;
static uint16_t expectedWhite;
static uint16_t expectedRed;
static uint32_t scratch;

void emLedTempPwmInitHandler(void);

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

// Test case declarations
uint16_t colorTemps[] = { 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250,
                          260, 270, 280, 290, 300, 310 };
uint16_t wValues1[] = { 0, 0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600,
                        4000, 4400, 4800, 5200, 5600, 6000, 6000 };
uint16_t rValues1[] = { 6000, 6000, 5600, 5200, 4800, 4400, 4000, 3600, 3200, 2800,
                        2400, 2000, 1600, 1200, 800, 400, 0, 0 };

uint16_t tempValues1[] = { 164, 178, 191, 205, 219, 232, 246, 260, 273, 287, 300 };
uint16_t tempValues2[] = { 290, 281, 271, 262, 253, 243, 234, 225, 215, 206, 196, 187, 178, 168,
                           159, 150 };
uint16_t tempValues3[] = { 160, 169, 179, 188, 197, 207, 216, 225, 235, 244, 254, 263, 272, 282,
                           291, 300 };

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

static void initTest(void)
{
  int i;
  int numValues = sizeof(colorTemps) / sizeof(uint16_t);

  onOff = 1;
  level = 0xfe;

  for (i = 0; i < numValues; i++) {
    colorTemp = colorTemps[i];
    expectedWhite = wValues1[i];
    expectedRed = rValues1[i];
    emLedTempPwmInitHandler();
  }

  onOff = 0;
  expectedWhite = 0;
  expectedRed = 0;
  emLedTempPwmInitHandler();

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
    emLedTempPwmInitHandler();
  }
}

int main(int argc, char* argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  fputc('.', stderr);
  initTest();
  fputc('.', stderr);

  fprintf(stderr, " done]\n");

  return 0;
}

void halBulbPwmDriverSetPwmLevel(uint16_t pwmValue, uint8_t pwmChannel)
{
  switch (pwmChannel) {
    case 1:
      assert(pwmValue == expectedRed);
      break;
    case 2:
      assert(pwmValue == expectedWhite);
      break;
    case 3:
      assert(pwmValue == 0);
    case 4:
      assert(pwmValue == 0);
      break;
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
        *(uint8_t *)buffer = level;
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
