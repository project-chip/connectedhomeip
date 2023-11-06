/*
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

#pragma once

#include <lib/core/Optional.h>
#include <lib/support/IntrusiveList.h>
#include <messaging/ExchangeContext.h>

#ifndef CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
#define CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING 0
#endif // CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING

namespace chip {
namespace Messaging {

/**
 * @brief
 *   This provides a RAII'fied wrapper for an ExchangeContext that automatically manages
 *   cleaning up the EC when the holder ceases to exist, or acquires a new exchange. This is
 *   meant to be used by application and protocol logic code that would otherwise need to closely
 *   manage their internal pointers to an ExchangeContext and correctly
 *   null-it out/abort it depending on the circumstances. This relies on clear rules
 *   established by ExchangeContext and the transfer of ownership at various points
 *   in its lifetime.
 *
 *   An equivalent but simplified version of the rules around exchange management as specified in
 *   ExchangeDelegate.h are provided here for consumers:
 *
 *   1. When an exchange is allocated, the holder takes over ownership of the exchange when Grab() is invoked.
 *      Until a message is sent successfully, the holder will automatically manage the exchange until its
 *      destructor or Release() is invoked.
 *
 *   2. If you send a message successfully that doesn't require a response, invoking Get() on the holder there-after will return
 *      nullptr.
 *
 *   3. If you send a message successfully that does require a response, invoking Get() on the holder will return a valid
 *      pointer until the response is received or times out.
 *
 *   4. On reception of a message on an exchange, if you return from OnMessageReceived() and no messages were sent on that exchange,
 *      invoking Get() on the holder will return a nullptr.
 *
 *   5. If you invoke WillSendMessage() on the exchange in your implementation of OnMessageReceived indicating a desire to send a
 *      message later on the exchange, invoking Get() on the holder will return a valid exchange until SendMessage() on the exchange
 *      is called, at which point, rules 2 and 3 apply.
 *
 *   6. This is a delegate forwarder -  consumers can still register to be an ExchangeDelegate
 *      and get notified of all relevant happenings on that delegate interface.
 *
 *   7. At no point shall you call Abort/Close/Release/Retain on the exchange tracked by the holder.
 *
 */
class ExchangeHolder : public ExchangeDelegate
{
public:
    /**
     * @brief
     *    Constructor that takes an ExchangeDelegate that is forwarded all relevant
     *    calls from the underlying exchange.
     */
    ExchangeHolder(ExchangeDelegate & delegate) : mpExchangeDelegate(delegate) {}

    virtual ~ExchangeHolder()
    {
#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
        ChipLogDetail(ExchangeManager, "[%p] ~ExchangeHolder", this);
#endif
        Release();
    }

    bool Contains(const ExchangeContext * exchange) const { return mpExchangeCtx == exchange; }

    /**
     * @brief
     *    Replaces the held exchange and associated delegate to instead track the given ExchangeContext, aborting
     *    and dereferencing any previously held exchange as necessary. This method should be called whenever protocol logic
     *    that is managing this holder is transitioning from an outdated Exchange to a new one, often during
     *    the start of a new transaction.
     */
    void Grab(ExchangeContext * exchange)
    {
        VerifyOrDie(exchange != nullptr);

        Release();

        mpExchangeCtx = exchange;
        mpExchangeCtx->SetDelegate(this);

#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
        ChipLogDetail(ExchangeManager, "[%p] ExchangeHolder::Grab: Acquired EC %p", this, exchange);
#endif
    }

    /*
     * @brief
     *    This shuts down the exchange (if a valid one is being tracked) and releases our reference to it.
     */
    void Release()
    {
#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
        ChipLogDetail(ExchangeManager, "[%p] ExchangeHolder::Release: mpExchangeCtx = %p", this, mpExchangeCtx);
#endif

        if (mpExchangeCtx)
        {
            mpExchangeCtx->SetDelegate(nullptr);

            /**
             * Shutting down the exchange requires calling Abort() on the exchange selectively in the following scenarios:
             *      1. The exchange is currently awaiting a response. This would have happened if our consumer just sent a message
             * on the exchange and is awaiting a response. Since we no longer care to wait for the response, we don't care about
             * doing MRP retries for the send we just did, so abort the exchange.
             *
             *      2. Our consumer has signaled an interest in sending a message. This could have been signaled right at exchange
             * creation time as the initiator, or when handling a message and the consumer intends to send a response, albeit,
             * asynchronously. In both cases, the stack expects the exchange consumer to close/abort the EC if it no longer has
             * interest in it. Since we don't have a pending message at this point, calling Abort is OK here as well.
             *
             */
            if (mpExchangeCtx->IsResponseExpected() || mpExchangeCtx->IsSendExpected())
            {
#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
                ChipLogDetail(ExchangeManager, "[%p] ExchangeHolder::Release: Aborting!", this);
#endif
                mpExchangeCtx->Abort();
            }
        }

        mpExchangeCtx = nullptr;
    }

    explicit operator bool() const { return mpExchangeCtx != nullptr; }
    ExchangeContext * Get() const { return mpExchangeCtx; }

    ExchangeContext * operator->() const
    {
        VerifyOrDie(mpExchangeCtx != nullptr);
        return mpExchangeCtx;
    }

private:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return mpExchangeDelegate.OnMessageReceived(ec, payloadHeader, std::move(payload));
    }

    void OnResponseTimeout(ExchangeContext * ec) override { return mpExchangeDelegate.OnResponseTimeout(ec); }

    void OnExchangeClosing(ExchangeContext * ec) override
    {
#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
        ChipLogDetail(ExchangeManager, "[%p] ExchangeHolder::OnExchangeClosing: mpExchangeCtx: %p", this, mpExchangeCtx);
#endif

        if (mpExchangeCtx)
        {
            mpExchangeCtx->SetDelegate(nullptr);

            /**
             * Unless our consumer has signalled an intention to send a message in the future, the exchange
             * is owned by the exchange layer and it will automatically handle releasing the ref. So, just null
             * out our reference to it.
             */
            if (!mpExchangeCtx->IsSendExpected())
            {
#if CHIP_EXCHANGE_HOLDER_DETAIL_LOGGING
                ChipLogDetail(ExchangeManager, "[%p] ExchangeHolder::OnExchangeClosing: nulling out ref...", this);
#endif
                mpExchangeCtx = nullptr;
            }
        }

        mpExchangeDelegate.OnExchangeClosing(ec);
    }

    ExchangeMessageDispatch & GetMessageDispatch() override { return mpExchangeDelegate.GetMessageDispatch(); }

    ExchangeDelegate & mpExchangeDelegate;
    ExchangeContext * mpExchangeCtx = nullptr;
};

} // namespace Messaging
} // namespace chip
