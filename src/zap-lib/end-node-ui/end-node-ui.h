/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#ifndef END_NODE_UI_H
#define END_NODE_UI_H

/** @brief Set UI LED
 *
 * This function can be used to set which LED should be used by the End Node
 * UI plugin to signify changes in network state.
 *
 * @param led This should be an LED defined in board.h.  When using a Silicon
 * Labs provided board header, this should be something like BOARDLED0 or
 * BOARD_ACTIVITY_LED.
 *
 */
void emberEndNodeUiSetUiLed(uint8_t led);

#endif // END_NODE_UI_H
