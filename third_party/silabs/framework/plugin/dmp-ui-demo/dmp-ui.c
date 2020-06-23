/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief User Interface rendering for DMP demo
 *******************************************************************************
   ******************************************************************************/

#include "hal/plugin/glib/graphics.h"
#include "em_types.h"
#include "glib.h"
#include "dmd/dmd.h"
#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include <string.h>
#include <stdio.h>
#include "dmp-bitmaps.h"
#include "dmp-ui.h"
#include "app/framework/include/af.h"

#define helpmenu_line1_light     "      **HELP**       "
#define helpmenu_line2_light     "PB0 - Toggle Light   "
#define helpmenu_line3_light     "PB1 - NWK Control    "
#define helpmenu_line4_light     " No NWK : Form NWK   "
#define helpmenu_line5_light     " NWK    : Permit join"
#define helpmenu_line6_light     " Press>3s: Leave NWK "

#define helpmenu_line1_lightSed  "      **HELP**       "
#define helpmenu_line2_lightSed  "PB0 - Toggle Light   "
#define helpmenu_line3_lightSed  "PB1 - NWK Control    "
#define helpmenu_line4_lightSed  " No NWK : Join NWK   "
#define helpmenu_line5_lightSed  " NWK: Identify       "
#define helpmenu_line6_lightSed  " Press>3s: Leave NWK "

#define helpmenu_line1_switch    "      **HELP**       "
#define helpmenu_line2_switch    "PB0/1-No NWK:Join NWK"
#define helpmenu_line3_switch    "PB0/1<3s:Toggle Light"
#define helpmenu_line4_switch    "                     "
#define helpmenu_line5_switch    "PB0>3s:Reset Bindings"
#define helpmenu_line6_switch    "PB1>3s:Leave NWK     "

#define TMP_STR_LEN        9
#define BLE_NAME_LEN       10
#define APP_NAME_LEN       20
#define DEV_NAME_LEN       20

extern EmberEventControl lcdPermitJoinEventControl;
extern GLIB_Context_t glibContext;          /* Global glib context */

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static bool isBluetoothConnected = false;
static DmpUiLightState_t light1State = DMP_UI_LIGHT_OFF;
static DmpUiLightState_t light2State = DMP_UI_LIGHT_OFF;
static DmpUiLightDirection_t lightDirection = DMP_UI_DIRECTION_INVALID;
static bool helpMenuDisplayed = false;
static uint16_t dmpUiDirectDisplayStartTime = 0;
static char bleName[BLE_NAME_LEN] = { 0 };
static char deviceName[DEV_NAME_LEN];
static bool eventTimeLeft = false;
static bool panIdDisplayToggle = false;
static bool blockPanIdDisplay = false;
static uint8_t numOfLightsDiscovered = 0;
static DmpUiDeviceType_t deviceType = DMP_UI_DEVICE_TYPE_UNKNOWN;
DmpUiZigBeeNetworkState_t last_nwState = DMP_UI_STATE_UNKNOWN;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void dmpUiUpdateZigbeeStatus(DmpUiZigBeeNetworkState_t nwState,
                                    bool withDisplayUpdate)
{
  int32_t xPosition = 2;
  char tempStr[TMP_STR_LEN] = { 0 };

  if (!helpMenuDisplayed) {
    EmberPanId panId = emberAfGetPanId();

    if (nwState == DMP_UI_STATE_UNKNOWN) {
      if (panId == 0xFFFF) {
        switch (last_nwState) {
          case DMP_UI_JOINING:
          case DMP_UI_FORMING:
            nwState = last_nwState;
            break;
          default:
            nwState = DMP_UI_NO_NETWORK;
            break;
        }
      } else {
        EmberNetworkStatus status = emberAfNetworkState();
        switch (status) {
          case EMBER_JOINED_NETWORK:
            nwState = DMP_UI_NETWORK_UP;
            break;
          case EMBER_JOINED_NETWORK_NO_PARENT:
            nwState = DMP_UI_LOST_NETWORK;
            break;
          default:
            nwState = DMP_UI_NO_NETWORK;
            break;
        }
      }
    }

    if (dmpUiGetNumLightsDiscovered() >= 2) {
      xPosition = ((glibContext.pDisplayGeometry->xSize - ZIGBEE_BITMAP_WIDTH) / 2) - 14;
    }

    switch (nwState) {
      case DMP_UI_NO_NETWORK:
        dmpUiDirectDisplayStartTime = 0;
        snprintf(tempStr, TMP_STR_LEN, "No Nwk");
        break;
      case DMP_UI_LOST_NETWORK:
        dmpUiDirectDisplayStartTime = 0;
        snprintf(tempStr, TMP_STR_LEN, "Lost Nwk");  // (== on Nwk but No Parent)
        break;
      case DMP_UI_SCANNING:
        snprintf(tempStr, TMP_STR_LEN, "Scanning");
        break;
      case DMP_UI_JOINING:
        snprintf(tempStr, TMP_STR_LEN, "Joining");
        break;
      case DMP_UI_FORMING:
        snprintf(tempStr, TMP_STR_LEN, "Forming");
        break;
      case DMP_UI_DISCOVERING:
        snprintf(tempStr, TMP_STR_LEN, "Discvrng");
        break;
      case DMP_UI_NETWORK_UP:
        snprintf(tempStr, TMP_STR_LEN, "PAN:%04X", panId);
        break;
    }

    GLIB_drawString(&glibContext, tempStr,
                    strlen(tempStr) + 1, xPosition, glibContext.pDisplayGeometry->ySize - 10, 0);

    if (withDisplayUpdate) {
      DMD_updateDisplay();
    }
    last_nwState = nwState;
  }
}

