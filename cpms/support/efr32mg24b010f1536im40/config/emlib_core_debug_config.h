/***************************************************************************//**
 * @file
 * @brief emlib_core Configuration
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

#ifndef EM_CORE_DEBUG_CONFIG_H
#define EM_CORE_DEBUG_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Core Configuration

// <q SL_EMLIB_CORE_ENABLE_INTERRUPT_DISABLED_TIMING> Enables measuring of interrupt disable time for debugging purposes.
// <i> Default: 0
// <i> If Enabled, either cycle_counter or systemview component must be added to project.
#define SL_EMLIB_CORE_ENABLE_INTERRUPT_DISABLED_TIMING    0

// </h>

// <<< end of configuration section >>>
#endif // EM_CORE_CONFIG_H
