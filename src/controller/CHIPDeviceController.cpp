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
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
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
#include <inttypes.h>
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

// This macro generates a key using node ID an key prefix, and performs the given action
// on that key.
#define PERSISTENT_KEY_OP(node, keyPrefix, key, action)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        const size_t len = strlen(keyPrefix);                                                                                      \
        /* 2 * sizeof(NodeId) to accomodate 2 character for each byte in Node Id */                                                \
        char key[len + 2 * sizeof(NodeId) + 1];                                                                                    \
        nlSTATIC_ASSERT_PRINT(sizeof(node) <= sizeof(uint64_t), "Node ID size is greater than expected");                          \
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
    mListenPort        = CHIP_PORT;
    mDeviceAddr        = IPAddress::Any;
    mDevicePort        = CHIP_PORT;
    mInterface         = INET_NULL_INTERFACEID;
    mLocalDeviceId     = 0;
    mNumCachedPackets  = 0;
    CHIP_ZERO_AT(mOnComplete);
    CHIP_ZERO_AT(mCachedPackets);
}

ChipDeviceController::~ChipDeviceController()
{
    if (mTestSecurePairingSecret != nullptr)
    {
        chip::Platform::Delete(mTestSecurePairingSecret);
    }
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, DevicePairingDelegate * pairingDelegate,
                                      PersistentStorageDelegate * storageDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    err = DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = Init(localNodeId, &DeviceLayer::SystemLayer, &DeviceLayer::InetLayer, pairingDelegate, storageDelegate);
#endif // CONFIG_DEVICE_LAYER

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, System::Layer * systemLayer, InetLayer * inetLayer,
                                      DevicePairingDelegate * pairingDelegate, PersistentStorageDelegate * storageDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mState           = kState_Initialized;
    mLocalDeviceId   = localNodeId;
    mSystemLayer     = systemLayer;
    mInetLayer       = inetLayer;
    mPairingDelegate = pairingDelegate;
    mStorageDelegate = storageDelegate;

    if (mStorageDelegate != nullptr)
    {
        mStorageDelegate->SetDelegate(this);
    }

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
    chip::Platform::Delete(mSystemLayer);
    chip::Platform::Delete(mInetLayer);
#endif // CONFIG_DEVICE_LAYER

    mSystemLayer = nullptr;
    mInetLayer   = nullptr;

    if (mSessionManager != nullptr)
    {
        chip::Platform::Delete(mSessionManager);
        mSessionManager = nullptr;
    }

    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
        mRendezvousSession = nullptr;
    }

    mConState = kConnectionState_NotConnected;
    CHIP_ZERO_AT(mOnComplete);
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

    mRendezvousSession = chip::Platform::New<RendezvousSession>(this);
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
        chip::Platform::Delete(mRendezvousSession);
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
        chip::Platform::Delete(mTestSecurePairingSecret);
    }

    mTestSecurePairingSecret = chip::Platform::New<SecurePairingUsingTestSecret>(
        Optional<NodeId>::Value(remoteDeviceId), static_cast<uint16_t>(0), static_cast<uint16_t>(0));

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

CHIP_ERROR ChipDeviceController::SetUdpListenPort(uint16_t listenPort)
{
    if (mState != kState_Initialized || mConState != kConnectionState_NotConnected) return CHIP_ERROR_INCORRECT_STATE;
    mListenPort = listenPort;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceController::EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mSecurePairingSession != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceAddr != IPAddress::Any, err = CHIP_ERROR_INCORRECT_STATE);

    mSessionManager = chip::Platform::New<SecureSessionMgr<Transport::UDP>>();

    err = mSessionManager->Init(mLocalDeviceId, mSystemLayer,
                                Transport::UdpListenParameters(mInetLayer).SetAddressType(mDeviceAddr.Type()).SetListenPort(mListenPort));
    SuccessOrExit(err);

    mSessionManager->SetDelegate(this);

    err = mSessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(mDeviceAddr, mDevicePort, mInterface)),
        mSecurePairingSession);
    SuccessOrExit(err);

    mConState = kConnectionState_SecureConnected;

    SendCachedPackets();

