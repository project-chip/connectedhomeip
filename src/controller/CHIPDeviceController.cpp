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

#include <app/common/gen/enums.h>
#include <controller/data_model/gen/CHIPClusters.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif

#include <app/InteractionModelEngine.h>
#include <app/util/DataModelHandler.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <credentials/CHIPCert.h>
#include <messaging/ExchangeContext.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/PersistentStorageMacros.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>
#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

#include <app/util/af-enums.h>

#include <errno.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Credentials;

// For some applications those does not implement IMDelegate, the DeviceControllerInteractionModelDelegate will dispatch the
// response to IMDefaultResponseCallback CHIPClientCallbacks, for the applications those implemented IMDelegate, this function will
// not be used.
bool __attribute__((weak)) IMDefaultResponseCallback(const chip::app::Command * commandObj, EmberAfStatus status)
{
    return false;
}

namespace chip {
namespace Controller {

using namespace chip::Encoding;

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
constexpr uint16_t kMdnsPort = 5353;
#endif

constexpr uint32_t kSessionEstablishmentTimeout = 30 * kMillisecondPerSecond;

constexpr uint32_t kMaxCHIPOpCertLength = 1024;
constexpr uint32_t kMaxCHIPCSRLength    = 1024;
constexpr uint32_t kOpCSRNonceLength    = 32;

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

    mTransportMgr          = chip::Platform::New<DeviceTransportMgr>();
    mSessionMgr            = chip::Platform::New<SecureSessionMgr>();
    mExchangeMgr           = chip::Platform::New<Messaging::ExchangeManager>();
    mMessageCounterManager = chip::Platform::New<secure_channel::MessageCounterManager>();

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

    err = mAdmins.Init(mStorageDelegate);
    SuccessOrExit(err);

    admin = mAdmins.AssignAdminId(mAdminId, localDeviceId);
    VerifyOrExit(admin != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = mAdmins.LoadFromStorage(mAdminId);
    SuccessOrExit(err);

    err = mSessionMgr->Init(localDeviceId, mSystemLayer, mTransportMgr, &mAdmins, mMessageCounterManager);
    SuccessOrExit(err);

    err = mExchangeMgr->Init(mSessionMgr);
    SuccessOrExit(err);

    err = mMessageCounterManager->Init(mExchangeMgr);
    SuccessOrExit(err);

    err = mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::TempZCL::Id, this);
    SuccessOrExit(err);

    if (params.imDelegate != nullptr)
    {
        err = chip::app::InteractionModelEngine::GetInstance()->Init(mExchangeMgr, params.imDelegate);
    }
    else
    {
        mDefaultIMDelegate = chip::Platform::New<DeviceControllerInteractionModelDelegate>();
        err                = chip::app::InteractionModelEngine::GetInstance()->Init(mExchangeMgr, mDefaultIMDelegate);
    }
    SuccessOrExit(err);

    mExchangeMgr->SetDelegate(this);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    if (params.mDeviceAddressUpdateDelegate != nullptr)
    {
        err = Mdns::Resolver::Instance().SetResolverDelegate(this);
        SuccessOrExit(err);

        mDeviceAddressUpdateDelegate = params.mDeviceAddressUpdateDelegate;
    }
    Mdns::Resolver::Instance().StartResolver(mInetLayer, kMdnsPort);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

    InitDataModelHandler(mExchangeMgr);

    mState         = State::Initialized;
    mLocalDeviceId = localDeviceId;

