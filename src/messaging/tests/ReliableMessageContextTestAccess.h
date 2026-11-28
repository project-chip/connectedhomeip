/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <messaging/ReliableMessageContext.h>

namespace chip {
namespace Testing {

class ReliableMessageContextTestAccess
{
public:
    ReliableMessageContextTestAccess() = delete;
    ReliableMessageContextTestAccess(Messaging::ReliableMessageContext * rc) : mRc(rc) {}

    // Delivers an acknowledgement for `ackMessageCounter` to the exchange, as
    // the receive path does.
    void ReceiveAck(uint32_t ackMessageCounter) { mRc->HandleRcvdAck(ackMessageCounter); }

private:
    Messaging::ReliableMessageContext * mRc = nullptr;
};

} // namespace Testing
} // namespace chip
