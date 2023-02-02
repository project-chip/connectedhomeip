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
        mState = aTargetState;

        if (aTargetState != State::Connecting)
        {
            CleanupCASEClient();
        }
    }
}

bool OperationalSessionSetup::AttachToExistingSecureSession()
{
    VerifyOrReturnError(mState == State::NeedsAddress || mState == State::ResolvingAddress || mState == State::HasAddress, false);

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
                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    bool isConnected = false;

    //
    // Always enqueue our user provided callbacks into our callback list.
    // If anything goes wrong below, we'll trigger failures (including any queued from
    // a previous iteration which in theory shouldn't happen, but this is written to be more defensive)
    //
    EnqueueConnectionCallbacks(onConnection, onFailure);

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

void OperationalSessionSetup::UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config)
{
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

    CHIP_ERROR err = CHIP_NO_ERROR;
    mDeviceAddress = addr;

    // Initialize CASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by CASE session protocol messages that include MRP parameters.
    if (mCASEClient)
    {
        mCASEClient->SetRemoteMRPIntervals(config);
    }

    if (mState == State::ResolvingAddress)
    {
        MoveToState(State::HasAddress);
        mInitParams.sessionManager->UpdateAllSessionsPeerAddress(mPeerId, addr);
        if (!mPerformingAddressUpdate)
        {
            err = EstablishConnection(config);
            if (err != CHIP_NO_ERROR)
            {
                DequeueConnectionCallbacks(err);
                // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
                return;
            }
            // We expect to get a callback via OnSessionEstablished or OnSessionEstablishmentError to continue
            // the state machine forward.
            return;
        }

        DequeueConnectionCallbacks(CHIP_NO_ERROR);
        // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
        return;
    }

    ChipLogError(Discovery, "Received UpdateDeviceData in incorrect state");
    DequeueConnectionCallbacks(CHIP_ERROR_INCORRECT_STATE);
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
                                                         Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    if (onConnection != nullptr)
    {
        mConnectionSuccess.Enqueue(onConnection->Cancel());
    }

    if (onFailure != nullptr)
    {
        mConnectionFailure.Enqueue(onFailure->Cancel());
    }
}

void OperationalSessionSetup::DequeueConnectionCallbacks(CHIP_ERROR error)
{
    Cancelable failureReady, successReady;

    //
    // Dequeue both failure and success callback lists into temporary stack args before invoking either of them.
    // We do this since we may not have a valid 'this' pointer anymore upon invoking any of those callbacks
    // since the callee may destroy this object as part of that callback.
    //
    mConnectionFailure.DequeueAll(failureReady);
    mConnectionSuccess.DequeueAll(successReady);

    //
    // If we encountered no error, go ahead and call all success callbacks. Otherwise,
    // call the failure callbacks.
    //
    while (failureReady.mNext != &failureReady)
    {
        // We expect that we only have callbacks if we are not performing just address update.
        VerifyOrDie(!mPerformingAddressUpdate);
        Callback::Callback<OnDeviceConnectionFailure> * cb =
            Callback::Callback<OnDeviceConnectionFailure>::FromCancelable(failureReady.mNext);

        cb->Cancel();

        if (error != CHIP_NO_ERROR)
        {
            cb->mCall(cb->mContext, mPeerId, error);
        }
    }

    while (successReady.mNext != &successReady)
    {
        // We expect that we only have callbacks if we are not performing just address update.
        VerifyOrDie(!mPerformingAddressUpdate);
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(successReady.mNext);

        cb->Cancel();
        if (error == CHIP_NO_ERROR)
        {
            auto * exchangeMgr = mInitParams.exchangeMgr;
            VerifyOrDie(exchangeMgr);
            // We know that we for sure have the SessionHandle in the successful case.
            auto optionalSessionHandle = mSecureSession.Get();
            cb->mCall(cb->mContext, *exchangeMgr, optionalSessionHandle.Value());
        }
    }
    VerifyOrDie(mReleaseDelegate != nullptr);
    mReleaseDelegate->ReleaseSession(this);
}

