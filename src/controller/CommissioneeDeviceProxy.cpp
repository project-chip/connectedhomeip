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
        if (mSecureSession)
        {
            // Check if the connection state has the correct transport information
            if (mSecureSession->AsSecureSession()->GetPeerAddress().GetTransportType() == Transport::Type::kUndefined)
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

CHIP_ERROR CommissioneeDeviceProxy::SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout)
{
    bool loadedSecureSession = false;
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(mSecureSession.Get(), timeout);
}

void CommissioneeDeviceProxy::OnSessionReleased()
{
    mState = ConnectionState::NotConnected;
}

CHIP_ERROR CommissioneeDeviceProxy::CloseSession()
{
    ReturnErrorCodeIf(mState != ConnectionState::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession)
    {
        mSessionManager->ExpirePairing(mSecureSession.Get());
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

    if (!mSecureSession)
    {
        // Nothing needs to be done here.  It's not an error to not have a
        // secureSession.  For one thing, we could have gotten an different
        // UpdateAddress already and that caused connections to be torn down and
        // whatnot.
        return CHIP_NO_ERROR;
    }

    Transport::SecureSession * secureSession = mSecureSession.Get()->AsSecureSession();
    secureSession->SetPeerAddress(addr);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::SetConnected()
{
    if (mState != ConnectionState::Connecting)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mState = ConnectionState::SecureConnected;
    bool _didLoad;
    CHIP_ERROR err = LoadSecureSessionParametersIfNeeded(_didLoad);
    if (err != CHIP_NO_ERROR)
    {
        mState = ConnectionState::NotConnected;
    }
    return err;
}

void CommissioneeDeviceProxy::Reset()
{
    SetActive(false);

    mState              = ConnectionState::NotConnected;
    mSessionManager     = nullptr;
    mUDPEndPointManager = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = nullptr;
#endif
    mExchangeMgr = nullptr;
}

CHIP_ERROR CommissioneeDeviceProxy::LoadSecureSessionParameters()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SessionHolder sessionHolder;

    if (mSessionManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    if (mState == ConnectionState::Connecting)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    SuccessOrExit(mSessionManager->NewPairing(mSecureSession, Optional<Transport::PeerAddress>::Value(mDeviceAddress),
                                              GetDeviceId(), &mPairing, CryptoContext::SessionRole::kInitiator, mFabricIndex));
    mState = ConnectionState::SecureConnected;

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

CommissioneeDeviceProxy::~CommissioneeDeviceProxy() {}

CHIP_ERROR CommissioneeDeviceProxy::SetPeerId(ByteSpan rcac, ByteSpan noc)
{
    CompressedFabricId compressedFabricId;
    NodeId nodeId;
    ReturnErrorOnFailure(Credentials::ExtractNodeIdCompressedFabricIdFromOpCerts(rcac, noc, compressedFabricId, nodeId));
    mPeerId = PeerId().SetCompressedFabricId(compressedFabricId).SetNodeId(nodeId);
    return CHIP_NO_ERROR;
}

} // namespace chip
