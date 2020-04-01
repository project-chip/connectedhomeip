/***************************************************************************//**
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
