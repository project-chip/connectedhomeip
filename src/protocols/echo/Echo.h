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

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Protocols {
namespace Echo {

enum
{
    kEchoMessageType_EchoRequest  = 1,
    kEchoMessageType_EchoResponse = 2
};

using EchoFunct = void (*)(Messaging::ExchangeContext * ec, System::PacketBufferHandle payload);

class DLL_EXPORT EchoClient : public Messaging::ExchangeDelegate
{
public:
    // TODO: Init function will take a Channel instead a SecureSessionHandle, when Channel API is ready
    /**
     *  Initialize the EchoClient object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    exchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    sessoin        A handle to the session.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session);

    /**
     *  Shutdown the EchoClient. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    /**
     * Set the application callback to be invoked when an echo response is received.
     *
     *  @param[in]    callback    The callback function to receive echo response message.
     *
     */
    void SetEchoResponseReceived(EchoFunct callback) { OnEchoResponseReceived = callback; }

    /**
     * Send an echo request to a CHIP node.
     *
     * @param nodeId        The destination's nodeId
     * @param payload       A PacketBufferHandle with the payload.
     *
     * @return CHIP_ERROR_NO_MEMORY if no ExchangeContext is available.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */
    CHIP_ERROR SendEchoRequest(System::PacketBufferHandle && payload);

private:
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    Messaging::ExchangeContext * mExchangeCtx = nullptr;
    EchoFunct OnEchoResponseReceived          = nullptr;
    SecureSessionHandle mSecureSession;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBufferHandle payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
};

class DLL_EXPORT EchoServer : public Messaging::ExchangeDelegate
{
public:
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
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);

    /**
     *  Shutdown the EchoServer. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    /**
     * Set the application callback to be invoked when an echo request is received.
     *
     *  @param[in]    callback    The callback function to receive echo request message.
     *
     */
    void SetEchoRequestReceived(EchoFunct callback) { OnEchoRequestReceived = callback; }

private:
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    EchoFunct OnEchoRequestReceived           = nullptr;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBufferHandle payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

} // namespace Echo
} // namespace Protocols
} // namespace chip