void OperationalSessionSetup::OnSessionEstablishmentError(CHIP_ERROR error)
{
    VerifyOrReturn(mState != State::Uninitialized && mState != State::NeedsAddress,
                   ChipLogError(Discovery, "HandleCASEConnectionFailure was called while the device was not initialized"));

    if (CHIP_ERROR_TIMEOUT == error)
    {
        if (CHIP_NO_ERROR == Resolver::Instance().TryNextResult(mAddressLookupHandle))
        {
            MoveToState(State::ResolvingAddress);
            return;
        }

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        if (mRemainingAttempts > 0)
        {
            CHIP_ERROR err = ScheduleSessionSetupReattempt();
            if (err == CHIP_NO_ERROR)
            {
                return;
            }
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    }

    DequeueConnectionCallbacks(error);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::OnSessionEstablished(const SessionHandle & session)
{
    VerifyOrReturn(mState != State::Uninitialized,
                   ChipLogError(Discovery, "HandleCASEConnected was called while the device was not initialized"));

    if (!mSecureSession.Grab(session))
        return; // Got an invalid session, do not change any state

    MoveToState(State::SecureConnected);

    DequeueConnectionCallbacks(CHIP_NO_ERROR);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::CleanupCASEClient()
{
    if (mCASEClient)
    {
        mClientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
}

void OperationalSessionSetup::OnSessionReleased()
{
    // This is unlikely to be called since within the same call that we get SessionHandle we
    // then call DequeueConnectionCallbacks which releases `this`. If this is called, and we
    // we have any callbacks we will just send an error.
    DequeueConnectionCallbacks(CHIP_ERROR_INCORRECT_STATE);
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

    // Does it make sense to ScheduleSessionSetupReattempt() here?  DNS-SD
    // resolution has its own retry/backoff mechanisms, so if it's failed we
    // have already done a lot of that.

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
}

CHIP_ERROR OperationalSessionSetup::ScheduleSessionSetupReattempt()
{
    VerifyOrDie(mRemainingAttempts > 0);
    // Try again, but not if things are in shutdown such that we can't get
    // to a system layer, and not if we've run out of attempts.
    if (!mInitParams.exchangeMgr->GetSessionManager() || !mInitParams.exchangeMgr->GetSessionManager()->SystemLayer())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    MoveToState(State::NeedsAddress);
    System::Clock::Seconds16 timerDelay;
    // Stop exponential backoff before our delays get too large.
    //
    // Note that mAttemptsDone is always > 0 here, because we have
    // just finished one attempt.
    VerifyOrDie(mAttemptsDone > 0);
    static_assert(UINT16_MAX / CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS >=
                      (1 << CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF),
                  "Our backoff calculation will overflow.");
    timerDelay = System::Clock::Seconds16(
        static_cast<uint16_t>(CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_INITIAL_DELAY_SECONDS
                              << min((mAttemptsDone - 1), CHIP_DEVICE_CONFIG_AUTOMATIC_CASE_RETRY_MAX_BACKOFF)));
    CHIP_ERROR err = mInitParams.exchangeMgr->GetSessionManager()->SystemLayer()->StartTimer(timerDelay, TrySetupAgain, this);
    // The cast on count() is needed because the type count() returns might not
    // actually be uint16_t; on some platforms it's int.
    ChipLogProgress(Discovery,
                    "OperationalSessionSetup:attempts done: %u, attempts left: %u, retry delay %us, status %" CHIP_ERROR_FORMAT,
                    mAttemptsDone, mRemainingAttempts, static_cast<unsigned>(timerDelay.count()), err.Format());
    return err;
}

void OperationalSessionSetup::TrySetupAgain(System::Layer * systemLayer, void * state)
{
    auto * self = static_cast<OperationalSessionSetup *>(state);

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (self->mState != State::NeedsAddress)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    else
    {
        self->MoveToState(State::ResolvingAddress);
        err = self->LookupPeerAddress();
        if (err == CHIP_NO_ERROR)
        {
            return;
        }
    }

    // Give up; we're either in a bad state or could not start a lookup.
    self->DequeueConnectionCallbacks(err);
    // Do not touch `self` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

} // namespace chip
