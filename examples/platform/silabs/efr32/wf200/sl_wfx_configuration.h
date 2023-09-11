/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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