exit:

    if (err != CHIP_NO_ERROR)
    {
        if (mSessionManager != nullptr)
        {
            chip::Platform::Delete(mSessionManager);
            mSessionManager = nullptr;
        }
        mConState = kConnectionState_NotConnected;
    }

    return err;
}

void ChipDeviceController::OnValue(const char * key, const char * value)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    NodeId peer    = mRemoteDeviceId.Value();

    VerifyOrExit(key != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    PERSISTENT_KEY_OP(
        peer, kDeviceCredentialsKeyPrefix, expectedKey, if (strcmp(key, expectedKey) == 0) {
            SecurePairingSessionSerialized serialized;
            size_t length = 0;

            VerifyOrExit(value != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
            length = strlen(value) + 1; // account for the null termination
            VerifyOrExit(length <= sizeof(serialized.inner), err = CHIP_ERROR_INVALID_ARGUMENT);
            memmove(serialized.inner, value, length);
            SuccessOrExit(mPairingSession.Deserialize(serialized));

            mSecurePairingSession = &mPairingSession;
        });

    PERSISTENT_KEY_OP(
        peer, kDeviceAddressKeyPrefix, expectedKey, if (strcmp(key, expectedKey) == 0) {
            VerifyOrExit(value != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrExit(IPAddress::FromString(value, mDeviceAddr), err = CHIP_ERROR_INVALID_ADDRESS);
        });

    if (mSecurePairingSession != nullptr && mDeviceAddr != IPAddress::Any)
    {
        SuccessOrExit(EstablishSecureSession());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        mConState = kConnectionState_NotConnected;
        DiscardCachedPackets();
    }
}

void ChipDeviceController::OnStatus(const char * key, Operation op, CHIP_ERROR err) {}

CHIP_ERROR ChipDeviceController::TryEstablishingSecureSession(NodeId peer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != kState_Initialized || mSessionManager != nullptr || mConState == kConnectionState_SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    if (mStorageDelegate != nullptr)
    {
        mSecurePairingSession = nullptr;
        mDeviceAddr           = IPAddress::Any;
        PERSISTENT_KEY_OP(peer, kDeviceCredentialsKeyPrefix, key, mStorageDelegate->GetKeyValue(key));
        PERSISTENT_KEY_OP(peer, kDeviceAddressKeyPrefix, key, mStorageDelegate->GetKeyValue(key));
        mConState = kConnectionState_SecureConnecting;
    }
    else
    {
        ExitNow(err = EstablishSecureSession());
    }

exit:

    if (err != CHIP_NO_ERROR)
    {
        DiscardCachedPackets();
    }
    return err;
}

CHIP_ERROR ChipDeviceController::ResumeSecureSession(NodeId peer)
{
    if (mConState == kConnectionState_SecureConnected)
    {
        mConState = kConnectionState_Connected;
    }

    if (mSessionManager != nullptr)
    {
        chip::Platform::Delete(mSessionManager);
        mSessionManager = nullptr;
    }

    CHIP_ERROR err = TryEstablishingSecureSession(peer);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ResumeSecureSession returning error %d\n", err);
    }
    return err;
}

bool ChipDeviceController::IsConnected() const
{
    return mState == kState_Initialized &&
        (mConState == kConnectionState_Connected || mConState == kConnectionState_SecureConnected);
}

bool ChipDeviceController::IsSecurelyConnected() const
{
    return mState == kState_Initialized && mConState == kConnectionState_SecureConnected;
}

bool ChipDeviceController::GetIpAddress(Inet::IPAddress & addr) const
{
    if (IsConnected())
        addr = mDeviceAddr;
    return IsConnected();
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
        chip::Platform::Delete(mSessionManager);
        mSessionManager = nullptr;
    }

    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
        mRendezvousSession = nullptr;
    }

    mConState = kConnectionState_NotConnected;
    return err;
}

