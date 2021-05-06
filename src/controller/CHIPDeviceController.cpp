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

#include <app/InteractionModelEngine.h>
#include <app/util/DataModelHandler.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

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
constexpr const char kNextAvailableKeyID[]        = "StartKeyID";

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
constexpr uint16_t kMdnsPort = 5353;
#endif

constexpr const uint32_t kSessionEstablishmentTimeout = 30 * kMillisecondPerSecond;

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
    mState                    = State::NotInitialized;
    mSessionMgr               = nullptr;
    mExchangeMgr              = nullptr;
    mLocalDeviceId            = 0;
    mStorageDelegate          = nullptr;
    mPairedDevicesInitialized = false;
    mListenPort               = CHIP_PORT;
}

CHIP_ERROR DeviceController::Init(NodeId localDeviceId, ControllerInitParams params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::AdminPairingInfo * admin = nullptr;

    VerifyOrExit(mState == State::NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    if (params.systemLayer != nullptr && params.inetLayer != nullptr)
    {
        mSystemLayer = params.systemLayer;
        mInetLayer   = params.inetLayer;
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

    mStorageDelegate = params.storageDelegate;
#if CONFIG_NETWORK_LAYER_BLE
#if CONFIG_DEVICE_LAYER
    if (params.bleLayer == nullptr)
    {
        params.bleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
    }
#endif // CONFIG_DEVICE_LAYER
    mBleLayer = params.bleLayer;
    VerifyOrExit(mBleLayer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
#endif

    mTransportMgr = chip::Platform::New<DeviceTransportMgr>();
    mSessionMgr   = chip::Platform::New<SecureSessionMgr>();
    mExchangeMgr  = chip::Platform::New<Messaging::ExchangeManager>();

    err = mTransportMgr->Init(
        Transport::UdpListenParameters(mInetLayer).SetAddressType(Inet::kIPAddressType_IPv6).SetListenPort(mListenPort)
#if INET_CONFIG_ENABLE_IPV4
            ,
        Transport::UdpListenParameters(mInetLayer).SetAddressType(Inet::kIPAddressType_IPv4).SetListenPort(mListenPort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
            ,
        Transport::BleListenParameters(mBleLayer)
#endif
    );
    SuccessOrExit(err);

    admin = mAdmins.AssignAdminId(mAdminId, localDeviceId);
    VerifyOrExit(admin != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = mSessionMgr->Init(localDeviceId, mSystemLayer, mTransportMgr, &mAdmins);
    SuccessOrExit(err);

    err = mExchangeMgr->Init(mSessionMgr);
    SuccessOrExit(err);

    err = mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::TempZCL::Id, this);
    SuccessOrExit(err);

#if CHIP_ENABLE_INTERACTION_MODEL
    err = chip::app::InteractionModelEngine::GetInstance()->Init(mExchangeMgr, params.imDelegate);
    SuccessOrExit(err);
#endif

    mExchangeMgr->SetDelegate(this);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    if (params.mDeviceAddressUpdateDelegate != nullptr)
    {
        err = Mdns::Resolver::Instance().SetResolverDelegate(this);
        SuccessOrExit(err);

        mDeviceAddressUpdateDelegate = params.mDeviceAddressUpdateDelegate;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

    InitDataModelHandler(mExchangeMgr);

    mState         = State::Initialized;
    mLocalDeviceId = localDeviceId;

    ReleaseAllDevices();

exit:
    return err;
}

CHIP_ERROR DeviceController::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the controller");

    mState = State::NotInitialized;

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().Shutdown());
#else
    mSystemLayer->Shutdown();
    mInetLayer->Shutdown();
    chip::Platform::Delete(mSystemLayer);
    chip::Platform::Delete(mInetLayer);
#endif // CONFIG_DEVICE_LAYER

    mSystemLayer     = nullptr;
    mInetLayer       = nullptr;
    mStorageDelegate = nullptr;

    if (mExchangeMgr != nullptr)
    {
        chip::Platform::Delete(mExchangeMgr);
        mExchangeMgr = nullptr;
    }

    if (mSessionMgr != nullptr)
    {
        chip::Platform::Delete(mSessionMgr);
        mSessionMgr = nullptr;
    }

    if (mTransportMgr != nullptr)
    {
        chip::Platform::Delete(mTransportMgr);
        mTransportMgr = nullptr;
    }

    mAdmins.ReleaseAdminId(mAdminId);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    if (mDeviceAddressUpdateDelegate != nullptr)
    {
        Mdns::Resolver::Instance().SetResolverDelegate(nullptr);
        mDeviceAddressUpdateDelegate = nullptr;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

    ReleaseAllDevices();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::SetUdpListenPort(uint16_t listenPort)
{
    if (mState == State::Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mListenPort = listenPort;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::GetDevice(NodeId deviceId, const SerializedDevice & deviceInfo, Device ** out_device)
{
    Device * device = nullptr;

    VerifyOrReturnError(out_device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    uint16_t index = FindDeviceIndex(deviceId);

    if (index < kNumMaxActiveDevices)
    {
        device = &mActiveDevices[index];
    }
    else
    {
        VerifyOrReturnError(mPairedDevices.Contains(deviceId), CHIP_ERROR_NOT_CONNECTED);

        index = GetInactiveDeviceIndex();
        VerifyOrReturnError(index < kNumMaxActiveDevices, CHIP_ERROR_NO_MEMORY);
        device = &mActiveDevices[index];

        CHIP_ERROR err = device->Deserialize(deviceInfo);
        if (err != CHIP_NO_ERROR)
        {
            ReleaseDevice(device);
            ReturnErrorOnFailure(err);
        }

        device->Init(GetControllerDeviceInitParams(), mListenPort, mAdminId);
    }

    *out_device = device;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::GetDevice(NodeId deviceId, Device ** out_device)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;
    uint16_t index  = 0;

    VerifyOrExit(out_device != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    index = FindDeviceIndex(deviceId);

    if (index < kNumMaxActiveDevices)
    {
        device = &mActiveDevices[index];
    }
    else
    {
        err = InitializePairedDeviceList();
        SuccessOrExit(err);

        VerifyOrExit(mPairedDevices.Contains(deviceId), err = CHIP_ERROR_NOT_CONNECTED);

        index = GetInactiveDeviceIndex();
        VerifyOrExit(index < kNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
        device = &mActiveDevices[index];

        {
            SerializedDevice deviceInfo;
            uint16_t size = sizeof(deviceInfo.inner);

            PERSISTENT_KEY_OP(deviceId, kPairedDeviceKeyPrefix, key,
                              err = mStorageDelegate->SyncGetKeyValue(key, deviceInfo.inner, size));
            SuccessOrExit(err);
            VerifyOrExit(size <= sizeof(deviceInfo.inner), err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

            err = device->Deserialize(deviceInfo);
            VerifyOrExit(err == CHIP_NO_ERROR, ReleaseDevice(device));

            device->Init(GetControllerDeviceInitParams(), mListenPort, mAdminId);
        }
    }

    *out_device = device;

exit:
    if (err != CHIP_NO_ERROR && device != nullptr)
    {
        ReleaseDevice(device);
    }
    return err;
}

CHIP_ERROR DeviceController::UpdateDevice(Device * device, uint64_t fabricId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    ReturnErrorOnFailure(Mdns::Resolver::Instance().StartResolver(mInetLayer, kMdnsPort));

    return Mdns::Resolver::Instance().ResolveNodeId(chip::PeerId().SetNodeId(device->GetDeviceId()).SetFabricId(fabricId),
                                                    chip::Inet::kIPAddressType_Any);
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS
}

void DeviceController::PersistDevice(Device * device)
{
    // mStorageDelegate would not be null for a typical pairing scenario, as Pair()
    // requires a valid storage delegate. However, test pairing usecase, that's used
    // mainly by test applications, do not require a storage delegate. This is to
    // reduce overheads on these tests.
    // Let's make sure the delegate object is available before calling into it.
    if (mStorageDelegate != nullptr)
    {
        SerializedDevice serialized;
        device->Serialize(serialized);

        // TODO: no need to base-64 the serialized values AGAIN
        PERSISTENT_KEY_OP(device->GetDeviceId(), kPairedDeviceKeyPrefix, key,
                          mStorageDelegate->SyncSetKeyValue(key, serialized.inner, sizeof(serialized.inner)));
    }
}

CHIP_ERROR DeviceController::ServiceEvents()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().StartEventLoopTask());
#endif // CONFIG_DEVICE_LAYER

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::ServiceEventSignal()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

#if CONFIG_DEVICE_LAYER && (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
    DeviceLayer::SystemLayer.WakeSelect();
#else
    ReturnErrorOnFailure(CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
#endif // CONFIG_DEVICE_LAYER && (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)

    return CHIP_NO_ERROR;
}

void DeviceController::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                         const PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf)
{
    uint16_t index;
    bool needClose = true;

    VerifyOrExit(mState == State::Initialized, ChipLogError(Controller, "OnMessageReceived was called in incorrect state"));

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(),
                 ChipLogError(Controller, "OnMessageReceived was called for unknown source node"));

    index = FindDeviceIndex(packetHeader.GetSourceNodeId().Value());
    VerifyOrExit(index < kNumMaxActiveDevices, ChipLogError(Controller, "OnMessageReceived was called for unknown device object"));

    needClose = false; // Device will handle it
    mActiveDevices[index].OnMessageReceived(ec, packetHeader, payloadHeader, std::move(msgBuf));

exit:
    if (needClose)
    {
        ec->Close();
    }
}

void DeviceController::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(Controller, "Time out! failed to receive response from Exchange: %p", ec);
}

void DeviceController::OnNewConnection(SecureSessionHandle session, Messaging::ExchangeManager * mgr)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnNewConnection was called in incorrect state"));

    uint16_t index = FindDeviceIndex(mgr->GetSessionMgr()->GetPeerConnectionState(session)->GetPeerNodeId());
    VerifyOrReturn(index < kNumMaxActiveDevices,
                   ChipLogDetail(Controller, "OnNewConnection was called for unknown device, ignoring it."));

    mActiveDevices[index].OnNewConnection(session);
}

void DeviceController::OnConnectionExpired(SecureSessionHandle session, Messaging::ExchangeManager * mgr)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnConnectionExpired was called in incorrect state"));

    uint16_t index = FindDeviceIndex(session);
    VerifyOrReturn(index < kNumMaxActiveDevices,
                   ChipLogDetail(Controller, "OnConnectionExpired was called for unknown device, ignoring it."));

    mActiveDevices[index].OnConnectionExpired(session);
}

uint16_t DeviceController::GetInactiveDeviceIndex()
{
    uint16_t i = 0;
    while (i < kNumMaxActiveDevices && mActiveDevices[i].IsActive())
        i++;
    if (i < kNumMaxActiveDevices)
    {
        mActiveDevices[i].SetActive(true);
    }

    return i;
}

void DeviceController::ReleaseDevice(Device * device)
{
    device->Reset();
}

void DeviceController::ReleaseDevice(uint16_t index)
{
    if (index < kNumMaxActiveDevices)
    {
        ReleaseDevice(&mActiveDevices[index]);
    }
}

void DeviceController::ReleaseDeviceById(NodeId remoteDeviceId)
{
    for (uint16_t i = 0; i < kNumMaxActiveDevices; i++)
    {
        if (mActiveDevices[i].GetDeviceId() == remoteDeviceId)
        {
            ReleaseDevice(&mActiveDevices[i]);
        }
    }
}

void DeviceController::ReleaseAllDevices()
{
    for (uint16_t i = 0; i < kNumMaxActiveDevices; i++)
    {
        ReleaseDevice(&mActiveDevices[i]);
    }
}

uint16_t DeviceController::FindDeviceIndex(SecureSessionHandle session)
{
    uint16_t i = 0;
    while (i < kNumMaxActiveDevices)
    {
        if (mActiveDevices[i].IsActive() && mActiveDevices[i].IsSecureConnected() && mActiveDevices[i].MatchesSession(session))
        {
            return i;
        }
        i++;
    }
    return i;
}

uint16_t DeviceController::FindDeviceIndex(NodeId id)
{
    uint16_t i = 0;
    while (i < kNumMaxActiveDevices)
    {
        if (mActiveDevices[i].IsActive() && mActiveDevices[i].GetDeviceId() == id)
        {
            return i;
        }
        i++;
    }
    return i;
}

CHIP_ERROR DeviceController::InitializePairedDeviceList()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char * buffer  = nullptr;

    VerifyOrExit(mStorageDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (!mPairedDevicesInitialized)
    {
        constexpr uint16_t max_size = CHIP_MAX_SERIALIZED_SIZE_U64(kNumMaxPairedDevices);
        buffer                      = static_cast<char *>(chip::Platform::MemoryAlloc(max_size));
        uint16_t size               = max_size;

        VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        CHIP_ERROR lookupError = CHIP_NO_ERROR;
        PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key,
                          lookupError = mStorageDelegate->SyncGetKeyValue(key, buffer, size));

        // It's ok to not have an entry for the Paired Device list. We treat it the same as having an empty list.
        if (lookupError != CHIP_ERROR_KEY_NOT_FOUND)
        {
            VerifyOrExit(size <= max_size, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
            err = SetPairedDeviceList(buffer);
            SuccessOrExit(err);
        }
    }

exit:
    if (buffer != nullptr)
    {
        chip::Platform::MemoryFree(buffer);
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to initialize the device list\n");
    }

    return err;
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

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
void DeviceController::OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    err = GetDevice(nodeData.mPeerId.GetNodeId(), &device);
    SuccessOrExit(err);

    err = device->UpdateAddress(Transport::PeerAddress::UDP(nodeData.mAddress, nodeData.mPort, nodeData.mInterfaceId));
    SuccessOrExit(err);

    PersistDevice(device);

exit:
    if (mDeviceAddressUpdateDelegate != nullptr)
    {
        mDeviceAddressUpdateDelegate->OnAddressUpdateComplete(nodeData.mPeerId.GetNodeId(), err);
    }
    return;
};

void DeviceController::OnNodeIdResolutionFailed(const chip::PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));

    if (mDeviceAddressUpdateDelegate != nullptr)
    {
        mDeviceAddressUpdateDelegate->OnAddressUpdateComplete(peer.GetNodeId(), error);
    }
};
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

ControllerDeviceInitParams DeviceController::GetControllerDeviceInitParams()
{
    return ControllerDeviceInitParams{
        .transportMgr = mTransportMgr, .sessionMgr = mSessionMgr, .exchangeMgr = mExchangeMgr, .inetLayer = mInetLayer
    };
}

DeviceCommissioner::DeviceCommissioner()
{
    mPairingDelegate      = nullptr;
    mDeviceBeingPaired    = kNumMaxActiveDevices;
    mPairedDevicesUpdated = false;
}

CHIP_ERROR DeviceCommissioner::Init(NodeId localDeviceId, CommissionerInitParams params)
{
    ReturnErrorOnFailure(DeviceController::Init(localDeviceId, params));

    uint16_t size = sizeof(mNextKeyId);
    if (!mStorageDelegate->SyncGetKeyValue(kNextAvailableKeyID, &mNextKeyId, size) || (size != sizeof(mNextKeyId)))
    {
        mNextKeyId = 0;
    }

    mPairingDelegate = params.pairingDelegate;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the commissioner");

    PersistDeviceList();

    FreeRendezvousSession();

    DeviceController::Shutdown();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & params)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    Device * device                    = nullptr;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    Messaging::ExchangeContext * exchangeCtxt = nullptr;

    Transport::AdminPairingInfo * admin = mAdmins.FindAdminWithId(mAdminId);

    VerifyOrExit(remoteDeviceId != kAnyNodeId && remoteDeviceId != kUndefinedNodeId, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingPaired == kNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(admin != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = InitializePairedDeviceList();
    SuccessOrExit(err);

    params.SetAdvertisementDelegate(&mRendezvousAdvDelegate);

    // TODO: We need to specify the peer address for BLE transport in bindings.
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle ||
        params.GetPeerAddress().GetTransportType() == Transport::Type::kUndefined)
    {
#if CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE
        if (!params.HasBleLayer())
        {
            params.SetPeerAddress(Transport::PeerAddress::BLE());
        }
        peerAddress = Transport::PeerAddress::BLE();
#endif // CONFIG_DEVICE_LAYER && CONFIG_NETWORK_LAYER_BLE
    }
    else if (params.GetPeerAddress().GetTransportType() == Transport::Type::kTcp ||
             params.GetPeerAddress().GetTransportType() == Transport::Type::kUdp)
    {
        peerAddress = Transport::PeerAddress::UDP(params.GetPeerAddress().GetIPAddress(), params.GetPeerAddress().GetPort(),
                                                  params.GetPeerAddress().GetInterface());
    }

    mDeviceBeingPaired = GetInactiveDeviceIndex();
    VerifyOrExit(mDeviceBeingPaired < kNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
    device = &mActiveDevices[mDeviceBeingPaired];

    mIsIPRendezvous = (params.GetPeerAddress().GetTransportType() != Transport::Type::kBle);

    err = mPairingSession.MessageDispatch().Init(mTransportMgr);
    SuccessOrExit(err);
    mPairingSession.MessageDispatch().SetPeerAddress(params.GetPeerAddress());

    device->Init(GetControllerDeviceInitParams(), mListenPort, remoteDeviceId, peerAddress, admin->GetAdminId());

    mSystemLayer->StartTimer(kSessionEstablishmentTimeout, OnSessionEstablishmentTimeoutCallback, this);
    if (params.GetPeerAddress().GetTransportType() != Transport::Type::kBle)
    {
        device->SetAddress(params.GetPeerAddress().GetIPAddress());
    }
#if CONFIG_NETWORK_LAYER_BLE
    else
    {
        if (params.HasConnectionObject())
        {
            SuccessOrExit(err = mBleLayer->NewBleConnectionByObject(params.GetConnectionObject()));
        }
        else if (params.HasDiscriminator())
        {
            SuccessOrExit(err = mBleLayer->NewBleConnectionByDiscriminator(params.GetDiscriminator()));
        }
        else
        {
            ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
#endif
    exchangeCtxt = mExchangeMgr->NewContext(SecureSessionHandle(), &mPairingSession);
    VerifyOrExit(exchangeCtxt != nullptr, err = CHIP_ERROR_INTERNAL);

    err = mPairingSession.Pair(params.GetPeerAddress(), params.GetSetupPINCode(), mNextKeyId++, exchangeCtxt, this);

exit:
    if (err != CHIP_NO_ERROR)
    {
        // Delete the current rendezvous session only if a device is not currently being paired.
        if (mDeviceBeingPaired == kNumMaxActiveDevices)
        {
            FreeRendezvousSession();
        }

        if (device != nullptr)
        {
            ReleaseDevice(device);
            mDeviceBeingPaired = kNumMaxActiveDevices;
        }
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::PairTestDeviceWithoutSecurity(NodeId remoteDeviceId, const Transport::PeerAddress & peerAddress,
                                                             SerializedDevice & serialized)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    SecurePairingUsingTestSecret * testSecurePairingSecret = nullptr;

    VerifyOrExit(peerAddress.GetTransportType() == Transport::Type::kUdp, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(remoteDeviceId != kUndefinedNodeId && remoteDeviceId != kAnyNodeId, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingPaired == kNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

    testSecurePairingSecret = chip::Platform::New<SecurePairingUsingTestSecret>();
    VerifyOrExit(testSecurePairingSecret != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mDeviceBeingPaired = GetInactiveDeviceIndex();
    VerifyOrExit(mDeviceBeingPaired < kNumMaxActiveDevices, err = CHIP_ERROR_NO_MEMORY);
    device = &mActiveDevices[mDeviceBeingPaired];

    testSecurePairingSecret->ToSerializable(device->GetPairing());

    device->Init(GetControllerDeviceInitParams(), mListenPort, remoteDeviceId, peerAddress, mAdminId);

    device->Serialize(serialized);

    OnSessionEstablished();

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
            mDeviceBeingPaired = kNumMaxActiveDevices;
        }
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::StopPairing(NodeId remoteDeviceId)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceBeingPaired < kNumMaxActiveDevices, CHIP_ERROR_INCORRECT_STATE);

    Device * device = &mActiveDevices[mDeviceBeingPaired];
    VerifyOrReturnError(device->GetDeviceId() == remoteDeviceId, CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    FreeRendezvousSession();

    ReleaseDevice(device);
    mDeviceBeingPaired = kNumMaxActiveDevices;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::UnpairDevice(NodeId remoteDeviceId)
{
    // TODO: Send unpairing message to the remote device.

    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    if (mDeviceBeingPaired < kNumMaxActiveDevices)
    {
        Device * device = &mActiveDevices[mDeviceBeingPaired];
        if (device->GetDeviceId() == remoteDeviceId)
        {
            FreeRendezvousSession();
        }
    }

    if (mStorageDelegate != nullptr)
    {
        PERSISTENT_KEY_OP(remoteDeviceId, kPairedDeviceKeyPrefix, key, mStorageDelegate->SyncDeleteKeyValue(key));
    }

    mPairedDevices.Remove(remoteDeviceId);
    mPairedDevicesUpdated = true;
    ReleaseDeviceById(remoteDeviceId);

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::FreeRendezvousSession()
{
    PersistNextKeyId();
}

void DeviceCommissioner::RendezvousCleanup(CHIP_ERROR status)
{
    mRendezvousAdvDelegate.StopAdvertisement();
    mRendezvousAdvDelegate.RendezvousComplete();

    FreeRendezvousSession();

    // TODO: make mStorageDelegate mandatory once all controller applications implement the interface.
    if (mDeviceBeingPaired != kNumMaxActiveDevices && mStorageDelegate != nullptr)
    {
        // Let's release the device that's being paired.
        // If pairing was successful, its information is
        // already persisted. The application will use GetDevice()
        // method to get access to the device, which will fetch
        // the device information from the persistent storage.
        DeviceController::ReleaseDevice(mDeviceBeingPaired);
    }

    mDeviceBeingPaired = kNumMaxActiveDevices;

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(status);
    }
}

void DeviceCommissioner::OnSessionEstablishmentError(CHIP_ERROR err)
{
    mSystemLayer->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingFailed);
    }

    RendezvousCleanup(err);
}

void DeviceCommissioner::OnSessionEstablished()
{
    VerifyOrReturn(mDeviceBeingPaired < kNumMaxActiveDevices, OnSessionEstablishmentError(CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR));

    Device * device = &mActiveDevices[mDeviceBeingPaired];

    mPairingSession.PeerConnection().SetPeerNodeId(device->GetDeviceId());

    CHIP_ERROR err =
        mSessionMgr->NewPairing(Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
                                mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession,
                                SecureSessionMgr::PairingDirection::kInitiator, mAdminId, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake\n");
    mPairingSession.ToSerializable(device->GetPairing());
    mSystemLayer->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

    mPairedDevices.Insert(device->GetDeviceId());
    mPairedDevicesUpdated = true;

    PersistDevice(device);

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
    }

    RendezvousCleanup(CHIP_NO_ERROR);
}

void DeviceCommissioner::PersistDeviceList()
{
    if (mStorageDelegate != nullptr && mPairedDevicesUpdated)
    {
        constexpr uint16_t size = CHIP_MAX_SERIALIZED_SIZE_U64(kNumMaxPairedDevices);
        char * serialized       = static_cast<char *>(chip::Platform::MemoryAlloc(size));
        uint16_t requiredSize   = size;
        if (serialized != nullptr)
        {
            const char * value = mPairedDevices.SerializeBase64(serialized, requiredSize);
            if (value != nullptr && requiredSize <= size)
            {
                // TODO: no need to base64 again the value
                PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key,
                                  mStorageDelegate->SyncSetKeyValue(key, value, static_cast<uint16_t>(strlen(value))));
                mPairedDevicesUpdated = false;
            }
            chip::Platform::MemoryFree(serialized);
        }
    }
}

void DeviceCommissioner::PersistNextKeyId()
{
    if (mStorageDelegate != nullptr)
    {
        mStorageDelegate->SyncSetKeyValue(kNextAvailableKeyID, &mNextKeyId, sizeof(mNextKeyId));
    }
}

void DeviceCommissioner::ReleaseDevice(Device * device)
{
    PersistDeviceList();
    DeviceController::ReleaseDevice(device);
}

#if CONFIG_NETWORK_LAYER_BLE
CHIP_ERROR DeviceCommissioner::CloseBleConnection()
{
    // It is fine since we can only commission one device at the same time.
    // We should be able to distinguish different BLE connections if we want
    // to commission multiple devices at the same time over BLE.
    return mBleLayer->CloseAllBleConnections();
}
#endif

void DeviceCommissioner::OnSessionEstablishmentTimeout()
{
    VerifyOrReturn(mState == State::Initialized);
    VerifyOrReturn(mDeviceBeingPaired < kNumMaxActiveDevices);

    Device * device = &mActiveDevices[mDeviceBeingPaired];
    StopPairing(device->GetDeviceId());

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(CHIP_ERROR_TIMEOUT);
    }
}

void DeviceCommissioner::OnSessionEstablishmentTimeoutCallback(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    reinterpret_cast<DeviceCommissioner *>(aAppState)->OnSessionEstablishmentTimeout();
}

} // namespace Controller
} // namespace chip
