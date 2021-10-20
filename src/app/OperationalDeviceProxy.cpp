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

#include <app/OperationalDeviceProxy.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <app/CommandSender.h>
#include <app/ReadPrepareParams.h>
#include <app/util/DataModelHandler.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/Protocols.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/MessageCounter.h>
#include <transport/PeerMessageCounter.h>

using namespace chip::Inet;
using namespace chip::System;
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

CHIP_ERROR OperationalDeviceProxy::UpdateAddress(const Transport::PeerAddress & addr)
{
    VerifyOrReturnLogError(mState != State::Uninitialized, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;
    mDeviceAddress = addr;
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
        secureSession->SetPeerAddress(addr);
    }

    return err;
}

CHIP_ERROR OperationalDeviceProxy::EstablishConnection()
{
    // Create a UnauthenticatedSession for CASE pairing.
    // Don't use mSecureSession here, because mSecureSession is for encrypted communication.
    Optional<SessionHandle> session = mInitParams.sessionManager->CreateUnauthenticatedSession(mDeviceAddress);
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
    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::Clear()
{
    mCASESession.Clear();

    mState          = State::Uninitialized;
    mStatusDelegate = nullptr;
    if (mInitParams.exchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mInitParams.exchangeMgr->CloseAllContextsForDelegate(this);
    }
    mInitParams = DeviceProxyInitParams();
}

CHIP_ERROR OperationalDeviceProxy::SendCommands(app::CommandSender * commandObj)
{
    VerifyOrReturnLogError(mState == State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(mPeerId.GetNodeId(), mInitParams.fabricInfo->GetFabricIndex(), mSecureSession);
}

void OperationalDeviceProxy::OnConnectionExpired(SessionHandle session)
{
    VerifyOrReturn(mSecureSession.HasValue() && mSecureSession.Value() == session,
                   ChipLogDetail(Controller, "Connection expired, but it doesn't match the current session"));
    mState = State::Initialized;
    mSecureSession.ClearValue();
}

CHIP_ERROR OperationalDeviceProxy::OnMessageReceived(Messaging::ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                                     System::PacketBufferHandle && msgBuf)
{
    if (mState == State::SecureConnected)
    {
        if (mStatusDelegate != nullptr)
        {
            mStatusDelegate->OnMessage(std::move(msgBuf));
        }
        //        else
        //        {
        //            HandleDataModelMessage(exchange, std::move(msgBuf));
        //        }
    }
    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback,
                                                Callback::Cancelable * onFailureCallback, app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddResponseCallback(mPeerId.GetNodeId(), seqNum, onSuccessCallback, onFailureCallback, tlvDataFilter);
}

void OperationalDeviceProxy::CancelResponseHandler(uint8_t seqNum)
{
    mCallbacksMgr.CancelResponseCallback(mPeerId.GetNodeId(), seqNum);
}

void OperationalDeviceProxy::AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                                                  Callback::Cancelable * onFailureCallback)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.AddResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */, onSuccessCallback,
                                      onFailureCallback);
}

void OperationalDeviceProxy::CancelIMResponseHandler(app::CommandSender * commandObj)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.CancelResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */);
}

void OperationalDeviceProxy::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                                              Callback::Cancelable * onReportCallback, app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddReportCallback(mPeerId.GetNodeId(), endpoint, cluster, attribute, onReportCallback, tlvDataFilter);
}

CHIP_ERROR OperationalDeviceProxy::SendReadAttributeRequest(app::AttributePathParams aPath,
                                                            Callback::Cancelable * onSuccessCallback,
                                                            Callback::Cancelable * onFailureCallback,
                                                            app::TLVDataFilter aTlvDataFilter)
{
    VerifyOrReturnLogError(mState == State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);

    uint8_t seqNum = GetNextSequenceNumber();
    aPath.mNodeId  = mPeerId.GetNodeId();

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback, aTlvDataFilter);
    }
    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    chip::app::ReadPrepareParams readPrepareParams(mSecureSession.Value());
    readPrepareParams.mpAttributePathParamsList    = &aPath;
    readPrepareParams.mAttributePathParamsListSize = 1;
    CHIP_ERROR err =
        chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, seqNum /* application context */);
    if (err != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

CHIP_ERROR OperationalDeviceProxy::SendSubscribeAttributeRequest(app::AttributePathParams aPath, uint16_t mMinIntervalFloorSeconds,
                                                                 uint16_t mMaxIntervalCeilingSeconds,
                                                                 Callback::Cancelable * onSuccessCallback,
                                                                 Callback::Cancelable * onFailureCallback)
{
    VerifyOrReturnLogError(mState == State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
#if 0
    uint8_t seqNum = GetNextSequenceNumber();
    aPath.mNodeId  = mPeerId.GetNodeId();

    app::AttributePathParams * path = mpIMDelegate->AllocateAttributePathParam(1, seqNum);

    VerifyOrReturnError(path != nullptr, CHIP_ERROR_NO_MEMORY);

    *path = aPath;

    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    VerifyOrReturnError(mSecureSession.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    app::ReadPrepareParams params(mSecureSession.Value());
    params.mpAttributePathParamsList    = path;
    params.mAttributePathParamsListSize = 1;
    params.mMinIntervalFloorSeconds     = mMinIntervalFloorSeconds;
    params.mMaxIntervalCeilingSeconds   = mMaxIntervalCeilingSeconds;

    CHIP_ERROR err =
        chip::app::InteractionModelEngine::GetInstance()->SendSubscribeRequest(params, seqNum /* application context */);
    if (err != CHIP_NO_ERROR)
    {
        mpIMDelegate->FreeAttributePathParam(seqNum);
        return err;
    }

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalDeviceProxy::SendWriteAttributeRequest(app::WriteClientHandle aHandle,
                                                             Callback::Cancelable * onSuccessCallback,
                                                             Callback::Cancelable * onFailureCallback)
{
    VerifyOrReturnLogError(mState == State::SecureConnected, CHIP_ERROR_INCORRECT_STATE);

    uint8_t seqNum = GetNextSequenceNumber();
    CHIP_ERROR err = CHIP_NO_ERROR;

    aHandle->SetAppIdentifier(seqNum);

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }
    if ((err = aHandle.SendWriteRequest(mPeerId.GetNodeId(), 0, mSecureSession)) != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

OperationalDeviceProxy::~OperationalDeviceProxy()
{
    if (mInitParams.exchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mInitParams.exchangeMgr->CloseAllContextsForDelegate(this);
    }
}

} // namespace chip
