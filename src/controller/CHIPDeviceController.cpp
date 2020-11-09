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
#include <support/SafeInt.h>
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
namespace Controller {

using namespace chip::Encoding;

constexpr const char kPairedDeviceListKeyPrefix[] = "ListPairedDevices";
constexpr const char kPairedDeviceKeyPrefix[]     = "PairedDevice";

// This macro generates a key using node ID an key prefix, and performs the given action
// on that key.
#define PERSISTENT_KEY_OP(node, keyPrefix, key, action)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        constexpr size_t len = std::extent<decltype(keyPrefix)>::value;                                                            \
        nlSTATIC_ASSERT_PRINT(len > 0, "keyPrefix length must be known at compile time");                                          \
        /* 2 * sizeof(NodeId) to accomodate 2 character for each byte in Node Id */                                                \
        char key[len + 2 * sizeof(NodeId) + 1];                                                                                    \
        nlSTATIC_ASSERT_PRINT(sizeof(node) <= sizeof(uint64_t), "Node ID size is greater than expected");                          \
        snprintf(key, sizeof(key), "%s%" PRIx64, keyPrefix, node);                                                                 \
        action;                                                                                                                    \
    } while (0)

DeviceController::DeviceController()
{
    mState                    = kState_NotInitialized;
    mSessionManager           = nullptr;
    mLocalDeviceId            = 0;
    mStorageDelegate          = nullptr;
    mPairedDevicesInitialized = false;
}

CHIP_ERROR DeviceController::Init(NodeId localDeviceId, PersistentStorageDelegate * storageDelegate, System::Layer * systemLayer,
                                  Inet::InetLayer * inetLayer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    if (systemLayer != nullptr && inetLayer != nullptr)
    {
        mSystemLayer = systemLayer;
        mInetLayer   = inetLayer;
    }
    else
    {
#if CONFIG_DEVICE_LAYER
        err = DeviceLayer::PlatformMgr().InitChipStack();
        SuccessOrExit(err);

        mSystemLayer = &DeviceLayer::SystemLayer;
        mInetLayer   = &DeviceLayer::InetLayer;
#endif // CONFIG_DEVICE_LAYER
    }

    VerifyOrExit(mSystemLayer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mInetLayer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    mStorageDelegate = storageDelegate;

    if (mStorageDelegate != nullptr)
    {
        mStorageDelegate->SetDelegate(this);
    }

    mSessionManager = chip::Platform::New<SecureSessionMgr<Transport::UDP>>();

    err = mSessionManager->Init(localDeviceId, mSystemLayer,
                                Transport::UdpListenParameters(mInetLayer).SetAddressType(Inet::kIPAddressType_IPv6));
    SuccessOrExit(err);

    mSessionManager->SetDelegate(this);

    mState         = kState_Initialized;
    mLocalDeviceId = localDeviceId;

exit:
    return err;
}

CHIP_ERROR DeviceController::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the controller");

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

exit:
    return err;
}

CHIP_ERROR DeviceController::GetDevice(NodeId deviceId, const SerializedDevice & deviceInfo, Device ** out_device)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;
    uint16_t index  = FindDevice(deviceId);

    if (index < mNumMaxActiveDevices)
    {
        device = &mActiveDevices[index];
    }
    else
    {
        VerifyOrExit(mPairedDevices.Contains(deviceId), err = CHIP_ERROR_NOT_CONNECTED);

        index = GetAvailableDevice();
        VerifyOrExit(index < mNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
        device = &mActiveDevices[index];

        err = device->Deserialize(deviceInfo);
        VerifyOrExit(err == CHIP_NO_ERROR, ReleaseDevice(device));

        device->mSessionManager = mSessionManager;
        device->mInetLayer      = mInetLayer;
    }

    (*out_device) = device;

exit:
    return err;
}

