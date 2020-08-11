/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This file defines the configuration parameters that are required
 *      for the Weave Reliable Messaging Protocol.
 *
 */
#ifndef WEAVE_WRM_CONFIG_H_
#define WEAVE_WRM_CONFIG_H_

namespace nl {
namespace Weave {

// clang-format off

/**
 *  @def WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
 *
 *  @brief
 *    If set to (1), use of the WRMP implementation is
 *    enabled. Default value is (1) or enabled.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
#define WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING              1
#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING

/**
 *  @def WEAVE_CONFIG_WRMP_TIMER_DEFAULT_PERIOD
 *
 *  @brief
 *    The default WRMP timer tick interval in milliseconds.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_TIMER_DEFAULT_PERIOD
#define WEAVE_CONFIG_WRMP_TIMER_DEFAULT_PERIOD              200
#endif // WEAVE_CONFIG_WRMP_TIMER_DEFAULT_PERIOD

/**
 *  @def WEAVE_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT
 *
 *  @brief
 *    The default retransmission timeout in milliseconds.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT
#define WEAVE_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT       (2000)
#endif // WEAVE_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT

/**
 *  @def WEAVE_CONFIG_WRMP_DEFAULT_INITIAL_RETRANS_TIMEOUT
 *
 *  @brief
 *    The default long retransmission timeout in milliseconds
 *    to include sleepy destinaton nodes.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_DEFAULT_INITIAL_RETRANS_TIMEOUT
#define WEAVE_CONFIG_WRMP_DEFAULT_INITIAL_RETRANS_TIMEOUT      (2000)
#endif // WEAVE_CONFIG_WRMP_DEFAULT_INITIAL_RETRANS_TIMEOUT

/**
 *  @def WEAVE_CONFIG_WRMP_DEFAULT_ACK_TIMEOUT
 *
 *  @brief
 *    The default acknowledgment timeout in milliseconds.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_DEFAULT_ACK_TIMEOUT
#define WEAVE_CONFIG_WRMP_DEFAULT_ACK_TIMEOUT               (WEAVE_CONFIG_WRMP_TIMER_DEFAULT_PERIOD)
#endif // WEAVE_CONFIG_WRMP_DEFAULT_ACK_TIMEOUT

/**
 *  @def WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE
 *
 *  @brief
 *    The default size of the WRMP retransmission table.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE
#ifdef PBUF_POOL_SIZE
#define WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE                (PBUF_POOL_SIZE)
#else
#define WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE                (WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC)
#endif // PBUF_POOL_SIZE
#endif // WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE

/**
 *  @def WEAVE_CONFIG_WRMP_DEFAULT_MAX_RETRANS
 *
 *  @brief
 *    The maximum number of retransmissions before giving up.
 *
 */
#ifndef WEAVE_CONFIG_WRMP_DEFAULT_MAX_RETRANS
#define WEAVE_CONFIG_WRMP_DEFAULT_MAX_RETRANS               (3)
#endif // WEAVE_CONFIG_WRMP_DEFAULT_MAX_RETRANS

/**
 *  @brief
 *    The WRMP configuration.
 */
struct WRMPConfig
{
    uint32_t mInitialRetransTimeout;            /**< Configurable timeout in msec for retransmission of the first sent message. */
    uint32_t mActiveRetransTimeout;             /**< Configurable timeout in msec for retransmission of all subsequent messages. */
    uint16_t mAckPiggybackTimeout;              /**< Configurable timeout in msec for transmission of a solitary Ack message. */
    uint8_t  mMaxRetrans;                       /**< Configurable max value for retransmissions in the ExchangeContext. */
};

const WRMPConfig gDefaultWRMPConfig = { WEAVE_CONFIG_WRMP_DEFAULT_INITIAL_RETRANS_TIMEOUT,
                                        WEAVE_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT,
                                        WEAVE_CONFIG_WRMP_DEFAULT_ACK_TIMEOUT,
                                        WEAVE_CONFIG_WRMP_DEFAULT_MAX_RETRANS };

// clang-format on

}; // Weave
}; // nl

#endif // WEAVE_WRM_CONFIG_H_
