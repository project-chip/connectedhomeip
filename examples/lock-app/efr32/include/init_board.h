/***************************************************************************//**
 * @file
 * @brief init_board.h
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

#ifndef INIT_BOARD_H
#define INIT_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

void initBoard(void);
void initVcomEnable(void);

#ifdef __cplusplus
}
#endif

#endif // INIT_BOARD_H
