/***************************************************************************//**
 * @file
 * @brief HFXO Manager configuration file.
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

#ifndef SL_HFXO_MANAGER_CONFIG_H
#define SL_HFXO_MANAGER_CONFIG_H

// <h>Power Manager Configuration

// <q SL_HFXO_MANAGER_CUSTOM_HFXO_IRQ_HANDLER> Enable custom IRQ handler for crystal HF oscillator.
// <i> Enable if HFXO0_IRQHandler is needed from your application.
// <i> The HFXO IRQ priority must not be changed as the HFXO Manager module needs it to be high priority
// <i> and to stay enabled through atomic sections.
// <i> The function sl_hfxo_manager_irq_handler() will have to be called from you custom handler if this is enabled.
// <i> Default: 0
#define SL_HFXO_MANAGER_CUSTOM_HFXO_IRQ_HANDLER  0

// <q SL_HFXO_MANAGER_SLEEPY_CRYSTAL_SUPPORT> Enable support for Sleepy Crystals.
// <i> If Enabled and if HFXO fails to startup due to a sleepy crystal, HFXO Manager will retry the startup with more aggressive settings
// <i> before falling back to the configured settings.
// <i> Default: 0
#define SL_HFXO_MANAGER_SLEEPY_CRYSTAL_SUPPORT  0

// </h>

#endif /* SL_HFXO_MANAGER_CONFIG_H */

// <<< end of configuration section >>>