static void dmpUiDisplayLogo(void)
{
  GLIB_drawBitmap(&glibContext,
                  SILICONLABS_X_POSITION,
                  SILICONLABS_Y_POSITION,
                  SILICONLABS_BITMAP_WIDTH,
                  SILICONLABS_BITMAP_HEIGHT,
                  siliconlabsBitmap);
}

static void dmpUiDisplayZigbeeLogo(void)
{
  int32_t xPosition = ZIGBEE_X_POSITION;
  int32_t yPosition = ZIGBEE_Y_POSITION;

  if (dmpUiGetNumLightsDiscovered() >= 2) {
    xPosition = ((glibContext.pDisplayGeometry->xSize - ZIGBEE_BITMAP_WIDTH) / 2);
    yPosition = ZIGBEE_Y_POSITION + 10;
  }
  GLIB_drawBitmap(&glibContext,
                  xPosition,
                  yPosition,
                  ZIGBEE_BITMAP_WIDTH,
                  ZIGBEE_BITMAP_HEIGHT,
                  zigbeeBitmap);

  if (!blockPanIdDisplay) {
    dmpUiUpdateZigbeeStatus(DMP_UI_STATE_UNKNOWN, false);
  }
}

static void dmpUiDisplayBluetoothLogo(void)
{
  if (strlen(bleName)) {
    GLIB_drawString(&glibContext, bleName,
                    strlen(bleName) + 1, 79, glibContext.pDisplayGeometry->ySize - 10, 0);
  }

  if (isBluetoothConnected) {
    GLIB_drawBitmap(&glibContext,
                    BLUETOOTH_X_POSITION,
                    BLUETOOTH_Y_POSITION,
                    BLUETOOTH_BITMAP_WIDTH,
                    BLUETOOTH_BITMAP_HEIGHT,
                    bluetoothConnectedBitmap);
  } else {
    GLIB_drawBitmap(&glibContext,
                    BLUETOOTH_X_POSITION,
                    BLUETOOTH_Y_POSITION,
                    BLUETOOTH_BITMAP_WIDTH,
                    BLUETOOTH_BITMAP_HEIGHT,
                    bluetoothBitmap);
  }
}

static void dmpUiDisplayAppName(const char *device)
{
  char appName[APP_NAME_LEN];
  snprintf(appName, APP_NAME_LEN, "DMP Demo %s", device);

  GLIB_drawString(&glibContext, appName,
                  strlen(appName) + 1, 16, SILICONLABS_BITMAP_HEIGHT + 2, 0);
}