CHIP_ERROR DeviceController::GetDevice(NodeId deviceId, Device ** out_device)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;
    uint16_t index  = FindDevice(deviceId);

    if (index < mNumMaxActiveDevices)
    {
        device = &mActiveDevices[index];
    }
    else
    {
        if (!mPairedDevicesInitialized)
        {
            const uint16_t max_size = mPairedDevices.MaxSerializedSize();
            char buffer[max_size];
            uint16_t size = max_size;

            PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key,
                              err = mStorageDelegate->GetKeyValue(key, buffer, size));
            SuccessOrExit(err);
            VerifyOrExit(size <= max_size, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

            err = SetPairedDeviceList(buffer);
            SuccessOrExit(err);
        }

        VerifyOrExit(mPairedDevices.Contains(deviceId), err = CHIP_ERROR_NOT_CONNECTED);

        index = GetAvailableDevice();
        VerifyOrExit(index < mNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
        device = &mActiveDevices[index];

        {
            SerializedDevice deviceInfo;
            uint16_t size = sizeof(deviceInfo.inner);

            PERSISTENT_KEY_OP(deviceId, kPairedDeviceKeyPrefix, key,
                              err = mStorageDelegate->GetKeyValue(key, Uint8::to_char(deviceInfo.inner), size));
            SuccessOrExit(err);
            VerifyOrExit(size <= sizeof(deviceInfo.inner), err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

            err = device->Deserialize(deviceInfo);
            VerifyOrExit(err == CHIP_NO_ERROR, ReleaseDevice(device));

            device->mSessionManager = mSessionManager;
            device->mInetLayer      = mInetLayer;
        }
    }

    (*out_device) = device;

exit:
    return err;
}

CHIP_ERROR DeviceController::ServiceEvents()
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

CHIP_ERROR DeviceController::ServiceEventSignal()
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

void DeviceController::OnNewConnection(Transport::PeerConnectionState * peerConnection, SecureSessionMgrBase * mgr) {}

void DeviceController::OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader,
                                         Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                                         SecureSessionMgrBase * mgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t index = 0;
    NodeId peer;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(header.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);

    peer  = header.GetSourceNodeId().Value();
    index = FindDevice(peer);
    VerifyOrExit(index < mNumMaxActiveDevices, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    mActiveDevices[index].OnMessageReceived(header, payloadHeader, state, msgBuf, mgr);

exit:
    return;
}

uint16_t DeviceController::GetAvailableDevice()
{
    uint16_t i = 0;
    while (i < mNumMaxActiveDevices && mActiveDevices[i].mActive)
        i++;
    if (i < mNumMaxActiveDevices)
    {
        mActiveDevices[i].mActive = true;
    }

    return i;
}

void DeviceController::ReleaseDevice(Device * device)
{
    device->mActive = false;
}

void DeviceController::ReleaseDevice(uint16_t index)
{
    if (index < mNumMaxActiveDevices)
    {
        ReleaseDevice(&mActiveDevices[index]);
    }
}

uint16_t DeviceController::FindDevice(NodeId id)
{
    uint16_t i = 0;
    while (i < mNumMaxActiveDevices)
    {
        if (mActiveDevices[i].mActive && mActiveDevices[i].mDeviceId == id)
        {
            return i;
        }
        i++;
    }
    return i;
}

CHIP_ERROR DeviceController::SetPairedDeviceList(const char * serialized)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    size_t len      = strlen(serialized) + 1;
    uint16_t lenU16 = static_cast<uint16_t>(len);
    VerifyOrExit(CanCastTo<uint16_t>(len), err = CHIP_ERROR_INVALID_ARGUMENT);
    err = mPairedDevices.DeserializeBase64(serialized, lenU16);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to recreate the device list\n");
    }
    else
    {
        mPairedDevicesInitialized = true;
    }

    return err;
}

void DeviceController::OnValue(const char * key, const char * value) {}

void DeviceController::OnStatus(const char * key, Operation op, CHIP_ERROR err) {}

