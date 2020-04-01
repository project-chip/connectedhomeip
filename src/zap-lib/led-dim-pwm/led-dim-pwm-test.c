/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_BULB_PWM_DRIVER

void emLedDimPwmInitHandler(void);
void emLedDimPwmPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclAttributeId_t attributeId,
                                           const void *buffer,
                                           size_t bufferLength);

static bool onOff = 1;
static uint8_t expectedLevel = 0xff;
static uint16_t expectedPwmOutput;

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

uint8_t levelValues[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130,
                          140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240,
                          250, 255 };
uint16_t pwmValues1[] = { 0, 285, 501, 718, 936, 1152, 1369, 1586, 1802, 2019, 2237, 2454,
                          2670, 2887, 3104, 3320, 3538, 3755, 3971, 4188, 4405, 4621,
                          4838, 5056, 5273, 5489, 6000 };

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
  int numValues = COUNTOF(levelValues);

  onOff = 1;

  for (i = 0; i < numValues; i++) {
    expectedLevel = levelValues[i];
    expectedPwmOutput = pwmValues1[i];
    emLedDimPwmInitHandler();
  }

  onOff = 0;
  expectedLevel = 0x80;
  expectedPwmOutput = 0;
  emLedDimPwmInitHandler();
}

static void driveTest(void)
{
  void *dummy = NULL;
  size_t dummySize = 0;
  onOff = 1;
  expectedLevel = 0xff;
  expectedPwmOutput = 6000;
  emLedDimPwmInitHandler();

  onOff = 1;
  expectedLevel = 20;
  expectedPwmOutput = 501;
  emLedDimPwmPostAttributeChangeHandler(1,
                                        &emberZclClusterLevelControlServerSpec,
                                        0,
                                        dummy,
                                        sizeof(dummySize));

  expectedLevel = 30;
  expectedPwmOutput = 718;
  emLedDimPwmPostAttributeChangeHandler(1,
                                        &emberZclClusterOnOffServerSpec,
                                        0,
                                        dummy,
                                        sizeof(dummySize));

  expectedLevel = 40;
  expectedPwmOutput = 936;
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
  assert(pwmChannel == 1);

  assert(pwmValue == expectedPwmOutput);
}

uint16_t halBulbPwmDriverTicksPerMicrosecond(void)
{
  return 6;
}

uint16_t halBulbPwmDriverTicksPerPeriod(void)
{
  return 6000;
}

EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec, clusterSpec)) {
    switch (attributeId) {
      case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF:
        *(bool *)buffer = onOff;
        break;
      default:
        printf("ON OFF attribute 0x%x read unexpected\n", attributeId);
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
