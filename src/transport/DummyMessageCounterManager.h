/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <transport/PeerConnectionState.h>
#include <transport/SecureSessionHandle.h>

namespace chip {
namespace Transport {

/* A dummy MessageCounterManager for test-cases and migration process */
class DummyMessageCounterManager : public MessageCounterManagerInterface
{
public:
    ~DummyMessageCounterManager() override {}

    MessageCounter & GetGlobalUnsecureCounter() override { return mGlobalUnencryptedMessageCounter; }
    MessageCounter & GetGlobalSecureCounter() override { return mGlobalEncryptedMessageCounter; }
    MessageCounter & GetLocalSessionCounter(Transport::PeerConnectionState * state) override
    {
        return state->GetSessionMessageCounter().GetLocalMessageCounter();
    }

    bool IsSyncCompleted(Transport::PeerConnectionState * state) override { return true; }
    CHIP_ERROR VerifyCounter(Transport::PeerConnectionState * state, const PacketHeader & packetHeader) override
    {
        return CHIP_NO_ERROR;
    }
    void CommitCounter(Transport::PeerConnectionState * state, const PacketHeader & packetHeader) override {}

    CHIP_ERROR StartSync(SecureSessionHandle session, Transport::PeerConnectionState * state) override { return CHIP_NO_ERROR; }
    CHIP_ERROR QueueSendMessageAndStartSync(SecureSessionHandle session, Transport::PeerConnectionState * state,
                                            PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR QueueReceivedMessageAndStartSync(SecureSessionHandle session, Transport::PeerConnectionState * state,
                                                const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle msgBuf) override
    {
        return CHIP_NO_ERROR;
    }

private:
    GlobalUnencryptedMessageCounter mGlobalUnencryptedMessageCounter;
    GlobalEncryptedMessageCounter mGlobalEncryptedMessageCounter;
};

} // namespace Transport
} // namespace chip
