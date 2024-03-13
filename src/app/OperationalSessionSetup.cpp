/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *  @file
 *    This file contains implementation of Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#include <app/OperationalSessionSetup.h>

#include <app/CASEClient.h>
#include <app/InteractionModelEngine.h>
#include <transport/SecureSession.h>

#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

using namespace chip::Callback;
using chip::AddressResolve::NodeLookupRequest;
using chip::AddressResolve::Resolver;
using chip::AddressResolve::ResolveResult;

namespace chip {

void OperationalSessionSetup::MoveToState(State aTargetState)
{
    if (mState != aTargetState)
    {
        ChipLogDetail(Discovery, "OperationalSessionSetup[%u:" ChipLogFormatX64 "]: State change %d --> %d",
                      mPeerId.GetFabricIndex(), ChipLogValueX64(mPeerId.GetNodeId()), to_underlying(mState),
                      to_underlying(aTargetState));

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        if (mState == State::WaitingForRetry)
        {
            CancelSessionSetupReattempt();
        }
#endif

        mState = aTargetState;

        if (aTargetState != State::Connecting)
        {
            CleanupCASEClient();
        }
    }
}

bool OperationalSessionSetup::AttachToExistingSecureSession()
{
    VerifyOrReturnError(mState == State::NeedsAddress || mState == State::ResolvingAddress || mState == State::HasAddress ||
                            mState == State::WaitingForRetry,
                        false);

    auto sessionHandle =
        mInitParams.sessionManager->FindSecureSessionForNode(mPeerId, MakeOptional(Transport::SecureSession::Type::kCASE));
    if (!sessionHandle.HasValue())
        return false;

    ChipLogProgress(Discovery, "Found an existing secure session to [%u:" ChipLogFormatX64 "]!", mPeerId.GetFabricIndex(),
                    ChipLogValueX64(mPeerId.GetNodeId()));

    mDeviceAddress = sessionHandle.Value()->AsSecureSession()->GetPeerAddress();
    if (!mSecureSession.Grab(sessionHandle.Value()))
        return false;

    return true;
}

void OperationalSessionSetup::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                                      Callback::Callback<OnSetupFailure> * onSetupFailure)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    bool isConnected = false;

    //
    // Always enqueue our user provided callbacks into our callback list.
    // If anything goes wrong below, we'll trigger failures (including any queued from
    // a previous iteration which in theory shouldn't happen, but this is written to be more defensive)
    //
    EnqueueConnectionCallbacks(onConnection, onFailure, onSetupFailure);

    switch (mState)
    {
    case State::Uninitialized:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;

    case State::NeedsAddress:
        isConnected = AttachToExistingSecureSession();
        if (!isConnected)
        {
            // LookupPeerAddress could perhaps call back with a result
            // synchronously, so do our state update first.
            MoveToState(State::ResolvingAddress);
            err = LookupPeerAddress();
            if (err != CHIP_NO_ERROR)
            {
                // Roll back the state change, since we are presumably not in
                // the middle of a lookup.
                MoveToState(State::NeedsAddress);
            }
        }

        break;

    case State::ResolvingAddress:
    case State::WaitingForRetry:
        isConnected = AttachToExistingSecureSession();
        break;

    case State::HasAddress:
        isConnected = AttachToExistingSecureSession();
        if (!isConnected)
        {
            // We should not actually every be in be in State::HasAddress. This
            // is because in the same call that we moved to State::HasAddress
            // we either move to State::Connecting or call
            // DequeueConnectionCallbacks with an error thus releasing
            // ourselves before any call would reach this section of code.
            err = CHIP_ERROR_INCORRECT_STATE;
        }

        break;

    case State::Connecting:
        break;

    case State::SecureConnected:
        isConnected = true;
        break;

    default:
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    if (isConnected)
    {
        MoveToState(State::SecureConnected);
    }

    //
    // Dequeue all our callbacks on either encountering an error
    // or if we successfully connected. Both should not be set
    // simultaneously.
    //
    if (err != CHIP_NO_ERROR || isConnected)
    {
        DequeueConnectionCallbacks(err);
        // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
        // While it is odd to have an explicit return here at the end of the function, we do so
        // as a precaution in case someone later on adds something to the end of this function.
        return;
    }
}

