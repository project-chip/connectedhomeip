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

// <o SL_DEVICE_INIT_EMU_EM4_VSCALE> EM4H Voltage scaling level
// <emuVScaleEM4H_FastWakeup=> Fast-wakeup voltage level
// <emuVScaleEM4H_LowPower=> Low-power optimized voltage level
//<i> Default: emuVScaleEM4H_LowPower
#define SL_DEVICE_INIT_EMU_EM4_VSCALE            emuVScaleEM4H_LowPower

// <q SL_DEVICE_INIT_EMU_EM4_RETAIN_LFXO> Retain LFXO in EM4
#define SL_DEVICE_INIT_EMU_EM4_RETAIN_LFXO         0

// <q SL_DEVICE_INIT_EMU_EM4_RETAIN_LFRCO> Retain LFRCO in EM4
#define SL_DEVICE_INIT_EMU_EM4_RETAIN_LFRCO        0

// <q SL_DEVICE_INIT_EMU_EM4_RETAIN_ULFRCO> Retain ULFRCO in EM4S
#define SL_DEVICE_INIT_EMU_EM4_RETAIN_ULFRCO       0

// <o SL_DEVICE_INIT_EMU_EM4_STATE> Hibernate or shutoff EM4 state
// <emuEM4Shutoff=> EM4 Shutoff
// <emuEM4Hibernate=> EM4 Hibernate
// <i> Default: emuEM4Shutoff
#define SL_DEVICE_INIT_EMU_EM4_STATE               emuEM4Shutoff

// <o SL_DEVICE_INIT_EMU_EM4_PIN_RETENTION_MODE> EM4 pin retention mode
// <emuPinRetentionDisable=> No Retention: Pads enter reset state when entering EM4.
// <emuPinRetentionEm4Exit=> Retention through EM4: Pads enter reset state when exiting EM4.
// <emuPinRetentionLatch=> Retention through EM4 and wakeup.
// <i> Default: emuPinRetentionDisable
#define SL_DEVICE_INIT_EMU_EM4_PIN_RETENTION_MODE  emuPinRetentionDisable

// <<< end of configuration section >>>

#endif // SL_DEVICE_INIT_EMU_CONFIG_H