static void dmpUiDisplayDirection(DmpUiLightDirection_t direction)
{
  if (direction == DMP_UI_DIRECTION_ZIGBEE) {
    GLIB_drawLine(&glibContext,
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 5),
                  (ZIGBEE_Y_POSITION + ZIGBEE_BITMAP_HEIGHT / 2),
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 15),
                  (ZIGBEE_Y_POSITION + ZIGBEE_BITMAP_HEIGHT / 2));

    GLIB_drawLine(&glibContext,
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 12),
                  (ZIGBEE_Y_POSITION + (ZIGBEE_BITMAP_HEIGHT / 2) - 3),
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 15),
                  (ZIGBEE_Y_POSITION + ZIGBEE_BITMAP_HEIGHT / 2));

    GLIB_drawLine(&glibContext,
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 12),
                  (ZIGBEE_Y_POSITION + (ZIGBEE_BITMAP_HEIGHT / 2) + 3),
                  (ZIGBEE_X_POSITION + ZIGBEE_BITMAP_WIDTH + 15),
                  (ZIGBEE_Y_POSITION + (ZIGBEE_BITMAP_HEIGHT / 2)));
  } else if ((direction == DMP_UI_DIRECTION_BLUETOOTH)
             && ((deviceType == DMP_UI_DEVICE_TYPE_LIGHT || deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED))) {
    GLIB_drawLine(&glibContext,
                  (BLUETOOTH_X_POSITION - 5),
                  (BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2),
                  (BLUETOOTH_X_POSITION - 15),
                  (BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2));

    GLIB_drawLine(&glibContext,
                  ((BLUETOOTH_X_POSITION - 15) + 3),
                  ((BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2) - 3),
                  (BLUETOOTH_X_POSITION - 15),
                  (BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2));

    GLIB_drawLine(&glibContext,
                  ((BLUETOOTH_X_POSITION - 15) + 3),
                  ((BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2) + 3),
                  (BLUETOOTH_X_POSITION - 15),
                  (BLUETOOTH_Y_POSITION + BLUETOOTH_BITMAP_HEIGHT / 2));
  }

  lightDirection = direction;

  DMD_updateDisplay();
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
void dmpUiInit(void)
{
  GRAPHICS_Init();
  glibContext.backgroundColor = White;
  GLIB_clear(&glibContext);
  light1State = DMP_UI_LIGHT_OFF;
  light2State = DMP_UI_LIGHT_OFF;
  deviceType = (DmpUiDeviceType_t)DEVICE_TYPE;

  if (deviceType == DMP_UI_DEVICE_TYPE_LIGHT) {
    strncpy(deviceName, "Light", DEV_NAME_LEN);
  } else if (deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED) {
    strncpy(deviceName, "LightSed", DEV_NAME_LEN);
  } else if (deviceType == DMP_UI_DEVICE_TYPE_SWITCH) {
    strncpy(deviceName, "Switch", DEV_NAME_LEN);
  } else {
    strncpy(deviceName, "Unknown", DEV_NAME_LEN);
  }
}

void dmpUiLightOn(void)
{
  bool displayLight = false;
  helpMenuDisplayed = false;
  GLIB_clear(&glibContext);

  if ((dmpUiGetNumLightsDiscovered() > 0)
      || (deviceType != DMP_UI_DEVICE_TYPE_SWITCH)) {
    displayLight = true;
  }
  if (displayLight) {
    light1State = DMP_UI_LIGHT_ON;
    GLIB_drawBitmap(&glibContext,
                    LIGHT_X_POSITION,
                    LIGHT_Y_POSITION,
                    LIGHT_BITMAP_WIDTH,
                    LIGHT_BITMAP_HEIGHT,
                    lightOnBitMap);
  }
  dmpUiDisplayLogo();
  dmpUiDisplayAppName(deviceName);
  dmpUiDisplayZigbeeLogo();

  if ((deviceType == DMP_UI_DEVICE_TYPE_LIGHT) || (deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED)) {
    dmpUiDisplayBluetoothLogo();
  }

  DMD_updateDisplay();
}

void dmpUiLightUpdateLight(DmpUiLightState_t updateLight1, DmpUiLightState_t updateLight2)
{
  if (updateLight1 != DMP_UI_LIGHT_UNCHANGED) {
    light1State = updateLight1;
  }

  if (updateLight2 != DMP_UI_LIGHT_UNCHANGED) {
    light2State = updateLight2;
  }

  const uint8_t *picLight1 = (light1State == DMP_UI_LIGHT_ON) ? lightOnBitMap : lightOffBitMap;
  const uint8_t *picLight2 = (light2State == DMP_UI_LIGHT_ON) ? lightOnBitMap : lightOffBitMap;

  helpMenuDisplayed = false;
  GLIB_clear(&glibContext);
  GLIB_drawBitmap(&glibContext,
                  0,
                  LIGHT_Y_POSITION,
                  LIGHT_BITMAP_WIDTH,
                  LIGHT_BITMAP_HEIGHT,
                  picLight1);

  GLIB_drawBitmap(&glibContext,
                  64,
                  LIGHT_Y_POSITION,
                  LIGHT_BITMAP_WIDTH,
                  LIGHT_BITMAP_HEIGHT,
                  picLight2);

  dmpUiDisplayLogo();
  dmpUiDisplayAppName(deviceName);
  dmpUiDisplayZigbeeLogo();
  DMD_updateDisplay();
}

