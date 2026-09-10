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

#include <controller/ControllerOperation.h>
#include <controller/InvokeInteraction.h>
#include <controller/NetworkIdentityRegistrar.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Defer.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

#include <limits>
#include <utility>

namespace chip {
namespace Controller {

/**
 * Callback for NetworkIdentityManagementRegistrar::WaitForIdle().
 */
typedef void (*OnNetworkIdentityRegistrarIdleFunct)(void * context);

/**
 * A NetworkIdentityRegistrar that drives the Network Identity Management cluster on a Network
 * Infrastructure Manager (NIM), using a borrowed DeviceController to reach it. (Note that this
 * may or may not be the same DeviceController that uses this registrar during commissioning.)
 *
 * Each operation independently obtains a CASE session to the NIM via the controller (relying on the
 * session caching in CASESessionManager) and then invokes a single command on it, so the registrar
 * holds no session of its own in between. Both the controller and the NIM node must remain valid
 * for the lifetime of the registrar.
 */
class DLL_EXPORT NetworkIdentityManagementRegistrar : public NetworkIdentityRegistrar
{
public:
    /**
     * @param controller  Controller used to reach the NIM. Borrowed; must outlive this object.
     * @param nodeId      Node ID of the NIM on the controller's fabric.
     * @param endpoint    Endpoint hosting the Network Identity Management cluster. There is no
     *                    fixed endpoint for it, so this has to be discovered or configured.
     */
    NetworkIdentityManagementRegistrar(DeviceController & controller, NodeId nodeId, EndpointId endpoint) :
        mController(controller), mNodeId(nodeId), mEndpoint(endpoint)
    {}
    ~NetworkIdentityManagementRegistrar() override { Shutdown(); }

    // Not copyable
    NetworkIdentityManagementRegistrar(const NetworkIdentityManagementRegistrar &)             = delete;
    NetworkIdentityManagementRegistrar & operator=(const NetworkIdentityManagementRegistrar &) = delete;

    /**
     * Refuses further calls, so that whatever is in flight now is all this registrar will ever have
     * to do, but leaves those outstanding operations running. Idempotent.
     *
     * Combine with WaitForIdle() to determine when the registrar is safe to deallocate.
     */
    void StopAcceptingRequests();

    /**
     * Refuses further calls and completes any outstanding operations with CHIP_ERROR_CANCELLED, so
     * that the registrar can be deallocated once this returns. Idempotent, and called by the
     * destructor, so an owner only needs it to reclaim one early.
     *
     * Note this releases any WaitForIdle() callbacks as well, since the registrar does end up idle:
     * an owner that goes on to deallocate the registrar itself must cancel its waiter first, or it
     * will be told the registrar is idle while this call is still unwinding.
     *
     * If WaitForIdle() is not used, the registrar may also be deallocated directly from a request
     * completion -- but not from one this method delivers: it aborts each outstanding operation in
     * turn, so it is still touching the registrar after any one of those completions returns. An
     * owner reclaiming the registrar from here deallocates it once this returns instead.
     */
    void Shutdown();

    /**
     * Registers a callback to be invoked once the registrar has no requests in flight, called
     * synchronously if it is idle already. The callback is unregistered once called.
     *
     * The callback may deallocate the registrar, which is the main reason to wait on it in the first
     * place. It may also start another request, in which case any remaining waiters stay registered
     * until the registrar is idle again.
     *
     * An owner that does not use this method at all may instead deallocate the registrar from the
     * completion of a request. Mixing the two is not supported: a completion runs while the registrar
     * is still unwinding, and it is only safe to deallocate there because with no waiters registered
     * there is nothing left for the registrar to do afterwards.
     */
    void WaitForIdle(Callback::Callback<OnNetworkIdentityRegistrarIdleFunct>::Owned onIdle);

    /**
     * Returns true if the registrar has no operations in flight.
     */
    bool IsIdle() const;