DeviceCommissioner::DeviceCommissioner() : DeviceController()
{
    mPairingDelegate      = nullptr;
    mRendezvousSession    = nullptr;
    mDeviceBeingPaired    = mNumMaxActiveDevices;
    mPairedDevicesUpdated = false;
}

DeviceCommissioner::~DeviceCommissioner()
{
    PersistDeviceList();
}

CHIP_ERROR DeviceCommissioner::Init(NodeId localDeviceId, PersistentStorageDelegate * storageDelegate,
                                    DevicePairingDelegate * pairingDelegate, System::Layer * systemLayer,
                                    Inet::InetLayer * inetLayer)
{
    CHIP_ERROR err = DeviceController::Init(localDeviceId, storageDelegate, systemLayer, inetLayer);
    SuccessOrExit(err);

    mPairingDelegate = pairingDelegate;

exit:
    return err;
}

CHIP_ERROR DeviceCommissioner::SetDevicePairingDelegate(DevicePairingDelegate * pairingDelegate)
{
    mPairingDelegate = pairingDelegate;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the commissioner");

    PersistDeviceList();

    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
        mRendezvousSession = nullptr;
    }

    DeviceController::Shutdown();

exit:
    return err;
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState,
                                          uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingPaired == mNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE
    if (!params.HasBleLayer())
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());
    }
