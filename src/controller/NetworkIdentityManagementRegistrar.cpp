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

#include <controller/NetworkIdentityManagementRegistrar.h>

#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/data-model/NullObject.h>
#include <clusters/NetworkIdentityManagement/Commands.h>
#include <clusters/NetworkIdentityManagement/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>
#include <utility>

namespace chip {
namespace Controller {

using namespace app::Clusters::NetworkIdentityManagement;

namespace {

// AddClient and RemoveClient are timed invokes.
constexpr uint16_t kTimedInvokeTimeoutMs = 10000;

using OnIdleCallback = Callback::Callback<OnNetworkIdentityRegistrarIdleFunct>;

} // namespace

void NetworkIdentityManagementRegistrar::StopAcceptingRequests()
{
    mStopped = true;
}

void NetworkIdentityManagementRegistrar::Shutdown()
{
    StopAcceptingRequests(); // refuse further calls first, otherwise a callback could attempt to start a new operation

    mQueryIdentity.AbortIfPending();
    mAddClient.AbortIfPending();
    mRemoveClient.AbortIfPending();
    OperationFinished(); // whether or not we finished any operations, see OperationFinished() sentinel logic.
}

bool NetworkIdentityManagementRegistrar::IsIdle() const
{
    return !mQueryIdentity.IsPending() && !mAddClient.IsPending() && !mRemoveClient.IsPending();
}

void NetworkIdentityManagementRegistrar::WaitForIdle(OnIdleCallback::Owned onIdle)
{
    VerifyOrReturn(!IsIdle(), onIdle.Invoke());
    mIdleWaiters.Enqueue(onIdle.Take());
}

void NetworkIdentityManagementRegistrar::OperationFinished()
{
    VerifyOrReturn(IsIdle() && !mIdleWaiters.IsEmpty());

    // An idle callback can cause us to no longer be idle (by starting an operation), and can also
    // deallocate us (which calls Shutdown() and recurses into OperationFinished()), or trigger
    // a recursive OperationFinished() call in other ways. By always letting the innermost frame
    // do the work of calling any remaining callbacks, we can avoid touching `this` after it may
    // have been destroyed. The sentinel is enqueued last, so reaching the sentinel guarantees that
    // all waiters have been called.
    bool recursed = false;
    OnIdleCallback sentinel([](void * context) { *static_cast<bool *>(context) = true; }, &recursed);
    mIdleWaiters.Enqueue(sentinel.Cancel());

    while (!mIdleWaiters.IsEmpty())
    {
        auto * waiter = mIdleWaiters.First();
        Callback::CallbackDeque::Dequeue(waiter);
        OnIdleCallback::FromCancelable(waiter)->Invoke();
        VerifyOrReturn(!recursed); // destructor recurses, so after this point we know we're still alive
        VerifyOrReturn(IsIdle());
    }
}

void NetworkIdentityManagementRegistrar::GetNetworkIdentity(Callback::Callback<OnNetworkIdentityAvailableFunct>::Owned onCompletion)
{
    VerifyOrReturn(!mStopped, onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE, ByteSpan()));
    VerifyOrReturn(!mQueryIdentity.IsPending(), onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE, ByteSpan()));
    mQueryIdentity.Start(mController, mNodeId, mEndpoint, std::move(onCompletion));
}

void NetworkIdentityManagementRegistrar::RegisterClient(ByteSpan clientIdentity,
                                                        Callback::Callback<OnClientRegisteredFunct>::Owned onCompletion)
{
    VerifyOrReturn(!mStopped, onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE, /* determinate = */ true));
    VerifyOrReturn(!mAddClient.IsPending(), onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE, /* determinate = */ true));
    mAddClient.Start(mController, mNodeId, mEndpoint, clientIdentity, std::move(onCompletion));
}

void NetworkIdentityManagementRegistrar::UnregisterClient(Credentials::CertificateKeyId clientIdentifier,
                                                          Callback::Callback<OnClientUnregisteredFunct>::Owned onCompletion)
{
    VerifyOrReturn(!mStopped, onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE));
    VerifyOrReturn(!mRemoveClient.IsPending(), onCompletion.Invoke(CHIP_ERROR_INCORRECT_STATE));
    mRemoveClient.Start(mController, mNodeId, mEndpoint, clientIdentifier, std::move(onCompletion));
}

void NetworkIdentityManagementRegistrar::Operation::AbortIfPending()
{
    if (IsPending())
    {
        Fail(CHIP_ERROR_CANCELLED); // don't DeferOperationFinished()
    }
}

void NetworkIdentityManagementRegistrar::Operation::OnConnectionFailure(CHIP_ERROR error)
{
    ChipLogFailure(error, Controller, "Failed to establish a session with the Network Infrastructure Manager");
    auto notify = mRegistrar.DeferOperationFinished();
    Fail(error);
}

