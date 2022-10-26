/***************************************************************************//**
 * @file
 * @brief DEVICE_INIT_EMU Config
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_DEVICE_INIT_EMU_CONFIG_H
#define SL_DEVICE_INIT_EMU_CONFIG_H

#include "em_emu.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <q> Allow debugger to remain connected in EM2
// <i> Force PD0B to stay on on EM2 entry. This allows the debugger to remain connected in EM2 and EM3.
// <i> Enabling debug connectivity results in an increased power consumption in EM2/EM3.
// <i> Default: 1
#define SL_DEVICE_INIT_EMU_EM2_DEBUG_ENABLE   1

// <o SL_DEVICE_INIT_EMU_EM4_PIN_RETENTION_MODE> EM4 pin retention mode
// <emuPinRetentionDisable=> No Retention: Pads enter reset state when entering EM4.
// <emuPinRetentionEm4Exit=> Retention through EM4: Pads enter reset state when exiting EM4.
// <emuPinRetentionLatch=> Retention through EM4 and wakeup.
// <i> Default: emuPinRetentionDisable
#define SL_DEVICE_INIT_EMU_EM4_PIN_RETENTION_MODE  emuPinRetentionDisable

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_EMU_CONFIG_H
