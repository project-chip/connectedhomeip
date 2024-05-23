/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
