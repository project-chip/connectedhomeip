/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines state relevant for an active connection to a peer.
 */

#pragma once

#include <transport/CryptoContext.h>
#include <transport/MessageCounter.h>
#include <transport/PeerMessageCounter.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

class SessionMessageCounter
{
public:
    MessageCounter & GetLocalMessageCounter() { return mLocalMessageCounter; }
    PeerMessageCounter & GetPeerMessageCounter() { return mPeerMessageCounter; }

private:
    LocalSessionMessageCounter mLocalMessageCounter;
    PeerMessageCounter mPeerMessageCounter;
};

} // namespace Transport
} // namespace chip
