/*
 *    Copyright (c) 2020 Project CHIP Authors
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

#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 0
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE 0
#define OPENTHREAD_CONFIG_DIAG_ENABLE 0
#define OPENTHREAD_CONFIG_DNSSD_SERVER_ENABLE 0
#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE 1
#define OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE 1
#define OPENTHREAD_CONFIG_MAC_FILTER_ENABLE 0

#define OPENTHREAD_CONFIG_MLE_IP_ADDRS_PER_CHILD 6

// TCP disabled until OpenThread has a GN/Ninja build for the tcplp library
#define OPENTHREAD_CONFIG_TCP_ENABLE 0
#define OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS 0
#define OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE 0
#define OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE 0
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 0
#define OPENTHREAD_CONFIG_JOINER_ENABLE 0
#define UART_AS_SERIAL_TRANSPORT 1

#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP
// Use the TI-supplied default platform configuration for remainder
#ifdef OPENTHREAD_PROJECT_CORE_CONFIG_FILE
#include OPENTHREAD_PROJECT_CORE_CONFIG_FILE
#endif
