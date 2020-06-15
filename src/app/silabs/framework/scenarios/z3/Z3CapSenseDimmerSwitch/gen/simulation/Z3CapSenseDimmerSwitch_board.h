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
 * @brief  Sample API functions for
 *******************************************************************************
   ******************************************************************************/

#ifndef __BOARD_H__
#define __BOARD_H__

#define halInternalInitBoard()                         \
  do {                                                 \
    /*halInternalInitLed(); not currently simulated */ \
  } while (0)

#define halInternalPowerDownBoard()                    \
  do {                                                 \
    /*halInternalInitLed(); not currently simulated */ \
  } while (0)

#define halInternalPowerUpBoard() \
  do {                            \
  } while (0)

enum HalBoardLedPins {
  BOARDLED0 = 0,
  BOARDLED1 = 1,
  BOARDLED2 = 2,
  BOARDLED3 = 3,
  BOARDLED4 = 4,
  BOARDLED5 = 5,
  BOARD_ACTIVITY_LED  = BOARDLED0,
  BOARD_HEARTBEAT_LED = BOARDLED1
};

#define BUTTON0         0
#define BUTTON1         1
#define TEMP_SENSOR_ADC_CHANNEL 0
#define TEMP_SENSOR_SCALE_FACTOR 3

// This stuff is for the si446x simulation. These variables need to be declared
// somewhere in your simulation framework.
extern bool RF_NIRQ;
extern bool RF_NSEL;
extern bool RF_PWRDN;

#endif /* __BOARD_H__ */
