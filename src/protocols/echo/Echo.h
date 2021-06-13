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

/**
 * Echo Protocol Message Types
 */
enum class MsgType : uint8_t
{
    EchoRequest  = 0x01,
    EchoResponse = 0x02
};

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by EchoClient/EchoServer object on specific events. If the user of EchoClient/EchoServer
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT EchoDelegate
{
public:
    virtual ~EchoDelegate() {}

    /**
     * @brief
     *   This function is the protocol callback for handling a received CHIP message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *  @param[in]    payload       A handle to the PacketBuffer object holding the message payload.
     */
    virtual void OnMessageReceived(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload) = 0;
};

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

    EchoDelegate * GetDelegate() const { return mDelegate; }

    void SetDelegate(EchoDelegate * delegate) { mDelegate = delegate; }

    /**
     * Send an echo request to a CHIP node.
     *
     * @param payload       A PacketBufferHandle with the payload.
     * @param sendFlags     Flags set by the application for the CHIP message being sent.
     *
     * @return CHIP_ERROR_NO_MEMORY if no ExchangeContext is available.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */
    CHIP_ERROR SendEchoRequest(System::PacketBufferHandle && payload,
                               const Messaging::SendFlags & sendFlags = Messaging::SendFlags(Messaging::SendMessageFlags::kNone));

private:
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    Messaging::ExchangeContext * mExchangeCtx = nullptr;
    EchoDelegate * mDelegate                  = nullptr;
    SecureSessionHandle mSecureSession;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && payload) override;
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

    EchoDelegate * GetDelegate() const { return mDelegate; }

    void SetDelegate(EchoDelegate * delegate) { mDelegate = delegate; }

private:
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    EchoDelegate * mDelegate                  = nullptr;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

} // namespace Echo

template <>
struct MessageTypeTraits<Echo::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return Echo::Id; }
};

} // namespace Protocols
} // namespace chip
