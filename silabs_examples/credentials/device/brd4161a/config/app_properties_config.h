/***************************************************************************//**
 * @file
 * @brief Application Properties Header File
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

#ifndef APP_PROPERTIES_CONFIG_H
#define APP_PROPERTIES_CONFIG_H

#include "sl_application_type.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h>App Properties settings

// Type of signature this application is signed with
// Default: APPLICATION_SIGNATURE_NONE(0)
#define SL_APPLICATION_SIGNATURE               0

// Location of the signature
// Default: 0xFFFFFFFF
#define SL_APPLICATION_SIGNATURE_LOCATION      0xFFFFFFFF

// Bitfield representing type of application
#define SL_APPLICATION_TYPE                    APPLICATION_TYPE

// <o SL_APPLICATION_VERSION> Version number for this application
// <0-4294967295:1>
// <i> Default: 1 [0-4294967295]
#define SL_APPLICATION_VERSION                 1

// Capabilities of this application
// Default: 0
#define SL_APPLICATION_CAPABILITIES            0

//Product ID of the device for which the application is built
#define SL_APPLICATION_PRODUCT_ID             { 0 }

// </h>

#endif // APP_PROPERTIES_CONFIG_H
