/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <transport/SecureSession.h>
#include <transport/Session.h>

namespace chip {
namespace Transport {

class MessageCounterManagerInterface
{
public:
    virtual ~MessageCounterManagerInterface() {}

    /**
     * Start sync if the sync procedure is not started yet.
     */
    virtual CHIP_ERROR StartSync(const SessionHandle & session, Transport::SecureSession * state) = 0;

    /**
     * Called when have received a message but session message counter is not synced.  It will queue the message and start sync if
     * the sync procedure is not started yet.
     */
    virtual CHIP_ERROR QueueReceivedMessageAndStartSync(const PacketHeader & packetHeader, const SessionHandle & session,
                                                        Transport::SecureSession * state,
                                                        const Transport::PeerAddress & peerAddress,
                                                        System::PacketBufferHandle && msgBuf) = 0;
};

} // namespace Transport
} // namespace chip
