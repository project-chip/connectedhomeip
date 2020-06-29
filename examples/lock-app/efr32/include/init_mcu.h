/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
/*******************************************************************************
 * @file
 * @brief init_mcu.h
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

#ifndef INIT_MCU_H
#define INIT_MCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "board_features.h"
#include "hal-config-board.h"

/*
 * If Studio is allowed to use the CTUNE value from EEPROM on Silicon Labs radio board,
 * it will store it in the User page memory on the MFG_CTUNE_ADDR address.
 * If not, for example user already storing a value there, then the MFG_CTUNE_EN
 * must be set to 0, to avoid wrong value to be read out.
 * Studio can only read out the CTUNE value from the EEPROM if that is a
 * Silicon Labs radio board.
 * Please verify that CTUNE value is stored at the MFG_CTUNE_ADDR of
 * the User page, if want to overwrite the default MFG_CTUNE_EN define
 * for self use on custom boards.
 */

// This is a Silicon Labs radio board, the CTUNE value can be read out from EEPROM
#define MFG_CTUNE_EN 1

// Address for CTUNE in User page
#define MFG_CTUNE_ADDR 0x0FE00100UL
// Value of the CTUNE in User page
#define MFG_CTUNE_VAL (*((uint16_t *) (MFG_CTUNE_ADDR)))

void initMcu(void);

#ifdef __cplusplus
}
#endif

#endif // INIT_MCU_H
