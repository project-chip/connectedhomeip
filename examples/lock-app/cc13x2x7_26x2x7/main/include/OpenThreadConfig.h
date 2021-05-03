/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#ifndef OPENTHREAD_PLATFORM_CONFIG_H
#define OPENTHREAD_PLATFORM_CONFIG_H

// Disable the Nordic-supplied OpenThread logging facilities and use
// the facilities provided by the CHIP Device Layer
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP

// When operating in a less than ideal RF environment, having a more forgiving configuration
// of OpenThread makes thread a great deal more reliable.
#define OPENTHREAD_CONFIG_TMF_ADDRESS_QUERY_MAX_RETRY_DELAY 120    // default is 28800
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_DIRECT 15  // default is 3
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_INDIRECT 1 // default is 0
#define OPENTHREAD_CONFIG_MAC_MAX_TX_ATTEMPTS_INDIRECT_POLLS 16    // default is 4

// Enable periodic parent search to speed up finding a better parent.
#define OPENTHREAD_CONFIG_PARENT_SEARCH_ENABLE 1                   // default is 0
#define OPENTHREAD_CONFIG_PARENT_SEARCH_RSS_THRESHOLD -45          // default is -65
#define OPENTHREAD_CONFIG_MLE_INFORM_PREVIOUS_PARENT_ON_REATTACH 1 // default is 0

// Use smaller maximum interval to speed up reattaching.
#define OPENTHREAD_CONFIG_MLE_ATTACH_BACKOFF_MAXIMUM_INTERVAL (60 * 10 * 1000) // default 1200000 ms

#define OPENTHREAD_CONFIG_JOINER_ENABLE 1
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 0
#define UART_AS_SERIAL_TRANSPORT 1
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE 1

#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1
#define OPENTHREAD_CONFIG_ECDSA_ENABLE 1
#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE 1

// Use the TI-supplied default platform configuration for remainder
#include "openthread-core-cc13x2_26x2-config.h"

#endif // OPENTHREAD_PLATFORM_CONFIG_H
