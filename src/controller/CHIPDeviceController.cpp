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
 *      that implements discovery, pairing and provisioning of Weave
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
        delete mUnsecuredTransport;
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

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, const uint16_t discriminator, const uint32_t setupPINCode,
                                               void * appReqState, NewConnectionHandler onConnected,
                                               MessageReceiveHandler onMessageReceived, ErrorHandler onError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE
    Transport::BLE * transport;

    VerifyOrExit(mState == kState_Initialized && mConState == kConnectionState_NotConnected, err = CHIP_ERROR_INCORRECT_STATE);

    mRemoteDeviceId  = Optional<NodeId>::Value(remoteDeviceId);
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    transport = new Transport::BLE();
    err       = transport->Init(Transport::BleConnectionParameters(this, DeviceLayer::ConnectivityMgr().GetBleLayer())
                              .SetDiscriminator(discriminator)
                              .SetSetupPINCode(setupPINCode));
    SuccessOrExit(err);
    mUnsecuredTransport = transport->Retain();

    // connected state before 'OnConnect'
    mConState = kConnectionState_Connected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

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

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState,
                                               NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                               ErrorHandler onError, uint16_t devicePort)
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

    // connected state before 'OnConnect' so that key exchange is accepted
    mConState = kConnectionState_Connected;

    err = mSessionManager->Connect(remoteDeviceId, Transport::PeerAddress::UDP(deviceAddr, devicePort));
    SuccessOrExit(err);

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    mMessageNumber = 1;

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

CHIP_ERROR ChipDeviceController::ManualKeyExchange(Transport::PeerConnectionState * state, const unsigned char * remote_public_key,
                                                   const size_t public_key_length, const unsigned char * local_private_key,
                                                   const size_t private_key_length)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!IsConnected() || mSessionManager == NULL)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, public_key_length, local_private_key,
                                                               private_key_length);
    SuccessOrExit(err);
    mConState = kConnectionState_SecureConnected;

exit:
    return err;
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
        delete mUnsecuredTransport;
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

void ChipDeviceController::OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr)
{
    mOnNewConnection(this, state, mAppReqState);
}

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

} // namespace DeviceController
} // namespace chip
