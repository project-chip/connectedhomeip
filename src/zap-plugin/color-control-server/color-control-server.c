/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #include EMBER_AF_API_ZCL_SCENES_SERVER
#endif
#include "thread-callbacks.h"
#include "color-control-server.h"

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_COLOR_CONTROL_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_COLOR_CONTROL_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
EmZclColorControlSceneSubTableEntry_t emZclPluginColorControlServerSceneSubTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = emZclPluginColorControlServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (emZclPluginColorControlServerSceneSubTable[i] = entry)
  #endif
#endif

#define COLOR_TEMP_CONTROL emZclColorControlServerTempTransitionEventControl
#define COLOR_XY_CONTROL   emZclColorControlServerXyTransitionEventControl
#define COLOR_HSV_CONTROL  emZclColorControlServerHueSatTransitionEventControl

// direction
enum {
  DIRECTION_SHORTEST_DISTANCE = 0x00,
  DIRECTION_LONGEST_DISTANCE  = 0x01,
  DIRECTION_UP                = 0x02,
  DIRECTION_DOWN              = 0x03,
};

// move mode
enum {
  MOVE_MODE_STOP     = 0x00,
  MOVE_MODE_UP       = 0x01,
  MOVE_MODE_DOWN     = 0x03
};

enum {
  COLOR_MODE_HSV         = 0x00,
  COLOR_MODE_CIE_XY      = 0x01,
  COLOR_MODE_TEMPERATURE = 0x02
};

enum {
  HSV_TO_HSV         = 0x00,
  HSV_TO_CIE_XY      = 0x01,
  HSV_TO_TEMPERATURE = 0x02,
  CIE_XY_TO_HSV         = 0x10,
  CIE_XY_TO_CIE_XY      = 0x11,
  CIE_XY_TO_TEMPERATURE = 0x12,
  TEMPERATURE_TO_HSV         = 0x20,
  TEMPERATURE_TO_CIE_XY      = 0x21,
  TEMPERATURE_TO_TEMPERATURE = 0x22
};

EmberEventControl emZclColorControlServerTempTransitionEventControl;
EmberEventControl emZclColorControlServerXyTransitionEventControl;
EmberEventControl emZclColorControlServerHueSatTransitionEventControl;

#define UPDATE_TIME_MS 100
#define TRANSITION_TIME_1S 10
#define MIN_CIE_XY_VALUE 0
// this value comes directly from the ZCL specification table 5.3
#define MAX_CIE_XY_VALUE 0xfeff
#define MIN_TEMPERATURE_VALUE 0
#define MAX_TEMPERATURE_VALUE 0xfeff
#define MIN_HUE_VALUE 0
#define MAX_HUE_VALUE 254
#define MIN_SATURATION_VALUE 0
#define MAX_SATURATION_VALUE 254

typedef struct {
  uint8_t  initialHue;
  uint8_t  currentHue;
  uint8_t  finalHue;
  uint16_t stepsRemaining;
  uint16_t stepsTotal;
  uint8_t  endpoint;
  bool     up;
  bool     repeat;
} ColorHueTransitionState;

static ColorHueTransitionState colorHueTransitionState;

typedef struct {
  uint16_t initialValue;
  uint16_t currentValue;
  uint16_t finalValue;
  uint16_t stepsRemaining;
  uint16_t stepsTotal;
  uint16_t lowLimit;
  uint16_t highLimit;
  uint8_t  endpoint;
} Color16uTransitionState;

static Color16uTransitionState colorXTransitionState;
static Color16uTransitionState colorYTransitionState;
static Color16uTransitionState colorTempTransitionState;
static Color16uTransitionState colorSaturationTransitionState;

// Forward declarations:
static bool computeNewColor16uValue(Color16uTransitionState *p);
static void stopAllColorTransitions(void);
static void handleModeSwitch(uint8_t endpoint,
                             uint8_t newColorMode);
static uint8_t addHue(uint8_t hue1, uint8_t hue2);
static uint8_t subtractHue(uint8_t hue1, uint8_t hue2);
static uint8_t addSaturation(uint8_t saturation1, uint8_t saturation2);
static uint8_t subtractSaturation(uint8_t saturation1, uint8_t saturation2);
static void initHueSat(uint8_t endpoint);
static uint16_t findNewColorValueFromStep(uint16_t oldValue, int16_t step);
static uint16_t computeTransitionTimeFromStateAndRate(Color16uTransitionState *p,
                                                      uint16_t rate);

// convenient token handling functions
static uint8_t readColorMode(EmberZclEndpointId_t endpoint)
{
  uint8_t colorMode;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_MODE,
                        &colorMode,
                        sizeof(colorMode));

  return colorMode;
}

static uint8_t readHue(EmberZclEndpointId_t endpoint)
{
  uint8_t hue;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_HUE,
                        &hue,
                        sizeof(hue));

  return hue;
}

static uint8_t readSaturation(EmberZclEndpointId_t endpoint)
{
  uint8_t saturation;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION,
                        &saturation,
                        sizeof(saturation));

  return saturation;
}

static uint16_t readColorX(EmberZclEndpointId_t endpoint)
{
  uint16_t colorX;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X,
                        &colorX,
                        sizeof(colorX));

  return colorX;
}

static uint16_t readColorY(EmberZclEndpointId_t endpoint)
{
  uint16_t colorY;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y,
                        &colorY,
                        sizeof(colorY));

  return colorY;
}

static uint16_t readColorTemperature(EmberZclEndpointId_t endpoint)
{
  uint16_t colorTemperature;

  emberZclReadAttribute(endpoint,
                        &emberZclClusterColorControlServerSpec,
                        EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMPERATURE,
                        (uint8_t *)&colorTemperature,
                        sizeof(colorTemperature));

  return colorTemperature;
}

