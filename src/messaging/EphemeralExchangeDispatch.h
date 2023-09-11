/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
                type == to_underlying(Protocols::SecureChannel::MsgType::StandaloneAck));
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
