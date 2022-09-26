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

CHIP_ERROR CommissioneeDeviceProxy::SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout)
{
    VerifyOrReturnError(mSecureSession, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mSecureSession, CHIP_ERROR_MISSING_SECURE_SESSION);
    return commandObj->SendCommandRequest(mSecureSession.Get().Value(), timeout);
}

void CommissioneeDeviceProxy::OnSessionReleased()
{
    mState = ConnectionState::NotConnected;
}

void CommissioneeDeviceProxy::CloseSession()
{
    VerifyOrReturn(mState == ConnectionState::SecureConnected);
    if (mSecureSession)
    {
        mSecureSession->AsSecureSession()->MarkForEviction();
    }

    mState = ConnectionState::NotConnected;
    mPairing.Clear();
}

CHIP_ERROR CommissioneeDeviceProxy::UpdateDeviceData(const Transport::PeerAddress & addr,
                                                     const ReliableMessageProtocolConfig & config)
{
    mDeviceAddress = addr;

    mRemoteMRPConfig = config;

    // Initialize PASE session state with any MRP parameters that DNS-SD has provided.
    // It can be overridden by PASE session protocol messages that include MRP parameters.
    mPairing.SetRemoteMRPConfig(mRemoteMRPConfig);

    if (!mSecureSession)
    {
        // Nothing needs to be done here.  It's not an error to not have a
        // secureSession.  For one thing, we could have gotten an different
        // UpdateAddress already and that caused connections to be torn down and
        // whatnot.
        return CHIP_NO_ERROR;
    }

    Transport::SecureSession * secureSession = mSecureSession.Get().Value()->AsSecureSession();
    secureSession->SetPeerAddress(addr);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioneeDeviceProxy::SetConnected(const SessionHandle & session)
{
    VerifyOrReturnError(mState == ConnectionState::Connecting, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(session->AsSecureSession()->IsPASESession(), CHIP_ERROR_INVALID_ARGUMENT);

    if (!mSecureSession.Grab(session))
    {
        mState = ConnectionState::NotConnected;
        return CHIP_ERROR_INTERNAL;
    }

    mState = ConnectionState::SecureConnected;
    return CHIP_NO_ERROR;
}

CommissioneeDeviceProxy::~CommissioneeDeviceProxy()
{
    auto session = GetSecureSession();
    if (session.HasValue())
    {
        session.Value()->AsSecureSession()->MarkForEviction();
    }
}

CHIP_ERROR CommissioneeDeviceProxy::SetPeerId(ByteSpan rcac, ByteSpan noc)
{
    CompressedFabricId compressedFabricId;
    NodeId nodeId;
    ReturnErrorOnFailure(Credentials::ExtractNodeIdCompressedFabricIdFromOpCerts(rcac, noc, compressedFabricId, nodeId));
    mPeerId = PeerId().SetCompressedFabricId(compressedFabricId).SetNodeId(nodeId);
    return CHIP_NO_ERROR;
}

} // namespace chip
