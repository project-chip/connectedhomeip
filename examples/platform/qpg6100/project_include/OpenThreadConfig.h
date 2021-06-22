/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      Overrides to default OpenThread configuration.
 *
 */

#pragma once

// Disable the Qorvo-supplied OpenThread logging facilities and use
// the facilities provided by the Device Layer (see
// src/platform/qpg6100/Logging.cpp).
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP

// Turn on a moderate level of logging in OpenThread
// Enable use of external heap allocator (calloc/free) for OpenThread.
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1

#if CHIP_DEVICE_CONFIG_THREAD_FTD
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 22
#else
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 10
#endif

// Limit CLI buffers
#define OPENTHREAD_CONFIG_CLI_UART_RX_BUFFER_SIZE 384
#define OPENTHREAD_CONFIG_CLI_UART_TX_BUFFER_SIZE 512

#define OPENTHREAD_CONFIG_JOINER_ENABLE 1
#define OPENTHREAD_CONFIG_NCP_HDLC_ENABLE 1
#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1

#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_ECDSA_ENABLE 1

// Use the Qorvo-supplied default platform configuration for remainder
// of OpenThread config options.
//
// NB: This file gets included during the build of OpenThread.  Hence
// it cannot use "openthread" in the path to the included file.
//
#include "openthread-core-qpg6100-config.h"
