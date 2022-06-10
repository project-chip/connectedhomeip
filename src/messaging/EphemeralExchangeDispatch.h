/*
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file defines Application Channel class. The object of this
 *      class can be used by CHIP data model cluster applications to send
 *      and receive messages. The messages are encrypted using session keys.
 */

#pragma once

#include <messaging/ExchangeMessageDispatch.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Messaging {

class EphemeralExchangeDispatch : public ExchangeMessageDispatch
{
public:
    static ExchangeMessageDispatch & Instance()
    {
        static EphemeralExchangeDispatch instance;
        return instance;
    }

    EphemeralExchangeDispatch() {}
    ~EphemeralExchangeDispatch() override {}

protected:
    bool MessagePermitted(Protocols::Id protocol, uint8_t type) override
    {
        // Only permit StandaloneAck
        return (protocol == Protocols::SecureChannel::Id &&
                type == static_cast<uint8_t>(Protocols::SecureChannel::MsgType::StandaloneAck));
    }

    bool IsEncryptionRequired() const override
    {
        // This function should not be called at all
        VerifyOrDie(false);
        return false;
    }
};

} // namespace Messaging
} // namespace chip
