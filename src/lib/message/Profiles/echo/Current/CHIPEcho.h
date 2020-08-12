/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines objects for a CHIP Echo unsolicitied
 *      initaitor (client) and responder (server).
 *
 */

#ifndef CHIP_ECHO_CURRENT_H_
#define CHIP_ECHO_CURRENT_H_

#include <core/CHIPCore.h>
#include <message/CHIPServerBase.h>
#include <profiles/CHIPProfiles.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Profiles {

using namespace Inet;

enum
{
    kEchoMessageType_EchoRequest  = 1,
    kEchoMessageType_EchoResponse = 2
};

class DLL_EXPORT ChipEchoClient
{
public:
    ChipEchoClient(void);

    const ChipFabricState * FabricState; // [READ ONLY] Fabric state object
    ChipExchangeManager * ExchangeMgr;   // [READ ONLY] Exchange manager object
    uint8_t EncryptionType;              // Encryption type to use when sending an Echo Request
    uint16_t KeyId;                      // Encryption key to use when sending an Echo Request

    CHIP_ERROR Init(ChipExchangeManager * exchangeMgr);
    CHIP_ERROR Shutdown(void);

    CHIP_ERROR SendEchoRequest(ChipConnection * con, PacketBuffer * payload);
    CHIP_ERROR SendEchoRequest(uint64_t nodeId, IPAddress nodeAddr, PacketBuffer * payload);
    CHIP_ERROR SendEchoRequest(uint64_t nodeId, IPAddress nodeAddr, uint16_t port, InterfaceId sendIntfId, PacketBuffer * payload);

#if CHIP_CONFIG_ENABLE_RELIABLE_MESSAGING
    void SetRequestAck(bool requestAck);
    void SetWRMPACKDelay(uint16_t aWRMPACKDelay);
    void SetWRMPRetransInterval(uint32_t aRetransInterval);
    void SetWRMPRetransCount(uint8_t aRetransCount);
    typedef void (*EchoAckFunct)(void * msgCtxt);
    EchoAckFunct OnAckRcvdReceived;
#endif // CHIP_CONFIG_ENABLE_RELIABLE_MESSAGING

    typedef void (*EchoFunct)(uint64_t nodeId, IPAddress nodeAddr, PacketBuffer * payload);
    EchoFunct OnEchoResponseReceived;

private:
    ExchangeContext * ExchangeCtx; // The exchange context for the most recently started Echo exchange.
#if CHIP_CONFIG_ENABLE_RELIABLE_MESSAGING
    bool RequestAck;
    bool AckReceived;
    bool ResponseReceived;
    uint16_t WRMPACKDelay;
    uint32_t WRMPRetransInterval;
    uint8_t WRMPRetransCount;
    uint32_t appContext;
    static void HandleAckRcvd(ExchangeContext * ec, void * msgCtxt);
#endif // CHIP_CONFIG_ENABLE_RELIABLE_MESSAGING
    static void HandleError(ExchangeContext * ec, CHIP_ERROR err);
    static void HandleConnectionClosed(ExchangeContext * ec, ChipConnection * con, CHIP_ERROR conErr);
    static void HandleSendError(ExchangeContext * ec, CHIP_ERROR sendErr, void * msgCtxt);
    static void HandleKeyError(ExchangeContext * ec, CHIP_ERROR keyErr);

    CHIP_ERROR SendEchoRequest(PacketBuffer * payload);
    static void HandleResponse(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                               uint32_t profileId, uint8_t msgType, PacketBuffer * payload);
    ChipEchoClient(const ChipEchoClient &); // not defined
};

class DLL_EXPORT ChipEchoServer : public ChipServerBase
{
public:
    ChipEchoServer(void);

    CHIP_ERROR Init(ChipExchangeManager * exchangeMgr);
    CHIP_ERROR Shutdown(void);

    typedef void (*EchoFunct)(uint64_t nodeId, IPAddress nodeAddr, PacketBuffer * payload);
    EchoFunct OnEchoRequestReceived;

private:
    static void HandleEchoRequest(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                                  uint32_t profileId, uint8_t msgType, PacketBuffer * payload);

    ChipEchoServer(const ChipEchoServer &); // not defined
};

} // namespace Profiles
} // namespace chip

#endif // CHIP_ECHO_CURRENT_H_
