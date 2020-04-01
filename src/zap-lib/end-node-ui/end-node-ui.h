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
