/*
 *  Copyright (c) 2018, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifdef DeviceFamily_CC13X2_CC26X2

/* Number of message buffers reduced to fit into CC26x2x3 RAM */
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 22

#endif // DeviceFamily_CC13X2_CC26X2

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

#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1
//#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 0
#define UART_AS_SERIAL_TRANSPORT 1
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE 1

#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1
#define OPENTHREAD_CONFIG_ECDSA_ENABLE 1
#define OPENTHREAD_CONFIG_SRP_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE 1
#define OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE 1

#define OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE 0
#define OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE 0
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 0
#define OPENTHREAD_CONFIG_JOINER_ENABLE 0

#define OPENTHREAD_CONFIG_ENABLE_BUILTIN_MBEDTLS 0
// TCP disabled until OpenThread has a GN/Ninja build for the tcplp library
#define OPENTHREAD_CONFIG_TCP_ENABLE 0
