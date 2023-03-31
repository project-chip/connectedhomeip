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

#include <lib/core/Optional.h>
#include <system/SystemClock.h>
#include <system/SystemConfig.h>

namespace chip {

/**
 *  @def CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
 *
 *  @brief Base retry interval of the present node when it is in the active state.
 *
 *  Base interval that a peer node should use to calculate the retransmission
 *  timeout when it sends a message to the present node and the present node is
 *  perceived by the peer as active.
 *
 *  This value is announced to the peer using SAI (Sleepy Active Interval) key
 *  in the advertised DNS Service Discovery TXT records. Additionally, it is
 *  exchanged in the initial phase of the PASE/CASE session establishment.
 *
 *  In the case of a Thread device, the default value is increased to limit the
 *  possibility of spurious retransmissions. The assumption is that the average
 *  round-trip time of a big request-response pair is substantially longer in
 *  a Thread network that potentially constists of multiple intermediate hops.
 */
#ifndef CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD && !CHIP_DEVICE_LAYER_TARGET_LINUX
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (800_ms32)
#else
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (300_ms32)
#endif
#endif // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL && !CHIP_DEVICE_LAYER_TARGET_LINUX

/**
 *  @def CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
 *
 *  @brief Base retry interval of the present node when it is in the idle state.
 *
 *  Base interval that a peer node should use to calculate the retransmission
 *  timeout when it sends a message to the present node and the present node is
 *  perceived by the peer as idle.
 *
 *  This value is announced to the peer using SII (Sleepy Idle Interval) key
 *  in the advertised DNS Service Discovery TXT records. Additionally, it is
 *  exchanged in the initial phase of the PASE/CASE session establishment.
 *
 *  In the case of a Thread device, the default value is increased to limit the
 *  possibility of spurious retransmissions. The assumption is that the average
 *  round-trip time of a big request-response pair is substantially longer in
 *  a Thread network that potentially constists of multiple intermediate hops.
 */
#ifndef CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
#if CHIP_ENABLE_OPENTHREAD && !CHIP_DEVICE_LAYER_TARGET_LINUX
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL (800_ms32)
#else
#define CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL (300_ms32)
#endif
#endif // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL && !CHIP_DEVICE_LAYER_TARGET_LINUX

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
 *  @def CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE
 *
 *  @brief
 *    Should an address lookup of the peer happen on every first message that fails
 *    to send on the link.
 *
 *  The default value to not perform lookup was selected because most implementations
 *  of address lookup are not cache the and a request is sent on the link. Failing
 *  to deliver the first message is far more likely to happen due to lossy link
 *  than an actual address change where the peer did not reset. In the lossy link
 *  situation, doing further DNS resolutions on a degraded link can exacerbate that
 *  problem greatly. Additionally, every message that arrives from a peer updates the
 *  address. If the peer has fallen off the link due to any other reason, a re-resolve
 *  may not achieve an address that is reachable, even if a resolve response occurs.
 */
#ifndef CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE
#define CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE 0
#endif // CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE

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
 *  @def CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST
 *
 *  @brief
 *    A constant value that should be added to the calculated retransmission
 *    timeout when the present node transmits a message.
 *
 *  The purpose for this constant is to limit the possibility of spurious
 *  retransmissions in the scenario in which a sender that operates in a high-
 *  latency network (such as Thread) sends a message to a receiver that operates
 *  in a low-latency network (such as Wi-Fi). In this scenario, the SAI and SII
 *  parameters advertised by the receiver are low although the average round-
 *  trip time of a big request-response pair is long due to the nature of the
 *  sender.
 */
#ifndef CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST
#if CHIP_ENABLE_OPENTHREAD && !CHIP_DEVICE_LAYER_TARGET_LINUX
#define CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST (500_ms)
#endif
#endif // CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST && !CHIP_DEVICE_LAYER_TARGET_LINUX

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

    bool operator==(const ReliableMessageProtocolConfig & that) const
    {
        return mIdleRetransTimeout == that.mIdleRetransTimeout && mActiveRetransTimeout == that.mActiveRetransTimeout;
    }
};

/// @brief The default MRP config. The value is defined by spec, and shall be same for all implementations,
ReliableMessageProtocolConfig GetDefaultMRPConfig();

/**
 *  @brief  The custom value of MRP config for the platform.
 *  @return Missing   If the value is same as default value defined by spec
 *          Value     The custom value for the platform
 *
 *  @note   This value is not used by our MRP manager. The value is advertised via mDNS or during PASE/CASE paring, and our peers
 *          use it when communicating with us.
 */
Optional<ReliableMessageProtocolConfig> GetLocalMRPConfig();

/**
 * @brief
 * Returns the maximum transmission time depending on the last activity time.
 *
 * @param[in] activeInterval    The active interval to use for the backoff calculation.
 * @param[in] idleInterval      The idle interval to use for the backoff calculation.
 * @param[in] lastActivityTime  The last time some activity has been recorded.
 * @param[in] activityThreshold The activity threshold for a node to be considered active.
 *
 * @return The maximum transmission time
 */
System::Clock::Timestamp GetRetransmissionTimeout(System::Clock::Timestamp activeInterval, System::Clock::Timestamp idleInterval,
                                                  System::Clock::Timestamp lastActivityTime,
                                                  System::Clock::Timestamp activityThreshold);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST

/**
 * @brief
 *
 * Overrides the local idle and active retransmission timeout parameters (which are usually set through compile
 * time defines). This is reserved for tests that need the ability to set these at runtime to make certain test scenarios possible.
 *
 */
void OverrideLocalMRPConfig(System::Clock::Timeout idleRetransTimeout, System::Clock::Timeout activeRetransTimeout);

/**
 * @brief
 *
 * Disables the overrides set previously in OverrideLocalMRPConfig().
 *
 */
void ClearLocalMRPConfigOverride();
#endif

} // namespace chip