    VerifyOrExit(params.operationalCredentialsDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mOperationalCredentialsDelegate = params.operationalCredentialsDelegate;

    err = LoadLocalCredentials(admin);
    SuccessOrExit(err);

    ReleaseAllDevices();

exit:
    return err;
}

CHIP_ERROR DeviceController::LoadLocalCredentials(Transport::AdminPairingInfo * admin)
{
    ChipLogProgress(Controller, "Getting operational keys");
    Crypto::P256Keypair * keypair = admin->GetOperationalKey();

    ReturnErrorCodeIf(keypair == nullptr, CHIP_ERROR_NO_MEMORY);

    if (!admin->AreCredentialsAvailable())
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer1;
        ReturnErrorCodeIf(!buffer1.Alloc(kMaxCHIPCSRLength), CHIP_ERROR_NO_MEMORY);

        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer2;
        ReturnErrorCodeIf(!buffer2.Alloc(kMaxCHIPOpCertLength), CHIP_ERROR_NO_MEMORY);

        uint8_t * CSR    = buffer1.Get();
        size_t csrLength = kMaxCHIPCSRLength;
        ReturnErrorOnFailure(keypair->NewCertificateSigningRequest(CSR, csrLength));

        uint8_t * cert   = buffer2.Get();
        uint32_t certLen = 0;

        // TODO - Match the generated cert against CSR and operational keypair
        //        Make sure it chains back to the trusted root.
        ChipLogProgress(Controller, "Generating operational certificate for the controller");
        ReturnErrorOnFailure(mOperationalCredentialsDelegate->GenerateNodeOperationalCertificate(
            PeerId().SetNodeId(mLocalDeviceId), ByteSpan(CSR, csrLength), 1, cert, kMaxCHIPOpCertLength, certLen));

        uint8_t * chipCert   = buffer1.Get();
        uint32_t chipCertLen = 0;
        ReturnErrorOnFailure(ConvertX509CertToChipCert(cert, certLen, chipCert, kMaxCHIPOpCertLength, chipCertLen));

        ReturnErrorOnFailure(admin->SetOperationalCert(ByteSpan(chipCert, chipCertLen)));

        ChipLogProgress(Controller, "Getting root certificate for the controller from the issuer");
        ReturnErrorOnFailure(mOperationalCredentialsDelegate->GetRootCACertificate(0, cert, kMaxCHIPOpCertLength, certLen));

        chipCertLen = 0;
        ReturnErrorOnFailure(ConvertX509CertToChipCert(cert, certLen, chipCert, kMaxCHIPOpCertLength, chipCertLen));

        ReturnErrorOnFailure(admin->SetRootCert(ByteSpan(chipCert, chipCertLen)));

        ReturnErrorOnFailure(mAdmins.Store(admin->GetAdminId()));
    }

    ChipLogProgress(Controller, "Generating credentials");
    ReturnErrorOnFailure(admin->GetCredentials(mCredentials, mCertificates, mRootKeyId));

    ChipLogProgress(Controller, "Loaded credentials successfully");
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the controller");

    mState = State::NotInitialized;

    // TODO(#6668): Some exchange has leak, shutting down ExchangeManager will cause a assert fail.
    // if (mExchangeMgr != nullptr)
    // {
    //     mExchangeMgr->Shutdown();
    // }
    if (mSessionMgr != nullptr)
    {
        mSessionMgr->Shutdown();
    }

#if CONFIG_DEVICE_LAYER
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().Shutdown());
#else
    mInetLayer->Shutdown();
    mSystemLayer->Shutdown();
    chip::Platform::Delete(mInetLayer);
    chip::Platform::Delete(mSystemLayer);
#endif // CONFIG_DEVICE_LAYER

    mSystemLayer     = nullptr;
    mInetLayer       = nullptr;
    mStorageDelegate = nullptr;

    ReleaseAllDevices();

    if (mMessageCounterManager != nullptr)
    {
        chip::Platform::Delete(mMessageCounterManager);
        mMessageCounterManager = nullptr;
    }

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

    if (mDefaultIMDelegate != nullptr)
    {
        chip::Platform::Delete(mDefaultIMDelegate);
        mDefaultIMDelegate = nullptr;
    }

    mAdmins.ReleaseAdminId(mAdminId);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    Mdns::Resolver::Instance().SetResolverDelegate(nullptr);
    mDeviceAddressUpdateDelegate = nullptr;
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

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
    // TODO - Detect when the device is fully provisioned, instead of relying on UpdateDevice()
    device->ProvisioningComplete(mNextKeyId++);
    PersistDevice(device);
    PersistNextKeyId();
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    return Mdns::Resolver::Instance().ResolveNodeId(chip::PeerId().SetNodeId(device->GetDeviceId()).SetFabricId(fabricId),
                                                    chip::Inet::kIPAddressType_Any);
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS
}

void DeviceController::PersistDevice(Device * device)
{
    if (mState == State::Initialized)
    {
        device->Persist();
    }
    else
    {
        ChipLogError(Controller, "Failed to persist device. Controller not initialized.");
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
                                         const PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf)
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
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t * buffer = nullptr;

    VerifyOrExit(mStorageDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (!mPairedDevicesInitialized)
    {
        constexpr uint16_t max_size = sizeof(uint64_t) * kNumMaxPairedDevices;
        buffer                      = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(max_size, 1));
        uint16_t size               = max_size;

        VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        CHIP_ERROR lookupError = CHIP_NO_ERROR;
        PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key,
                          lookupError = mStorageDelegate->SyncGetKeyValue(key, buffer, size));

        // It's ok to not have an entry for the Paired Device list. We treat it the same as having an empty list.
        if (lookupError != CHIP_ERROR_KEY_NOT_FOUND)
        {
            VerifyOrExit(size <= max_size, err = CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
            err = SetPairedDeviceList(ByteSpan(buffer, size));
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
        ChipLogError(Controller, "Failed to initialize the device list with error: %d\n", err);
    }

    return err;
}

CHIP_ERROR DeviceController::SetPairedDeviceList(ByteSpan serialized)
{
    CHIP_ERROR err = mPairedDevices.Deserialize(serialized);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to recreate the device list with buffer %s\n", serialized);
    }
    else
    {
        mPairedDevicesInitialized = true;
    }

    return err;
}

