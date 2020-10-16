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

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

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
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

using namespace chip::Inet;
using namespace chip::System;

namespace chip {
namespace DeviceController {

using namespace chip::Encoding;

constexpr const char * kDeviceCredentialsKeyPrefix = "DeviceCredentials";
constexpr const char * kDeviceAddressKeyPrefix     = "DeviceAddress";

// This macro generats a key using node ID an key prefix, and performs the given action
// on that key.
#define PERSISTENT_KEY_OP(node, keyPrefix, key, action)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        const size_t len = strlen(keyPrefix);                                                                                      \
        char key[len + 2 * sizeof(NodeId) + 1];                                                                                    \
        snprintf(key, sizeof(key), "%s%" PRIx64, keyPrefix, node);                                                                 \
        action;                                                                                                                    \
    } while (0)

ChipDeviceController::ChipDeviceController()
{
    mState             = kState_NotInitialized;
    AppState           = nullptr;
    mConState          = kConnectionState_NotConnected;
    mRendezvousSession = nullptr;
    mSessionManager    = nullptr;
    mCurReqMsg         = nullptr;
    mOnError           = nullptr;
    mOnNewConnection   = nullptr;
    mPairingDelegate   = nullptr;
    mStorageDelegate   = nullptr;
    mDeviceAddr        = IPAddress::Any;
    mDevicePort        = CHIP_PORT;
    mInterface         = INET_NULL_INTERFACEID;
    mLocalDeviceId     = 0;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
}

ChipDeviceController::~ChipDeviceController()
{
    if (mTestSecurePairingSecret != nullptr)
    {
        delete mTestSecurePairingSecret;
    }
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

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, DevicePairingDelegate * pairingDelegate,
                                      PersistentStorageDelegate * storage)
{
    CHIP_ERROR err = Init(localNodeId);
    SuccessOrExit(err);

    mPairingDelegate = pairingDelegate;
    mStorageDelegate = storage;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(Controller, "Shutting down the controller");

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

    mSystemLayer = nullptr;
    mInetLayer   = nullptr;

    if (mSessionManager != nullptr)
    {
        delete mSessionManager;
        mSessionManager = nullptr;
    }

    if (mRendezvousSession != nullptr)
    {
        delete mRendezvousSession;
        mRendezvousSession = nullptr;
    }

    mConState = kConnectionState_NotConnected;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
    mOnError         = nullptr;
    mOnNewConnection = nullptr;
    mRemoteDeviceId.ClearValue();

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState,
                                               NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                               ErrorHandler onError, uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mConState == kConnectionState_NotConnected, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE
    if (!params.HasBleLayer())
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());
    }
#endif // CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE

    mRendezvousSession = new RendezvousSession(this);
    err                = mRendezvousSession->Init(params.SetLocalNodeId(mLocalDeviceId));
    SuccessOrExit(err);

    mRemoteDeviceId  = Optional<NodeId>::Value(remoteDeviceId);
    mDevicePort      = devicePort;
    mInterface       = interfaceId;
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    // connected state before 'OnConnect'
    mConState = kConnectionState_Connected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

