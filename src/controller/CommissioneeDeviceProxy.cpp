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

#include <app/CommandSender.h>
#include <app/ReadPrepareParams.h>
#include <app/util/DataModelHandler.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::Callback;

namespace chip {

// TODO - Refactor LoadSecureSessionParametersIfNeeded() as device object is no longer persisted
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
    return commandObj->SendCommandRequest(mSecureSession.Value());
}

void CommissioneeDeviceProxy::OnNewConnection(SessionHandle session)
{
    mState = ConnectionState::SecureConnected;
    mSecureSession.SetValue(session);
}

void CommissioneeDeviceProxy::OnSessionReleased(SessionHandle session)
{
    VerifyOrReturn(mSecureSession.HasValue() && mSecureSession.Value() == session,
                   ChipLogDetail(Controller, "Connection expired, but it doesn't match the current session"));
    mState = ConnectionState::NotConnected;
    mSecureSession.ClearValue();
}

CHIP_ERROR CommissioneeDeviceProxy::CloseSession()
{
    ReturnErrorCodeIf(mState != ConnectionState::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession.HasValue())
    {
        mSessionManager->ExpirePairing(mSecureSession.Value());
    }
    mState = ConnectionState::NotConnected;
    mPairing.Clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::UpdateDeviceData(const Transport::PeerAddress & addr,
                                                     const ReliableMessageProtocolConfig & config)
{
    bool didLoad;

    mDeviceAddress = addr;

    mMRPConfig = config;

    // Initialize PASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by PASE session protocol messages that include MRP parameters.
    mPairing.SetMRPConfig(mMRPConfig);

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
    mInetLayer      = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = nullptr;
#endif
    mExchangeMgr = nullptr;

    ReleaseDAC();
    ReleasePAI();
}

CHIP_ERROR CommissioneeDeviceProxy::LoadSecureSessionParameters()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mSessionManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    if (mState == ConnectionState::Connecting)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceAddress), mDeviceId, &mPairing,
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

CommissioneeDeviceProxy::~CommissioneeDeviceProxy()
{
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