void NetworkIdentityManagementRegistrar::Operation::OnFinished(bool cancelled)
{
    // Tear down an invocation we are no longer interested in. Reaching this from within one of the
    // handlers installed by InvokeCommand() is not a concern: they clear mCancelInvoke first.
    if (mCancelInvoke)
    {
        mCancelInvoke();
        mCancelInvoke = nullptr;
    }

    ControllerOperationBase::OnFinished(cancelled);
    mCommandSent = false; // whatever we sent is done with; the operation is free to be started again

    if (cancelled)
    {
        mRegistrar.OperationFinished(); // no completion coming, we're finished now
    }
}

void NetworkIdentityManagementRegistrar::QueryIdentityOperation::OnConnected(Messaging::ExchangeManager & exchangeMgr,
                                                                             const SessionHandle & session)
{
    // Ask for the network's current identity of the only type PDC defines, rather than for a
    // specific entry in the NIM's table: which one is current is up to the NIM.
    Commands::QueryIdentity::Type request;
    request.networkIdentityType.Emplace(IdentityTypeEnum::kEcdsa);

    InvokeCommand(exchangeMgr, session, request,
                  [this](const app::ConcreteCommandPath &, const app::StatusIB &,
                         const Commands::QueryIdentityResponse::DecodableType & response) {
                      // The identity points into the response message, which is exactly as long-lived
                      // as the OnNetworkIdentityAvailable callback needs it to be.
                      Complete(CHIP_NO_ERROR, response.identity);
                  });
}

void NetworkIdentityManagementRegistrar::AddClientOperation::Start(DeviceController & controller, NodeId nodeId,
                                                                   EndpointId endpoint, ByteSpan clientIdentity,
                                                                   Completion::Owned onCompletion)
{
    // Refuse before starting, so that the completion is delivered without the operation ever taking
    // it on. This is also why the identity buffer is only touched once we know we are taking the
    // call on. Also reject an empty identity outright (otherwise memcpy would need a null guard).
    VerifyOrReturn(!clientIdentity.empty() && clientIdentity.size() <= sizeof(mClientIdentity),
                   onCompletion.Invoke(CHIP_ERROR_INVALID_ARGUMENT, /* determinate = */ true));
    memcpy(mClientIdentity, clientIdentity.data(), clientIdentity.size());
    mClientIdentityLength = static_cast<decltype(mClientIdentityLength)>(clientIdentity.size()); // range asserted at declaration

    Base::Start(controller, nodeId, endpoint, std::move(onCompletion));
}

void NetworkIdentityManagementRegistrar::AddClientOperation::OnConnected(Messaging::ExchangeManager & exchangeMgr,
                                                                         const SessionHandle & session)
{
    Commands::AddClient::Type request;
    request.clientIdentity = ByteSpan(mClientIdentity, mClientIdentityLength);

    InvokeCommand(
        exchangeMgr, session, request,
        [this](const app::ConcreteCommandPath &, const app::StatusIB &,
               const Commands::AddClientResponse::DecodableType & response) {
            ChipLogProgress(Controller, "Network Client Identity registered at client index %u", response.clientIndex);
            Complete(CHIP_NO_ERROR, /* determinate = */ true);
        },
        MakeOptional(kTimedInvokeTimeoutMs));
}

void NetworkIdentityManagementRegistrar::RemoveClientOperation::Start(DeviceController & controller, NodeId nodeId,
                                                                      EndpointId endpoint,
                                                                      Credentials::CertificateKeyId clientIdentifier,
                                                                      Completion::Owned onCompletion)
{
    memcpy(mClientIdentifier.data(), clientIdentifier.data(), mClientIdentifier.size());
    Base::Start(controller, nodeId, endpoint, std::move(onCompletion));
}

void NetworkIdentityManagementRegistrar::RemoveClientOperation::OnConnected(Messaging::ExchangeManager & exchangeMgr,
                                                                            const SessionHandle & session)
{
    Commands::RemoveClient::Type request;
    request.clientIdentifier.Emplace(ByteSpan(mClientIdentifier));

    InvokeCommand(
        exchangeMgr, session, request,
        [this](const app::ConcreteCommandPath &, const app::StatusIB &, const app::DataModel::NullObjectType &) {
            ChipLogProgress(Controller, "Network Client Identity revoked");
            Complete(CHIP_NO_ERROR);
        },
        MakeOptional(kTimedInvokeTimeoutMs));
}

void NetworkIdentityManagementRegistrar::RemoveClientOperation::Fail(CHIP_ERROR error)
{
    if (error == CHIP_IM_GLOBAL_STATUS(NotFound))
    {
        // Revocation is required to be idempotent, so this is a success as far as we care.
        ChipLogDetail(Controller, "Network Client Identity was already revoked");
        Complete(CHIP_NO_ERROR);
        return;
    }
    Complete(error);
}

} // namespace Controller
} // namespace chip
