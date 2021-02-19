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
/**
 *    @file
 *      This file defines types and objects for CHIP Secure Channel protocol.
 *
 */

#pragma once

namespace chip {
namespace Messaging {

constexpr uint16_t kMsgCounterSyncRespMsgSize = 12;  // The size of the message counter synchronization response message.
constexpr uint32_t kMsgCounterSyncTimeout     = 500; // The amount of time(in milliseconds) which a peer is given to respond
                                                     // to a message counter synchronization request.

class ExchangeManager;

class MessageCounterSyncMgr : public Messaging::ExchangeDelegate
{
public:
    MessageCounterSyncMgr() : mExchangeMgr(nullptr) {}

    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);
    void Shutdown();

    /**
     * Send peer message counter synchronization request.
     * This function is called while processing a message encrypted with an application key from a peer whose message counter is not
     * synchronized. This message is sent on a newly created exchange, which is closed immediately after.
     *
     * @param[in]  session  The secure session handle of the received message.
     *
     * @retval  #CHIP_ERROR_NO_MEMORY         If memory could not be allocated for the new
     *                                         exchange context or new message buffer.
     * @retval  #CHIP_NO_ERROR                On success.
     *
     */
    CHIP_ERROR SendMsgCounterSyncReq(SecureSessionHandle session);

private:
    Messaging::ExchangeManager * mExchangeMgr; // [READ ONLY] Associated Exchange Manager object.

    CHIP_ERROR NewMsgCounterSyncExchange(SecureSessionHandle session, Messaging::ExchangeContext *& exchangeContext);

    CHIP_ERROR SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, SecureSessionHandle session);

    void HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                 System::PacketBufferHandle msgBuf);

    void HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                  System::PacketBufferHandle msgBuf);

    void OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                           const PayloadHeader & payloadHeader, System::PacketBufferHandle payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * exchangeContext) override;
};

} // namespace Messaging
} // namespace chip
