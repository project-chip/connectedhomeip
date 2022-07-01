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

/** @brief
 *      This provides a wrapper around ExchangeContext that automatically manages
 *      cleaning up the EC without any extra involvement. This is meant to be used
 *      by application and protocol logic code that would otherwise need to closely
 *      manange their internal pointers to an ExchangeContext and correctly
 *      null-it out/abort it depending on the circumstances. This relies on clear rules
 *      established by ExchangeContext and the transfer of ownership at various points
 *      in its lifetime.
 *
 *      It does this by listening in on OnExchangeClosing and looking at the various
 *      states the exchange might be in to decide amongst a couple of different tactics.
 *
 *      This is a delegate forwarder - consumers can still register to be an ExchangeDelegate
 *      and get notified of all relevant happenings on that delegate interface.
 *
 */
class ExchangeHolder : public ExchangeDelegate
{
public:
    ExchangeHolder(ExchangeDelegate & delegate) : mpExchangeDelegate(delegate) {}

    virtual ~ExchangeHolder() { AbortIfNeeded(); }

    bool Contains(const ExchangeContext * exchange) const { return mpExchangeCtx == exchange; }

    /*
     *  Aborts any previously tracked exchange and
     *  re-acquires the provided exchange. As part of that,
     *  this will set itself as the delegate on that exchange (and forward
     *  calls to the registered delegate on this object).
     *
     */
    void Grab(ExchangeContext * exchange)
    {
        AbortIfNeeded();

        mpExchangeCtx = exchange;
        mpExchangeCtx->SetDelegate(this);
    }

    /*
     *  Releases any previously tracked exchanges by nulling
     *  out ourselves as a delegate and then, our reference to that
     *  exchange
     */
    void Release()
    {
        if (mpExchangeCtx)
        {
            mpExchangeCtx->SetDelegate(nullptr);
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
    /*
     * This asseses if an Abort() needs to be called on the exchange. This method is only called when the holder itself
     * is being destroyed and it's still holding onto to a reference to an exchange. It then calls Abort() only if:
     *      1. The exchange is currently awaiting a response. This would have happened if our consumer just sent a message on the
     * exchange and is awaiting a response. Consequently, we should go ahead and Abort() it given destruction of the holder
     * indicates a lack of further interest in this exchange.
     *
     *      2. Our consumer has signaled an interest in sending a message. This could have been signaled right at exchange creation
     * time as the initiator, or when handling a message and the consumer intends to send a response, albeit, asynchronously.
     *
     */
    void AbortIfNeeded()
    {
        if (mpExchangeCtx)
        {
            mpExchangeCtx->SetDelegate(nullptr);

            if (mpExchangeCtx->IsResponseExpected() || mpExchangeCtx->IsSendExpected())
            {
                mpExchangeCtx->Abort();
            }
        }

        mpExchangeCtx = nullptr;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return mpExchangeDelegate.OnMessageReceived(ec, payloadHeader, std::move(payload));
    }

    void OnResponseTimeout(ExchangeContext * ec) override { return mpExchangeDelegate.OnResponseTimeout(ec); }

    void OnExchangeClosing(ExchangeContext * ec) override
    {
        Release();
        mpExchangeDelegate.OnExchangeClosing(ec);
    }

    ExchangeDelegate & mpExchangeDelegate;
    ExchangeContext * mpExchangeCtx = nullptr;
};

} // namespace Messaging
} // namespace chip
