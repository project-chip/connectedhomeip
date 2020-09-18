/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *    @file
 *      Implementation of CHIP Device Controller, a common class
 *      that implements discovery, pairing and provisioning of CHIP
 *      devices.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
// module header, comes first
#include <controller/CHIPDeviceController.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

namespace chip {
namespace DeviceController {

using namespace chip::Encoding;

ChipDeviceController::ChipDeviceController()
{
    mState             = kState_NotInitialized;
    AppState           = NULL;
    mConState          = kConnectionState_NotConnected;
    mRendezvousSession = NULL;
    mSessionManager    = NULL;
    mCurReqMsg         = NULL;
    mOnError           = NULL;
    mOnNewConnection   = NULL;
    mDeviceAddr        = IPAddress::Any;
    mDevicePort        = CHIP_PORT;
    mLocalDeviceId     = 0;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    err = DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = Init(localNodeId, &DeviceLayer::SystemLayer, &DeviceLayer::InetLayer);
#endif // CONFIG_DEVICE_LAYER

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, System::Layer * systemLayer, InetLayer * inetLayer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = systemLayer;
    mInetLayer   = inetLayer;

    mState         = kState_Initialized;
    mLocalDeviceId = localNodeId;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    mState = kState_NotInitialized;

#if CONFIG_DEVICE_LAYER
    err = DeviceLayer::PlatformMgr().Shutdown();
    SuccessOrExit(err);
#else
    mSystemLayer->Shutdown();
    mInetLayer->Shutdown();
    delete mSystemLayer;
    delete mInetLayer;
#endif // CONFIG_DEVICE_LAYER

    mSystemLayer = NULL;
    mInetLayer   = NULL;

    if (mSessionManager != NULL)
    {
        delete mSessionManager;
        mSessionManager = NULL;
    }

    if (mRendezvousSession != NULL)
    {
        delete mRendezvousSession;
        mRendezvousSession = NULL;
    }

    mConState = kConnectionState_NotConnected;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
    mOnError         = NULL;
    mOnNewConnection = NULL;
    mMessageNumber   = 0;
    mRemoteDeviceId.ClearValue();

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState,
                                               NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                               ErrorHandler onError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RendezvousSession * rendezvousSession;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mConState == kConnectionState_NotConnected, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    if (!params.HasBleLayer())
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());
    }
#endif // CONFIG_DEVICE_LAYER

    rendezvousSession = new RendezvousSession(this, params.SetLocalNodeId(mLocalDeviceId));
    err               = rendezvousSession->Init();
    SuccessOrExit(err);

    mRendezvousSession = rendezvousSession;

    mRemoteDeviceId  = Optional<NodeId>::Value(remoteDeviceId);
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    // connected state before 'OnConnect'
    mConState = kConnectionState_Connected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDeviceUsingPairing(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                                           NewConnectionHandler onConnected,
                                                           MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                           uint16_t devicePort, Inet::InterfaceId interfaceId,
                                                           SecurePairingSession * pairing)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != kState_Initialized || mSessionManager != NULL || mConState != kConnectionState_NotConnected)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mRemoteDeviceId  = Optional<NodeId>::Value(remoteDeviceId);
    mDeviceAddr      = deviceAddr;
    mDevicePort      = devicePort;
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    mSessionManager = new SecureSessionMgr<Transport::UDP>();

    err = mSessionManager->Init(mLocalDeviceId, mSystemLayer,
                                Transport::UdpListenParameters(mInetLayer).SetAddressType(deviceAddr.Type()));
    SuccessOrExit(err);

    mSessionManager->SetDelegate(this);

    mConState = kConnectionState_SecureConnected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    err = mSessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(deviceAddr, devicePort, interfaceId)), pairing);
    SuccessOrExit(err);

    mMessageNumber = 1;

    if (mOnNewConnection)
    {
        mOnNewConnection(this, NULL, mAppReqState);
    }

exit:

    if (err != CHIP_NO_ERROR)
    {
        if (mSessionManager != NULL)
        {
            delete mSessionManager;
            mSessionManager = NULL;
        }
        mConState = kConnectionState_NotConnected;
    }
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                               NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                               ErrorHandler onError, uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    return ConnectDeviceUsingPairing(remoteDeviceId, deviceAddr, appReqState, onConnected, onMessageReceived, onError, devicePort,
                                     interfaceId, &mPairingSession);
}

