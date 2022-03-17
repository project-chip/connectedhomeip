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
 *      Definition of a handler for timed interactions.
 *
 */

#pragma once

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

/**
 * A TimedHandler handles a Timed Request action and then waits for a
 * subsequent Invoke or Write action and hands those on to
 * InteractionModelEngine if they arrive soon enough.
 *
 * Lifetime handling:
 *
 * A TimedHandler is initially allocated when the Timed Request is received and
 * becomes the delegate for that exchange.  After that it remains alive until
 * either the exchange is closed or the interaction is handed on to the
 * InteractionModelEngine.
 */

namespace chip {
namespace app {

class TimedHandler : public Messaging::ExchangeDelegate
{
public:
    TimedHandler() {}
    ~TimedHandler() override {}

    // ExchangeDelegate implementation.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * aExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;

private:
    // ExchangeDelegate implementation.
    void OnResponseTimeout(Messaging::ExchangeContext *) override
    { /* We just want to allow the exchange to close */
    }
    void OnExchangeClosing(Messaging::ExchangeContext * aExchangeContext) override;

    void CancelTimer();

    /**
     * Handler for the Timed Request action.  This returns success if the Timed
     * Request action is parsed successfully and the success Status Response
     * action is sent, failure otherwise.
     */
    CHIP_ERROR HandleTimedRequestAction(Messaging::ExchangeContext * aExchangeContext, const PayloadHeader & aPayloadHeader,
                                        System::PacketBufferHandle && aPayload);

    enum class State : uint8_t
    {
        kExpectingTimedAction,     // Initial state: expecting a timed action.
        kReceivedTimedAction,      // Have received the timed action.  This can
                                   // be a terminal state if the action ends up
                                   // malformed.
        kExpectingFollowingAction, // Expecting write or invoke.
    };

    // Because we have a vtable pointer and mTimeLimit needs to be 8-byte
    // aligned on ARM, putting mState first here means we fit in 16 bytes on
    // 32-bit ARM, whereas if we put it second we'd be 24 bytes.
    // On platforms where either vtable pointers are 8 bytes or 64-bit ints can
    // be 4-byte-aligned the ordering here does not matter.
    State mState = State::kExpectingTimedAction;
    // We keep track of the time limit for message reception, in case our
    // exchange's "response expected" timer gets delayed and does not fire when
    // the time runs out.
    System::Clock::Timestamp mTimeLimit;
};

} // namespace app
} // namespace chip