    // NetworkIdentityRegistrar implementation
    void GetNetworkIdentity(Callback::Callback<OnNetworkIdentityAvailableFunct>::Owned onCompletion) override;
    void RegisterClient(ByteSpan clientIdentity, Callback::Callback<OnClientRegisteredFunct>::Owned onCompletion) override;
    void UnregisterClient(Credentials::CertificateKeyId clientIdentifier,
                          Callback::Callback<OnClientUnregisteredFunct>::Owned onCompletion) override;

private:
    // Called as an operation finishes, i.e. once it has stopped being pending *and* delivered its
    // completion, to release the WaitForIdle() callers once the last one has.
    void OperationFinished();

    // Arranges for OperationFinished() to be called as the enclosing scope exits, which is how an
    // operation reports itself finished only once it has delivered its completion. Notifying
    // after the fact rather than from Operation::OnFinished() is what lets us see if a completion
    // that starts a new request: we are not idle then, and a caller waiting for us to be must not
    // hear otherwise.
    // Whether there is anything to notify is decided up front, to allow an owner that does not use
    // WaitForIdle() to deallocate the registrar from a completion callback; otherwise the deferred
    // OperationFinished() call would be a use-after-free. Note that WaitForIdle() from within the
    // completion works as expected regardless: If we're already idle it invokes the callback
    // synchronously, and if we are not they will be enqueued, and the deferred OperationFinished()
    // wouldn't have called them yet anyway.
    // Defined ahead of Operation, which uses it, because its return type is deduced.
    [[nodiscard]] auto DeferOperationFinished()
    {
        return MakeDefer([registrar = mIdleWaiters.IsEmpty() ? nullptr : this] {
            VerifyOrReturn(registrar != nullptr);
            registrar->OperationFinished();
        });
    }

    // What our three operations have in common: each connects to the NIM, invokes a single command
    // on it, and reports anything that stops it getting an answer back to the caller. A subclass
    // sends its command from OnConnected() via InvokeCommand(), which is the only thing it should
    // be doing there: InvokeCommand() completes the operation one way or the other.
    class Operation : public ControllerOperationBase
    {
        using Base = ControllerOperationBase;

    public:
        explicit Operation(NetworkIdentityManagementRegistrar & registrar) : mRegistrar(registrar) {}

        // Completes the operation with CHIP_ERROR_CANCELLED if it is in flight, otherwise a no-op.
        // Does not DeferOperationFinished(): the registrar is the one calling this method.
        void AbortIfPending();

    protected:
        void Start(DeviceController & controller, NodeId nodeId, EndpointId endpoint, Callback::Cancelable::Owned onCompletion)
        {
            mEndpoint = endpoint;
            Base::Start(controller, nodeId, std::move(onCompletion));
        }

        // Completes the operation (whatever its completion signature is) based on the given error.
        // Every failure a started operation can suffer arrives here, so a subclass has a single
        // place to make sense of them.
        virtual void Fail(CHIP_ERROR error) = 0;

        // Whether the command has gone out, i.e. whether a failure from here on could still have
        // taken effect on the NIM. Note this is cleared as the operation finishes, which happens
        // before the completion is delivered, so it has to be read on the way into Complete().
        bool CommandSent() const { return mCommandSent; }