void DeviceController::PersistNextKeyId()
{
    if (mStorageDelegate != nullptr && mState == State::Initialized)
    {
        mStorageDelegate->SyncSetKeyValue(kNextAvailableKeyID, &mNextKeyId, sizeof(mNextKeyId));
    }
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

void DeviceController::OnCommissionableNodeFound(const chip::Mdns::CommissionableNodeData & nodeData)
{
    for (int i = 0; i < kMaxCommissionableNodes; ++i)
    {
        if (!mCommissionableNodes[i].IsValid())
        {
            continue;
        }
        if (strcmp(mCommissionableNodes[i].hostName, nodeData.hostName) == 0)
        {
            mCommissionableNodes[i] = nodeData;
            return;
        }
    }
    // Didn't find the host name already in our list, return an invalid
    for (int i = 0; i < kMaxCommissionableNodes; ++i)
    {
        if (!mCommissionableNodes[i].IsValid())
        {
            mCommissionableNodes[i] = nodeData;
            return;
        }
    }
    ChipLogError(Discovery, "Failed to add discovered commissionable node with hostname %s- Insufficient space", nodeData.hostName);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

ControllerDeviceInitParams DeviceController::GetControllerDeviceInitParams()
{
    return ControllerDeviceInitParams{
        .transportMgr    = mTransportMgr,
        .sessionMgr      = mSessionMgr,
        .exchangeMgr     = mExchangeMgr,
        .inetLayer       = mInetLayer,
        .storageDelegate = mStorageDelegate,
        .credentials     = &mCredentials,
    };
}

DeviceCommissioner::DeviceCommissioner() :
    mSuccess(BasicSuccess, this), mFailure(BasicFailure, this),
    mOpCSRResponseCallback(OnOperationalCertificateSigningRequest, this),
    mOpCertResponseCallback(OnOperationalCertificateAddResponse, this), mRootCertResponseCallback(OnRootCertSuccessResponse, this),
    mOnCSRFailureCallback(OnCSRFailureResponse, this), mOnCertFailureCallback(OnAddOpCertFailureResponse, this),
    mOnRootCertFailureCallback(OnRootCertFailureResponse, this)
{
    mPairingDelegate      = nullptr;
    mDeviceBeingPaired    = kNumMaxActiveDevices;
    mPairedDevicesUpdated = false;
}

CHIP_ERROR DeviceCommissioner::Init(NodeId localDeviceId, CommissionerInitParams params)
{
    ReturnErrorOnFailure(DeviceController::Init(localDeviceId, params));

    uint16_t size    = sizeof(mNextKeyId);
    CHIP_ERROR error = mStorageDelegate->SyncGetKeyValue(kNextAvailableKeyID, &mNextKeyId, size);
    if (error || (size != sizeof(mNextKeyId)))
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

    mPairingSession.Clear();

    PersistDeviceList();

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
#if CONFIG_NETWORK_LAYER_BLE
        if (!params.HasBleLayer())
        {
            params.SetPeerAddress(Transport::PeerAddress::BLE());
        }
        peerAddress = Transport::PeerAddress::BLE();
#endif // CONFIG_NETWORK_LAYER_BLE
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
    // Immediately persist the updted mNextKeyID value
    // TODO maybe remove FreeRendezvousSession() since mNextKeyID is always persisted immediately
    PersistNextKeyId();

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

    // Check that the caller has provided an IP address (instead of a BLE peer address)
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

    err = mSessionMgr->NewPairing(Optional<Transport::PeerAddress>::Value(peerAddress), device->GetDeviceId(),
                                  testSecurePairingSecret, SecureSession::SessionRole::kInitiator, mAdminId, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
    }
    SuccessOrExit(err);

    mPairedDevices.Insert(device->GetDeviceId());
    mPairedDevicesUpdated = true;

    // Note - This assumes storage is synchronous, the device must be in storage before we can cleanup
    // the rendezvous session and mark pairing success
    PersistDevice(device);
    // Also persist the device list at this time
    // This makes sure that a newly added device is immediately available
    PersistDeviceList();

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
    }

    RendezvousCleanup(CHIP_NO_ERROR);

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
                                SecureSession::SessionRole::kInitiator, mAdminId, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake\n");

    // TODO: Add code to receive OpCSR from the device, and process the signing request
    // For IP rendezvous, this is sent as part of the state machine.
#if CONFIG_USE_CLUSTERS_FOR_IP_COMMISSIONING
    bool sendOperationalCertsImmediately = !mIsIPRendezvous;
#else
    bool sendOperationalCertsImmediately = true;
#endif

    if (sendOperationalCertsImmediately)
    {
        err = SendOperationalCertificateSigningRequestCommand(device);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Ble, "Failed in sending opcsr request command to the device: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
            return;
        }
    }
    else
    {
        AdvanceCommissioningStage(CHIP_NO_ERROR);
    }
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificateSigningRequestCommand(Device * device)
{
    ChipLogDetail(Controller, "Sending OpCSR request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mOpCSRResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnCSRFailureCallback.Cancel();

    uint8_t CSRNonce[kOpCSRNonceLength];
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(CSRNonce, sizeof(CSRNonce)));

    ReturnErrorOnFailure(cluster.OpCSRRequest(successCallback, failureCallback, ByteSpan(CSRNonce, sizeof(CSRNonce))));
    ChipLogDetail(Controller, "Sent OpCSR request, waiting for the CSR");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCSRFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the CSR request Response: 0x%02x", status);
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCSRFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnOperationalCertificateSigningRequest(void * context, ByteSpan CSR, ByteSpan CSRNonce,
                                                                ByteSpan VendorReserved1, ByteSpan VendorReserved2,
                                                                ByteSpan VendorReserved3, ByteSpan Signature)
{
    ChipLogProgress(Controller, "Received certificate signing request from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCSRFailureCallback.Cancel();

    if (commissioner->ProcessOpCSR(CSR, CSRNonce, VendorReserved1, VendorReserved2, VendorReserved3, Signature) != CHIP_NO_ERROR)
    {
        // Handle error, and notify session failure to the commissioner application.
        ChipLogError(Controller, "Failed to process the certificate signing request");
        // TODO: Map error status to correct error code
        commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
    }
}

CHIP_ERROR DeviceCommissioner::ProcessOpCSR(const ByteSpan & CSR, const ByteSpan & CSRNonce, const ByteSpan & VendorReserved1,
                                            const ByteSpan & VendorReserved2, const ByteSpan & VendorReserved3,
                                            const ByteSpan & Signature)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceBeingPaired < kNumMaxActiveDevices, CHIP_ERROR_INCORRECT_STATE);

    Device * device = &mActiveDevices[mDeviceBeingPaired];

    chip::Platform::ScopedMemoryBuffer<uint8_t> opCert;
    ReturnErrorCodeIf(!opCert.Alloc(kMaxCHIPOpCertLength), CHIP_ERROR_NO_MEMORY);

    // TODO: Validate CSR Nonce and signature

    uint32_t opCertLen = 0;
    ChipLogProgress(Controller, "Generating operational certificate for device %llx", device->GetDeviceId());
    ReturnErrorOnFailure(mOperationalCredentialsDelegate->GenerateNodeOperationalCertificate(
        PeerId().SetNodeId(device->GetDeviceId()), CSR, 1, opCert.Get(), kMaxCHIPOpCertLength, opCertLen));

    chip::Platform::ScopedMemoryBuffer<uint8_t> issuerCert;
    ReturnErrorCodeIf(!issuerCert.Alloc(kMaxCHIPOpCertLength), CHIP_ERROR_NO_MEMORY);

    ChipLogProgress(Controller, "Getting intermediate CA certificate from the issuer");
    uint32_t issuerCertLen = 0;
    CHIP_ERROR err =
        mOperationalCredentialsDelegate->GetIntermediateCACertificate(0, issuerCert.Get(), kMaxCHIPOpCertLength, issuerCertLen);
    ChipLogProgress(Controller, "GetIntermediateCACertificate returned %d", err);
    if (err == CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED)
    {
        // This implies that the commissioner application uses root CA to sign the operational
        // certificates, and an intermediate CA is not needed. It's not an error condition, so
        // let's just send operational certificate and root CA certificate to the device.
        err           = CHIP_NO_ERROR;
        issuerCertLen = 0;
        ChipLogProgress(Controller, "Intermediate CA is not needed");
    }
    ReturnErrorOnFailure(err);

    chip::Platform::ScopedMemoryBuffer<uint8_t> chipCert;

    ReturnErrorCodeIf(!chipCert.Alloc(kMaxCHIPOpCertLength), CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(ConvertX509CertToChipCert(opCert.Get(), opCertLen, chipCert.Get(), kMaxCHIPOpCertLength, opCertLen));

    // TODO - convert ICA cert to ChipCert format and send it to the device.

    ChipLogProgress(Controller, "Sending operational certificate to the device. Op Cert Len %d, ICA Cert Len %d", opCertLen,
                    issuerCertLen);
    ReturnErrorOnFailure(
        SendOperationalCertificate(device, ByteSpan(chipCert.Get(), opCertLen), ByteSpan(issuerCert.Get(), issuerCertLen)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificate(Device * device, const ByteSpan & opCertBuf, const ByteSpan & icaCertBuf)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mOpCertResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnCertFailureCallback.Cancel();

    ReturnErrorOnFailure(
        cluster.AddOpCert(successCallback, failureCallback, opCertBuf, icaCertBuf, ByteSpan(nullptr, 0), mLocalDeviceId, 0));

    ChipLogProgress(Controller, "Sent operational certificate to the device");

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnAddOpCertFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the operational certificate Response: 0x%02x", status);
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCertFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnOperationalCertificateAddResponse(void * context, uint8_t StatusCode, uint64_t FabricIndex,
                                                             uint8_t * DebugText)
{
    ChipLogProgress(Controller, "Device confirmed that it has received the operational certificate");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCertFailureCallback.Cancel();

    VerifyOrExit(commissioner->mDeviceBeingPaired < kNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

    device = &commissioner->mActiveDevices[commissioner->mDeviceBeingPaired];

    err = commissioner->SendTrustedRootCertificate(device);

exit:
    if (err != CHIP_NO_ERROR)
    {
        commissioner->OnSessionEstablishmentError(err);
    }
}

CHIP_ERROR DeviceCommissioner::SendTrustedRootCertificate(Device * device)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Transport::AdminPairingInfo * admin = mAdmins.FindAdminWithId(mAdminId);
    VerifyOrReturnError(admin != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t rootCertLen     = 0;
    const uint8_t * rootCert = admin->GetTrustedRoot(rootCertLen);
    VerifyOrReturnError(rootCert != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "Sending root certificate to the device");

    chip::Controller::TrustedRootCertificatesCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mRootCertResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnRootCertFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.AddTrustedRootCertificate(successCallback, failureCallback, ByteSpan(rootCert, rootCertLen)));

    ChipLogProgress(Controller, "Sent root certificate to the device");

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnRootCertSuccessResponse(void * context)
{
    ChipLogProgress(Controller, "Device confirmed that it has received the root certificate");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err  = CHIP_NO_ERROR;
    Device * device = nullptr;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    commissioner->mRootCertResponseCallback.Cancel();
    commissioner->mOnRootCertFailureCallback.Cancel();

    VerifyOrExit(commissioner->mDeviceBeingPaired < kNumMaxActiveDevices, err = CHIP_ERROR_INCORRECT_STATE);

    device = &commissioner->mActiveDevices[commissioner->mDeviceBeingPaired];

    err = commissioner->OnOperationalCredentialsProvisioningCompletion(device);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        commissioner->OnSessionEstablishmentError(err);
    }
}

void DeviceCommissioner::OnRootCertFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the root certificate Response: 0x%02x", status);
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->mRootCertResponseCallback.Cancel();
    commissioner->mOnRootCertFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

CHIP_ERROR DeviceCommissioner::OnOperationalCredentialsProvisioningCompletion(Device * device)
{
    ChipLogProgress(Controller, "Operational credentials provisioned on device %p", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_USE_CLUSTERS_FOR_IP_COMMISSIONING
    if (mIsIPRendezvous)
    {
        AdvanceCommissioningStage(CHIP_NO_ERROR);
    }
    else
#endif
    {
        mPairingSession.ToSerializable(device->GetPairing());
        mSystemLayer->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

        mPairedDevices.Insert(device->GetDeviceId());
        mPairedDevicesUpdated = true;

        // Note - This assumes storage is synchronous, the device must be in storage before we can cleanup
        // the rendezvous session and mark pairing success
        PersistDevice(device);
        // Also persist the device list at this time
        // This makes sure that a newly added device is immediately available
        PersistDeviceList();
        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
        }
        RendezvousCleanup(CHIP_NO_ERROR);
    }

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::PersistDeviceList()
{
    if (mStorageDelegate != nullptr && mPairedDevicesUpdated && mState == State::Initialized)
    {
        mPairedDevices.Serialize([&](ByteSpan data) -> CHIP_ERROR {
            VerifyOrReturnError(data.size() <= UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);
            PERSISTENT_KEY_OP(static_cast<uint64_t>(0), kPairedDeviceListKeyPrefix, key,
                              mStorageDelegate->SyncSetKeyValue(key, data.data(), static_cast<uint16_t>(data.size())));
            mPairedDevicesUpdated = false;
            return CHIP_NO_ERROR;
        });
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
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
CHIP_ERROR DeviceCommissioner::DiscoverAllCommissioning()
{
    for (int i = 0; i < kMaxCommissionableNodes; ++i)
    {
        mCommissionableNodes[i].Reset();
    }
    return Mdns::Resolver::Instance().FindCommissionableNodes();
}

CHIP_ERROR DeviceCommissioner::DiscoverCommissioningLongDiscriminator(uint16_t long_discriminator)
{
    // TODO(cecille): Add assertion about main loop.
    for (int i = 0; i < kMaxCommissionableNodes; ++i)
    {
        mCommissionableNodes[i].Reset();
    }
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kLong, long_discriminator);
    return Mdns::Resolver::Instance().FindCommissionableNodes(filter);
}

const Mdns::CommissionableNodeData * DeviceCommissioner::GetDiscoveredDevice(int idx)
{
    // TODO(cecille): Add assertion about main loop.
    if (mCommissionableNodes[idx].IsValid())
    {
        return &mCommissionableNodes[idx];
    }
    return nullptr;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

CHIP_ERROR DeviceControllerInteractionModelDelegate::CommandResponseStatus(
    const app::CommandSender * apCommandSender, const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
    const uint32_t aProtocolId, const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
    chip::CommandId aCommandId, uint8_t aCommandIndex)
{
    // #6308, #6559: Invoking success Callbacks in `CommandResponseProcessed` is not desired, but this is used to met current
    // requirement of current callback framework that we should be able to send another command once ResponseCallback is called. By
    // resolving #6308, the app can wait for the right event, and by #6559 the app can send command in a now command sender.
    VerifyOrReturnError(aProtocolCode != 0, CHIP_NO_ERROR);

    // Generally IM has more detailed errors than ember library, here we always use the, the actual handling of the
    // commands should implement full IMDelegate.
    // #6308 By implement app side IM delegate, we should be able to accept detailed error codes.
    // Note: The IMDefaultResponseCallback is a bridge to the old CallbackMgr before IM is landed, so it still accepts EmberAfStatus
    // instead of IM status code.
    IMDefaultResponseCallback(apCommandSender, EMBER_ZCL_STATUS_FAILURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControllerInteractionModelDelegate::CommandResponseProtocolError(const app::CommandSender * apCommandSender,
                                                                                  uint8_t aCommandIndex)
{
    // Generally IM has more detailed errors than ember library, here we always use EMBER_ZCL_STATUS_FAILURE before #6308 is landed
    // and the app can take care of these error codes, the actual handling of the commands should implement full IMDelegate.
    // #6308: By implement app side IM delegate, we should be able to accept detailed error codes.
    // Note: The IMDefaultResponseCallback is a bridge to the old CallbackMgr before IM is landed, so it still accepts EmberAfStatus
    // instead of IM status code.
    IMDefaultResponseCallback(apCommandSender, EMBER_ZCL_STATUS_FAILURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControllerInteractionModelDelegate::CommandResponseError(const app::CommandSender * apCommandSender,
                                                                          CHIP_ERROR aError)
{
    // Generally IM has more detailed errors than ember library, here we always use EMBER_ZCL_STATUS_FAILURE before #6308 is landed
    // and the app can take care of these error codes, the actual handling of the commands should implement full IMDelegate.
    // #6308: By implement app side IM delegate, we should be able to accept detailed error codes.
    // Note: The IMDefaultResponseCallback is a bridge to the old CallbackMgr before IM is landed, so it still accepts EmberAfStatus
    // instead of IM status code.
    IMDefaultResponseCallback(apCommandSender, EMBER_ZCL_STATUS_FAILURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceControllerInteractionModelDelegate::CommandResponseProcessed(const app::CommandSender * apCommandSender)
{
    // #6308, #6559: Invoking Callbacks in `CommandResponseProcessed` is not desired, but this is used to met current requirement of
    // current callback framework that we should be able to send another command once ResponseCallback is called.
    // By resolving #6308, the app can wait for the right event, and by #6559 the app can send command in a now command sender.
    IMDefaultResponseCallback(apCommandSender, EMBER_ZCL_STATUS_SUCCESS);

    return CHIP_NO_ERROR;
}
void BasicSuccess(void * context, uint16_t val)
{
    ChipLogProgress(Controller, "Received success response 0x%x\n", val);
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->AdvanceCommissioningStage(CHIP_NO_ERROR);
}

void BasicFailure(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Received failure response %d\n", (int) status);
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->OnSessionEstablishmentError(static_cast<CHIP_ERROR>(status));
}

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
void DeviceCommissioner::OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData)
{
    if (mDeviceBeingPaired < kNumMaxActiveDevices)
    {
        Device * device = &mActiveDevices[mDeviceBeingPaired];
        if (device->GetDeviceId() == nodeData.mPeerId.GetNodeId() && mCommissioningStage == CommissioningStage::kFindOperational)
        {
            AdvanceCommissioningStage(CHIP_NO_ERROR);
        }
    }
    DeviceController::OnNodeIdResolved(nodeData);
}

void DeviceCommissioner::OnNodeIdResolutionFailed(const chip::PeerId & peer, CHIP_ERROR error)
{
    if (mDeviceBeingPaired < kNumMaxActiveDevices)
    {
        Device * device = &mActiveDevices[mDeviceBeingPaired];
        if (device->GetDeviceId() == peer.GetNodeId() && mCommissioningStage == CommissioningStage::kFindOperational)
        {
            OnSessionEstablishmentError(error);
        }
    }
    DeviceController::OnNodeIdResolutionFailed(peer, error);
}

#endif

CommissioningStage DeviceCommissioner::GetNextCommissioningStage()
{
    switch (mCommissioningStage)
    {
    case CommissioningStage::kSecurePairing:
        return CommissioningStage::kArmFailsafe;
    case CommissioningStage::kArmFailsafe:
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kConfigRegulatory:
        return CommissioningStage::kCheckCertificates;
    case CommissioningStage::kCheckCertificates:
        return CommissioningStage::kNetworkEnable; // TODO : for softAP, this needs to be network setup
    case CommissioningStage::kNetworkEnable:
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
        return CommissioningStage::kFindOperational; // TODO : once case is working, need to add stages to find and reconnect
                                                     // here.
#else
        return CommissioningStage::kSendComplete;
#endif
    case CommissioningStage::kFindOperational:
        return CommissioningStage::kSendComplete;
    case CommissioningStage::kSendComplete:
        return CommissioningStage::kCleanup;

    // Currently unimplemented.
    case CommissioningStage::kConfigACL:
    case CommissioningStage::kNetworkSetup:
    case CommissioningStage::kScanNetworks:
        return CommissioningStage::kError;
    // Neither of these have a next stage so return kError;
    case CommissioningStage::kCleanup:
    case CommissioningStage::kError:
        return CommissioningStage::kError;
    }
    return CommissioningStage::kError;
}

void DeviceCommissioner::AdvanceCommissioningStage(CHIP_ERROR err)
{
    // For now, we ignore errors coming in from the device since not all commissioning clusters are implemented on the device
    // side.
    CommissioningStage nextStage = GetNextCommissioningStage();
    if (nextStage == CommissioningStage::kError)
    {
        return;
    }

    if (!mIsIPRendezvous)
    {
        return;
    }
    Device * device = nullptr;
    if (mDeviceBeingPaired >= kNumMaxActiveDevices)
    {
        return;
    }

    device = &mActiveDevices[mDeviceBeingPaired];

    // TODO(cecille): We probably want something better than this for breadcrumbs.
    uint64_t breadcrumb = static_cast<uint64_t>(nextStage);

    // TODO(cecille): This should be customized per command.
    constexpr uint32_t kCommandTimeoutMs = 3000;

    switch (nextStage)
    {
    case CommissioningStage::kArmFailsafe: {
        // TODO(cecille): This is NOT the right way to do this - we should consider attaching an im delegate per command or
        // something. Per exchange context?
        ChipLogProgress(Controller, "Arming failsafe");
        // TODO(cecille): Find a way to enumerate the clusters here.
        GeneralCommissioningCluster genCom;
        uint16_t commissioningExpirySeconds = 5;
        // TODO: should get the endpoint information from the descriptor cluster.
        genCom.Associate(device, 0);
        genCom.ArmFailSafe(mSuccess.Cancel(), mFailure.Cancel(), commissioningExpirySeconds, breadcrumb, kCommandTimeoutMs);
    }
    break;
    case CommissioningStage::kConfigRegulatory: {
        // To set during config phase:
        // UTC time
        // time zone
        // dst offset
        // Regulatory config
        // TODO(cecille): Set time as well once the time cluster is implemented
        // TODO(cecille): Worthwhile to keep this around as part of the class?
        // TODO(cecille): Where is the country config actually set?
        ChipLogProgress(Controller, "Setting Regulatory Config");
        uint32_t regulatoryLocation = EMBER_ZCL_REGULATORY_LOCATION_TYPE_OUTDOOR;
#if CONFIG_DEVICE_LAYER
        CHIP_ERROR status = DeviceLayer::ConfigurationMgr().GetRegulatoryLocation(regulatoryLocation);
#else
        CHIP_ERROR status = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to find regulatory location, defaulting to outdoor");
        }

        static constexpr size_t kMaxCountryCodeSize = 3;
        char countryCodeStr[kMaxCountryCodeSize]    = "WW";
        size_t actualCountryCodeSize                = 2;

#if CONFIG_DEVICE_LAYER
        status = DeviceLayer::ConfigurationMgr().GetCountryCode(countryCodeStr, kMaxCountryCodeSize, actualCountryCodeSize);
#else
        status            = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to find country code, defaulting to WW");
        }
        chip::ByteSpan countryCode(reinterpret_cast<uint8_t *>(countryCodeStr), actualCountryCodeSize);

        GeneralCommissioningCluster genCom;
        genCom.Associate(device, 0);
        genCom.SetRegulatoryConfig(mSuccess.Cancel(), mFailure.Cancel(), static_cast<uint8_t>(regulatoryLocation), countryCode,
                                   breadcrumb, kCommandTimeoutMs);
    }
    break;
    case CommissioningStage::kCheckCertificates: {
        ChipLogProgress(Controller, "Exchanging certificates");
        // TODO(cecille): Once this is implemented through the clusters, it should be moved to the proper stage and the callback
        // should advance the commissioning stage
        CHIP_ERROR status = SendOperationalCertificateSigningRequestCommand(device);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in sending opcsr request command to the device: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
            return;
        }
    }
    break;
    // TODO: Right now, these stages are not implemented as a separate stage because they are no-ops.
    // Once these are implemented through the clusters, these should be moved into their separate stages and the callbacks
    // should advance the commissioning stage.
    case CommissioningStage::kConfigACL:
    case CommissioningStage::kNetworkSetup:
    case CommissioningStage::kScanNetworks:
        // TODO: Implement
        break;
    case CommissioningStage::kNetworkEnable: {
        ChipLogProgress(Controller, "Enabling Network");
        // TODO: For ethernet, we actually need a scan stage to get the ethernet netif name. Right now, default to using a magic
        // value to enable without checks.
        NetworkCommissioningCluster netCom;
        // TODO: should get the endpoint information from the descriptor cluster.
        netCom.Associate(device, 0);
        // TODO: Once network credential sending is implemented, attempting to set wifi credential on an ethernet only device
        // will cause an error to be sent back. At that point, we should scan and we shoud see the proper ethernet network ID
        // returned in the scan results. For now, we use magic.
        char magicNetworkEnableCode[] = "ETH0";
        netCom.EnableNetwork(mSuccess.Cancel(), mFailure.Cancel(),
                             ByteSpan(reinterpret_cast<uint8_t *>(&magicNetworkEnableCode), sizeof(magicNetworkEnableCode)),
                             breadcrumb, kCommandTimeoutMs);
    }
    break;
    case CommissioningStage::kFindOperational: {
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
        ChipLogProgress(Controller, "Finding node on operational network");
        Mdns::Resolver::Instance().ResolveNodeId(PeerId().SetFabricId(0).SetNodeId(device->GetDeviceId()),
                                                 Inet::IPAddressType::kIPAddressType_Any);
#endif
    }
    break;
    case CommissioningStage::kSendComplete: {
        // TODO this is actualy not correct - we must reconnect over CASE to send this command.
        ChipLogProgress(Controller, "Calling commissioning complete");
        GeneralCommissioningCluster genCom;
        genCom.Associate(device, 0);
        genCom.CommissioningComplete(mSuccess.Cancel(), mFailure.Cancel());
    }
    break;
    case CommissioningStage::kCleanup:
        ChipLogProgress(Controller, "Rendezvous cleanup");
        mPairingSession.ToSerializable(device->GetPairing());
        mSystemLayer->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

        mPairedDevices.Insert(device->GetDeviceId());
        mPairedDevicesUpdated = true;

        // Note - This assumes storage is synchronous, the device must be in storage before we can cleanup
        // the rendezvous session and mark pairing success
        PersistDevice(device);
        // Also persist the device list at this time
        // This makes sure that a newly added device is immediately available
        PersistDeviceList();
        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
        }
        RendezvousCleanup(CHIP_NO_ERROR);
        break;
    case CommissioningStage::kSecurePairing:
    case CommissioningStage::kError:
        break;
    }
    mCommissioningStage = nextStage;
}

} // namespace Controller
} // namespace chip

namespace chip {
namespace Platform {
namespace PersistedStorage {

/*
 * Dummy implementations of PersistedStorage platform methods. These aren't
 * used in the context of the Device Controller, but are required to satisfy
 * the linker.
 */

CHIP_ERROR Read(const char * aKey, uint32_t & aValue)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Write(const char * aKey, uint32_t aValue)
{
    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