void dmpUiLightOff(void)
{
  bool displayLight = false;
  helpMenuDisplayed = false;
  GLIB_clear(&glibContext);

  if ((dmpUiGetNumLightsDiscovered() > 0)
      || (deviceType != DMP_UI_DEVICE_TYPE_SWITCH)) {
    displayLight = true;
  }
  if (displayLight) {
    light1State = DMP_UI_LIGHT_OFF;
    GLIB_drawBitmap(&glibContext,
                    LIGHT_X_POSITION,
                    LIGHT_Y_POSITION,
                    LIGHT_BITMAP_WIDTH,
                    LIGHT_BITMAP_HEIGHT,
                    lightOffBitMap);
  }
  dmpUiDisplayLogo();
  dmpUiDisplayAppName(deviceName);
  dmpUiDisplayZigbeeLogo();

  if ((deviceType == DMP_UI_DEVICE_TYPE_LIGHT) || (deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED)) {
    dmpUiDisplayBluetoothLogo();
  }

  DMD_updateDisplay();
}

void dmpUiDisplayHelp(void)
{
  uint8_t y_position = SILICONLABS_BITMAP_HEIGHT + 20;
  char *line1, *line2, *line3, *line4, *line5, *line6;

  helpMenuDisplayed = true;

  dmpUiDisplayLogo();
  dmpUiDisplayAppName(deviceName);
  if (deviceType == DMP_UI_DEVICE_TYPE_LIGHT) {
    line1 = helpmenu_line1_light;
    line2 = helpmenu_line2_light;
    line3 = helpmenu_line3_light;
    line4 = helpmenu_line4_light;
    line5 = helpmenu_line5_light;
    line6 = helpmenu_line6_light;
  } else if (deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED) {
    line1 = helpmenu_line1_lightSed;
    line2 = helpmenu_line2_lightSed;
    line3 = helpmenu_line3_lightSed;
    line4 = helpmenu_line4_lightSed;
    line5 = helpmenu_line5_lightSed;
    line6 = helpmenu_line6_lightSed;
  } else {
    line1 = helpmenu_line1_switch;
    line2 = helpmenu_line2_switch;
    line3 = helpmenu_line3_switch;
    line4 = helpmenu_line4_switch;
    line5 = helpmenu_line5_switch;
    line6 = helpmenu_line6_switch;
  }

  GLIB_drawString(&glibContext, line1, strlen(line1) + 1, 2, y_position, 0);
  GLIB_drawString(&glibContext, line2, strlen(line2) + 1, 2, y_position + 10, 0);
  GLIB_drawString(&glibContext, line3, strlen(line3) + 1, 2, y_position + 20, 0);
  GLIB_drawString(&glibContext, line4, strlen(line4) + 1, 2, y_position + 30, 0);
  GLIB_drawString(&glibContext, line5, strlen(line5) + 1, 2, y_position + 40, 0);
  GLIB_drawString(&glibContext, line6, strlen(line6) + 1, 2, y_position + 50, 0);

  DMD_updateDisplay();
}

void dmpUiZigBeePjoin(bool enable)
{
  eventTimeLeft = (enable) ? DMP_UI_PJOIN_EVENT_DURATION : 0;
  panIdDisplayToggle = false;
  blockPanIdDisplay = true;

  emberAfCustom2Println("dmpUiZigBeePjoin called, duration : %s", ((enable) ? "Enable" : "Disable"));

  emberEventControlSetDelayMS(lcdPermitJoinEventControl,
                              eventTimeLeft);
}

