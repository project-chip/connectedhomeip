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

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_LAYER_BLE
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
#if CONFIG_NETWORK_LAYER_BLE
    mOnBleError      = NULL;
    mOnBleConnection = NULL;
    mOnBleMessage    = NULL;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = new System::Layer();
    mInetLayer   = new Inet::InetLayer();

    // Initialize the CHIP System Layer.
    err = mSystemLayer->Init(NULL);
    if (err != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(Controller, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = mInetLayer->Init(*mSystemLayer, NULL);
    if (err != INET_NO_ERROR)
    {
        ChipLogError(Controller, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    mState         = kState_Initialized;
    mLocalDeviceId = localNodeId;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Shutdown()
{
    if (mState != kState_Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    mState         = kState_NotInitialized;

    if (mSessionManager != NULL)
    {
        delete mSessionManager;
        mSessionManager = NULL;
    }
    mSystemLayer->Shutdown();
    mInetLayer->Shutdown();
    delete mSystemLayer;
    delete mInetLayer;
    mSystemLayer = NULL;
    mInetLayer   = NULL;

    mConState = kConnectionState_NotConnected;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
    mOnError         = NULL;
    mOnNewConnection = NULL;
    mMessageNumber   = 0;
    mRemoteDeviceId.ClearValue();

#if CONFIG_NETWORK_LAYER_BLE
    if (mBleEndPoint != NULL)
    {
        mBleEndPoint->Close();
        mBleEndPoint = NULL;
    }

    mOnBleError      = NULL;
    mOnBleConnection = NULL;
    mOnBleMessage    = NULL;
#endif // CONFIG_NETWORK_LAYER_BLE

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

    mSessionManager = new SecureSessionMgr();

    err = mSessionManager->Init(mLocalDeviceId, mInetLayer, Transport::UdpListenParameters().SetAddressType(deviceAddr.Type()));
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

    delete mSessionManager;
    mSessionManager = NULL;
    mConState       = kConnectionState_NotConnected;
    return err;
};

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);

    mAppReqState = appReqState;
    VerifyOrExit(IsSecurelyConnected(), err = CHIP_ERROR_INCORRECT_STATE);

    err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
exit:

    return err;
}

CHIP_ERROR ChipDeviceController::GetLayers(Layer ** systemLayer, InetLayer ** inetLayer)
{
    if (mState != kState_Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (systemLayer != NULL)
    {
        *systemLayer = mSystemLayer;
    }
    if (inetLayer != NULL)
    {
        *inetLayer = mInetLayer;
    }

    return CHIP_NO_ERROR;
}

void ChipDeviceController::ServiceEvents()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    if (mState != kState_Initialized)
    {
        return;
    }

    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    static bool printed = false;

    if (!printed)
    {
        {
            ChipLogProgress(Controller, "CHIP node ready to service events; PID: %d; PPID: %d\n", getpid(), getppid());
            printed = true;
        }
    }
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (mSystemLayer->State() == System::kLayerState_Initialized)
    {
        mSystemLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);
    }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (mInetLayer->State == Inet::InetLayer::kState_Initialized)
    {
        mInetLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);
    }
#endif

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        ChipLogError(Controller, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (mSystemLayer->State() == System::kLayerState_Initialized)
    {
        mSystemLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (mInetLayer->State == Inet::InetLayer::kState_Initialized)
    {
        mInetLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
#endif

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
}

void ChipDeviceController::ClearRequestState()
{
    if (mCurReqMsg != NULL)
    {
        PacketBuffer::Free(mCurReqMsg);
        mCurReqMsg = NULL;
    }
}

void ChipDeviceController::OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgr * mgr)
{
    mOnNewConnection(this, state, mAppReqState);
}

void ChipDeviceController::OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                             System::PacketBuffer * msgBuf, SecureSessionMgr * mgr)
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

#if CONFIG_NETWORK_LAYER_BLE

CHIP_ERROR ChipDeviceController::InitBle(BlePlatformDelegate * platformDelegate, BleApplicationDelegate * applicationDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    err = mBleLayer.Init(platformDelegate, applicationDelegate, mSystemLayer);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectBle(BLE_CONNECTION_OBJECT connObj, BleNewConnectionHandler onBleConnection,
                                            BleMessageReceiveHandler onBleMessage, BleErrorHandler onBleError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = mBleLayer.NewBleEndPoint(&mBleEndPoint, connObj, kBleRole_Central, true);
    SuccessOrExit(err);

    // Set up callbacks we need to negotiate CHIPoBle connection.
    mBleEndPoint->mAppState          = this;
    mBleEndPoint->OnConnectComplete  = HandleBleConnectComplete;
    mBleEndPoint->OnConnectionClosed = HandleBleConnectionClosed;
    mBleEndPoint->OnMessageReceived  = HandleBleMessageReceived;

    // Initiate CHIP over BLE protocol connection.
    err = mBleEndPoint->StartConnect();
    SuccessOrExit(err);

    mOnBleConnection = onBleConnection;
    mOnBleMessage    = onBleMessage;
    mOnBleError      = onBleError;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::DisconnectBle(BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::SendBleMessage(PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mBleEndPoint, err = CHIP_ERROR_INCORRECT_STATE);

    mBleEndPoint->Send(buffer);

exit:
    return err;
}

void ChipDeviceController::HandleBleConnectComplete(BLEEndPoint * endPoint, BLE_ERROR err)
{
    ChipDeviceController * dc = (ChipDeviceController *) endPoint->mAppState;
    if (dc && dc->mOnBleConnection != NULL)
    {
        dc->mOnBleConnection(dc);
    }
}

void ChipDeviceController::HandleBleConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    ChipDeviceController * dc = (ChipDeviceController *) endPoint->mAppState;
    if (dc && dc->mOnBleError != NULL)
    {
        dc->mOnBleError(dc, err);
    }
}

void ChipDeviceController::HandleBleMessageReceived(BLEEndPoint * endPoint, PacketBuffer * data)
{

    ChipDeviceController * dc = (ChipDeviceController *) endPoint->mAppState;
    if (dc && dc->mOnBleMessage != NULL)
    {
        dc->mOnBleMessage(dc, data);
    }
}

#endif // CONFIG_NETWORK_LAYER_BLE

} // namespace DeviceController
} // namespace chip
