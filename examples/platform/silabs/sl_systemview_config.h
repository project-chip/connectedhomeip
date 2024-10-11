/**
 * @file
 * @brief Segger System View configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon
 *Laboratories Inc.
 *
 * This software is provided 'as-is', without any
 *express or implied warranty. In no event will the
 *authors be held liable for any damages arising from
 *the use of this software.
 *
 * Permission is granted to anyone to use this
 *software for any purpose, including commercial
 *applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be
 *misrepresented; you must not claim that you wrote
 *the original software. If you use this software in a
 *product, an acknowledgment in the product
 *documentation would be appreciated but is not
 *required.
 * 2. Altered source versions must be plainly marked
 *as such, and must not be misrepresented as being the
 *original software.
 * 3. This notice may not be removed or altered from
 *any source distribution.
 *
 ******************************************************************************/

// Overwirte SystemView config for GN build, SHOULD NOT BE USED WITH SLC!!!

#pragma once

#define SEGGER_SYSVIEW_TIMESTAMP_SOURCE_DWT 0
#define SEGGER_SYSVIEW_TIMESTAMP_SOURCE_SLEEPTIMER 1

// <o SEGGER_SYSVIEW_TIMESTAMP_SOURCE> Source for the time stamps
//   <SEGGER_SYSVIEW_TIMESTAMP_SOURCE_DWT=> DWT Cycle Counter
//   <SEGGER_SYSVIEW_TIMESTAMP_SOURCE_SLEEPTIMER=> Sleep Timer
// <i> Source for time stamps. Only meaningful when no OS is present. When an OS is present, time stamp is provided by the OS.
// <i> SEGGER_SYSVIEW_TIMESTAMP_SOURCE_DWT is not available on Cortex-M0+ and cannot be selected.
// <i> Default: SEGGER_SYSVIEW_TIMESTAMP_SOURCE_SLEEPTIMER
#define SEGGER_SYSVIEW_TIMESTAMP_SOURCE SEGGER_SYSVIEW_TIMESTAMP_SOURCE_SLEEPTIMER

// <o SEGGER_SYSVIEW_RTT_BUFFER_SIZE> RTT buffer size (in bytes)
// <i> Default: 1024
#define SEGGER_SYSVIEW_RTT_BUFFER_SIZE 8192

// <o SEGGER_SYSVIEW_RTT_CHANNEL> RTT channel for SystemView
// <i> Default: 1
#define SEGGER_SYSVIEW_RTT_CHANNEL 1

// <q SEGGER_SYSVIEW_USE_STATIC_BUFFER> Use static buffer to generate events
// <i> Determines if a single static buffer is used to generate the events. If disabled, the buffer is allocated on the stack.
// <i> Default: 1
#define SEGGER_SYSVIEW_USE_STATIC_BUFFER 1

// <q SEGGER_SYSVIEW_POST_MORTEM_MODE> Enable post mortem mode
// <i> Default: 0
#define SEGGER_SYSVIEW_POST_MORTEM_MODE 0

// <q SEGGER_SYSVIEW_CAN_RESTART> Enable SystemView restart
// <i> If enabled, start sequence is sent on every startup. It is not recommended to disabled this feature.
// <i> Default: 1
#define SEGGER_SYSVIEW_CAN_RESTART 1

// <q SEGGER_SYSVIEW_ID_SHIFT> Number of bits to shift the Id to save bandwidth. (i.e. 2 when Ids are 4 byte aligned)
// <i> Default: 0
#define SEGGER_SYSVIEW_ID_SHIFT 0

// <<< end of configuration section >>>
