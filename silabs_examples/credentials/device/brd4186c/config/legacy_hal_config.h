/***************************************************************************//**
 * @file
 * @brief Legacy HAL configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef LEGACY_HAL_CONFIG_H
#define LEGACY_HAL_CONFIG_H

// <q LEGACY_HAL_TRANSLATE_BUTTON_INTERRUPT> Translate button interrupt callback
// <i> When the Simple Button component is included, it provides a callback for
// <i> buttons configured in interrupt mode. When this option is 1, Legacy HAL
// <i> will try to consume that callback and translate it to "halButtonIsr",
// <i> the legacy callback. If anything else in the application consumes the
// <i> Simple button callback, it will override Legacy HAL's version.
// <i> Default: 1
#define LEGACY_HAL_TRANSLATE_BUTTON_INTERRUPT    (1)

#endif /* LEGACY_HAL_CONFIG_H */

// <<< end of configuration section >>>