static uint16_t readColorTemperatureMin(EmberZclEndpointId_t endpoint)
{
  uint16_t colorTemperatureMin;
  EmberStatus status;

  status =
    emberZclReadAttribute(endpoint,
                          &emberZclClusterColorControlServerSpec,
                          EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN,
                          (uint8_t *)&colorTemperatureMin,
                          sizeof(colorTemperatureMin));

  if (status != EMBER_SUCCESS) {
    colorTemperatureMin = MIN_TEMPERATURE_VALUE;
  }

  return colorTemperatureMin;
}

static uint16_t readColorTemperatureMax(EmberZclEndpointId_t endpoint)
{
  uint16_t colorTemperatureMax;
  EmberStatus status;

  status =
    emberZclReadAttribute(endpoint,
                          &emberZclClusterColorControlServerSpec,
                          EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX,
                          (uint8_t *)&colorTemperatureMax,
                          sizeof(colorTemperatureMax));

  if (status != EMBER_SUCCESS) {
    colorTemperatureMax = MAX_TEMPERATURE_VALUE;
  }

  return colorTemperatureMax;
}

static EmberStatus writeRemainingTime(EmberZclEndpointId_t endpoint,
                                      uint16_t remainingTime)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_REMAINING_TIME,
                                (uint8_t *)&remainingTime,
                                sizeof(remainingTime));
}

static EmberStatus writeColorMode(EmberZclEndpointId_t endpoint,
                                  uint8_t colorMode)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_MODE,
                                (uint8_t *)&colorMode,
                                sizeof(colorMode));
}

static EmberStatus writeHue(EmberZclEndpointId_t endpoint,
                            uint8_t hue)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_HUE,
                                (uint8_t *)&hue,
                                sizeof(hue));
}

static EmberStatus writeSaturation(EmberZclEndpointId_t endpoint,
                                   uint8_t saturation)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION,
                                (uint8_t *)&saturation,
                                sizeof(saturation));
}

static EmberStatus writeColorX(EmberZclEndpointId_t endpoint,
                               uint16_t colorX)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X,
                                (uint8_t *)&colorX,
                                sizeof(colorX));
}

static EmberStatus writeColorY(EmberZclEndpointId_t endpoint,
                               uint16_t colorY)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y,
                                (uint8_t *)&colorY,
                                sizeof(colorY));
}

static EmberStatus writeColorTemperature(EmberZclEndpointId_t endpoint,
                                         uint16_t colorTemperature)
{
  return emberZclWriteAttribute(endpoint,
                                &emberZclClusterColorControlServerSpec,
                                EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMPERATURE,
                                (uint8_t *)&colorTemperature,
                                sizeof(colorTemperature));
}

// any time we call a hue or saturaiton transition, we need to assume certain
// things about the hue and saturation data structures.  This function will
// properly initialize them.
static void initHueSat(EmberZclEndpointId_t endpoint)
{
  colorHueTransitionState.stepsRemaining = 0;
  colorHueTransitionState.currentHue = readHue(endpoint);
  colorHueTransitionState.endpoint = endpoint;

  colorSaturationTransitionState.stepsRemaining = 0;
  colorSaturationTransitionState.currentValue = readSaturation(endpoint);
  colorSaturationTransitionState.endpoint = endpoint;
}
// -------------------------------------------------------------------------
// ****** callback section *******

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
/** @brief Move To Hue And Saturation
 *
 *
 *
 * @param hue   Ver.: always
 * @param saturation   Ver.: always
 * @param transitionTime   Ver.: always
 */
