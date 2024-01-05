/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      This file defines objects for a CHIP check-in message unsolicited
 *      handler
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandSender.h>
#include <app/OperationalSessionSetup.h>
#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/DefaultICDClientStorage.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {

class CheckInHandler : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{

public:
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage, CheckInDelegate * delegate);
    void Shutdown();

    CheckInHandler();

    virtual ~CheckInHandler() = default;

    /**
     * @brief Callback received on successfully establishing a CASE session in order to re-register the client with the peer node
     * using a new key to avoid counter rollover problems.
     *
     * @param[in] context context of the client establishing the CASE session
     * @param[in] exchangeMgr exchange manager to use for the re-registration
     * @param[in] sessionHandle session handle to use for the re-registration
     */
    static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                      const SessionHandle & sessionHandle);
    /**
     * @brief Callback received on failure to establish a CASE session in order to re-register the client with the peer node using a
     * new key to avoid counter rollover problems.
     *
     * @param[in] context context of the client establishing the CASE session
     * @param[in] peerId Scoped Node ID of the peer node
     * @param[in] err failure reason
     */
    static void HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err);

    class RegisterCommandSenderCallback : public CommandSender::Callback
    {
    public:
        void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                        const chip::app::StatusIB & aStatus, chip::TLV::TLVReader * aData) override
        {}
        void OnError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override { mError = aError; }
        void OnDone(chip::app::CommandSender * apCommandSender) override {}

        CHIP_ERROR mError = CHIP_NO_ERROR;
    } registerCommandSenderDelegate;

protected:
    // ExchangeDelegate
    CHIP_ERROR
    OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                      System::PacketBufferHandle && payload) override;

    // UnsolicitedMessageHandler
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    // TODO : Follow up to check if this really needs to be a pure virtual function in Exchange delegate
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return CheckInExchangeDispatch::Instance(); }

private:
    /**
     * @brief Used by the application to set a new key to avoid counter rollover problems.
     *
     * @param[in] clientInfo clientInfo object
     * @param[in] keyData New key data to use to re-register the client with the server
     * @param[in] exchangeMgr exchange manager to use for the re-registration
     * @param[in] sessionHandle session handle to use for the re-registration
     */
    CHIP_ERROR RegisterClientWithNewKey(ICDClientInfo & clientInfo, ByteSpan newKey, Messaging::ExchangeManager & exchangeMgr,
                                        const SessionHandle & sessionHandle);

    /**
     * @brief Sets up a CASE session to the peer for re-registering a client with the peer when a key refresh is required to avoid
     * ICD counter rollover. Returns error if we did not even manage to kick off a CASE attempt.
     *
     * @param[in] peerId Node ID of the peer
     */
    void EstablishSessionToPeer(ScopedNodeId peerId);

    chip::Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    class CheckInExchangeDispatch : public Messaging::ExchangeMessageDispatch
    {
    public:
        static ExchangeMessageDispatch & Instance()
        {
            static CheckInExchangeDispatch instance;
            return instance;
        }

        CheckInExchangeDispatch() {}
        ~CheckInExchangeDispatch() override {}

    protected:
        bool MessagePermitted(Protocols::Id, uint8_t type) override
        {
            return type == to_underlying(Protocols::SecureChannel::MsgType::ICD_CheckIn);
        }
        bool IsEncryptionRequired() const override { return false; }
    };

    Messaging::ExchangeManager * mpExchangeManager = nullptr;
    CheckInDelegate * mpCheckInDelegate            = nullptr;
    ICDClientStorage * mpICDClientStorage          = nullptr;
};

} // namespace app
} // namespace chip
