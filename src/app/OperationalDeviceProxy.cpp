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

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemLayer.h>

using namespace chip::Callback;

namespace chip {

CHIP_ERROR OperationalDeviceProxy::Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                                           Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                                           Dnssd::ResolverProxy * resolver)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mState)
    {
    case State::Uninitialized:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;

    case State::NeedsAddress:
        VerifyOrReturnError(resolver != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        if (resolver->ResolveNodeIdFromInternalCache(mPeerId, Inet::IPAddressType::kAny))
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = resolver->ResolveNodeId(mPeerId, Inet::IPAddressType::kAny);
        }
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
    };

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
        mState = State::Initialized;
        err    = EstablishConnection();
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

    mState = State::Connecting;

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

    device->mState = State::Initialized;

    device->DequeueConnectionSuccessCallbacks(/* executeCallback */ false);
    device->DequeueConnectionFailureCallbacks(error, /* executeCallback */ true);
    device->CloseCASESession();
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
    }
    else
    {
        device->mState = State::SecureConnected;

        device->DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, /* executeCallback */ false);
        device->DequeueConnectionSuccessCallbacks(/* executeCallback */ true);
        device->CloseCASESession();
    }
}

CHIP_ERROR OperationalDeviceProxy::Disconnect()
{
    ReturnErrorCodeIf(mState != State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession)
    {
        mInitParams.sessionManager->ExpirePairing(mSecureSession.Get());
    }
    mState = State::Initialized;
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
    mState = State::SecureConnected;
}

void OperationalDeviceProxy::Clear()
{
    if (mCASEClient)
    {
        mInitParams.clientPool->Release(mCASEClient);
        mCASEClient = nullptr;
    }

    mState      = State::Uninitialized;
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
    mState = State::Initialized;
}

CHIP_ERROR OperationalDeviceProxy::ShutdownSubscriptions()
{
    return app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mFabricInfo->GetFabricIndex(), GetDeviceId());
}

OperationalDeviceProxy::~OperationalDeviceProxy() {}

} // namespace chip