void emberZclClusterColorControlServerCommandMoveToHueAndSaturationRequestHandler(const EmberZclCommandContext_t *context,
                                                                                  const EmberZclClusterColorControlServerCommandMoveToHueAndSaturationRequest_t *request)
{
  uint8_t hue;
  uint8_t saturation;
  uint16_t transitionTime;

  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;

  hue = request->hue;
  saturation = request->saturation;
  transitionTime = request->transitionTime;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
  // this and report a malformed packet.
  if (hue > MAX_HUE_VALUE || saturation > MAX_SATURATION_VALUE) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
    return;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.initialHue = readHue(endpoint);
  colorHueTransitionState.currentHue = readHue(endpoint);
  colorHueTransitionState.finalHue   = hue;
  colorHueTransitionState.stepsRemaining = transitionTime;
  colorHueTransitionState.stepsTotal     = transitionTime;
  colorHueTransitionState.endpoint = endpoint;
  colorHueTransitionState.up = (hue > readHue(endpoint));
  colorHueTransitionState.repeat = false;

  colorSaturationTransitionState.initialValue = readSaturation(endpoint);
  colorSaturationTransitionState.currentValue = readSaturation(endpoint);
  colorSaturationTransitionState.finalValue = saturation;
  colorSaturationTransitionState.stepsRemaining = transitionTime;
  colorSaturationTransitionState.stepsTotal     = transitionTime;
  colorSaturationTransitionState.endpoint       = endpoint;
  colorSaturationTransitionState.lowLimit  = MIN_SATURATION_VALUE;
  colorSaturationTransitionState.highLimit = MAX_SATURATION_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveHueRequestHandler(const EmberZclCommandContext_t *context,
                                                                   const EmberZclClusterColorControlServerCommandMoveHueRequest_t *request)
{
  uint8_t moveMode;
  uint8_t rate;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;

  moveMode = request->moveMode;
  rate = request->rate;

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (moveMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.initialHue = readHue(endpoint);
  colorHueTransitionState.currentHue = readHue(endpoint);
  if (moveMode == MOVE_MODE_UP) {
    colorHueTransitionState.finalHue = addHue(readHue(endpoint), rate);
    colorHueTransitionState.up = true;
  } else {
    colorHueTransitionState.finalHue = subtractHue(readHue(endpoint), rate);
    colorHueTransitionState.up = false;
  }
  colorHueTransitionState.stepsRemaining = TRANSITION_TIME_1S;
  colorHueTransitionState.stepsTotal     = TRANSITION_TIME_1S;
  colorHueTransitionState.endpoint = endpoint;
  colorHueTransitionState.repeat = true;
  // hue movement can last forever.  Indicate this with a remaining time of
  // maxint.
  writeRemainingTime(endpoint, MAX_INT16U_VALUE);

  colorSaturationTransitionState.stepsRemaining = 0;

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveSaturationRequestHandler(const EmberZclCommandContext_t *context,
                                                                          const EmberZclClusterColorControlServerCommandMoveSaturationRequest_t *request)
{
  uint8_t moveMode;
  uint8_t rate;

  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t transitionTime;

  moveMode = request->moveMode;
  rate = request->rate;

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (moveMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.stepsRemaining = 0;

  colorSaturationTransitionState.initialValue = readSaturation(endpoint);
  colorSaturationTransitionState.currentValue = readSaturation(endpoint);
  if (moveMode == MOVE_MODE_UP) {
    colorSaturationTransitionState.finalValue = MAX_SATURATION_VALUE;
  } else {
    colorSaturationTransitionState.finalValue = MIN_SATURATION_VALUE;
  }

  transitionTime =
    computeTransitionTimeFromStateAndRate(&colorSaturationTransitionState,
                                          rate);

  colorSaturationTransitionState.stepsRemaining = transitionTime;
  colorSaturationTransitionState.stepsTotal     = transitionTime;
  colorSaturationTransitionState.endpoint       = endpoint;
  colorSaturationTransitionState.lowLimit  = MIN_SATURATION_VALUE;
  colorSaturationTransitionState.highLimit = MAX_SATURATION_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveToHueRequestHandler(const EmberZclCommandContext_t *context,
                                                                     const EmberZclClusterColorControlServerCommandMoveToHueRequest_t *request)
{
  uint8_t hue;
  uint8_t direction;
  uint16_t transitionTime;
  bool up = false;
  uint8_t currentHue;

  hue = request->hue;
  direction = request->direction;
  transitionTime = request->transitionTime;

  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
  // this and report a malformed packet.
  if (hue > MAX_HUE_VALUE) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
    return;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);
  currentHue = readHue(endpoint);

  // Calculate direction
  switch (direction) {
    // Hue is a circle of size 128, choose up/down based on the difference
    // between current and desired values and "direction" parameter
    case DIRECTION_SHORTEST_DISTANCE:
      if (((hue >= currentHue) && (hue - currentHue <= 64))
          || ((hue <= currentHue) && (currentHue - hue >= 64))) {
        up = true;
      } else {
        up = false;
      }
      break;
    case DIRECTION_LONGEST_DISTANCE:
      if (((hue > currentHue) && (hue - currentHue < 64))
          || ((hue < currentHue) && (currentHue - hue > 64))) {
        up = false;
      } else {
        up = true;
      }
      break;
    case DIRECTION_UP:
      up = true;
      break;
    case DIRECTION_DOWN:
      up = false;
      break;
    default:
      emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
      return;
  }

  colorHueTransitionState.initialHue = currentHue;
  colorHueTransitionState.currentHue = currentHue;
  colorHueTransitionState.finalHue   = hue;
  colorHueTransitionState.stepsRemaining = transitionTime;
  colorHueTransitionState.stepsTotal     = transitionTime;
  colorHueTransitionState.endpoint = endpoint;
  colorHueTransitionState.up = up;
  colorHueTransitionState.repeat = false;

  colorSaturationTransitionState.stepsRemaining = 0;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveToSaturationRequestHandler(const EmberZclCommandContext_t *context,
                                                                            const EmberZclClusterColorControlServerCommandMoveToSaturationRequest_t *request)
{
  uint8_t saturation;
  uint16_t transitionTime;

  saturation = request->saturation;
  transitionTime = request->transitionTime;

  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // limit checking:  hue and saturation are 0..254.  Spec dictates we ignore
  // this and report a malformed packet.
  if (saturation > MAX_SATURATION_VALUE) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
    return;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.stepsRemaining = 0;

  colorSaturationTransitionState.initialValue = readSaturation(endpoint);
  colorSaturationTransitionState.currentValue = readSaturation(endpoint);
  colorSaturationTransitionState.finalValue = saturation;
  colorSaturationTransitionState.stepsRemaining = transitionTime;
  colorSaturationTransitionState.stepsTotal     = transitionTime;
  colorSaturationTransitionState.endpoint       = endpoint;
  colorSaturationTransitionState.lowLimit  = MIN_SATURATION_VALUE;
  colorSaturationTransitionState.highLimit = MAX_SATURATION_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandStepHueRequestHandler(const EmberZclCommandContext_t *context,
                                                                   const EmberZclClusterColorControlServerCommandStepHueRequest_t *request)
{
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint8_t currentHue = readHue(endpoint);

  stepMode = request->stepMode;
  stepSize = request->stepSize;
  transitionTime = request->transitionTime;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (stepMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.initialHue = currentHue;
  colorHueTransitionState.currentHue = currentHue;

  if (stepMode == MOVE_MODE_UP) {
    colorHueTransitionState.finalHue = addHue(currentHue, stepSize);
    colorHueTransitionState.up = true;
  } else {
    colorHueTransitionState.finalHue = subtractHue(currentHue, stepSize);
    colorHueTransitionState.up = false;
  }
  colorHueTransitionState.stepsRemaining = transitionTime;
  colorHueTransitionState.stepsTotal     = transitionTime;
  colorHueTransitionState.endpoint = endpoint;
  colorHueTransitionState.repeat = false;

  colorSaturationTransitionState.stepsRemaining = 0;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandStepSaturationRequestHandler(const EmberZclCommandContext_t *context,
                                                                          const EmberZclClusterColorControlServerCommandStepSaturationRequest_t *request)
{
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint8_t currentSaturation = readSaturation(endpoint);

  stepMode = request->stepMode;
  stepSize = request->stepSize;
  transitionTime = request->transitionTime;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (stepMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_HSV);

  // now, kick off the state machine.
  initHueSat(endpoint);

  colorHueTransitionState.stepsRemaining = 0;

  colorSaturationTransitionState.initialValue = currentSaturation;
  colorSaturationTransitionState.currentValue = currentSaturation;

  if (stepMode == MOVE_MODE_UP) {
    colorSaturationTransitionState.finalValue = addSaturation(currentSaturation,
                                                              stepSize);
  } else {
    colorSaturationTransitionState.finalValue =
      subtractSaturation(currentSaturation,
                         stepSize);
  }
  colorSaturationTransitionState.stepsRemaining = transitionTime;
  colorSaturationTransitionState.stepsTotal     = transitionTime;
  colorSaturationTransitionState.endpoint       = endpoint;
  colorSaturationTransitionState.lowLimit  = MIN_SATURATION_VALUE;
  colorSaturationTransitionState.highLimit = MAX_SATURATION_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

#endif

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY

void emberZclClusterColorControlServerCommandMoveToColorRequestHandler(const EmberZclCommandContext_t *context,
                                                                       const EmberZclClusterColorControlServerCommandMoveToColorRequest_t *request)
{
  uint16_t colorX;
  uint16_t colorY;
  uint16_t transitionTime;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;

  colorX = request->colorX;
  colorY = request->colorY;
  transitionTime = request->transitionTime;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

  // now, kick off the state machine.
  colorXTransitionState.initialValue = readColorX(endpoint);
  colorXTransitionState.currentValue = readColorX(endpoint);
  colorXTransitionState.finalValue = colorX;
  colorXTransitionState.stepsRemaining = transitionTime;
  colorXTransitionState.stepsTotal = transitionTime;
  colorXTransitionState.endpoint = endpoint;
  colorXTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorXTransitionState.highLimit = MAX_CIE_XY_VALUE;

  colorYTransitionState.initialValue = readColorY(endpoint);
  colorYTransitionState.currentValue = readColorY(endpoint);
  colorYTransitionState.finalValue = colorY;
  colorYTransitionState.stepsRemaining = transitionTime;
  colorYTransitionState.stepsTotal = transitionTime;
  colorYTransitionState.endpoint = endpoint;
  colorYTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorYTransitionState.highLimit = MAX_CIE_XY_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_XY_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveColorRequestHandler(const EmberZclCommandContext_t *context,
                                                                     const EmberZclClusterColorControlServerCommandMoveColorRequest_t *request)
{
  int16_t rateX;
  int16_t rateY;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t transitionTimeX, transitionTimeY;
  uint16_t unsignedRate;

  rateX = request->rateX;
  rateY = request->rateY;

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

  // now, kick off the state machine.
  colorXTransitionState.initialValue = readColorX(endpoint);
  colorXTransitionState.currentValue = colorXTransitionState.initialValue;
  if (rateX > 0) {
    colorXTransitionState.finalValue = MAX_CIE_XY_VALUE;
    unsignedRate = (uint16_t) rateX;
  } else {
    colorXTransitionState.finalValue = MIN_CIE_XY_VALUE;
    unsignedRate = (uint16_t) (rateX * -1);
  }
  transitionTimeX =
    computeTransitionTimeFromStateAndRate(&colorXTransitionState,
                                          unsignedRate);
  colorXTransitionState.stepsRemaining = transitionTimeX;
  colorXTransitionState.stepsTotal = transitionTimeX;
  colorXTransitionState.endpoint = endpoint;
  colorXTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorXTransitionState.highLimit = MAX_CIE_XY_VALUE;

  colorYTransitionState.initialValue = readColorY(endpoint);
  colorYTransitionState.currentValue = colorYTransitionState.initialValue;
  if (rateY > 0) {
    colorYTransitionState.finalValue = MAX_CIE_XY_VALUE;
    unsignedRate = (uint16_t) rateY;
  } else {
    colorYTransitionState.finalValue = MIN_CIE_XY_VALUE;
    unsignedRate = (uint16_t) (rateY * -1);
  }
  transitionTimeY =
    computeTransitionTimeFromStateAndRate(&colorYTransitionState,
                                          unsignedRate);
  colorYTransitionState.stepsRemaining = transitionTimeY;
  colorYTransitionState.stepsTotal = transitionTimeY;
  colorYTransitionState.endpoint = endpoint;
  colorYTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorYTransitionState.highLimit = MAX_CIE_XY_VALUE;

  if (transitionTimeX < transitionTimeY) {
    writeRemainingTime(endpoint, transitionTimeX);
  } else {
    writeRemainingTime(endpoint, transitionTimeY);
  }

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_XY_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandStepColorRequestHandler(const EmberZclCommandContext_t *context,
                                                                     const EmberZclClusterColorControlServerCommandStepColorRequest_t *request)
{
  int16_t stepX;
  int16_t stepY;
  uint16_t transitionTime;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t colorX;
  uint16_t colorY;

  stepX = request->stepX;
  stepY = request->stepY;
  transitionTime = request->transitionTime;

  colorX = findNewColorValueFromStep(readColorX(endpoint), stepX);
  colorY = findNewColorValueFromStep(readColorY(endpoint), stepY);

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_CIE_XY);

  // now, kick off the state machine.
  colorXTransitionState.initialValue = readColorX(endpoint);
  colorXTransitionState.currentValue = readColorX(endpoint);
  colorXTransitionState.finalValue = colorX;
  colorXTransitionState.stepsRemaining = transitionTime;
  colorXTransitionState.stepsTotal = transitionTime;
  colorXTransitionState.endpoint = endpoint;
  colorXTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorXTransitionState.highLimit = MAX_CIE_XY_VALUE;

  colorYTransitionState.initialValue = readColorY(endpoint);
  colorYTransitionState.currentValue = readColorY(endpoint);
  colorYTransitionState.finalValue = colorY;
  colorYTransitionState.stepsRemaining = transitionTime;
  colorYTransitionState.stepsTotal = transitionTime;
  colorYTransitionState.endpoint = endpoint;
  colorYTransitionState.lowLimit  = MIN_CIE_XY_VALUE;
  colorYTransitionState.highLimit = MAX_CIE_XY_VALUE;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_XY_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

#endif

#ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP

void emberZclClusterColorControlServerCommandMoveToColorTemperatureRequestHandler(const EmberZclCommandContext_t *context,
                                                                                  const EmberZclClusterColorControlServerCommandMoveToColorTemperatureRequest_t *request)
{
  uint16_t colorTemperature;
  uint16_t transitionTime;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t temperatureMin = readColorTemperatureMin(endpoint);
  uint16_t temperatureMax = readColorTemperatureMax(endpoint);

  colorTemperature = request->colorTemperature;
  transitionTime = request->transitionTime;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

  if (colorTemperature < temperatureMin) {
    colorTemperature = temperatureMin;
  }

  if (colorTemperature > temperatureMax) {
    colorTemperature = temperatureMax;
  }

  // now, kick off the state machine.
  colorTempTransitionState.initialValue = readColorTemperature(endpoint);
  colorTempTransitionState.currentValue = readColorTemperature(endpoint);
  colorTempTransitionState.finalValue   = colorTemperature;
  colorTempTransitionState.stepsRemaining = transitionTime;
  colorTempTransitionState.stepsTotal     = transitionTime;
  colorTempTransitionState.endpoint  = endpoint;
  colorTempTransitionState.lowLimit  = temperatureMin;
  colorTempTransitionState.highLimit = temperatureMax;

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_TEMP_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandMoveColorTemperatureRequestHandler(const EmberZclCommandContext_t *context,
                                                                                const EmberZclClusterColorControlServerCommandMoveColorTemperatureRequest_t *request)
{
  uint8_t moveMode;
  uint16_t rate;
  uint16_t colorTemperatureMinimum;
  uint16_t colorTemperatureMaximum;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t tempPhysicalMin = readColorTemperatureMin(endpoint);
  uint16_t tempPhysicalMax = readColorTemperatureMax(endpoint);
  uint16_t transitionTime;

  moveMode = request->moveMode;
  rate = request->rate;
  colorTemperatureMinimum = request->colorTemperatureMinimum;
  colorTemperatureMaximum = request->colorTemperatureMaximum;

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (moveMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  if (colorTemperatureMinimum < tempPhysicalMin) {
    colorTemperatureMinimum = tempPhysicalMin;
  }
  if (colorTemperatureMaximum > tempPhysicalMax) {
    colorTemperatureMaximum = tempPhysicalMax;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

  // now, kick off the state machine.
  colorTempTransitionState.initialValue = readColorTemperature(endpoint);
  colorTempTransitionState.currentValue = readColorTemperature(endpoint);
  if (moveMode == MOVE_MODE_UP) {
    if (tempPhysicalMax > colorTemperatureMaximum) {
      colorTempTransitionState.finalValue = colorTemperatureMaximum;
    } else {
      colorTempTransitionState.finalValue = tempPhysicalMax;
    }
  } else {
    if (tempPhysicalMin < colorTemperatureMinimum) {
      colorTempTransitionState.finalValue = colorTemperatureMinimum;
    } else {
      colorTempTransitionState.finalValue = tempPhysicalMin;
    }
  }
  transitionTime =
    computeTransitionTimeFromStateAndRate(&colorTempTransitionState,
                                          rate);
  colorTempTransitionState.stepsRemaining = transitionTime;
  colorTempTransitionState.stepsTotal     = transitionTime;
  colorTempTransitionState.endpoint  = endpoint;
  colorTempTransitionState.lowLimit  = colorTemperatureMinimum;
  colorTempTransitionState.highLimit = colorTemperatureMaximum;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_TEMP_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

void emberZclClusterColorControlServerCommandStepColorTemperatureRequestHandler(const EmberZclCommandContext_t *context,
                                                                                const EmberZclClusterColorControlServerCommandStepColorTemperatureRequest_t *request)
{
  uint8_t stepMode;
  uint16_t stepSize;
  uint16_t transitionTime;
  uint16_t colorTemperatureMinimum;
  uint16_t colorTemperatureMaximum;
  //TODO: Add multi endpoint support once it is available from the stack
  EmberZclEndpointId_t endpoint = 1;//context->endpointId;
  uint16_t tempPhysicalMin = readColorTemperatureMin(endpoint);
  uint16_t tempPhysicalMax = readColorTemperatureMax(endpoint);

  stepMode = request->stepMode;
  stepSize = request->stepSize;
  transitionTime = request->transitionTime;
  colorTemperatureMinimum = request->colorTemperatureMinimum;
  colorTemperatureMaximum = request->colorTemperatureMaximum;

  if (transitionTime == 0) {
    transitionTime++;
  }

  // New command.  Need to stop any active transitions.
  stopAllColorTransitions();

  if (stepMode == MOVE_MODE_STOP) {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
    return;
  }

  if (colorTemperatureMinimum < tempPhysicalMin) {
    colorTemperatureMinimum = tempPhysicalMin;
  }
  if (colorTemperatureMaximum > tempPhysicalMax) {
    colorTemperatureMaximum = tempPhysicalMax;
  }

  // Handle color mode transition, if necessary.
  handleModeSwitch(endpoint, COLOR_MODE_TEMPERATURE);

  // now, kick off the state machine.
  colorTempTransitionState.initialValue = readColorTemperature(endpoint);
  colorTempTransitionState.currentValue = readColorTemperature(endpoint);
  if (stepMode == MOVE_MODE_UP) {
    colorTempTransitionState.finalValue
      = readColorTemperature(endpoint) + stepSize;
  } else {
    colorTempTransitionState.finalValue
      = readColorTemperature(endpoint) - stepSize;
  }
  colorTempTransitionState.stepsRemaining = transitionTime;
  colorTempTransitionState.stepsTotal = transitionTime;
  colorTempTransitionState.endpoint = endpoint;
  colorTempTransitionState.lowLimit = colorTemperatureMinimum;
  colorTempTransitionState.highLimit = colorTemperatureMaximum;

  writeRemainingTime(endpoint, transitionTime);

  // kick off the state machine:
  emberEventControlSetDelayMS(COLOR_TEMP_CONTROL, UPDATE_TIME_MS);

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

#endif

void emberZclClusterColorControlServerCommandStopMoveStepRequestHandler(const EmberZclCommandContext_t *context,
                                                                        const EmberZclClusterColorControlServerCommandStopMoveStepRequest_t *request)
{
  // Received a stop command.  This is all we need to do.
  stopAllColorTransitions();

  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  return;
}

// **************** transition state machines ***********

static void stopAllColorTransitions(void)
{
  emberEventControlSetInactive(COLOR_TEMP_CONTROL);
  emberEventControlSetInactive(COLOR_XY_CONTROL);
  emberEventControlSetInactive(COLOR_HSV_CONTROL);
}

void emberAfPluginColorControlServerStopTransition(void)
{
  stopAllColorTransitions();
}

// The specification says that if we are transitioning from one color mode
// into another, we need to compute the new mode's attribute values from the
// old mode.  However, it also says that if the old mode doesn't translate into
// the new mode, this must be avoided.
// I am putting in this function to compute the new attributes based on the old
// color mode.
static void handleModeSwitch(uint8_t endpoint, uint8_t newColorMode)
{
  uint8_t oldColorMode = readColorMode(endpoint);
  uint8_t colorModeTransition;

  if (oldColorMode == newColorMode) {
    return;
  } else {
    writeColorMode(endpoint, newColorMode);
  }

  colorModeTransition = (newColorMode << 4) + oldColorMode;

  // Note:  It may be OK to not do anything here.
  switch (colorModeTransition) {
    case HSV_TO_CIE_XY:
      emberAfPluginColorControlServerComputePwmFromXyCallback(endpoint);
      break;
    case TEMPERATURE_TO_CIE_XY:
      emberAfPluginColorControlServerComputePwmFromXyCallback(endpoint);
      break;
    case CIE_XY_TO_HSV:
      emberAfPluginColorControlServerComputePwmFromHsvCallback(endpoint);
      break;
    case TEMPERATURE_TO_HSV:
      emberAfPluginColorControlServerComputePwmFromHsvCallback(endpoint);
      break;
    case HSV_TO_TEMPERATURE:
      emberAfPluginColorControlServerComputePwmFromTempCallback(endpoint);
      break;
    case CIE_XY_TO_TEMPERATURE:
      emberAfPluginColorControlServerComputePwmFromTempCallback(endpoint);
      break;

    // for the following cases, there is no transition.
    case HSV_TO_HSV:
    case CIE_XY_TO_CIE_XY:
    case TEMPERATURE_TO_TEMPERATURE:
    default:
      return;
  }
}

static uint16_t findNewColorValueFromStep(uint16_t oldValue, int16_t step)
{
  uint16_t newValue;

  if (step < 0) {
    step = step * -1;
    newValue = oldValue - (uint16_t) step;

    if (newValue > oldValue) {
      newValue = 0;
    }
  } else {
    newValue = oldValue + (uint16_t) step;

    if (newValue < oldValue) {
      newValue = MAX_CIE_XY_VALUE;
    }
  }

  return newValue;
}

static uint8_t addHue(uint8_t hue1, uint8_t hue2)
{
  uint16_t hue16;

  hue16 =  ((uint16_t) hue1);
  hue16 += ((uint16_t) hue2);

  if (hue16 > MAX_HUE_VALUE) {
    hue16 -= MAX_HUE_VALUE;
  }

  return ((uint8_t) hue16);
}

static uint8_t subtractHue(uint8_t hue1, uint8_t hue2)
{
  uint16_t hue16;

  hue16 =  ((uint16_t) hue1);
  if (hue2 > hue1) {
    hue16 += MAX_HUE_VALUE;
  }

  hue16 -= ((uint16_t) hue2);

  return ((uint8_t) hue16);
}

static uint8_t addSaturation(uint8_t saturation1, uint8_t saturation2)
{
  uint16_t saturation16;

  saturation16 =  ((uint16_t) saturation1);
  saturation16 += ((uint16_t) saturation2);

  if (saturation16 > MAX_SATURATION_VALUE) {
    saturation16 = MAX_SATURATION_VALUE;
  }

  return ((uint8_t) saturation16);
}
static uint8_t subtractSaturation(uint8_t saturation1, uint8_t saturation2)
{
  if (saturation2 > saturation1) {
    return 0;
  }

  return saturation1 - saturation2;
}

static bool computeNewHueValue(ColorHueTransitionState *p)
{
  uint32_t newHue32;
  uint8_t newHue;

  // exit with a false if hue is not currently moving
  if (p->stepsRemaining == 0) {
    return false;
  }

  (p->stepsRemaining)--;

  if (p->repeat == false) {
    writeRemainingTime(p->endpoint, p->stepsRemaining);
  }

  // are we going up or down?
  if (p->finalHue == p->currentHue) {
    // do nothing
  } else if (p->up) {
    newHue32 = (uint32_t) subtractHue(p->finalHue, p->initialHue);
    newHue32 *= ((uint32_t) (p->stepsRemaining));
    newHue32 /= ((uint32_t) (p->stepsTotal));
    p->currentHue = subtractHue((uint8_t) p->finalHue,
                                (uint8_t) newHue32);
  } else {
    newHue32 = (uint32_t) subtractHue(p->initialHue, p->finalHue);
    newHue32 *= ((uint32_t) (p->stepsRemaining));
    newHue32 /= ((uint32_t) (p->stepsTotal));

    p->currentHue = addHue((uint8_t) p->finalHue,
                           (uint8_t) newHue32);
  }

  if (p->stepsRemaining == 0) {
    if (p->repeat == false) {
      // we are performing a move to and not a move.
      return true;
    } else {
      // we are performing a Hue move.  Need to compute the new values for the
      // next move period.
      if (p->up) {
        newHue = subtractHue(p->finalHue, p->initialHue);
        newHue = addHue(p->finalHue, newHue);

        p->initialHue = p->finalHue;
        p->finalHue = newHue;
      } else {
        newHue = subtractHue(p->initialHue, p->finalHue);
        newHue = subtractHue(p->finalHue, newHue);

        p->initialHue = p->finalHue;
        p->finalHue = newHue;
      }
      p->stepsRemaining = TRANSITION_TIME_1S;
    }
  }
  return false;
}

void emZclColorControlServerHueSatTransitionEventHandler(void)
{
  uint8_t endpoint = colorHueTransitionState.endpoint;
  boolean limitReached1, limitReached2;

  limitReached1 = computeNewHueValue(&colorHueTransitionState);
  limitReached2 = computeNewColor16uValue(&colorSaturationTransitionState);

  if (limitReached1 || limitReached2) {
    stopAllColorTransitions();
  } else {
    emberEventControlSetDelayMS(COLOR_HSV_CONTROL, UPDATE_TIME_MS);
  }

  writeHue(colorHueTransitionState.endpoint,
           colorHueTransitionState.currentHue);
  writeSaturation(colorSaturationTransitionState.endpoint,
                  (uint8_t) colorSaturationTransitionState.currentValue);

  emberAfCorePrintln("Hue %d Saturation %d endpoint %d",
                     colorHueTransitionState.currentHue,
                     colorSaturationTransitionState.currentValue,
                     endpoint);

  emberAfPluginColorControlServerComputePwmFromHsvCallback(endpoint);
}

// Return value of true means we need to stop.
static bool computeNewColor16uValue(Color16uTransitionState *p)
{
  uint32_t newValue32u;

  if (p->stepsRemaining == 0) {
    return false;
  }

  (p->stepsRemaining)--;

  writeRemainingTime(p->endpoint, p->stepsRemaining);

  // handle sign
  if (p->finalValue == p->currentValue) {
    // do nothing
  } else if (p->finalValue > p->initialValue) {
    newValue32u = ((uint32_t) (p->finalValue - p->initialValue));
    newValue32u *= ((uint32_t) (p->stepsRemaining));
    newValue32u /= ((uint32_t) (p->stepsTotal));
    p->currentValue = p->finalValue - ((uint16_t) (newValue32u));
  } else {
    newValue32u = ((uint32_t) (p->initialValue - p->finalValue));
    newValue32u *= ((uint32_t) (p->stepsRemaining));
    newValue32u /= ((uint32_t) (p->stepsTotal));
    p->currentValue = p->finalValue + ((uint16_t) (newValue32u));
  }

  if (p->stepsRemaining == 0) {
    // we have completed our move.
    return true;
  }

  return false;
}

static uint16_t computeTransitionTimeFromStateAndRate(Color16uTransitionState *p,
                                                      uint16_t rate)
{
  uint32_t transitionTime;
  uint16_t max, min;

  if (p->currentValue > p->finalValue) {
    max = p->currentValue;
    min = p->finalValue;
  } else {
    max = p->finalValue;
    min = p->currentValue;
  }

  transitionTime = max - min;
  transitionTime *= 10;
  transitionTime /= rate;

  if (transitionTime > MAX_INT16U_VALUE) {
    return MAX_INT16U_VALUE;
  }

  return (uint16_t) transitionTime;
}

void emZclColorControlServerXyTransitionEventHandler(void)
{
  uint8_t endpoint = colorXTransitionState.endpoint;
  boolean limitReachedX, limitReachedY;

  // compute new values for X and Y.
  limitReachedX = computeNewColor16uValue(&colorXTransitionState);

  limitReachedY = computeNewColor16uValue(&colorYTransitionState);

  if (limitReachedX || limitReachedY) {
    stopAllColorTransitions();
  } else {
    emberEventControlSetDelayMS(COLOR_XY_CONTROL, UPDATE_TIME_MS);
  }

  // update the attributes
  writeColorX(colorXTransitionState.endpoint,
              colorXTransitionState.currentValue);
  writeColorY(colorXTransitionState.endpoint,
              colorYTransitionState.currentValue);

  emberAfCorePrintln("Color X %d Color Y %d",
                     colorXTransitionState.currentValue,
                     colorYTransitionState.currentValue);

  emberAfPluginColorControlServerComputePwmFromXyCallback(endpoint);
}

void emZclColorControlServerTempTransitionEventHandler(void)
{
  uint8_t endpoint = colorTempTransitionState.endpoint;
  boolean limitReached;

  limitReached = computeNewColor16uValue(&colorTempTransitionState);

  if (limitReached) {
    stopAllColorTransitions();
  } else {
    emberEventControlSetDelayMS(COLOR_TEMP_CONTROL, UPDATE_TIME_MS);
  }

  writeColorTemperature(colorTempTransitionState.endpoint,
                        colorTempTransitionState.currentValue);

  emberAfCorePrintln("Color Temperature %d",
                     colorTempTransitionState.currentValue);

  emberAfPluginColorControlServerComputePwmFromTempCallback(endpoint);
}

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void emZclColorControlServerEraseSceneHandler(uint8_t tableIdx)
{
  EmZclColorControlSceneSubTableEntry_t entry;

  entry.hasCurrentXValue = false;
  entry.hasCurrentYValue = false;
  entry.hasEnhancedCurrentHueValue = false;
  entry.hasCurrentSaturationValue = false;
  entry.hasColorLoopActiveValue = false;
  entry.hasColorLoopDirectionValue = false;
  entry.hasColorLoopTimeValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool emZclColorControlServerAddSceneHandler(EmberZclClusterId_t clusterId,
                                            uint8_t tableIdx,
                                            const uint8_t *sceneData,
                                            uint8_t length)
{
  if (clusterId == EMBER_ZCL_CLUSTER_COLOR_CONTROL) {
    if (length < 2) {
      return false; // ext field format error (currentXValue bytes must be present, other bytes optional).
    }

    // Extract bytes from input data block and update scene subtable fields.
    EmZclColorControlSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasCurrentXValue = true;
    entry.currentXValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
    length -= 2;
    if (length >= 2) {
      entry.hasCurrentYValue = true;
      entry.currentYValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
      length -= 2;
      if (length >= 2) {
        entry.hasEnhancedCurrentHueValue = true;
        entry.enhancedCurrentHueValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
        length -= 2;
        if (length >= 1) {
          entry.hasCurrentSaturationValue = true;
          entry.currentSaturationValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
          length--;
          if (length >= 1) {
            entry.hasColorLoopActiveValue = true;
            entry.colorLoopActiveValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
            length--;
            if (length >= 1) {
              entry.hasColorLoopDirectionValue = true;
              entry.colorLoopDirectionValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
              length--;
              if (length >= 2) {
                entry.hasColorLoopTimeValue = true;
                entry.colorLoopTimeValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
              }
            }
          }
        }
      }
    }

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void emZclColorControlServerRecallSceneHandler(EmberZclEndpointId_t endpointId,
                                               uint8_t tableIdx,
                                               uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the color control cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant color control command handler to actually change the
  // hw state at the rate specified by the transition time.

  EmZclColorControlSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasCurrentXValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X,
                           (uint8_t *)&entry.currentXValue,
                           sizeof(entry.currentXValue));
  }
  if (entry.hasCurrentYValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y,
                           (uint8_t *)&entry.currentYValue,
                           sizeof(entry.currentYValue));
  }

  // Per ZLL CCB, value of 0 for X and Y means enhanced scene attributes will be used.
  if ((entry.hasCurrentXValue)
      && (entry.hasCurrentYValue)
      && (entry.currentXValue == 0)
      && (entry.currentYValue == 0)) {
    if (entry.hasEnhancedCurrentHueValue) {
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterColorControlServerSpec,
                             EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_ENHANCED_CURRENT_HUE,
                             (uint8_t *)&entry.enhancedCurrentHueValue,
                             sizeof(entry.enhancedCurrentHueValue));
    }
    if (entry.hasCurrentSaturationValue) {
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterColorControlServerSpec,
                             EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION,
                             (uint8_t *)&entry.currentSaturationValue,
                             sizeof(entry.currentSaturationValue));
    }
    if (entry.hasColorLoopActiveValue) {
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterColorControlServerSpec,
                             EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_ACTIVE,
                             (uint8_t *)&entry.colorLoopActiveValue,
                             sizeof(entry.colorLoopActiveValue));
    }
    if (entry.hasColorLoopDirectionValue) {
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterColorControlServerSpec,
                             EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_DIRECTION,
                             (uint8_t *)&entry.colorLoopDirectionValue,
                             sizeof(entry.colorLoopDirectionValue));
    }
    if (entry.hasColorLoopTimeValue) {
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterColorControlServerSpec,
                             EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_TIME,
                             (uint8_t *)&entry.colorLoopTimeValue,
                             sizeof(entry.colorLoopTimeValue));
    }
  }
}

void emZclColorControlServerStoreSceneHandler(EmberZclEndpointId_t endpointId,
                                              uint8_t tableIdx)
{
  EmZclColorControlSceneSubTableEntry_t entry;

  entry.hasCurrentXValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X,
                           (uint8_t *)&entry.currentXValue,
                           sizeof(entry.currentXValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasCurrentYValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y,
                           (uint8_t *)&entry.currentYValue,
                           sizeof(entry.currentYValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasEnhancedCurrentHueValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_ENHANCED_CURRENT_HUE,
                           (uint8_t *)&entry.enhancedCurrentHueValue,
                           sizeof(entry.enhancedCurrentHueValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasCurrentSaturationValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION,
                           (uint8_t *)&entry.currentSaturationValue,
                           sizeof(entry.currentSaturationValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasColorLoopActiveValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_ACTIVE,
                           (uint8_t *)&entry.colorLoopActiveValue,
                           sizeof(entry.colorLoopActiveValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasColorLoopDirectionValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_DIRECTION,
                           (uint8_t *)&entry.colorLoopDirectionValue,
                           sizeof(entry.colorLoopDirectionValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasColorLoopTimeValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterColorControlServerSpec,
                           EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_LOOP_TIME,
                           (uint8_t *)&entry.colorLoopTimeValue,
                           sizeof(entry.colorLoopTimeValue)) == EMBER_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void emZclColorControlServerCopySceneHandler(uint8_t srcTableIdx,
                                             uint8_t dstTableIdx)
{
  EmZclColorControlSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void emZclColorControlServerViewSceneHandler(uint8_t tableIdx,
                                             uint8_t **ppExtFldData)
{
  EmZclColorControlSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if ((entry.hasCurrentXValue)
      || (entry.hasCurrentYValue)
      || (entry.hasEnhancedCurrentHueValue)
      || (entry.hasCurrentSaturationValue)
      || (entry.hasColorLoopActiveValue)
      || (entry.hasColorLoopDirectionValue)
      || (entry.hasColorLoopTimeValue)) {
    emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                EMBER_ZCL_CLUSTER_COLOR_CONTROL);

    uint8_t *pLength = *ppExtFldData;  // Save pointer to length byte.
    *pLength = 0;
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, *pLength); // Insert temporary length value.

    if (entry.hasCurrentXValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.currentXValue);
    }
    if (entry.hasCurrentYValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.currentYValue);
    }
    if (entry.hasEnhancedCurrentHueValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.enhancedCurrentHueValue);
    }
    if (entry.hasCurrentSaturationValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.currentSaturationValue);
    }
    if (entry.hasColorLoopActiveValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.colorLoopActiveValue);
    }
    if (entry.hasColorLoopDirectionValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.colorLoopDirectionValue);
    }
    if (entry.hasColorLoopTimeValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.colorLoopTimeValue);
    }

    // Update length byte value.
    *pLength = *ppExtFldData - pLength - 1;
  }
}

void emZclColorControlServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  EmZclColorControlSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  emberAfCorePrint(" color:%2x %2x",
                   entry.currentXValue,
                   entry.currentYValue);

  emberAfCorePrint(" %2x %x %x %x %2x",
                   entry.enhancedCurrentHueValue,
                   entry.currentSaturationValue,
                   entry.colorLoopActiveValue,
                   entry.colorLoopDirectionValue,
                   entry.colorLoopTimeValue);
  emberAfCoreFlush();
}
#endif