CHIP_ERROR ChipDeviceController::CachePacket(System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mNumCachedPackets < kPacketCacheMaxSize, err = CHIP_ERROR_INTERNAL);
    mCachedPackets[mNumCachedPackets++] = buffer;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::SendCachedPackets()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(IsSecurelyConnected(), err = CHIP_ERROR_INCORRECT_STATE);

    for (uint16_t i = 0; i < mNumCachedPackets; i++)
    {
        err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), mCachedPackets[i]);
        ChipLogDetail(Controller, "SendMessage returned %d", err);
    }

    mNumCachedPackets = 0;
    CHIP_ZERO_AT(mCachedPackets);

exit:
    return err;
}

void ChipDeviceController::DiscardCachedPackets()
{
    for (uint16_t i = 0; i < mNumCachedPackets; i++)
    {
        PacketBuffer::Free(mCachedPackets[i]);
    }

    mNumCachedPackets = 0;
    CHIP_ZERO_AT(mCachedPackets);
}

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer, NodeId peerDevice)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    bool trySessionResumption = true;

    VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    mAppReqState = appReqState;

    if (peerDevice != kUndefinedNodeId)
    {
        mRemoteDeviceId = Optional<NodeId>::Value(peerDevice);
    }
    VerifyOrExit(mRemoteDeviceId.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);

    // If there is no secure connection to the device, try establishing it
    if (!IsSecurelyConnected())
    {
        err = TryEstablishingSecureSession(mRemoteDeviceId.Value());
        SuccessOrExit(err);

        trySessionResumption = false;

        if (mConState == kConnectionState_SecureConnecting)
        {
            // Cache the packet while connection is being established
            ExitNow(err = CachePacket(buffer));
        }
    }

    // Hold on to the buffer, in case of session resumption and resend is needed
    buffer->AddRef();

    err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
    ChipLogDetail(Controller, "SendMessage returned %d", err);

    // The send could fail due to network timeouts (e.g. broken pipe)
    // Try sesion resumption if needed
    if (err != CHIP_NO_ERROR && trySessionResumption)
    {
        err = ResumeSecureSession(mRemoteDeviceId.Value());
        // If session resumption failed, let's free the extra reference to
        // the buffer. If not, SendMessage would free it.
        VerifyOrExit(err == CHIP_NO_ERROR, PacketBuffer::Free(buffer));

        if (mConState == kConnectionState_SecureConnecting)
        {
            // Cache the packet while connection is being established
            ExitNow(err = CachePacket(buffer));
        }

        err = mSessionManager->SendMessage(mRemoteDeviceId.Value(), buffer);
        SuccessOrExit(err);
    }
    else
    {
        // Free the extra reference to the buffer
        PacketBuffer::Free(buffer);
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

CHIP_ERROR ChipDeviceController::SetDevicePairingDelegate(DevicePairingDelegate * pairingDelegate)
{
    mPairingDelegate = pairingDelegate;
    return CHIP_NO_ERROR;
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

void ChipDeviceController::OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader,
                                             Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                                             SecureSessionMgrBase * mgr)
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

void ChipDeviceController::OnRendezvousError(CHIP_ERROR err)
{
    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
        mRendezvousSession = nullptr;
    }

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(err);
    }
}

void ChipDeviceController::OnRendezvousComplete()
{
    OnRendezvousError(CHIP_NO_ERROR);
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
        ChipLogProgress(Controller, "Remote device completed SPAKE2+ handshake\n");
        mPairingSession       = mRendezvousSession->GetPairingSession();
        mSecurePairingSession = &mPairingSession;

        if (mOnNewConnection)
        {
            ChipLogProgress(Controller, "Will Call on mOnNewConnection(%p)\n", mOnNewConnection);
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

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(status);
    }
}

} // namespace DeviceController
} // namespace chip