#endif // CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE

    mDeviceBeingPaired = GetAvailableDevice();
    VerifyOrExit(mDeviceBeingPaired < mNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
    device = &mActiveDevices[mDeviceBeingPaired];

    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
    }

    mRendezvousSession = chip::Platform::New<RendezvousSession>(this);
    err                = mRendezvousSession->Init(params.SetLocalNodeId(mLocalDeviceId));
    SuccessOrExit(err);

    device->mDeviceId       = remoteDeviceId;
    device->mDevicePort     = devicePort;
    device->mInterface      = interfaceId;
    device->mState          = Device::kConnectionState_Connecting;
    device->mSessionManager = mSessionManager;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (mRendezvousSession != nullptr)
        {
            chip::Platform::Delete(mRendezvousSession);
            mRendezvousSession = nullptr;
        }

        if (device != nullptr)
        {
            ReleaseDevice(device);
            mDeviceBeingPaired = mNumMaxActiveDevices;
        }
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::PairTestDeviceWithoutSecurity(NodeId remoteDeviceId, const Inet::IPAddress & deviceAddr,
                                                             SerializedDevice & serialized, void * appReqState, uint16_t devicePort,
                                                             Inet::InterfaceId interfaceId)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    SecurePairingUsingTestSecret * testSecurePairingSecret = nullptr;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingPaired == mNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

    testSecurePairingSecret = chip::Platform::New<SecurePairingUsingTestSecret>(Optional<NodeId>::Value(remoteDeviceId),
                                                                                static_cast<uint16_t>(0), static_cast<uint16_t>(0));
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mDeviceBeingPaired = GetAvailableDevice();
    VerifyOrExit(mDeviceBeingPaired < mNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
    device = &mActiveDevices[mDeviceBeingPaired];

    testSecurePairingSecret->Serializable(device->mPairing);

    device->mDeviceId       = remoteDeviceId;
    device->mDevicePort     = devicePort;
    device->mInterface      = interfaceId;
    device->mState          = Device::kConnectionState_Connecting;
    device->mDeviceAddr     = deviceAddr;
    device->mSessionManager = mSessionManager;

    device->Serialize(serialized);

    OnRendezvousComplete();

exit:
    if (testSecurePairingSecret != nullptr)
    {
        chip::Platform::Delete(testSecurePairingSecret);
    }

    if (err != CHIP_NO_ERROR)
    {
        if (device != nullptr)
        {
            ReleaseDevice(device);
            mDeviceBeingPaired = mNumMaxActiveDevices;
        }
    }

    return err;
}
CHIP_ERROR DeviceCommissioner::StopPairing(NodeId remoteDeviceId)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    VerifyOrExit(mState == kState_Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingPaired < mNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

    device = &mActiveDevices[mDeviceBeingPaired];
    VerifyOrExit(device->mDeviceId == remoteDeviceId, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    if (mRendezvousSession != nullptr)
    {
        chip::Platform::Delete(mRendezvousSession);
        mRendezvousSession = nullptr;
    }

    ReleaseDevice(device);
    mDeviceBeingPaired = mNumMaxActiveDevices;

exit:
    return err;
}

CHIP_ERROR DeviceCommissioner::UnpairDevice(NodeId remoteDeviceId)
{
    // TODO: Send unpairing message to the remote device.

    if (mStorageDelegate != nullptr)
    {
        PERSISTENT_KEY_OP(remoteDeviceId, kPairedDeviceKeyPrefix, key, mStorageDelegate->DeleteKeyValue(key));
        mPairedDevices.Remove(remoteDeviceId);
        mPairedDevicesUpdated = true;
    }

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnRendezvousError(CHIP_ERROR err)
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

    if (mDeviceBeingPaired != mNumMaxActiveDevices)
    {
        DeviceController::ReleaseDevice(mDeviceBeingPaired);
        mDeviceBeingPaired = mNumMaxActiveDevices;
    }
}

void DeviceCommissioner::OnRendezvousComplete()
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;
    VerifyOrExit(mDeviceBeingPaired < mNumMaxActiveDevices, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    device = &mActiveDevices[mDeviceBeingPaired];
    mPairedDevices.Insert(device->mDeviceId);
    mPairedDevicesUpdated = true;
    SuccessOrExit(err);

    if (mStorageDelegate != nullptr)
    {
        SerializedDevice serialized;
        device->Serialize(serialized);
        PERSISTENT_KEY_OP(device->mDeviceId, kPairedDeviceKeyPrefix, key,
                          mStorageDelegate->SetKeyValue(key, Uint8::to_const_char(serialized.inner)));
    }

exit:
    OnRendezvousError(err);
}

void DeviceCommissioner::OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err)
{
    Device * device = nullptr;
    VerifyOrExit(mDeviceBeingPaired < mNumMaxActiveDevices, /**/);
    device = &mActiveDevices[mDeviceBeingPaired];
    switch (status)
    {
    case RendezvousSessionDelegate::SecurePairingSuccess:
        ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake\n");
        mRendezvousSession->GetPairingSession().Serializable(device->mPairing);

        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnNetworkCredentialsRequested(mRendezvousSession);
        }
        break;

    case RendezvousSessionDelegate::SecurePairingFailed:
        ChipLogDetail(Controller, "Remote device failed in SPAKE2+ handshake\n");
        break;

    case RendezvousSessionDelegate::NetworkProvisioningSuccess:
        ChipLogDetail(Controller, "Remote device was assigned an ip address\n");
        device->mDeviceAddr = mRendezvousSession->GetIPAddress();
        break;

    case RendezvousSessionDelegate::NetworkProvisioningFailed:
        ChipLogDetail(Controller, "Remote device failed in network provisioning\n");
        break;

    default:
        break;
    };
exit:
    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(status);
    }
}

void DeviceCommissioner::PersistDeviceList()
{
    if (mStorageDelegate != nullptr && mPairedDevicesUpdated)
    {
        const uint16_t size = mPairedDevices.MaxSerializedSize();
        char serialized[size];
        uint16_t requiredSize = size;
        const char * value    = mPairedDevices.SerializeBase64(serialized, requiredSize);
        if (value != nullptr && requiredSize <= size)
        {
            PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key, mStorageDelegate->SetKeyValue(key, value));
            mPairedDevicesUpdated = false;
        }
    }
}

void DeviceCommissioner::ReleaseDevice(Device * device)
{
    PersistDeviceList();
    DeviceController::ReleaseDevice(device);
}

} // namespace Controller
} // namespace chip
