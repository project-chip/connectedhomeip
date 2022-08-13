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
        ChipLogDetail(Controller, "OperationalSessionSetup[%u:" ChipLogFormatX64 "]: State change %d --> %d",
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

    ChipLogProgress(Controller, "Found an existing secure session to [%u:" ChipLogFormatX64 "]!", mPeerId.GetFabricIndex(),
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
            err = EstablishConnection();
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

    mRemoteMRPConfig = config;

    // Initialize CASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by CASE session protocol messages that include MRP parameters.
    if (mCASEClient)
    {
        mCASEClient->SetRemoteMRPIntervals(mRemoteMRPConfig);
    }

    if (mState == State::ResolvingAddress)
    {
        MoveToState(State::HasAddress);
        mInitParams.sessionManager->UpdateAllSessionsPeerAddress(mPeerId, addr);
        if (!mPerformingAddressUpdate)
        {
            err = EstablishConnection();
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

    ChipLogError(Controller, "Received UpdateDeviceData in incorrect state");
    DequeueConnectionCallbacks(CHIP_ERROR_INCORRECT_STATE);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

CHIP_ERROR OperationalSessionSetup::EstablishConnection()
{
    mCASEClient = mInitParams.clientPool->Allocate(CASEClientInitParams{
        mInitParams.sessionManager, mInitParams.sessionResumptionStorage, mInitParams.certificateValidityPolicy,
        mInitParams.exchangeMgr, mFabricTable, mInitParams.groupDataProvider, mInitParams.mrpLocalConfig });
    ReturnErrorCodeIf(mCASEClient == nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = mCASEClient->EstablishSession(mPeerId, mDeviceAddress, mRemoteMRPConfig, this);
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

    // TODO Issue #20452: For now we need to make a copy of member fields inside `this` before calling any of the
    // callbacks since the callbacks themselves can potentially release `this` OperationalSessionSetup.
    auto * exchangeMgr = mInitParams.exchangeMgr;
    auto sessionHandle = mSecureSession.Get();
    auto peerId        = mPeerId;
    VerifyOrDie(mReleaseDelegate != nullptr);
    auto * releaseDelegate = mReleaseDelegate;

    //
    // If we encountered no error, go ahead and call all success callbacks. Otherwise,
    // call the failure callbacks.
    //
    while (failureReady.mNext != &failureReady)
    {
        Callback::Callback<OnDeviceConnectionFailure> * cb =
            Callback::Callback<OnDeviceConnectionFailure>::FromCancelable(failureReady.mNext);

        cb->Cancel();

        if (error != CHIP_NO_ERROR)
        {
            cb->mCall(cb->mContext, peerId, error);
        }
    }

    while (successReady.mNext != &successReady)
    {
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(successReady.mNext);

        cb->Cancel();
        if (error == CHIP_NO_ERROR)
        {
            // We know that we for sure have the SessionHandle in the successful case.
            VerifyOrDie(exchangeMgr);
            cb->mCall(cb->mContext, *exchangeMgr, sessionHandle.Value());
        }
    }

    releaseDelegate->ReleaseSession(this);
}

void OperationalSessionSetup::OnSessionEstablishmentError(CHIP_ERROR error)
{
    VerifyOrReturn(mState != State::Uninitialized && mState != State::NeedsAddress,
                   ChipLogError(Controller, "HandleCASEConnectionFailure was called while the device was not initialized"));

    //
    // We don't need to reset the state all the way back to NeedsAddress since all that transpired
    // was just CASE connection failure. So let's re-use the cached address to re-do CASE again
    // if need-be.
    //
    MoveToState(State::HasAddress);

    DequeueConnectionCallbacks(error);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::OnSessionEstablished(const SessionHandle & session)
{
    VerifyOrReturn(mState != State::Uninitialized,
                   ChipLogError(Controller, "HandleCASEConnected was called while the device was not initialized"));

    if (!mSecureSession.Grab(session))
        return; // Got an invalid session, do not change any state

    MoveToState(State::SecureConnected);

    DequeueConnectionCallbacks(CHIP_NO_ERROR);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

void OperationalSessionSetup::Disconnect()
{
    VerifyOrReturn(mState == State::SecureConnected);

    if (mSecureSession)
    {
        //
        // Mark the session as defunct to signal that we no longer want to use this
        // session anymore for further interactions to this peer. However, if we receive
        // messages back from that peer on the defunct session, it will bring it back into an active
        // state again.
        //
        mSecureSession.Get().Value()->AsSecureSession()->MarkAsDefunct();
    }

    mSecureSession.Release();

    MoveToState(State::HasAddress);
}

void OperationalSessionSetup::CleanupCASEClient()
{
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
}

void OperationalSessionSetup::OnSessionReleased()
{
    MoveToState(State::HasAddress);
}

void OperationalSessionSetup::OnSessionHang()
{
    Disconnect();
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
        mInitParams.clientPool->Release(mCASEClient);
    }
}

CHIP_ERROR OperationalSessionSetup::LookupPeerAddress()
{
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

    auto const * fabricInfo = mFabricTable->FindFabricWithIndex(mPeerId.GetFabricIndex());
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
        ChipLogError(Controller, "PerformAddressUpdate could not perform lookup");
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

    if (IsResolvingAddress())
    {
        MoveToState(State::NeedsAddress);
    }

    // No need to set mPerformingAddressUpdate to false since call below releases `this`.
    DequeueConnectionCallbacks(reason);
    // Do not touch `this` instance anymore; it has been destroyed in DequeueConnectionCallbacks.
}

} // namespace chip
