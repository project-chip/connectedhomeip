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
#pragma once

#include <app/InteractionModelDelegatePointers.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace app {

class TimedHandler;

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
class TimedHandlerDelegate
{
public:
    virtual ~TimedHandlerDelegate() = default;

    /**
     * Called when a timed invoke is received.  This function takes over all
     * handling of the exchange, status reporting, and so forth.
     */
    virtual void OnTimedInvoke(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                               const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) = 0;

    /**
     * Called when a timed write is received.  This function takes over all
     * handling of the exchange, status reporting, and so forth.
     */
    virtual void OnTimedWrite(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                              const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) = 0;

    /**
     * Called when a timed interaction has failed (i.e. the exchange it was
     * happening on has closed while the exchange delegate was the timed
     * handler).
     */
    virtual void OnTimedInteractionFailed(TimedHandler * apTimedHandler) = 0;
};

class TimedHandler : public Messaging::ExchangeDelegate
{
public:
    TimedHandler(TimedHandlerDelegate * delegate) : mDelegate(delegate) {}
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

    State mState = State::kExpectingTimedAction;

    /// This may be "fake" pointer or a real delegate pointer, depending
    /// on CHIP_CONFIG_STATIC_GLOBAL_INTERACTION_MODEL_ENGINE setting.
    ///
    /// When this is not a real pointer, it checks that the value is always
    /// set to the global InteractionModelEngine and the size of this
    /// member is 1 byte.
    InteractionModelDelegatePointer<TimedHandlerDelegate> mDelegate;

    // We keep track of the time limit for message reception, in case our
    // exchange's "response expected" timer gets delayed and does not fire when
    // the time runs out.
    //
    // NOTE: mTimeLimit needs to be 8-byte aligned on ARM so we place this last,
    //       to allow previous values to potentially use remaining packing space.
    //       Rationale:
    //         -   vtable is 4-byte aligned on 32-bit arm
    //         -   mTimeLimit requires 8-byte aligment
    //         =>  As a result we may gain 4 bytes if we place mTimeLimit last.
    // Expectation of memory layout:
    //   - vtable pointer (4 bytes & 4 byte alignment)
    //   - other members  (2 bytes on embedded "global pointer" arm)
    //                    (2 bytes padding for 8-byte alignment)
    //   - mTimeLimit     (8 bytes & 8 byte alignment)
    System::Clock::Timestamp mTimeLimit;
};

} // namespace app
} // namespace chip
