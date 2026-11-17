/*
 *    Copyright (c) 2026 Project CHIP Authors
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

namespace chip {
namespace bdx {

/**
 * An exchange delegate that answers the message opening an exchange with StatusReport(FAILURE, BDX, RESPONDER_BUSY), and
 * does nothing else.
 *
 * This object is stateless, so the same instance can be used across multiple exchanges.
 */
class BusyResponder : public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    // We never expect responses to our StatusReport; this is only here because
    // ExchangeDelegate requires subclasses to implement it.
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

} // namespace bdx
} // namespace chip
