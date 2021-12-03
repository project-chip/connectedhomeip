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

#include "CommandSender.h"
#include "ReadPrepareParams.h"

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::Callback;

namespace chip {

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
        err = Dnssd::Resolver::Instance().ResolveNodeId(mPeerId, chip::Inet::IPAddressType::kAny);
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
        onFailure->mCall(onFailure->mContext, mPeerId.GetNodeId(), err);
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
    mCASESession.SetMRPConfig(mMRPConfig);

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
        if (!mSecureSession.HasValue())
        {
            // Nothing needs to be done here.  It's not an error to not have a
            // secureSession.  For one thing, we could have gotten an different
            // UpdateAddress already and that caused connections to be torn down and
            // whatnot.
            return CHIP_NO_ERROR;
        }

        Transport::SecureSession * secureSession = mInitParams.sessionManager->GetSecureSession(mSecureSession.Value());
        if (secureSession != nullptr)
        {
            secureSession->SetPeerAddress(addr);
        }
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
    // Create a UnauthenticatedSession for CASE pairing.
    // Don't use mSecureSession here, because mSecureSession is for encrypted communication.
    Optional<SessionHandle> session = mInitParams.sessionManager->CreateUnauthenticatedSession(mDeviceAddress, mMRPConfig);
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    Messaging::ExchangeContext * exchange = mInitParams.exchangeMgr->NewContext(session.Value(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(mCASESession.MessageDispatch().Init(mInitParams.sessionManager));

    uint16_t keyID = 0;
    ReturnErrorOnFailure(mInitParams.idAllocator->Allocate(keyID));

    ReturnErrorOnFailure(
        mCASESession.EstablishSession(mDeviceAddress, mInitParams.fabricInfo, mPeerId.GetNodeId(), keyID, exchange, this));

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
            cb->mCall(cb->mContext, mPeerId.GetNodeId(), error);
        }
    }
}

void OperationalDeviceProxy::OnSessionEstablishmentError(CHIP_ERROR error)
{
    VerifyOrReturn(mState != State::Uninitialized && mState != State::NeedsAddress,
                   ChipLogError(Controller, "OnSessionEstablishmentError was called while the device was not initialized"));

    mState = State::Initialized;
    mInitParams.idAllocator->Free(mCASESession.GetLocalSessionId());

    DequeueConnectionSuccessCallbacks(/* executeCallback */ false);
    DequeueConnectionFailureCallbacks(error, /* executeCallback */ true);
}

void OperationalDeviceProxy::OnSessionEstablished()
{
    VerifyOrReturn(mState != State::Uninitialized,
                   ChipLogError(Controller, "OnSessionEstablished was called while the device was not initialized"));

    CHIP_ERROR err = mInitParams.sessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(mDeviceAddress), mPeerId.GetNodeId(), &mCASESession,
        CryptoContext::SessionRole::kInitiator, mInitParams.fabricInfo->GetFabricIndex());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up CASE secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }
    mSecureSession.SetValue(SessionHandle(mPeerId.GetNodeId(), mCASESession.GetLocalSessionId(), mCASESession.GetPeerSessionId(),
                                          mInitParams.fabricInfo->GetFabricIndex()));

    mState = State::SecureConnected;

    DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, /* executeCallback */ false);
    DequeueConnectionSuccessCallbacks(/* executeCallback */ true);
}

CHIP_ERROR OperationalDeviceProxy::Disconnect()
{
    ReturnErrorCodeIf(mState != State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession.HasValue())
    {
        mInitParams.sessionManager->ExpirePairing(mSecureSession.Value());
    }
    mState = State::Initialized;
    mCASESession.Clear();
    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::Clear()
{
    mCASESession.Clear();

    mState      = State::Uninitialized;
    mInitParams = DeviceProxyInitParams();
}

void OperationalDeviceProxy::OnSessionReleased(SessionHandle session)
{
    VerifyOrReturn(mSecureSession.HasValue() && mSecureSession.Value() == session,
                   ChipLogDetail(Controller, "Connection expired, but it doesn't match the current session"));
    mState = State::Initialized;
    mSecureSession.ClearValue();
}

CHIP_ERROR OperationalDeviceProxy::ShutdownSubscriptions()
{
    return app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mInitParams.fabricInfo->GetFabricIndex(),
                                                                             GetDeviceId());
}

OperationalDeviceProxy::~OperationalDeviceProxy() {}

} // namespace chip
