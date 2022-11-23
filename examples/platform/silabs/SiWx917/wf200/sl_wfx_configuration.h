/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
