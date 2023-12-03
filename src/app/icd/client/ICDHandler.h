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
 *      This file defines objects for a CHIP CheckInMessage unsolicited
 *      handler
 *
 */

#pragma once

#include "DefaultICDClientStorage.h"
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {
class CheckInMessageHandler : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
    class Callback
    {
    public:
        virtual ~Callback() = default;

        // TODO : Include the callback message from ICDClientManagement
    };

public:
    /**
     * @brief Retrieve the singleton CheckIn handler
     *
     *  @return  A pointer to the shared CheckIn handler
     *
     */
    static CheckInMessageHandler * GetInstance(void);

    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage);
    void Shutdown();

protected:
    // ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    // UnsolicitedMessageHandler
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    // TODO : Follow up to check if this really needs to be a pure virtual function in Exchange delegate
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

private:
    Messaging::ExchangeManager * mExchangeManager = nullptr;
    Messaging::ExchangeManager * GetExchangeManager(void) const { return mExchangeManager; }
    DefaultICDClientStorage * mICDClientStorage = nullptr;
};

} // namespace app
} // namespace chip
