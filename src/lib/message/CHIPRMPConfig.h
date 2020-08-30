/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      for the CHIP Reliable Messaging Protocol.
 *
 */
#ifndef CHIP_RMP_CONFIG_H_
#define CHIP_RMP_CONFIG_H_

namespace chip {

// clang-format off

/**
 *  @def CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD
 *
 *  @brief
 *    The default RMP timer tick interval in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD              200
#endif // CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT
 *
 *  @brief
 *    The default retransmission timeout in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT
#define CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT       (2000)
#endif // CHIP_CONFIG_WRMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT
 *
 *  @brief
 *    The default long retransmission timeout in milliseconds
 *    to include sleepy destinaton nodes.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT
#define CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT      (2000)
#endif // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT
 *
 *  @brief
 *    The default acknowledgment timeout in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT
#define CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT               (CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD)
#endif // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT

/**
 *  @def CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
 *
 *  @brief
 *    The default size of the RMP retransmission table.
 *
 */
#ifndef CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
#ifdef PBUF_POOL_SIZE
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE                (PBUF_POOL_SIZE)
#else
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE                (CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC)
#endif // PBUF_POOL_SIZE
#endif // CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
 *
 *  @brief
 *    The maximum number of retransmissions before giving up.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
#define CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS               (3)
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS

/**
 *  @brief
 *    The RMP configuration.
 */
struct RMPConfig
{
    uint32_t mInitialRetransTimeout;            /**< Configurable timeout in msec for retransmission of the first sent message. */
    uint32_t mActiveRetransTimeout;             /**< Configurable timeout in msec for retransmission of all subsequent messages. */
    uint16_t mAckPiggybackTimeout;              /**< Configurable timeout in msec for transmission of a solitary Ack message. */
    uint8_t  mMaxRetrans;                       /**< Configurable max value for retransmissions in the ExchangeContext. */
};

const RMPConfig gDefaultRMPConfig = { CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT,
                                        CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT,
                                        CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT,
                                        CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS };

// clang-format on

} // namespace chip

#endif // CHIP_RMP_CONFIG_H_
