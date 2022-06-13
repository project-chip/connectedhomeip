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

#include "OperationalDeviceProxy.h"

#include "CASEClient.h"
#include "CommandSender.h"
#include "ReadPrepareParams.h"
#include "transport/SecureSession.h"

#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemLayer.h>

using namespace chip::Callback;
using chip::AddressResolve::NodeLookupRequest;
using chip::AddressResolve::Resolver;
using chip::AddressResolve::ResolveResult;

namespace chip {

void OperationalDeviceProxy::MoveToState(State aTargetState)
{
    if (mState != aTargetState)
    {
        ChipLogDetail(Controller, "OperationalDeviceProxy[" ChipLogFormatX64 ":" ChipLogFormatX64 "]: State change %d --> %d",
                      ChipLogValueX64(mPeerId.GetCompressedFabricId()), ChipLogValueX64(mPeerId.GetNodeId()), to_underlying(mState),
                      to_underlying(aTargetState));
        mState = aTargetState;

        if (aTargetState != State::Connecting)
        {
            CleanupCASEClient();
        }
    }
}

bool OperationalDeviceProxy::AttachToExistingSecureSession()
{
    VerifyOrReturnError(mState == State::NeedsAddress || mState == State::ResolvingAddress || mState == State::HasAddress, false);

    ScopedNodeId peerNodeId(mPeerId.GetNodeId(), mFabricIndex);
    auto sessionHandle =
        mInitParams.sessionManager->FindSecureSessionForNode(peerNodeId, MakeOptional(Transport::SecureSession::Type::kCASE));
    if (!sessionHandle.HasValue())
        return false;

    ChipLogProgress(Controller, "Found an existing secure session to [" ChipLogFormatX64 "-" ChipLogFormatX64 "]!",
                    ChipLogValueX64(mPeerId.GetCompressedFabricId()), ChipLogValueX64(mPeerId.GetNodeId()));
    mDeviceAddress = sessionHandle.Value()->AsSecureSession()->GetPeerAddress();
    if (!mSecureSession.Grab(sessionHandle.Value()))
        return false;

    return true;
}

void OperationalDeviceProxy::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
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
    }
}

void OperationalDeviceProxy::UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config)
{
    if (mState == State::Uninitialized)
    {
        return;
    }

#if CHIP_DETAIL_LOGGING
    char peerAddrBuff[Transport::PeerAddress::kMaxToStringSize];
    addr.ToString(peerAddrBuff);

    ChipLogDetail(Controller, "Updating device address to %s while in state %d", peerAddrBuff, static_cast<int>(mState));
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
        err = EstablishConnection();
        if (err != CHIP_NO_ERROR)
        {
            DequeueConnectionCallbacks(err);
        }
    }
    else
    {
        if (!mSecureSession)
        {
            // Nothing needs to be done here.  It's not an error to not have a
            // secureSession.  For one thing, we could have gotten a different
            // UpdateAddress already and that caused connections to be torn down and
            // whatnot.
            return;
        }

        mSecureSession.Get().Value()->AsSecureSession()->SetPeerAddress(addr);
    }
}

CHIP_ERROR OperationalDeviceProxy::EstablishConnection()
{
    mCASEClient = mInitParams.clientPool->Allocate(CASEClientInitParams{
        mInitParams.sessionManager, mInitParams.sessionResumptionStorage, mInitParams.certificateValidityPolicy,
        mInitParams.exchangeMgr, mFabricTable, mFabricIndex, mInitParams.groupDataProvider, mInitParams.mrpLocalConfig });
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

void OperationalDeviceProxy::EnqueueConnectionCallbacks(Callback::Callback<OnDeviceConnected> * onConnection,
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

void OperationalDeviceProxy::DequeueConnectionCallbacks(CHIP_ERROR error)
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
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(successReady.mNext);

        cb->Cancel();
        if (error == CHIP_NO_ERROR)
        {
            cb->mCall(cb->mContext, this);
        }
    }
}

void OperationalDeviceProxy::OnSessionEstablishmentError(CHIP_ERROR error)
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

    // Do not touch device instance anymore; it might have been destroyed by a failure callback.
}

void OperationalDeviceProxy::OnSessionEstablished(const SessionHandle & session)
{
    VerifyOrReturn(mState != State::Uninitialized,
                   ChipLogError(Controller, "HandleCASEConnected was called while the device was not initialized"));

    if (!mSecureSession.Grab(session))
        return; // Got an invalid session, do not change any state

    MoveToState(State::SecureConnected);
    DequeueConnectionCallbacks(CHIP_NO_ERROR);

    // Do not touch this instance anymore; it might have been destroyed by a callback.
}

CHIP_ERROR OperationalDeviceProxy::Disconnect()
{
    ReturnErrorCodeIf(mState != State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession)
    {
        mInitParams.sessionManager->ExpirePairing(mSecureSession.Get().Value());
    }
    MoveToState(State::HasAddress);

    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::CleanupCASEClient()
{
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
}

void OperationalDeviceProxy::OnSessionReleased()
{
    MoveToState(State::HasAddress);
}

void OperationalDeviceProxy::OnFirstMessageDeliveryFailed()
{
    LookupPeerAddress();
}

void OperationalDeviceProxy::OnSessionHang()
{
    // TODO: establish a new session
}

CHIP_ERROR OperationalDeviceProxy::ShutdownSubscriptions()
{
    return app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mFabricIndex, GetDeviceId());
}

OperationalDeviceProxy::~OperationalDeviceProxy()
{
    if (mAddressLookupHandle.IsActive())
    {
        ChipLogProgress(Discovery, "Cancelling incomplete address resolution as device is being deleted.");

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

CHIP_ERROR OperationalDeviceProxy::LookupPeerAddress()
{
    // NOTE: This is public API that can be used to update our stored peer
    // address even when we are in State::Connected, so we do not make any
    // MoveToState calls in this method.
    if (mAddressLookupHandle.IsActive())
    {
        ChipLogProgress(Discovery, "Operational node lookup already in progress. Will NOT start a new one.");
        return CHIP_NO_ERROR;
    }

    NodeLookupRequest request(mPeerId);

    return Resolver::Instance().LookupNode(request, mAddressLookupHandle);
}

void OperationalDeviceProxy::OnNodeAddressResolved(const PeerId & peerId, const ResolveResult & result)
{
    UpdateDeviceData(result.address, result.mrpConfig);
}

void OperationalDeviceProxy::OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason)
{
    ChipLogError(Discovery, "Operational discovery failed for 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueX64(peerId.GetNodeId()), reason.Format());

    if (IsResolvingAddress())
    {
        MoveToState(State::NeedsAddress);
    }

    DequeueConnectionCallbacks(reason);
}

} // namespace chip
