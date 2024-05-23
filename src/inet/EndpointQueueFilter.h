/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <inet/IPPacketInfo.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

/**
 * @brief Filter for UDP Packets going into and out of UDPEndPoint queue.
 *
 * NOTE: This is only used by some low-level implementations of UDPEndPoint
 */
class EndpointQueueFilter
{
public:
    enum FilterOutcome
    {
        kAllowPacket = 0,
        kDropPacket  = 1,
    };

    virtual ~EndpointQueueFilter() {}

    /**
     * @brief Run filter prior to inserting in queue.
     *
     * If filter returns `kAllowPacket`, packet will be enqueued, and `FilterAfterDequeue` will
     * be called when it gets dequeued. If filter returns `kDropPacket`, packet will be dropped
     * rather than enqueued and `FilterAfterDequeue` method will not be called.
     *
     * WARNING: This is likely called from non-Matter-eventloop context, from network layer code.
     *          Be extremely careful about accessing any system data which may belong to Matter
     *          stack from this method.
     *
     * @param endpoint - pointer to endpoint instance (platform-dependent, which is why it's void)
     * @param pktInfo - info about source/dest of packet
     * @param pktPayload - payload content of packet
     *
     * @return kAllowPacket to allow packet to enqueue or kDropPacket to drop the packet
     */
    virtual FilterOutcome FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                              const chip::System::PacketBufferHandle & pktPayload)
    {
        return FilterOutcome::kAllowPacket;
    }

    /**
     * @brief Run filter after dequeuing, prior to processing.
     *
     * If filter returns `kAllowPacket`, packet will be processed after dequeuing. If filter returns
     * `kDropPacket`, packet will be dropped and not processed, even though it was dequeued.
     *
     * WARNING: This is called from Matter thread context. Be extremely careful about accessing any
     *          data which may belong to different threads from this method.
     *
     * @param endpoint - pointer to endpoint instance (platform-dependent, which is why it's void)
     * @param pktInfo - info about source/dest of packet
     * @param pktPayload - payload content of packet
     *
     * @return kAllowPacket to allow packet to be processed or kDropPacket to drop the packet
     */
    virtual FilterOutcome FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                             const chip::System::PacketBufferHandle & pktPayload)
    {
        return FilterOutcome::kAllowPacket;
    }
};

} // namespace Inet
} // namespace chip
