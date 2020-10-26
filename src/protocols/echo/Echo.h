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

enum
{
    kEchoMessageType_EchoRequest  = 1,
    kEchoMessageType_EchoResponse = 2
};

class DLL_EXPORT EchoClient : public ExchangeContextDelegate
{
public:
    EchoClient(void);

    /**
     *  Initialize the EchoClient object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    exchangeMgr    A pointer to the ExchangeManager object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR Init(ExchangeManager * exchangeMgr);

    /**
     *  Shutdown the EchoClient. This terminates this instance
     *  of the object and releases all held resources.
     *
     *  @return #CHIP_NO_ERROR unconditionally.
     */
    CHIP_ERROR Shutdown(void);

    /**
     * Send an echo request to a CHIP node.
     *
     * @param nodeId        The destination's nodeId
     * @param payload       A System::PacketBuffer with the payload. This function takes ownership of the System::PacketBuffer
     *
     * @return CHIP_ERROR_NO_MEMORY if no ExchangeContext is available.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     */
    CHIP_ERROR SendEchoRequest(uint64_t nodeId, System::PacketBuffer * payload);

    typedef void (*EchoFunct)(uint64_t nodeId, System::PacketBuffer * payload);
    EchoFunct OnEchoResponseReceived;

private:
    ExchangeManager * ExchangeMgr = nullptr;
    ExchangeContext * ExchangeCtx = nullptr;

    CHIP_ERROR SendEchoRequest(System::PacketBuffer * payload);
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBuffer * payload);
};

class DLL_EXPORT EchoServer
{
public:
    EchoServer(void);

    /**
     *  Initialize the EchoServer object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    exchangeMgr    A pointer to the ExchangeManager object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(ExchangeManager * exchangeMgr);

    /**
     *  Shutdown the EchoServer. This terminates this instance
     *  of the object and releases all held resources.
     *
     *  @return #CHIP_NO_ERROR unconditionally.
     */
    CHIP_ERROR Shutdown(void);

    typedef void (*EchoFunct)(uint64_t nodeId, System::PacketBuffer * payload);
    EchoFunct OnEchoRequestReceived;

private:
    ExchangeManager * ExchangeMgr = nullptr;

    static void HandleEchoRequest(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                  System::PacketBuffer * payload);
};

} // namespace Protocols
} // namespace chip

#endif // CHIP_ECHO_CURRENT_H_
