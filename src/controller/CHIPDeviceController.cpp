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

BLEDeviceConnectionParameters::BLEDeviceConnectionParameters()
{
#if CONFIG_DEVICE_LAYER
    SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());
#endif
}

static constexpr uint32_t kSpake2p_Iteration_Count = 50000;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

ChipDeviceController::ChipDeviceController()
{
    mState           = kState_NotInitialized;
    AppState         = NULL;
    mConState        = kConnectionState_NotConnected;
    mSessionManager  = NULL;
    mCurReqMsg       = NULL;
    mOnError         = NULL;
    mOnNewConnection = NULL;
    mDeviceAddr      = IPAddress::Any;
    mDevicePort      = CHIP_PORT;
    mLocalDeviceId   = 0;
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

    if (mUnsecuredTransport != NULL)
    {
        mUnsecuredTransport->Release();
        mUnsecuredTransport = NULL;
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

CHIP_ERROR ChipDeviceController::OnNewMessageForPeer(System::PacketBuffer * msgBuf)
{
    return SendMessage(mAppReqState, msgBuf);
}

void ChipDeviceController::OnPairingError(CHIP_ERROR error)
{
    ChipLogError(Controller, "Failed to pair with accessory. Error %d", error);
    mPairingInProgress = false;

    if (mOnError != nullptr)
    {
        mOnError(this, mAppReqState, error, nullptr);
    }
}

void ChipDeviceController::OnPairingComplete(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId)
{
    ChipLogProgress(Controller, "Successfully paired with accessory. Key Id %d", peerKeyId);
    mPairingInProgress = false;

    if (mPairingComplete != nullptr)
    {
        mPeerKeyId        = peerKeyId;
        mLocalPairedKeyId = localKeyId;
        ChipLogProgress(Controller, "Calling mPairingComplete");
        mPairingComplete(this, nullptr, mAppReqState);
    }
}

void ChipDeviceController::PairingMessageHandler(ChipDeviceController * controller, void * appReqState,
                                                 System::PacketBuffer * payload)
{
    if (controller->mPairingInProgress)
    {
        MessageHeader header;
        size_t headerSize = 0;
        CHIP_ERROR err    = header.Decode(payload->Start(), payload->DataLength(), &headerSize);
        SuccessOrExit(err);

        payload->ConsumeHead(headerSize);
        controller->mPairingSession.HandlePeerMessage(header, payload);
    }
    else if (controller->mAppMsgHandler != nullptr)
    {
        controller->mAppMsgHandler(controller, appReqState, payload);
    }

exit:
    return;
}

void ChipDeviceController::BLEConnectionHandler(ChipDeviceController * controller, Transport::PeerConnectionState * state,
                                                void * appReqState)
{
    ChipLogProgress(Controller, "Starting pairing session");
    controller->mPairingInProgress = true;
    CHIP_ERROR err                 = controller->mPairingSession.Pair(
        controller->mSetupPINCode, kSpake2p_Iteration_Count, Uint8::from_const_char(kSpake2pKeyExchangeSalt),
        strlen(kSpake2pKeyExchangeSalt), Optional<NodeId>::Value(controller->mLocalDeviceId), controller->mNextKeyId++, controller);
    SuccessOrExit(err);

exit:
    return;
}

CHIP_ERROR ChipDeviceController::ConnectDevice(const BLEDeviceConnectionParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_NETWORK_LAYER_BLE
    Transport::BLE * transport;

    ChipLogProgress(Controller, "Received new pairing request");
    ChipLogProgress(Controller, "mState %d. mConState %d", mState, mConState);
    VerifyOrExit(mState == kState_Initialized && mConState == kConnectionState_NotConnected, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(params.GetBleLayer() != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mPairingInProgress)
    {
        ChipLogError(Controller, "Pairing was already is progress. This will restart pairing.");
    }

    mRemoteDeviceId  = Optional<NodeId>::Value(params.GetRemoteDeviceId());
    mAppReqState     = params.GetAppReqState();
    mPairingComplete = params.GetOnConnected();
    mOnNewConnection = BLEConnectionHandler;
    mAppMsgHandler   = params.GetOnMessageReceived();

    mSetupPINCode = params.GetSetupPINCode();

    transport = new Transport::BLE();
    err       = transport->Init(Transport::BleConnectionParameters(this, params.GetBleLayer())
                              .SetDiscriminator(params.GetDiscriminator())
                              .SetSetupPINCode(params.GetSetupPINCode()));
    SuccessOrExit(err);

    mUnsecuredTransport = transport->Retain();
    transport->Release();

    // connected state before 'OnConnect'
    mConState = kConnectionState_Connected;

    mOnComplete.Response = PairingMessageHandler;
    mOnError             = params.GetOnError();

    if (err != CHIP_NO_ERROR)
    {
        mConState = kConnectionState_NotConnected;
    }
#else
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE

    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDeviceUsingPairing(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                                           NewConnectionHandler onConnected,
                                                           MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                           uint16_t devicePort, uint16_t localKeyId, SecurePairingSession * pairing)
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

    err = mSessionManager->NewPairing(mRemoteDeviceId,
                                      Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(deviceAddr, devicePort)),
                                      mPeerKeyId, localKeyId, pairing);
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
                                               ErrorHandler onError, uint16_t devicePort)
{
    return ConnectDeviceUsingPairing(remoteDeviceId, deviceAddr, appReqState, onConnected, onMessageReceived, onError, devicePort,
                                     mLocalPairedKeyId, &mPairingSession);
}

CHIP_ERROR ChipDeviceController::ConnectDeviceWithoutSecurePairing(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                                                   NewConnectionHandler onConnected,
                                                                   MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                                   uint16_t devicePort)
{
    SecurePairingUsingTestSecret pairing;
    return ConnectDeviceUsingPairing(remoteDeviceId, deviceAddr, appReqState, onConnected, onMessageReceived, onError, devicePort,
                                     0, &pairing);
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

    if (mUnsecuredTransport != NULL)
    {
        mUnsecuredTransport->Release();
        mUnsecuredTransport = NULL;
    }

    mConState = kConnectionState_NotConnected;
    return err;
};

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);

    mAppReqState = appReqState;

    if (mUnsecuredTransport != NULL)
    {
        VerifyOrExit(IsConnected(), err = CHIP_ERROR_INCORRECT_STATE);
        // Unsecured transport does not use a MessageHeader, but the Transport::Base API expects one, so
        // let build an empty one for now.
        MessageHeader header;
        Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
        err                                = mUnsecuredTransport->SendMessage(header, peerAddress, buffer);
    }
    else
    {
        VerifyOrExit(IsSecurelyConnected(), err = CHIP_ERROR_INCORRECT_STATE);
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

void ChipDeviceController::OnBLEConnectionError(BLE_ERROR err)
{
    if (mOnError)
    {
        mOnError(this, mAppReqState, err, NULL);
    }
}

void ChipDeviceController::OnBLEConnectionComplete(BLE_ERROR err)
{
    ChipLogDetail(Controller, "BLE Connection complete");

    if (mOnNewConnection)
    {
        mOnNewConnection(this, NULL, mAppReqState);
    }
}

void ChipDeviceController::OnBLEConnectionClosed(BLE_ERROR err)
{
    ChipLogDetail(Controller, "BLE Connection closed");

    // TODO: determine if connection closed is really to be treated as an error.
    if (mOnError)
    {
        mOnError(this, mAppReqState, err, NULL);
    }
}

void ChipDeviceController::OnBLEPacketReceived(PacketBuffer * buffer)
{
    if (mOnComplete.Response)
    {
        mOnComplete.Response(this, mAppReqState, buffer);
    }
}

} // namespace DeviceController
} // namespace chip
