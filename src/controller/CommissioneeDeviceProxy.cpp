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

#include <controller/CommissioneeDeviceProxy.h>

#include <controller-clusters/zap-generated/CHIPClusters.h>

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
using namespace chip::Controller;

namespace chip {

CHIP_ERROR CommissioneeDeviceProxy::LoadSecureSessionParametersIfNeeded(bool & didLoad)
{
    didLoad = false;

    // If there is no secure connection to the device, try establishing it
    if (mState != ConnectionState::SecureConnected)
    {
        ReturnErrorOnFailure(LoadSecureSessionParameters());
        didLoad = true;
    }
    else
    {
        if (mSecureSession.HasValue())
        {
            Transport::SecureSession * secureSession = mSessionManager->GetSecureSession(mSecureSession.Value());
            // Check if the connection state has the correct transport information
            if (secureSession->GetPeerAddress().GetTransportType() == Transport::Type::kUndefined)
            {
                mState = ConnectionState::NotConnected;
                ReturnErrorOnFailure(LoadSecureSessionParameters());
                didLoad = true;
            }
        }
        else
        {
            mState = ConnectionState::NotConnected;
            ReturnErrorOnFailure(LoadSecureSessionParameters());
            didLoad = true;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::SendCommands(app::CommandSender * commandObj)
{
    bool loadedSecureSession = false;
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(mDeviceId, mFabricIndex, mSecureSession);
}

void CommissioneeDeviceProxy::OnNewConnection(SessionHandle session)
{
    mState = ConnectionState::SecureConnected;
    mSecureSession.SetValue(session);

    // Reset the message counters here because this is the first time we get a handle to the secure session.
    // Since CHIPDevices can be serialized/deserialized in the middle of what is conceptually a single PASE session
    // we need to restore the session counters along with the session information.
    Transport::SecureSession * secureSession = mSessionManager->GetSecureSession(mSecureSession.Value());
    VerifyOrReturn(secureSession != nullptr);
    MessageCounter & localCounter = secureSession->GetSessionMessageCounter().GetLocalMessageCounter();
    if (localCounter.SetCounter(mLocalMessageCounter) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Unable to restore local counter to %" PRIu32, mLocalMessageCounter);
    }
    Transport::PeerMessageCounter & peerCounter = secureSession->GetSessionMessageCounter().GetPeerMessageCounter();
    peerCounter.SetCounter(mPeerMessageCounter);
}

void CommissioneeDeviceProxy::OnConnectionExpired(SessionHandle session)
{
    VerifyOrReturn(mSecureSession.HasValue() && mSecureSession.Value() == session,
                   ChipLogDetail(Controller, "Connection expired, but it doesn't match the current session"));
    mState = ConnectionState::NotConnected;
    mSecureSession.ClearValue();
}

CHIP_ERROR CommissioneeDeviceProxy::OnMessageReceived(Messaging::ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                                      System::PacketBufferHandle && msgBuf)
{
    if (mState == ConnectionState::SecureConnected)
    {
        if (mStatusDelegate != nullptr)
        {
            mStatusDelegate->OnMessage(std::move(msgBuf));
        }
        else
        {
            HandleDataModelMessage(exchange, std::move(msgBuf));
        }
    }
    return CHIP_NO_ERROR;
}

void CommissioneeDeviceProxy::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

CHIP_ERROR CommissioneeDeviceProxy::CloseSession()
{
    ReturnErrorCodeIf(mState != ConnectionState::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession.HasValue())
    {
        mSessionManager->ExpirePairing(mSecureSession.Value());
    }
    mState = ConnectionState::NotConnected;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::UpdateAddress(const Transport::PeerAddress & addr)
{
    bool didLoad;

    mDeviceAddress = addr;

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(didLoad));

    if (!mSecureSession.HasValue())
    {
        // Nothing needs to be done here.  It's not an error to not have a
        // secureSession.  For one thing, we could have gotten an different
        // UpdateAddress already and that caused connections to be torn down and
        // whatnot.
        return CHIP_NO_ERROR;
    }

    Transport::SecureSession * secureSession = mSessionManager->GetSecureSession(mSecureSession.Value());
    secureSession->SetPeerAddress(addr);

    return CHIP_NO_ERROR;
}

void CommissioneeDeviceProxy::Reset()
{
    SetActive(false);

    mState          = ConnectionState::NotConnected;
    mSessionManager = nullptr;
    mStatusDelegate = nullptr;
    mInetLayer      = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = nullptr;
#endif
    if (mExchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mExchangeMgr->CloseAllContextsForDelegate(this);
    }
    mExchangeMgr = nullptr;

    ReleaseDAC();
    ReleasePAI();
}

CHIP_ERROR CommissioneeDeviceProxy::LoadSecureSessionParameters()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PASESession pairingSession;

    if (mSessionManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    if (mState == ConnectionState::Connecting)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    err = pairingSession.FromSerializable(mPairing);
    SuccessOrExit(err);

    err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceAddress), mDeviceId, &pairingSession,
                                      CryptoContext::SessionRole::kInitiator, mFabricIndex);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "LoadSecureSessionParameters returning error %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

bool CommissioneeDeviceProxy::GetAddress(Inet::IPAddress & addr, uint16_t & port) const
{
    if (mState == ConnectionState::NotConnected)
        return false;

    addr = mDeviceAddress.GetIPAddress();
    port = mDeviceAddress.GetPort();
    return true;
}

void CommissioneeDeviceProxy::ReleaseDAC()
{
    if (mDAC != nullptr)
    {
        Platform::MemoryFree(mDAC);
    }
    mDACLen = 0;
    mDAC    = nullptr;
}

CHIP_ERROR CommissioneeDeviceProxy::SetDAC(const ByteSpan & dac)
{
    if (dac.size() == 0)
    {
        ReleaseDAC();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(dac.size() <= Credentials::kMaxDERCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mDACLen != 0)
    {
        ReleaseDAC();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(dac.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mDAC == nullptr)
    {
        mDAC = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(dac.size()));
    }
    VerifyOrReturnError(mDAC != nullptr, CHIP_ERROR_NO_MEMORY);
    mDACLen = static_cast<uint16_t>(dac.size());
    memcpy(mDAC, dac.data(), mDACLen);

    return CHIP_NO_ERROR;
}

void CommissioneeDeviceProxy::ReleasePAI()
{
    if (mPAI != nullptr)
    {
        chip::Platform::MemoryFree(mPAI);
    }
    mPAILen = 0;
    mPAI    = nullptr;
}

CHIP_ERROR CommissioneeDeviceProxy::SetPAI(const chip::ByteSpan & pai)
{
    if (pai.size() == 0)
    {
        ReleasePAI();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(pai.size() <= Credentials::kMaxDERCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    if (mPAILen != 0)
    {
        ReleasePAI();
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(pai.size()), CHIP_ERROR_INVALID_ARGUMENT);
    if (mPAI == nullptr)
    {
        mPAI = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(pai.size()));
    }
    VerifyOrReturnError(mPAI != nullptr, CHIP_ERROR_NO_MEMORY);
    mPAILen = static_cast<uint16_t>(pai.size());
    memcpy(mPAI, pai.data(), mPAILen);

    return CHIP_NO_ERROR;
}

void CommissioneeDeviceProxy::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback,
                                                 Callback::Cancelable * onFailureCallback, app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddResponseCallback(mDeviceId, seqNum, onSuccessCallback, onFailureCallback, tlvDataFilter);
}

void CommissioneeDeviceProxy::CancelResponseHandler(uint8_t seqNum)
{
    mCallbacksMgr.CancelResponseCallback(mDeviceId, seqNum);
}

void CommissioneeDeviceProxy::AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
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

void CommissioneeDeviceProxy::CancelIMResponseHandler(app::CommandSender * commandObj)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.CancelResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */);
}

void CommissioneeDeviceProxy::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                                               Callback::Cancelable * onReportCallback, app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddReportCallback(mDeviceId, endpoint, cluster, attribute, onReportCallback, tlvDataFilter);
}

CHIP_ERROR CommissioneeDeviceProxy::SendReadAttributeRequest(app::AttributePathParams aPath,
                                                             Callback::Cancelable * onSuccessCallback,
                                                             Callback::Cancelable * onFailureCallback,
                                                             app::TLVDataFilter aTlvDataFilter)
{
    bool loadedSecureSession = false;
    uint8_t seqNum           = GetNextSequenceNumber();
    aPath.mNodeId            = GetDeviceId();

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

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

CHIP_ERROR CommissioneeDeviceProxy::SendSubscribeAttributeRequest(app::AttributePathParams aPath, uint16_t mMinIntervalFloorSeconds,
                                                                  uint16_t mMaxIntervalCeilingSeconds,
                                                                  Callback::Cancelable * onSuccessCallback,
                                                                  Callback::Cancelable * onFailureCallback)
{
    bool loadedSecureSession = false;
    uint8_t seqNum           = GetNextSequenceNumber();
    aPath.mNodeId            = GetDeviceId();

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    app::AttributePathParams * path = mpIMDelegate->AllocateAttributePathParam(1, seqNum);

    VerifyOrReturnError(path != nullptr, CHIP_ERROR_NO_MEMORY);

    *path = aPath;

    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    VerifyOrReturnError(GetSecureSession().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    app::ReadPrepareParams params(GetSecureSession().Value());
    params.mpAttributePathParamsList    = path;
    params.mAttributePathParamsListSize = 1;
    params.mMinIntervalFloorSeconds     = mMinIntervalFloorSeconds;
    params.mMaxIntervalCeilingSeconds   = mMaxIntervalCeilingSeconds;
    params.mKeepSubscriptions           = false;

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
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::SendWriteAttributeRequest(app::WriteClientHandle aHandle,
                                                              Callback::Cancelable * onSuccessCallback,
                                                              Callback::Cancelable * onFailureCallback)
{
    bool loadedSecureSession = false;
    uint8_t seqNum           = GetNextSequenceNumber();
    CHIP_ERROR err           = CHIP_NO_ERROR;

    aHandle->SetAppIdentifier(seqNum);
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }
    if ((err = aHandle.SendWriteRequest(GetDeviceId(), 0, mSecureSession)) != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

CommissioneeDeviceProxy::~CommissioneeDeviceProxy()
{
    if (mExchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mExchangeMgr->CloseAllContextsForDelegate(this);
    }

    ReleaseDAC();
    ReleasePAI();
}

CHIP_ERROR CommissioneeDeviceProxy::SetNOCCertBufferSize(size_t new_size)
{
    ReturnErrorCodeIf(new_size > sizeof(mNOCCertBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    mNOCCertBufferSize = new_size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::SetICACertBufferSize(size_t new_size)
{
    ReturnErrorCodeIf(new_size > sizeof(mICACertBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    mICACertBufferSize = new_size;
    return CHIP_NO_ERROR;
}

} // namespace chip
