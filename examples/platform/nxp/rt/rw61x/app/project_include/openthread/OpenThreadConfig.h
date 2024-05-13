/*
 *
 *    Copyright (c) 2020 Google LLC.
 *    Copyright 2023 NXP
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

// When operating in a less than ideal RF environment, having a more forgiving configuration
// of OpenThread makes thread a great deal more reliable.
#define OPENTHREAD_CONFIG_TMF_ADDRESS_QUERY_MAX_RETRY_DELAY 120    // default is 28800
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_DIRECT 30  // default is 3
#define OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_INDIRECT 1 // default is 0
#define OPENTHREAD_CONFIG_MAC_MAX_TX_ATTEMPTS_INDIRECT_POLLS 16    // default is 4

// Enable periodic parent search to speed up finding a better parent.
#define OPENTHREAD_CONFIG_PARENT_SEARCH_ENABLE 1                   // default is 0
#define OPENTHREAD_CONFIG_PARENT_SEARCH_RSS_THRESHOLD -45          // default is -65
#define OPENTHREAD_CONFIG_MLE_INFORM_PREVIOUS_PARENT_ON_REATTACH 1 // default is 0

// Use smaller maximum interval to speed up reattaching.
#define OPENTHREAD_CONFIG_MLE_ATTACH_BACKOFF_MAXIMUM_INTERVAL (60 * 10 * 1000) // default 1200000 ms

// disable unused features
#define OPENTHREAD_CONFIG_COAP_API_ENABLE 0
#define OPENTHREAD_CONFIG_JOINER_ENABLE 0
#define OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE 0
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE 0
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 0
#define OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE 0

#ifndef OPENTHREAD_CONFIG_COMMISSIONER_ENABLE
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 0
#endif
#ifndef OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE 0
#endif

// Enable usage of external heap allocator for ot
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1

// Disable TCP
#define OPENTHREAD_CONFIG_TCP_ENABLE 0

#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1
#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_ECDSA_ENABLE 1

// Temporary workaround till we can enable this via the gn
#ifdef SPINEL_INTERFACE_RPMSG
#define OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE 1
#else
#define OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE 0
#endif /* SPINEL_INTERFACE_RPMSG */

/*
 * "ot-nxp/src/rw/rw612/platform/radio.c" has a dependency on
 * "examples/platforms/utils/link_metrics.cpp" which is not built in the Matter build system
 * for this reason, currently we are disabling this feature
 */
#define OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE 0
#define OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE 0

// Use the NXP-supplied default platform configuration for remainder
// of OpenThread config options.
//
// NB: This file gets included during the build of OpenThread.  Hence
// it cannot use "openthread" in the path to the included file.
//
#include "openthread-core-rw612-config.h"
