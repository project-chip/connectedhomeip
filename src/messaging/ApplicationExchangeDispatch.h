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

#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeMessageDispatch.h>

namespace chip {
namespace Messaging {

class ApplicationExchangeDispatch : public ExchangeMessageDispatch
{
public:
    static ExchangeMessageDispatch & Instance()
    {
        static ApplicationExchangeDispatch instance;
        return instance;
    }

    ApplicationExchangeDispatch() {}
    ~ApplicationExchangeDispatch() override {}

protected:
    bool MessagePermitted(Protocols::Id protocol, uint8_t type) override;
};

} // namespace Messaging
} // namespace chip
