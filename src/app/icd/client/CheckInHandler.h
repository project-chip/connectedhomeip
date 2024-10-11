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
class InteractionModelEngine;
class CheckInHandler : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{

public:
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage, CheckInDelegate * delegate,
                    InteractionModelEngine * engine);
    void Shutdown();

    CheckInHandler();

    virtual ~CheckInHandler() = default;

protected:
    // ExchangeDelegate
    CHIP_ERROR
    OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                      System::PacketBufferHandle && payload) override;

    // UnsolicitedMessageHandler
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    // TODO : Follow up to check if this really needs to be a pure virtual function in Exchange delegate
    // https://github.com/project-chip/connectedhomeip/issues/31322
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return CheckInExchangeDispatch::Instance(); }

private:
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
    InteractionModelEngine * mpImEngine            = nullptr;
};

} // namespace app
} // namespace chip