        // Sends the given request to the endpoint passed to Start(), reporting a response to
        // onSuccess, which is responsible for completing the operation, or any failure (including
        // failure to send at all) to Fail(). The operation is reported finished to the registrar
        // once either of those calls returns.
        //
        // The invocation is tied to the operation's lifecycle: it is cancelled if the operation is
        // cancelled or completed before the response arrives, so nothing is left pointing at the
        // operation. Note the handlers run with the invocation already released: cancelling one
        // deletes the CommandSender, which must not happen from within its own callback. The
        // CommandSender tears itself down as the callback returns, so there is nothing left to
        // cancel at that point anyway.
        template <typename RequestType, typename OnSuccess>
        void InvokeCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & session, const RequestType & request,
                           OnSuccess onSuccess, const Optional<uint16_t> & timedInvokeTimeoutMs = NullOptional,
                           const Optional<System::Clock::Timeout> & responseTimeout = NullOptional)
        {
            // Record the command as sent before it goes out, since a handler may run before
            // InvokeCommandRequest() returns and is free to reuse or destroy the operation.
            mCommandSent   = true;
            CHIP_ERROR err = InvokeCommandRequest(
                &exchangeMgr, session, mEndpoint, request,
                [this, onSuccess](auto &&... args) {
                    mCancelInvoke = nullptr;
                    auto notify   = mRegistrar.DeferOperationFinished();
                    onSuccess(std::forward<decltype(args)>(args)...);
                },
                [this](CHIP_ERROR error) {
                    mCancelInvoke = nullptr;
                    auto notify   = mRegistrar.DeferOperationFinished();
                    Fail(error);
                },
                timedInvokeTimeoutMs, responseTimeout, &mCancelInvoke);
            if (err != CHIP_NO_ERROR)
            {
                mCommandSent = false; // no handler ran, and nothing was sent after all
                auto notify  = mRegistrar.DeferOperationFinished();
                Fail(err);
            }
        }

    private:
        void OnConnectionFailure(CHIP_ERROR error) final;
        void OnFinished(bool cancelled) final;

        NetworkIdentityManagementRegistrar & mRegistrar;
        Internal::InvokeCancelFn mCancelInvoke;
        EndpointId mEndpoint = kInvalidEndpointId;
        bool mCommandSent    = false; // see CommandSent()
    };

    class QueryIdentityOperation final : public Callback::TypedOperation<Operation, CHIP_ERROR, ByteSpan>
    {
        using Base = Callback::TypedOperation<Operation, CHIP_ERROR, ByteSpan>;

    public:
        using Base::Base;
        using Base::Start;

    private:
        void OnConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & session) override;
        void Fail(CHIP_ERROR error) override { Complete(error, ByteSpan()); }
    };

    class AddClientOperation final : public Callback::TypedOperation<Operation, CHIP_ERROR, bool>
    {
        using Base = Callback::TypedOperation<Operation, CHIP_ERROR, bool>;

    public:
        using Base::Base;

        // Takes a copy of the identity, since the caller's span does not outlive the call.
        void Start(DeviceController & controller, NodeId nodeId, EndpointId endpoint, ByteSpan clientIdentity,
                   Completion::Owned onCompletion);

    private:
        void OnConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & session) override;

        // A failure is only determinate if the AddClient never went out: once it has, an error that
        // stopped the NIM from acting on it is indistinguishable from one that lost us the answer.
        void Fail(CHIP_ERROR error) override { Complete(error, /* determinate = */ !CommandSent()); }

        uint8_t mClientIdentity[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        uint8_t mClientIdentityLength = 0;
        static_assert(std::numeric_limits<decltype(mClientIdentityLength)>::max() >= sizeof(mClientIdentity));
    };

    class RemoveClientOperation final : public Callback::TypedOperation<Operation, CHIP_ERROR>
    {
        using Base = Callback::TypedOperation<Operation, CHIP_ERROR>;

    public:
        using Base::Base;

        // Takes a copy of the identifier, since the caller's span does not outlive the call.
        void Start(DeviceController & controller, NodeId nodeId, EndpointId endpoint,
                   Credentials::CertificateKeyId clientIdentifier, Completion::Owned onCompletion);

    private:
        void OnConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & session) override;
        void Fail(CHIP_ERROR error) override;

        Credentials::CertificateKeyIdStorage mClientIdentifier;
    };

    DeviceController & mController;
    const NodeId mNodeId;
    const EndpointId mEndpoint;

    bool mStopped = false;                // set by StopAcceptingRequests() and never cleared
    Callback::CallbackDeque mIdleWaiters; // WaitForIdle() callbacks

    QueryIdentityOperation mQueryIdentity{ *this };
    AddClientOperation mAddClient{ *this };
    RemoveClientOperation mRemoveClient{ *this };
};

} // namespace Controller
} // namespace chip
