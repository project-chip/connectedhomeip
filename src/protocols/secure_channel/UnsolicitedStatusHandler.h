/**
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

#pragma once

#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

class UnsolicitedStatusHandler : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeManager);

protected:
    // ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    // UnsolicitedMessageHandler
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