void dmpUiUpdateDirection(DmpUiLightDirection_t direction)
{
  emberAfCustom2Println("dmpUiUpdateDirection called, direction : %d", direction);

  dmpUiDisplayDirection(direction);

  dmpUiDirectDisplayStartTime = halCommonGetInt16uMillisecondTick();

  emberEventControlSetInactive(lcdPermitJoinEventControl);
  emberEventControlSetDelayMS(lcdPermitJoinEventControl, DMP_UI_PJOIN_EVENT_DURATION);
}

void dmpUiBluetoothConnected(bool connectionState)
{
  if (   (deviceType == DMP_UI_DEVICE_TYPE_LIGHT)
         || (deviceType == DMP_UI_DEVICE_TYPE_LIGHT_SED)
         && (!helpMenuDisplayed)) {
    isBluetoothConnected = connectionState;
    dmpUiDisplayBluetoothLogo();
    DMD_updateDisplay();
  }
}

void lcdPermitJoinEventHandler(void)
{
  if (!helpMenuDisplayed) {
    if (deviceType == DMP_UI_DEVICE_TYPE_LIGHT) {
      eventTimeLeft = emberGetPermitJoining();
    } else {
      //for non-COO devices, app will indicate when to stop
    }

    emberAfCustom2Println("lcdPermitJoinEventHandler - eventTimeLeft : %d", eventTimeLeft);

    dmpUiClrLcdDisplayMainScreen();

    if (panIdDisplayToggle) {
      dmpUiUpdateZigbeeStatus(DMP_UI_STATE_UNKNOWN, true);
    }
    panIdDisplayToggle = !panIdDisplayToggle; // toggle panId display flag.

    if (dmpUiDirectDisplayStartTime != 0) {
      if ((halCommonGetInt16uMillisecondTick() - dmpUiDirectDisplayStartTime) < 1000) {
        dmpUiDisplayDirection(lightDirection);
      } else {
        dmpUiDirectDisplayStartTime = 0;
      }
    }

    if (eventTimeLeft || (dmpUiDirectDisplayStartTime != 0)) {
      emberAfCustom2Println("lcdPermitJoinEventHandler - Event needs to be re-started, eventTimeLeft = %d, dmpUiDirectDisplayStartTime = %d",
                            eventTimeLeft, dmpUiDirectDisplayStartTime);
      emberEventControlSetDelayMS(lcdPermitJoinEventControl, DMP_UI_PJOIN_EVENT_DURATION);
    } else {
      blockPanIdDisplay = false;
      panIdDisplayToggle = false;
      dmpUiUpdateZigbeeStatus(DMP_UI_STATE_UNKNOWN, true);
      emberEventControlSetInactive(lcdPermitJoinEventControl);
      emberAfCustom2Println("lcdPermitJoinEventHandler - Event stopped");
    }
  } else {
    emberAfCustom2Println("Help Menu Displayed");
    emberEventControlSetDelayMS(lcdPermitJoinEventControl, DMP_UI_PJOIN_EVENT_DURATION);
  }
}

void dmpUiDisplayZigBeeState(DmpUiZigBeeNetworkState_t nwState)
{
  if (!helpMenuDisplayed) {
    bool restoreBlockPanId = blockPanIdDisplay;
    blockPanIdDisplay = true;

    emberAfCustom2Println("Help Menu Displayed");
    dmpUiClrLcdDisplayMainScreen();

    dmpUiUpdateZigbeeStatus(nwState, true);

    blockPanIdDisplay = restoreBlockPanId;
  }
}

void dmpUiClrLcdDisplayMainScreen(void)
{
  if (dmpUiGetNumLightsDiscovered() >= 2) {
    emberAfCustom2Println("dmpUiClrLcdDisplayMainScreen - demoUsesTwoLights");
    dmpUiLightUpdateLight(DMP_UI_LIGHT_UNCHANGED, DMP_UI_LIGHT_UNCHANGED);
  } else {
    emberAfCustom2Println("dmpUiClrLcdDisplayMainScreen");
    if (light1State == DMP_UI_LIGHT_OFF) {
      dmpUiLightOff();
    } else {
      dmpUiLightOn();
    }
  }
}

void dmpUiSetBleDeviceName(char *devName)
{
  strncpy(bleName, devName, BLE_NAME_LEN);
}

void dmpUiSetNumLightsDiscovered(uint8_t numberOfLights)
{
  numOfLightsDiscovered = numberOfLights;
}

uint8_t dmpUiGetNumLightsDiscovered(void)
{
  return numOfLightsDiscovered;
}
