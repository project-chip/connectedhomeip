/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/CHIPProtocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Protocols {

using namespace chip::Inet;

enum
{
    kEchoMessageType_EchoRequest  = 1,
    kEchoMessageType_EchoResponse = 2
};

class DLL_EXPORT ChipEchoClient : public ExchangeContextDelegate
{
public:
    ChipEchoClient(void);

    CHIP_ERROR Init(ExchangeManager * exchangeMgr);
    CHIP_ERROR Shutdown(void);

    CHIP_ERROR SendEchoRequest(uint64_t nodeId, System::PacketBuffer * payload);

    typedef void (*EchoFunct)(uint64_t nodeId, System::PacketBuffer * payload);
    EchoFunct OnEchoResponseReceived;

private:
    ExchangeManager * ExchangeMgr; // Exchange manager object
    ExchangeContext * ExchangeCtx; // The exchange context for the most recently started Echo exchange.

    CHIP_ERROR SendEchoRequest(System::PacketBuffer * payload);
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBuffer * payload);
};

class DLL_EXPORT ChipEchoServer
{
public:
    ChipEchoServer(void);

    CHIP_ERROR Init(ExchangeManager * exchangeMgr);
    CHIP_ERROR Shutdown(void);

    typedef void (*EchoFunct)(uint64_t nodeId, System::PacketBuffer * payload);
    EchoFunct OnEchoRequestReceived;

private:
    ExchangeManager * ExchangeMgr; // Exchange manager object

    static void HandleEchoRequest(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                  System::PacketBuffer * payload);
};

} // namespace Protocols
} // namespace chip

#endif // CHIP_ECHO_CURRENT_H_
