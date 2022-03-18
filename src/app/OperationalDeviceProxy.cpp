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
    }
}

CHIP_ERROR OperationalDeviceProxy::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                                           Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mState)
    {
    case State::Uninitialized:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;

    case State::NeedsAddress:
        err = LookupPeerAddress();
        EnqueueConnectionCallbacks(onConnection, onFailure);
        break;

    case State::Initialized:
        err = EstablishConnection();
        if (err == CHIP_NO_ERROR)
        {
            EnqueueConnectionCallbacks(onConnection, onFailure);
        }
        break;
    case State::Connecting:
        EnqueueConnectionCallbacks(onConnection, onFailure);
        break;

    case State::SecureConnected:
        if (onConnection != nullptr)
        {
            onConnection->mCall(onConnection->mContext, this);
        }
        break;

    default:
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    if (err != CHIP_NO_ERROR && onFailure != nullptr)
    {
        onFailure->mCall(onFailure->mContext, mPeerId, err);
    }

    return err;
}

CHIP_ERROR OperationalDeviceProxy::UpdateDeviceData(const Transport::PeerAddress & addr,
                                                    const ReliableMessageProtocolConfig & config)
{
    VerifyOrReturnLogError(mState != State::Uninitialized, CHIP_ERROR_INCORRECT_STATE);

#if CHIP_DETAIL_LOGGING
    char peerAddrBuff[Transport::PeerAddress::kMaxToStringSize];
    addr.ToString(peerAddrBuff);

    ChipLogDetail(Controller, "Updating device address to %s while in state %d", peerAddrBuff, static_cast<int>(mState));
#endif

    CHIP_ERROR err = CHIP_NO_ERROR;
    mDeviceAddress = addr;

    mMRPConfig = config;

    // Initialize CASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by CASE session protocol messages that include MRP parameters.
    if (mCASEClient)
    {
        mCASEClient->SetMRPIntervals(mMRPConfig);
    }

    if (mState == State::NeedsAddress)
    {
        MoveToState(State::Initialized);
        err = EstablishConnection();
        if (err != CHIP_NO_ERROR)
        {
            OnSessionEstablishmentError(err);
        }
    }
    else
    {
        if (!mSecureSession)
        {
            // Nothing needs to be done here.  It's not an error to not have a
            // secureSession.  For one thing, we could have gotten an different
            // UpdateAddress already and that caused connections to be torn down and
            // whatnot.
            return CHIP_NO_ERROR;
        }

        mSecureSession.Get()->AsSecureSession()->SetPeerAddress(addr);
    }

    return err;
}

bool OperationalDeviceProxy::GetAddress(Inet::IPAddress & addr, uint16_t & port) const
{
    if (mState == State::Uninitialized || mState == State::NeedsAddress)
    {
        return false;
    }

    addr = mDeviceAddress.GetIPAddress();
    port = mDeviceAddress.GetPort();
    return true;
}

CHIP_ERROR OperationalDeviceProxy::EstablishConnection()
{
    mCASEClient = mInitParams.clientPool->Allocate(CASEClientInitParams{
        mInitParams.sessionManager, mInitParams.exchangeMgr, mInitParams.idAllocator, mFabricInfo, mInitParams.mrpLocalConfig });
    ReturnErrorCodeIf(mCASEClient == nullptr, CHIP_ERROR_NO_MEMORY);
    CHIP_ERROR err =
        mCASEClient->EstablishSession(mPeerId, mDeviceAddress, mMRPConfig, HandleCASEConnected, HandleCASEConnectionFailure, this);
    ReturnErrorOnFailure(err);

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

void OperationalDeviceProxy::DequeueConnectionSuccessCallbacks(bool executeCallback)
{
    Cancelable ready;
    mConnectionSuccess.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(ready.mNext);

        cb->Cancel();
        if (executeCallback)
        {
            cb->mCall(cb->mContext, this);
        }
    }
}

void OperationalDeviceProxy::DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback)
{
    Cancelable ready;
    mConnectionFailure.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnDeviceConnectionFailure> * cb =
            Callback::Callback<OnDeviceConnectionFailure>::FromCancelable(ready.mNext);

        cb->Cancel();
        if (executeCallback)
        {
            cb->mCall(cb->mContext, mPeerId, error);
        }
    }
}

void OperationalDeviceProxy::HandleCASEConnectionFailure(void * context, CASEClient * client, CHIP_ERROR error)
{
    OperationalDeviceProxy * device = static_cast<OperationalDeviceProxy *>(context);
    VerifyOrReturn(device->mState != State::Uninitialized && device->mState != State::NeedsAddress,
                   ChipLogError(Controller, "HandleCASEConnectionFailure was called while the device was not initialized"));
    VerifyOrReturn(client == device->mCASEClient, ChipLogError(Controller, "HandleCASEConnectionFailure for unknown CASEClient"));

    device->MoveToState(State::Initialized);

    device->CloseCASESession();
    device->DequeueConnectionSuccessCallbacks(/* executeCallback */ false);
    device->DequeueConnectionFailureCallbacks(error, /* executeCallback */ true);
    // Do not touch device anymore; it might have been destroyed by a failure
    // callback.
}

void OperationalDeviceProxy::HandleCASEConnected(void * context, CASEClient * client)
{
    OperationalDeviceProxy * device = static_cast<OperationalDeviceProxy *>(context);
    VerifyOrReturn(device->mState != State::Uninitialized,
                   ChipLogError(Controller, "HandleCASEConnected was called while the device was not initialized"));
    VerifyOrReturn(client == device->mCASEClient, ChipLogError(Controller, "HandleCASEConnected for unknown CASEClient"));

    CHIP_ERROR err = client->DeriveSecureSessionHandle(device->mSecureSession);
    if (err != CHIP_NO_ERROR)
    {
        device->HandleCASEConnectionFailure(context, client, err);
        // Do not touch device anymore; it might have been destroyed by a
        // HandleCASEConnectionFailure.
    }
    else
    {
        device->MoveToState(State::SecureConnected);

        device->CloseCASESession();
        device->DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, /* executeCallback */ false);
        device->DequeueConnectionSuccessCallbacks(/* executeCallback */ true);
        // Do not touch device anymore; it might have been destroyed by a
        // success callback.
    }
}

CHIP_ERROR OperationalDeviceProxy::Disconnect()
{
    ReturnErrorCodeIf(mState != State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession)
    {
        mInitParams.sessionManager->ExpirePairing(mSecureSession.Get());
    }
    MoveToState(State::Initialized);
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::SetConnectedSession(const SessionHandle & handle)
{
    mSecureSession.Grab(handle);
    MoveToState(State::SecureConnected);
}

void OperationalDeviceProxy::Clear()
{
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }

    MoveToState(State::Uninitialized);
    mInitParams = DeviceProxyInitParams();
}

void OperationalDeviceProxy::CloseCASESession()
{
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }
}

void OperationalDeviceProxy::OnSessionReleased()
{
    MoveToState(State::Initialized);
}

CHIP_ERROR OperationalDeviceProxy::ShutdownSubscriptions()
{
    return app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mFabricInfo->GetFabricIndex(), GetDeviceId());
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

    DequeueConnectionSuccessCallbacks(/* executeCallback */ false);
    DequeueConnectionFailureCallbacks(reason, /* executeCallback */ true);
}

} // namespace chip
