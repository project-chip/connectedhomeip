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
 * @brief Implements the color control server for color temperature bulbs.
 * Note: This is HA certifiable and has passed HA certification for at least
 * one customer project.
 *******************************************************************************
   ******************************************************************************/

// Nothing here for now..
#ifndef SILABS_LED_TEMP_PWM_H
#define SILABS_LED_TEMP_PWM_H

enum {
  EMBER_ZCL_COLOR_TEMP_MOVE_MODE_STOP = 0x00,
  EMBER_ZCL_COLOR_TEMP_MOVE_MODE_UP   = 0x01,
  EMBER_ZCL_COLOR_TEMP_MOVE_MODE_DOWN = 0x03,
};

#endif
