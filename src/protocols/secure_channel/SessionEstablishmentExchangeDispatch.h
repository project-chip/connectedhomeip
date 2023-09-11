/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines Application Channel class. The object of this
 *      class can be used by CHIP data model cluster applications to send
 *      and receive messages. The messages are encrypted using session keys.
 */

#pragma once

#include <messaging/ExchangeMessageDispatch.h>

namespace chip {

class SessionEstablishmentExchangeDispatch : public Messaging::ExchangeMessageDispatch
{
public:
    static ExchangeMessageDispatch & Instance()
    {
        static SessionEstablishmentExchangeDispatch instance;
        return instance;
    }

    SessionEstablishmentExchangeDispatch() {}
    ~SessionEstablishmentExchangeDispatch() override {}

protected:
    bool MessagePermitted(Protocols::Id, uint8_t type) override;
    bool IsEncryptionRequired() const override { return false; }
};

} // namespace chip
