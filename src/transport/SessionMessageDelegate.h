/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <transport/Session.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by SecureSssionMgrBase object on specific events. If the user of SessionManager
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT SessionMessageDelegate
{
public:
    virtual ~SessionMessageDelegate() {}

    enum class DuplicateMessage : uint8_t
    {
        Yes,
        No,
    };

    /**
     * @brief
     *   Called when a new message is received. The function must internally release the
     *   msgBuf after processing it.
     *
     * @param packetHeader  The message header
     * @param payloadHeader The payload header
     * @param session       The handle to the secure session
     * @param isDuplicate   The message is a duplicate of previously received message
     * @param msgBuf        The received message
     */
    virtual void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                   const SessionHandle & session, DuplicateMessage isDuplicate,
                                   System::PacketBufferHandle && msgBuf) = 0;
};

} // namespace chip