CHIP_ERROR ChipDeviceController::ConnectDeviceWithoutSecurePairing(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                                                   NewConnectionHandler onConnected,
                                                                   MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                                   uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    SecurePairingUsingTestSecret pairing(Optional<NodeId>::Value(remoteDeviceId), 0, 0);
    return ConnectDeviceUsingPairing(remoteDeviceId, deviceAddr, appReqState, onConnected, onMessageReceived, onError, devicePort,
                                     interfaceId, &pairing);
}

CHIP_ERROR ChipDeviceController::PopulatePeerAddress(Transport::PeerAddress & peerAddress)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(IsSecurelyConnected(), err = CHIP_ERROR_INCORRECT_STATE);

    peerAddress.SetIPAddress(mDeviceAddr);
    peerAddress.SetPort(mDevicePort);
    peerAddress.SetTransportType(Transport::Type::kUdp);

exit:
    return err;
}

bool ChipDeviceController::IsConnected()
{
    return kState_Initialized && (mConState == kConnectionState_Connected || mConState == kConnectionState_SecureConnected);
}

bool ChipDeviceController::IsSecurelyConnected()
{
    return kState_Initialized && mConState == kConnectionState_SecureConnected;
}

CHIP_ERROR ChipDeviceController::DisconnectDevice()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!IsConnected())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mSessionManager != NULL)
    {
        delete mSessionManager;
        mSessionManager = NULL;
    }

    if (mRendezvousSession != NULL)
    {
        delete mRendezvousSession;
        mRendezvousSession = NULL;
    }

    mConState = kConnectionState_NotConnected;
    return err;
};

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(IsSecurelyConnected(), err = CHIP_ERROR_INCORRECT_STATE);

    mAppReqState = appReqState;

    if (mRendezvousSession != NULL)
    {
        err = mRendezvousSession->SendMessage(buffer);
    }
    else
    {
        err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
    }
exit:

    return err;
}

CHIP_ERROR ChipDeviceController::ServiceEvents()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    err = DeviceLayer::PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);
#endif // CONFIG_DEVICE_LAYER

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ServiceEventSignal()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER && (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    DeviceLayer::SystemLayer.WakeSelect();
#else
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_DEVICE_LAYER && (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)

exit:
    return err;
}

void ChipDeviceController::ClearRequestState()
{
    if (mCurReqMsg != NULL)
    {
        PacketBuffer::Free(mCurReqMsg);
        mCurReqMsg = NULL;
    }
}

void ChipDeviceController::OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) {}

void ChipDeviceController::OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                             System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr)
{
    if (header.GetSourceNodeId().HasValue())
    {
        if (!mRemoteDeviceId.HasValue())
        {
            ChipLogProgress(Controller, "Learned remote device id");
            mRemoteDeviceId = header.GetSourceNodeId();
        }
        else if (mRemoteDeviceId != header.GetSourceNodeId())
        {
            ChipLogError(Controller, "Received message from an unexpected source node id.");
        }
    }
    if (IsSecurelyConnected() && mOnComplete.Response != NULL)
    {
        mOnComplete.Response(this, mAppReqState, msgBuf);
    }
}

void ChipDeviceController::OnRendezvousError(CHIP_ERROR err)
{
    if (mOnError)
    {
        mOnError(this, mAppReqState, err, NULL);
    }
}

void ChipDeviceController::OnRendezvousConnectionOpened()
{
    mPairingSession = mRendezvousSession->GetPairingSession();
    mConState       = kConnectionState_SecureConnected;

    if (mOnNewConnection)
    {
        mOnNewConnection(this, NULL, mAppReqState);
    }
}

void ChipDeviceController::OnRendezvousConnectionClosed() {}

void ChipDeviceController::OnRendezvousMessageReceived(PacketBuffer * buffer)
{
    if (mOnComplete.Response)
    {
        mOnComplete.Response(this, mAppReqState, buffer);
    }
}

} // namespace DeviceController
} // namespace chip
