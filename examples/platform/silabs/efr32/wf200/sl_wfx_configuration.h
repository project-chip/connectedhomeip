/**
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#pragma once

// SL_WFX_DEFAULT_REQUEST_TIMEOUT_MS> Timeout period in milliseconds<250-10000>
// Default: 5000
// Timeout period in milliseconds for requests.
#define SL_WFX_DEFAULT_REQUEST_TIMEOUT_MS (5000)

// <h>WFx Secure Link configuration

// SL_WFX_SLK_CURVE25519> Use crypto curves
// Default: 1
// If this option is enabled ECDH crypto is used, KDF otherwise.
#define SL_WFX_SLK_CURVE25519 (1)
