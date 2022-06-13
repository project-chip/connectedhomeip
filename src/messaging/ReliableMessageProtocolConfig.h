/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <system/SystemClock.h>
#include <system/SystemConfig.h>

namespace chip {

/**
 *  @def CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
 *
 *  @brief
 *    Active retransmit interval, or time to wait before retransmission after
 *    subsequent failures in milliseconds.
 *
 *  This is the default value, that might be adjusted by end device depending on its
 *  needs (e.g. sleeping period) using Service Discovery TXT record CRA key.
 *
 */
#ifndef CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
#define CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL (300_ms32)
#endif // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL

/**
 *  @def CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL
 *
 *  @brief
 *    Initial base retransmission interval, or time to wait before retransmission after first
 *    failure in milliseconds.
 *
 * This is the default value, that might be adjusted by end device depending on its
 * needs (e.g. sleeping period) using Service Discovery TXT record CRI key.
 */
#ifndef CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL
#define CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL (5000_ms32)
#endif // CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL

/**
 *  @def CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT
 *
 *  @brief
 *    The default acknowledgment timeout in milliseconds.
 *
 */
#ifndef CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT
#define CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT (200_ms32)
#endif // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT

/**
 *  @def CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
 *
 *  @brief
 *    The default size of the ReliableMessageProtocol retransmission table.
 *
 */
#ifndef CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE
#if LWIP_PBUF_FROM_CUSTOM_POOLS
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#elif PBUF_POOL_SIZE
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE std::min(PBUF_POOL_SIZE, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS)
#elif CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE != 0
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE std::min(CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS)
#else
#define CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
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
#define CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS (4)
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS

/**
 *  @brief
 *    The ReliableMessageProtocol configuration.
 */
struct ReliableMessageProtocolConfig
{
    ReliableMessageProtocolConfig(System::Clock::Milliseconds32 idleInterval, System::Clock::Milliseconds32 activeInterval) :
        mIdleRetransTimeout(idleInterval), mActiveRetransTimeout(activeInterval)
    {}

    // Configurable timeout in msec for retransmission of the first sent message.
    System::Clock::Milliseconds32 mIdleRetransTimeout;

    // Configurable timeout in msec for retransmission of all subsequent messages.
    System::Clock::Milliseconds32 mActiveRetransTimeout;
};

ReliableMessageProtocolConfig GetLocalMRPConfig();

} // namespace chip
