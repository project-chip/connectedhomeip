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
 *   It does this by intercepting OnExchangeClosing and looking at the various
 *   states the exchange might be in to decide how best to correctly shutdown the exchange.
 *   (see AbortIfNeeded()).
 *
 *   This is a delegate forwarder - consumers can still register to be an ExchangeDelegate
 *   and get notified of all relevant happenings on that delegate interface.
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

    virtual ~ExchangeHolder() { Release(); }

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
    }

    /*
     * @brief
     *    This shuts down the exchange (if a valid one is being tracked) and releases our reference to it.
     */
    void Release()
    {
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
        if (mpExchangeCtx)
        {
            mpExchangeCtx->SetDelegate(nullptr);
            mpExchangeCtx = nullptr;
        }

        mpExchangeDelegate.OnExchangeClosing(ec);
    }

    ExchangeMessageDispatch & GetMessageDispatch() override { return mpExchangeDelegate.GetMessageDispatch(); }

    ExchangeDelegate & mpExchangeDelegate;
    ExchangeContext * mpExchangeCtx = nullptr;
};

} // namespace Messaging
} // namespace chip
