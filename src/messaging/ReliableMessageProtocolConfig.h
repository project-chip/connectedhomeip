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
#pragma once

#include <system/SystemConfig.h>

namespace chip {
namespace Messaging {

/**
 *  @def CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
 *
 *  @brief
 *    The default ReliableMessageProtocol timer tick interval shift in
 *    milliseconds. 6 bit equals 64 milliseconds
 *
 */
#ifndef CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT
#define CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT 6
#endif // CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
 *
 *  @brief
 *    The default retransmission timeout in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
#define CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK (8)
#endif // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
 *
 *  @brief
 *    The default long retransmission timeout in milliseconds
 *    to include sleepy destinaton nodes.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
#define CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK (8)
#endif // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
 *
 *  @brief
 *    The default acknowledgment timeout in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
#define CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK (1)
#endif // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK

/**
 *  @def CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
 *
 *  @brief
 *    The default size of the ReliableMessageProtocol retransmission table.
 *
 */
#ifndef CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
#ifdef PBUF_POOL_SIZE
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE (PBUF_POOL_SIZE)
#else
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE (CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC)
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
#define CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS (3)
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS

/**
 *  @brief
 *    The ReliableMessageProtocol configuration.
 */
struct ReliableMessageProtocolConfig
{
    uint32_t mInitialRetransTimeoutTick; /**< Configurable timeout in msec for retransmission of the first sent message. */
    uint32_t mActiveRetransTimeoutTick;  /**< Configurable timeout in msec for retransmission of all subsequent messages. */
    uint16_t mAckPiggybackTimeoutTick;   /**< Configurable timeout in msec for transmission of a solitary Ack message. */
    uint8_t mMaxRetrans;                 /**< Configurable max value for retransmissions in the ExchangeContext. */
};

const ReliableMessageProtocolConfig gDefaultReliableMessageProtocolConfig = { CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK,
                                                                              CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK,
                                                                              CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK,
                                                                              CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS };

// clang-format on

} // namespace Messaging
} // namespace chip
