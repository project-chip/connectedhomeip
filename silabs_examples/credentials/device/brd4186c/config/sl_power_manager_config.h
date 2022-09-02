/***************************************************************************//**
 * @file
 * @brief Power Manager configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_POWER_MANAGER_CONFIG_H
#define SL_POWER_MANAGER_CONFIG_H

// <h>Power Manager Configuration

// <q SL_POWER_MANAGER_CUSTOM_HF_OSCILLATOR_IRQ_HANDLER> Enable custom IRQ handler for external HF oscillator.
// <i> Enable if CMU_IRQHandler/HFXO0_IRQHandler is needed from your application.
// <i> The function sl_power_manager_irq_handler() will have to be called from you custom handler if this is enabled.
// <i> Default: 0
#define SL_POWER_MANAGER_CUSTOM_HF_OSCILLATOR_IRQ_HANDLER  0

// <o SL_POWER_MANAGER_LOWEST_EM_ALLOWED> Lowest Energy mode allowed
// <1=> EM1
// <2=> EM2
// <3=> EM3
// <i> Default: 2
#define SL_POWER_MANAGER_LOWEST_EM_ALLOWED   2

// <q SL_POWER_MANAGER_CONFIG_VOLTAGE_SCALING_FAST_WAKEUP> Enable fast wakeup (disable voltage scaling in EM2/3 mode)
// <i> Enable or disable voltage scaling in EM2/3 modes (when available). This decreases wakeup time by about 30 us.
// <i> Deprecated. It is replaced by the function sl_power_manager_em23_voltage_scaling_enable_fast_wakeup()
// <i> Default: 0
#define SL_POWER_MANAGER_CONFIG_VOLTAGE_SCALING_FAST_WAKEUP   0

// <e SL_POWER_MANAGER_DEBUG> Enable debugging feature
// <i> Enable or disable debugging features (trace the different modules that have requirements).
// <i> Default: 0
#define SL_POWER_MANAGER_DEBUG  0

// <o SL_POWER_MANAGER_DEBUG_POOL_SIZE> Maximum numbers of requirements that can be logged
// <i> Default: 10
#define SL_POWER_MANAGER_DEBUG_POOL_SIZE  10
// </e>

// </h>

#endif /* SL_POWER_MANAGER_CONFIG_H */

// <<< end of configuration section >>>
