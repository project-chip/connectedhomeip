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
 * @brief UI interface for DMP demo
 *******************************************************************************
   ******************************************************************************/

#ifndef DMP_UI_H
#define DMP_UI_H

/******************************************************************************/
/**
 * @addtogroup UI Interface for DMP Demo
 * @{
 *
 * DMP UI uses the underlying DMD interface and the GLIB and exposes several
 * wrapper functions to application. These functions are used to display
 * different bitmaps for the demo.
 *
 ******************************************************************************/

#define DMP_UI_PJOIN_EVENT_DURATION           (500)

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

typedef enum {
  DMP_UI_LIGHT_ON,
  DMP_UI_LIGHT_OFF,
  DMP_UI_LIGHT_UNCHANGED
} DmpUiLightState_t;

typedef enum {
  DMP_UI_DIRECTION_BLUETOOTH,
  DMP_UI_DIRECTION_ZIGBEE,
  DMP_UI_DIRECTION_SWITCH,
  DMP_UI_DIRECTION_INVALID
}DmpUiLightDirection_t;

typedef enum {
  DMP_UI_NO_NETWORK,
  DMP_UI_SCANNING,
  DMP_UI_JOINING,
  DMP_UI_FORMING,
  DMP_UI_NETWORK_UP,
  DMP_UI_LOST_NETWORK,
  DMP_UI_DISCOVERING,
  DMP_UI_STATE_UNKNOWN
}DmpUiZigBeeNetworkState_t;

typedef enum {
  DMP_UI_DEVICE_TYPE_LIGHT,
  DMP_UI_DEVICE_TYPE_SWITCH,
  DMP_UI_DEVICE_TYPE_LIGHT_SED,
  DMP_UI_DEVICE_TYPE_UNKNOWN
} DmpUiDeviceType_t;

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/

/**
 * @brief
 *   Initializes the GLIB and DMD interfaces.
 *
 * @param[in] void
 *
 * @return
 *      void
 */

void dmpUiInit (void);

/**
 * @brief
 *   Updates the display to light off bitmap. This function clears the display
 *   area and re-renders the LCD with the light off bitmap with other bitmaps.
 *
 * @param[in] void
 *
 * @return
 *      void
 */

void dmpUiLightOff(void);

/**
 * @brief
 *   Updates the display to light on bitmap. This function clears the display
 *   area and re-renders the LCD with the light on bitmap with other bitmaps.
 *
 * @param[in] void
 *
 * @return
 *      void
 */

void dmpUiLightOn(void);

/**
 * @brief
 *   Updates the display to show which interface toggled the light.
 *
 * @param[in] DMP_UI_DIRECTION_BLUETOOTH or DMP_UI_DIRECTION_ZIGBEE
 *
 * @return
 *      void
 */

void dmpUiUpdateDirection(DmpUiLightDirection_t direction);

/**
 * @brief
 *   Updates the display to show if the Bluetooth is connected to the mobile device.
 *
 * @param[in] bool, true if the Light is connected to mobile device, false otherwise.
 *
 * @return
 *      void
 */

void dmpUiBluetoothConnected(bool connectionState);

/**
 * @brief
 *   Updates the display with flashing PAN ID sequence to indicate that the pjoin
 * is active. The flashing sequence is stopped automatically once the pjoin
 * is disabled.
 *
 * @param[in] bool, indicating whether pjoin is being enabled.
 *
 * @return
 *      void
 */

void dmpUiZigBeePjoin(bool enable);

/**
 * @brief
 *   Updates the display with PAN ID, after the device joins or leaves the network.
 *
 * @param[in] DmpUiZigBeeNetworkState_t, based on the network state provided this
 * function will  display the corresponding nwk state on LCD. If DMP_UI_STATE_UNKNOWN
 * is passed, it will determine the network state internally and update the display
 * accordingly.
 *
 * @return
 *      void
 */

void dmpUiDisplayZigBeeState(DmpUiZigBeeNetworkState_t nwState);

/**
 * @brief
 *   Updates the display with Help menu.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
void dmpUiDisplayHelp(void);

/**
 * @brief
 *   Clears the LCD screen and display the main screen.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
void dmpUiClrLcdDisplayMainScreen(void);

/**
 * @brief
 *   Sets the BLE device name to be displayed on the LCD.
 *
 * @param[in] BLE device name.
 *
 * @return
 *      void
 */
void dmpUiSetBleDeviceName(char *devName);

/**
 * @brief
 *   Updates the number of lights discovered in the network.
 *
 * @param[in] uint8_t, number of lights dicovered in the nwk.
 *
 * @return
 *      void
 */
void dmpUiSetNumLightsDiscovered(uint8_t numberOfLights);

/**
 * @brief
 *   Returns number of lights discovered in the network.
 *
 * @param[in] void
 *
 * @return
 *      uint8_t, number of lights dicovered in the nwk.
 */
uint8_t dmpUiGetNumLightsDiscovered(void);

/**
 * @brief
 *   Updates the status of both the lights on LCD. If there is no status change
 *   application can pass DMP_UI_LIGHT_UNCHANGED as the state.
 * @param[in] DmpUiLightState_t, states of both the lights. If DMP_UI_LIGHT_UNCHANGED
 * is passed, previous light state is updated.
 *
 * @return
 *      void
 */

void dmpUiLightUpdateLight(DmpUiLightState_t updateLight1, DmpUiLightState_t updateLight2);
#endif //DMP_UI_H