void OperationalSessionSetup::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    Connect(onConnection, onFailure, nullptr);
}

void OperationalSessionSetup::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                                      Callback::Callback<OnSetupFailure> * onSetupFailure)
{
    Connect(onConnection, nullptr, onSetupFailure);
}

void OperationalSessionSetup::UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config)
{
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    // Make sure to clear out our reason for trying the next result first thing,
    // so it does not stick around in various error cases.
    bool tryingNextResultDueToSessionEstablishmentError = mTryingNextResultDueToSessionEstablishmentError;
    mTryingNextResultDueToSessionEstablishmentError     = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    if (mState == State::Uninitialized)
    {
        return;
    }

#if CHIP_DETAIL_LOGGING
    char peerAddrBuff[Transport::PeerAddress::kMaxToStringSize];
    addr.ToString(peerAddrBuff);

    ChipLogDetail(Discovery, "OperationalSessionSetup[%u:" ChipLogFormatX64 "]: Updating device address to %s while in state %d",
                  mPeerId.GetFabricIndex(), ChipLogValueX64(mPeerId.GetNodeId()), peerAddrBuff, static_cast<int>(mState));
#endif

    mDeviceAddress = addr;

    // Initialize CASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by CASE session protocol messages that include MRP parameters.
    if (mCASEClient)
    {
        mCASEClient->SetRemoteMRPIntervals(config);
    }

    if (mState != State::ResolvingAddress)
    {
        ChipLogError(Discovery, "Received UpdateDeviceData in incorrect state");
        DequeueConnectionCallbacks(CHIP_ERROR_INCORRECT_STATE);
        // Do not touch `this` instance anymore; it has been destroyed in
        // DequeueConnectionCallbacks.
        return;
    }

    MoveToState(State::HasAddress);
    mInitParams.sessionManager->UpdateAllSessionsPeerAddress(mPeerId, addr);

    if (mPerformingAddressUpdate)
    {
        // Nothing else to do here.
        DequeueConnectionCallbacks(CHIP_NO_ERROR);
        // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
        return;
    }

    CHIP_ERROR err = EstablishConnection(config);
    LogErrorOnFailure(err);
    if (err == CHIP_NO_ERROR)
    {
        // We expect to get a callback via OnSessionEstablished or OnSessionEstablishmentError to continue
        // the state machine forward.
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        if (tryingNextResultDueToSessionEstablishmentError)
        {
            // Our retry has already been kicked off, so claim 0 delay until it
            // starts.  We only reach this from OnSessionEstablishmentError when
            // the error is CHIP_ERROR_TIMEOUT.
            NotifyRetryHandlers(CHIP_ERROR_TIMEOUT, config, System::Clock::kZero);
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        return;
    }

    // Move to the ResolvingAddress state, in case we have more results,
    // since we expect to receive results in that state.  Pretend like we moved
    // on directly to this address from whatever triggered us to try this result
    // (so restore mTryingNextResultDueToSessionEstablishmentError to the value
    // it had at the start of this function).
    MoveToState(State::ResolvingAddress);
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    mTryingNextResultDueToSessionEstablishmentError = tryingNextResultDueToSessionEstablishmentError;
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    if (CHIP_NO_ERROR == Resolver::Instance().TryNextResult(mAddressLookupHandle))
    {
        // No need to NotifyRetryHandlers, since we never actually spent any
        // time trying the previous result.  Whatever work we need to do has
        // been handled by our recursive OnNodeAddressResolved callback.  Make
        // sure not to touch `this` under here, because it might have been
        // deleted by OnNodeAddressResolved.
        return;
    }

    // No need to reset mTryingNextResultDueToSessionEstablishmentError here,
    // because we're about to delete ourselves.

    DequeueConnectionCallbacks(err);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

CHIP_ERROR OperationalSessionSetup::EstablishConnection(const ReliableMessageProtocolConfig & config)
{
    mCASEClient = mClientPool->Allocate();
    ReturnErrorCodeIf(mCASEClient == nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = mCASEClient->EstablishSession(mInitParams, mPeerId, mDeviceAddress, config, this);
    if (err != CHIP_NO_ERROR)
    {
        CleanupCASEClient();
        return err;
    }

    MoveToState(State::Connecting);

    return CHIP_NO_ERROR;
}

void OperationalSessionSetup::EnqueueConnectionCallbacks(Callback::Callback<OnDeviceConnected> * onConnection,
                                                         Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                                                         Callback::Callback<OnSetupFailure> * onSetupFailure)
{
    if (onConnection != nullptr)
    {
        mConnectionSuccess.Enqueue(onConnection->Cancel());
    }

    if (onFailure != nullptr)
    {
        mConnectionFailure.Enqueue(onFailure->Cancel());
    }

    if (onSetupFailure != nullptr)
    {
        mSetupFailure.Enqueue(onSetupFailure->Cancel());
    }
}

void OperationalSessionSetup::DequeueConnectionCallbacks(CHIP_ERROR error, SessionEstablishmentStage stage,
                                                         ReleaseBehavior releaseBehavior)
{
    Cancelable failureReady, setupFailureReady, successReady;

    //
    // Dequeue both failure and success callback lists into temporary stack args before invoking either of them.
    // We do this since we may not have a valid 'this' pointer anymore upon invoking any of those callbacks
    // since the callee may destroy this object as part of that callback.
    //
    mConnectionFailure.DequeueAll(failureReady);
    mSetupFailure.DequeueAll(setupFailureReady);
    mConnectionSuccess.DequeueAll(successReady);

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    // Clear out mConnectionRetry, so that those cancelables are not holding
    // pointers to us, since we're about to go away.
    while (auto * cb = mConnectionRetry.First())
    {
        cb->Cancel();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    // Gather up state we will need for our notifications.
    bool performingAddressUpdate                  = mPerformingAddressUpdate;
    auto * exchangeMgr                            = mInitParams.exchangeMgr;
    Optional<SessionHandle> optionalSessionHandle = mSecureSession.Get();
    ScopedNodeId peerId                           = mPeerId;

    if (releaseBehavior == ReleaseBehavior::Release)
    {
        VerifyOrDie(mReleaseDelegate != nullptr);
        mReleaseDelegate->ReleaseSession(this);
    }

    // DO NOT touch any members of this object after this point.  It's dead.

    NotifyConnectionCallbacks(failureReady, setupFailureReady, successReady, error, stage, peerId, performingAddressUpdate,
                              exchangeMgr, optionalSessionHandle);
}

void OperationalSessionSetup::NotifyConnectionCallbacks(Cancelable & failureReady, Cancelable & setupFailureReady,
                                                        Cancelable & successReady, CHIP_ERROR error,
                                                        SessionEstablishmentStage stage, const ScopedNodeId & peerId,
                                                        bool performingAddressUpdate, Messaging::ExchangeManager * exchangeMgr,
                                                        const Optional<SessionHandle> & optionalSessionHandle)
{
    //
    // If we encountered no error, go ahead and call all success callbacks. Otherwise,
    // call the failure callbacks.
    //
    while (failureReady.mNext != &failureReady)
    {
        // We expect that we only have callbacks if we are not performing just address update.
        VerifyOrDie(!performingAddressUpdate);
        Callback::Callback<OnDeviceConnectionFailure> * cb =
            Callback::Callback<OnDeviceConnectionFailure>::FromCancelable(failureReady.mNext);

        cb->Cancel();

        if (error != CHIP_NO_ERROR)
        {
            cb->mCall(cb->mContext, peerId, error);
        }
    }

    while (setupFailureReady.mNext != &setupFailureReady)
    {
        // We expect that we only have callbacks if we are not performing just address update.
        VerifyOrDie(!performingAddressUpdate);
        Callback::Callback<OnSetupFailure> * cb = Callback::Callback<OnSetupFailure>::FromCancelable(setupFailureReady.mNext);

        cb->Cancel();

        if (error != CHIP_NO_ERROR)
        {
            // Initialize the ConnnectionFailureInfo object
            ConnnectionFailureInfo failureInfo(peerId, error, stage);
            cb->mCall(cb->mContext, failureInfo);
        }
    }

    while (successReady.mNext != &successReady)
    {
        // We expect that we only have callbacks if we are not performing just address update.
        VerifyOrDie(!performingAddressUpdate);
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(successReady.mNext);

        cb->Cancel();
        if (error == CHIP_NO_ERROR)
        {
            VerifyOrDie(exchangeMgr);
            // We know that we for sure have the SessionHandle in the successful case.
            cb->mCall(cb->mContext, *exchangeMgr, optionalSessionHandle.Value());
        }
    }
}

void OperationalSessionSetup::OnSessionEstablishmentError(CHIP_ERROR error, SessionEstablishmentStage stage)
{
    VerifyOrReturn(mState == State::Connecting,
                   ChipLogError(Discovery, "OnSessionEstablishmentError was called while we were not connecting"));

    // If this condition ever changes, we may need to store the error in a
    // member instead of having a boolean
    // mTryingNextResultDueToSessionEstablishmentError, so we can recover the
    // error in UpdateDeviceData.
    if (CHIP_ERROR_TIMEOUT == error || CHIP_ERROR_BUSY == error)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        // Make a copy of the ReliableMessageProtocolConfig, since our
        // mCaseClient is about to go away once we change state.
        ReliableMessageProtocolConfig remoteMprConfig = mCASEClient->GetRemoteMRPIntervals();
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

        // Move to the ResolvingAddress state, in case we have more results,
        // since we expect to receive results in that state.
        MoveToState(State::ResolvingAddress);
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        mTryingNextResultDueToSessionEstablishmentError = true;
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        if (CHIP_NO_ERROR == Resolver::Instance().TryNextResult(mAddressLookupHandle))
        {
            // Whatever work we needed to do has been handled by our
            // OnNodeAddressResolved callback.  Make sure not to touch `this`
            // under here, because it might have been deleted by
            // OnNodeAddressResolved.
            return;
        }
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        mTryingNextResultDueToSessionEstablishmentError = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

        // Moving back to the Connecting state would be a bit of a lie, since we
        // don't have an mCASEClient.  Just go back to NeedsAddress, since
        // that's really where we are now.
        MoveToState(State::NeedsAddress);

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        if (mRemainingAttempts > 0)
        {
            System::Clock::Seconds16 reattemptDelay;
            CHIP_ERROR err = ScheduleSessionSetupReattempt(reattemptDelay);
            if (err == CHIP_NO_ERROR)
            {
                MoveToState(State::WaitingForRetry);
                NotifyRetryHandlers(error, remoteMprConfig, reattemptDelay);
                return;
            }
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    }

    DequeueConnectionCallbacks(error, stage);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::OnResponderBusy(System::Clock::Milliseconds16 requestedDelay)
{
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    // Store the requested delay, so that we can use it for scheduling our
    // retry.
    mRequestedBusyDelay = requestedDelay;
#endif
}

void OperationalSessionSetup::OnSessionEstablished(const SessionHandle & session)
{
    VerifyOrReturn(mState == State::Connecting,
                   ChipLogError(Discovery, "OnSessionEstablished was called while we were not connecting"));

    if (!mSecureSession.Grab(session))
    {
        // Got an invalid session, just dispatch an error.  We have to do this
        // so we don't leak.
        DequeueConnectionCallbacks(CHIP_ERROR_INCORRECT_STATE);

        // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
        return;
    }

    MoveToState(State::SecureConnected);

    DequeueConnectionCallbacks(CHIP_NO_ERROR);
}

void OperationalSessionSetup::CleanupCASEClient()
{
    if (mCASEClient)
    {
        mClientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
}

OperationalSessionSetup::~OperationalSessionSetup()
{
    if (mAddressLookupHandle.IsActive())
    {
        ChipLogDetail(Discovery,
                      "OperationalSessionSetup[%u:" ChipLogFormatX64
                      "]: Cancelling incomplete address resolution as device is being deleted.",
                      mPeerId.GetFabricIndex(), ChipLogValueX64(mPeerId.GetNodeId()));

        // Skip cancel callback since the destructor is being called, so we assume that this object is
        // obviously not used anymore
        CHIP_ERROR err = Resolver::Instance().CancelLookup(mAddressLookupHandle, Resolver::FailureCallback::Skip);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Lookup cancel failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    if (mCASEClient)
    {
        // Make sure we don't leak it.
        mClientPool->Release(mCASEClient);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    CancelSessionSetupReattempt();
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    DequeueConnectionCallbacks(CHIP_ERROR_CANCELLED, ReleaseBehavior::DoNotRelease);
}

CHIP_ERROR OperationalSessionSetup::LookupPeerAddress()
{
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    if (mRemainingAttempts > 0)
    {
        --mRemainingAttempts;
    }
    if (mAttemptsDone < UINT8_MAX)
    {
        ++mAttemptsDone;
    }
    if (mResolveAttemptsAllowed > 0)
    {
        --mResolveAttemptsAllowed;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    // NOTE: This is public API that can be used to update our stored peer
    // address even when we are in State::Connected, so we do not make any
    // MoveToState calls in this method.
    if (mAddressLookupHandle.IsActive())
    {
        ChipLogProgress(Discovery,
                        "OperationalSessionSetup[%u:" ChipLogFormatX64
                        "]: Operational node lookup already in progress. Will NOT start a new one.",
                        mPeerId.GetFabricIndex(), ChipLogValueX64(mPeerId.GetNodeId()));
        return CHIP_NO_ERROR;
    }

    auto const * fabricInfo = mInitParams.fabricTable->FindFabricWithIndex(mPeerId.GetFabricIndex());
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);

    PeerId peerId(fabricInfo->GetCompressedFabricId(), mPeerId.GetNodeId());

    NodeLookupRequest request(peerId);

    return Resolver::Instance().LookupNode(request, mAddressLookupHandle);
}

void OperationalSessionSetup::PerformAddressUpdate()
{
    if (mPerformingAddressUpdate)
    {
        // We are already in the middle of a lookup from a previous call to
        // PerformAddressUpdate. In that case we will just exit right away as
        // we are already looking to update the results from the previous lookup.
        return;
    }

    // We must be newly-allocated to handle this address lookup, so must be in the NeedsAddress state.
    VerifyOrDie(mState == State::NeedsAddress);

    // We are doing an address lookup whether we have an active session for this peer or not.
    mPerformingAddressUpdate = true;
    MoveToState(State::ResolvingAddress);
    CHIP_ERROR err = LookupPeerAddress();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Failed to look up peer address: %" CHIP_ERROR_FORMAT, err.Format());
        DequeueConnectionCallbacks(err);
        // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
        return;
    }
}

void OperationalSessionSetup::OnNodeAddressResolved(const PeerId & peerId, const ResolveResult & result)
{
    UpdateDeviceData(result.address, result.mrpRemoteConfig);
}

void OperationalSessionSetup::OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason)
{
    ChipLogError(Discovery, "OperationalSessionSetup[%u:" ChipLogFormatX64 "]: operational discovery failed: %" CHIP_ERROR_FORMAT,
                 mPeerId.GetFabricIndex(), ChipLogValueX64(mPeerId.GetNodeId()), reason.Format());

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    // If we're in a mode where we would generally retry CASE, retry operational
    // discovery if we're allowed to.  That allows us to more-gracefully handle broken networks
    // where multicast DNS does not actually work and hence only the initial
    // unicast DNS-SD queries get a response.
    //
    // We check for State::ResolvingAddress just in case in the meantime
    // something weird happened and we are no longer trying to resolve an
    // address.
    if (mState == State::ResolvingAddress && mResolveAttemptsAllowed > 0)
    {
        ChipLogProgress(Discovery, "Retrying operational DNS-SD discovery. Attempts remaining: %u", mResolveAttemptsAllowed);

        // Pretend like our previous attempt (i.e. call to LookupPeerAddress)
        // has not happened for purposes of the generic attempt counters, so we
        // don't mess up the counters for our actual CASE retry logic.
        if (mRemainingAttempts < UINT8_MAX)
        {
            ++mRemainingAttempts;
        }
        if (mAttemptsDone > 0)
        {
            --mAttemptsDone;
        }

        CHIP_ERROR err = LookupPeerAddress();
        if (err == CHIP_NO_ERROR)
        {
            // We need to notify our consumer that the resolve will take more
            // time, but we don't actually know how much time it will take,
            // because the resolver does not expose that information.  Just use
            // one minute to be safe.
            using namespace chip::System::Clock::Literals;
            NotifyRetryHandlers(reason, 60_s16);
            return;
        }
    }
#endif

    // No need to modify any variables in `this` since call below releases `this`.
    DequeueConnectionCallbacks(reason);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
void OperationalSessionSetup::UpdateAttemptCount(uint8_t attemptCount)
{
    if (attemptCount == 0)
    {
        // Nothing to do.
        return;
    }

    if (mState != State::NeedsAddress)
    {
        // We're in the middle of an attempt already, so decrement attemptCount
        // by 1 to account for that.
        --attemptCount;
    }

    if (attemptCount > mRemainingAttempts)
    {
        mRemainingAttempts = attemptCount;
    }

    if (attemptCount > mResolveAttemptsAllowed)
    {
        mResolveAttemptsAllowed = attemptCount;
    }
}

CHIP_ERROR OperationalSessionSetup::ScheduleSessionSetupReattempt(System::Clock::Seconds16 & timerDelay)
{
    VerifyOrDie(mRemainingAttempts > 0);
    // Try again, but not if things are in shutdown such that we can't get
    // to a system layer, and not if we've run out of attempts.
    if (!mInitParams.exchangeMgr->GetSessionManager() || !mInitParams.exchangeMgr->GetSessionManager()->SystemLayer())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    MoveToState(State::NeedsAddress);
    // Stop exponential backoff before our delays get too large.
    //
    // Note that mAttemptsDone is always > 0 here, because we have
    // just finished one attempt.
    VerifyOrDie(mAttemptsDone > 0);
    static_assert(UINT16_MAX / CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS >=
                      (1 << CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF),
                  "Our backoff calculation will overflow.");
    System::Clock::Timeout actualTimerDelay = System::Clock::Seconds16(
        static_cast<uint16_t>(CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS
                              << min((mAttemptsDone - 1), CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF)));
    const bool responseWasBusy = mRequestedBusyDelay != System::Clock::kZero;
    if (responseWasBusy)
    {
        if (mRequestedBusyDelay > actualTimerDelay)
        {
            actualTimerDelay = mRequestedBusyDelay;
        }

        // Reset mRequestedBusyDelay now that we have consumed it, so it does
        // not affect future reattempts not triggered by a busy response.
        mRequestedBusyDelay = System::Clock::kZero;
    }

    if (mAttemptsDone % 2 == 0)
    {
        // It's possible that the other side received one of our Sigma1 messages
        // and then failed to get its Sigma2 back to us.  If that's the case, it
        // will be waiting for that Sigma2 to time out before it starts
        // listening for Sigma1 messages again.
        //
        // To handle that, on every other retry, add the amount of time it would
        // take the other side to time out.  It would be nice if we could rely
        // on the delay reported in a BUSY response to just tell us that value,
        // but in practice for old devices BUSY often sends some hardcoded value
        // that tells us nothing about when the other side will decide it has
        // timed out.
        auto additionalTimeout = CASESession::ComputeSigma2ResponseTimeout(GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
        actualTimerDelay += additionalTimeout;
    }
    timerDelay = std::chrono::duration_cast<System::Clock::Seconds16>(actualTimerDelay);

    CHIP_ERROR err = mInitParams.exchangeMgr->GetSessionManager()->SystemLayer()->StartTimer(actualTimerDelay, TrySetupAgain, this);

    // TODO: If responseWasBusy, should we increment, mRemainingAttempts and
    // mResolveAttemptsAllowed, since we were explicitly told to retry?  Hard to
    // tell what consumers expect out of a capped retry count here.

    // The cast on count() is needed because the type count() returns might not
    // actually be uint16_t; on some platforms it's int.
    ChipLogProgress(Discovery,
                    "OperationalSessionSetup:attempts done: %u, attempts left: %u, retry delay %us, status %" CHIP_ERROR_FORMAT,
                    mAttemptsDone, mRemainingAttempts, static_cast<unsigned>(timerDelay.count()), err.Format());
    return err;
}

void OperationalSessionSetup::CancelSessionSetupReattempt()
{
    // If we can't get a system layer, there is no way for us to cancel things
    // at this point, but hopefully that's because everything is torn down
    // anyway and hence the timer will not fire.
    auto * sessionManager = mInitParams.exchangeMgr->GetSessionManager();
    VerifyOrReturn(sessionManager != nullptr);

    auto * systemLayer = sessionManager->SystemLayer();
    VerifyOrReturn(systemLayer != nullptr);

    systemLayer->CancelTimer(TrySetupAgain, this);
}

void OperationalSessionSetup::TrySetupAgain(System::Layer * systemLayer, void * state)
{
    auto * self = static_cast<OperationalSessionSetup *>(state);

    self->MoveToState(State::ResolvingAddress);
    CHIP_ERROR err = self->LookupPeerAddress();
    if (err == CHIP_NO_ERROR)
    {
        return;
    }

    // Give up; we could not start a lookup.
    self->DequeueConnectionCallbacks(err);
    // Do not touch `self` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::AddRetryHandler(Callback::Callback<OnDeviceConnectionRetry> * onRetry)
{
    mConnectionRetry.Enqueue(onRetry->Cancel());
}

void OperationalSessionSetup::NotifyRetryHandlers(CHIP_ERROR error, const ReliableMessageProtocolConfig & remoteMrpConfig,
                                                  System::Clock::Seconds16 retryDelay)
{
    // Compute the time we are likely to need to detect that the retry has
    // failed.
    System::Clock::Timeout messageTimeout = CASESession::ComputeSigma1ResponseTimeout(remoteMrpConfig);
    auto timeoutSecs                      = std::chrono::duration_cast<System::Clock::Seconds16>(messageTimeout);
    // Add 1 second in case we had fractional milliseconds in messageTimeout.
    using namespace chip::System::Clock::Literals;
    NotifyRetryHandlers(error, timeoutSecs + 1_s16 + retryDelay);
}

void OperationalSessionSetup::NotifyRetryHandlers(CHIP_ERROR error, System::Clock::Seconds16 timeoutEstimate)
{
    // We have to be very careful here: Calling into these handlers might in
    // theory destroy the Callback objects involved, but unlike the
    // succcess/failure cases we don't want to just clear the handlers from our
    // list when we are calling them, because we might need to call a given
    // handler more than once.
    //
    // To handle this we:
    //
    // 1) Snapshot the list of handlers up front, so if any of the handlers
    //    triggers an AddRetryHandler with some other handler that does not
    //    affect the list we plan to notify here.
    //
    // 2) When planning to notify a handler move it to a new list that contains
    //    just that handler.  This way if it gets canceled as part of the
    //    notification we can tell it has been canceled.
    //
    // 3) If notifying the handler does not cancel it, add it back to our list
    //    of handlers so we will notify it on future retries.

    Cancelable retryHandlerListSnapshot;
    mConnectionRetry.DequeueAll(retryHandlerListSnapshot);

    while (retryHandlerListSnapshot.mNext != &retryHandlerListSnapshot)
    {
        auto * cb = Callback::Callback<OnDeviceConnectionRetry>::FromCancelable(retryHandlerListSnapshot.mNext);

        Callback::CallbackDeque currentCallbackHolder;
        currentCallbackHolder.Enqueue(cb->Cancel());

        cb->mCall(cb->mContext, mPeerId, error, timeoutEstimate);

        if (currentCallbackHolder.mNext != &currentCallbackHolder)
        {
            // Callback has not been canceled as part of the call, so is still
            // supposed to be registered with us.
            AddRetryHandler(cb);
        }
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

} // namespace chip