exit:
    if (err != CHIP_NO_ERROR && mRendezvousSession != nullptr)
    {
        delete mRendezvousSession;
        mRendezvousSession = nullptr;
    }

    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDeviceWithoutSecurePairing(NodeId remoteDeviceId, const IPAddress & deviceAddr,
                                                                   void * appReqState, NewConnectionHandler onConnected,
                                                                   MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                                   uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    if (mTestSecurePairingSecret != nullptr)
    {
        delete mTestSecurePairingSecret;
    }

    mTestSecurePairingSecret = new SecurePairingUsingTestSecret(Optional<NodeId>::Value(remoteDeviceId), 0, 0);

    mSecurePairingSession = mTestSecurePairingSecret;

    mDeviceAddr      = deviceAddr;
    mRemoteDeviceId  = Optional<NodeId>::Value(remoteDeviceId);
    mDevicePort      = devicePort;
    mInterface       = interfaceId;
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    mConState = kConnectionState_Connected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    if (mOnNewConnection)
    {
        mOnNewConnection(this, nullptr, mAppReqState);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceController::EstablishSecureSession(const NodeId & peer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SecurePairingSession pairing;
    SecurePairingSession * pairingSession = mSecurePairingSession;
    Inet::IPAddress peerAddr              = mDeviceAddr;

    if (mState != kState_Initialized || mSessionManager != nullptr || mConState == kConnectionState_SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    mSessionManager = new SecureSessionMgr<Transport::UDP>();

    err = mSessionManager->Init(mLocalDeviceId, mSystemLayer,
                                Transport::UdpListenParameters(mInetLayer).SetAddressType(mDeviceAddr.Type()));
    SuccessOrExit(err);

    mSessionManager->SetDelegate(this);

    mConState = kConnectionState_SecureConnected;

    if (mStorageDelegate != nullptr)
    {
        const char * credentials;
        const char * address;

        PERSISTENT_KEY_OP(peer, kDeviceCredentialsKeyPrefix, key, credentials = mStorageDelegate->GetKeyValue(key));
        PERSISTENT_KEY_OP(peer, kDeviceAddressKeyPrefix, key, address = mStorageDelegate->GetKeyValue(key));

        SecurePairingSessionSerialized serialized;

        VerifyOrExit(credentials != nullptr, err = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);
        strncpy(Uint8::to_char(serialized.inner), credentials, sizeof(serialized.inner));

        err = pairing.Deserialize(serialized);
        SuccessOrExit(err);

        pairingSession = &pairing;

        VerifyOrExit(address != nullptr, err = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);

        VerifyOrExit(IPAddress::FromString(address, peerAddr), err = CHIP_ERROR_INVALID_ADDRESS);
    }

    err = mSessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(peerAddr, mDevicePort, mInterface)), pairingSession);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        if (mSessionManager != nullptr)
        {
            delete mSessionManager;
            mSessionManager = nullptr;
        }
        mConState = kConnectionState_NotConnected;
    }
    return err;
}

CHIP_ERROR ChipDeviceController::ResumeSecureSession(const NodeId & peer)
{
    if (mConState == kConnectionState_SecureConnected)
    {
        mConState = kConnectionState_Connected;
    }

    if (mSessionManager != nullptr)
    {
        delete mSessionManager;
        mSessionManager = nullptr;
    }

    CHIP_ERROR err = EstablishSecureSession(peer);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ResumeSecureSession returning error %d\n", err);
    }
    return err;
}

bool ChipDeviceController::IsConnected()
{
    return mState == kState_Initialized &&
        (mConState == kConnectionState_Connected || mConState == kConnectionState_SecureConnected);
}

bool ChipDeviceController::IsSecurelyConnected()
{
    return mState == kState_Initialized && mConState == kConnectionState_SecureConnected;
}

CHIP_ERROR ChipDeviceController::DisconnectDevice()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!IsConnected())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mSessionManager != nullptr)
    {
        delete mSessionManager;
        mSessionManager = nullptr;
    }

    if (mRendezvousSession != nullptr)
    {
        delete mRendezvousSession;
        mRendezvousSession = nullptr;
    }

    mConState = kConnectionState_NotConnected;
    return err;
}

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mAppReqState = appReqState;

    VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mRendezvousSession != nullptr)
    {
        err = mRendezvousSession->SendMessage(buffer);
    }
    else
    {
        bool trySessionResumption = true;
        VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);

        // If there is no secure connection to the device, try establishing it
        if (!IsSecurelyConnected())
        {
            // For now, it's expected that the device is connected
            VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
            err = EstablishSecureSession(mRemoteDeviceId.Value());
            SuccessOrExit(err);

            trySessionResumption = false;
        }

        // Hold on to the buffer, in case a session resumption and resend is needed
        buffer->AddRef();

        err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
        ChipLogDetail(Controller, "SendMessage returned %d", err);

        // The send could fail due to network timeouts (e.g. broken pipe)
        // Try sesion resumption if needed
        if (err != CHIP_NO_ERROR && trySessionResumption)
        {
            // VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
            err = ResumeSecureSession(mRemoteDeviceId.Value());
            // If session resumption failed, let's free the extra reference to
            // the buffer. If not, SendMessage would free it.
            VerifyOrExit(err == CHIP_NO_ERROR, PacketBuffer::Free(buffer));

            err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
            SuccessOrExit(err);
        }
        else
        {
            // Free the extra reference to the buffer
            PacketBuffer::Free(buffer);
        }
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
    if (mCurReqMsg != nullptr)
    {
        PacketBuffer::Free(mCurReqMsg);
        mCurReqMsg = nullptr;
    }
}

void ChipDeviceController::OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) {}

void ChipDeviceController::OnMessageReceived(const PacketHeader & header, Transport::PeerConnectionState * state,
                                             System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr)
{
    if (header.GetSourceNodeId().HasValue())
    {
        if (!mRemoteDeviceId.HasValue())
        {
            ChipLogDetail(Controller, "Learned remote device id");
            mRemoteDeviceId = header.GetSourceNodeId();
        }
        else if (mRemoteDeviceId != header.GetSourceNodeId())
        {
            ChipLogError(Controller, "Received message from an unexpected source node id.");
        }
    }
    if (IsSecurelyConnected() && mOnComplete.Response != nullptr)
    {
        mOnComplete.Response(this, mAppReqState, msgBuf);
    }
}

void ChipDeviceController::OnRendezvousComplete()
{
    if (mRendezvousSession != nullptr)
    {
        delete mRendezvousSession;
        mRendezvousSession = nullptr;
    }
}

void ChipDeviceController::OnRendezvousMessageReceived(PacketBuffer * buffer)
{
    // TODO: this is a stop gap solution to clean up RendezvouSession.
    // Once the Network provisioning code changes to using delegate calls, this
    // function would be removed. At that time, OnRendezvousComplete() would be used
    // to clean up the session
    if (mRendezvousSession != nullptr)
    {
        delete mRendezvousSession;
        mRendezvousSession = nullptr;
    }
}

void ChipDeviceController::OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err)
{
    if (mOnError != nullptr && err != CHIP_NO_ERROR)
    {
        mOnError(this, mAppReqState, err, nullptr);
    }

    switch (status)
    {
    case RendezvousSessionDelegate::SecurePairingSuccess:
        ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake\n");
        mPairingSession       = mRendezvousSession->GetPairingSession();
        mSecurePairingSession = &mPairingSession;

        if (mOnNewConnection)
        {
            mOnNewConnection(this, nullptr, mAppReqState);
        }

        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnNetworkCredentialsRequested(mRendezvousSession);
        }

        if (mStorageDelegate != nullptr)
        {
            SecurePairingSessionSerialized serialized;
            CHIP_ERROR err = mSecurePairingSession->Serialize(serialized);
            if (err == CHIP_NO_ERROR)
            {
                PERSISTENT_KEY_OP(mSecurePairingSession->GetPeerNodeId(), kDeviceCredentialsKeyPrefix, key,
                                  mStorageDelegate->SetKeyValue(key, Uint8::to_const_char(serialized.inner)));
            }
        }
        break;

    case RendezvousSessionDelegate::NetworkProvisioningSuccess:

        ChipLogDetail(Controller, "Remote device was assigned an ip address\n");
        mDeviceAddr = mRendezvousSession->GetIPAddress();
        if (mStorageDelegate != nullptr)
        {
            char addrStr[INET6_ADDRSTRLEN];
            mDeviceAddr.ToString(addrStr, INET6_ADDRSTRLEN);
            PERSISTENT_KEY_OP(mRendezvousSession->GetPairingSession().GetPeerNodeId(), kDeviceAddressKeyPrefix, key,
                              mStorageDelegate->SetKeyValue(key, addrStr));
        }
        break;

    default:
        break;
    };
}

} // namespace DeviceController
} // namespace chip
