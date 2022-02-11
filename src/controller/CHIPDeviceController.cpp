/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <controller-clusters/zap-generated/CHIPClusters.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif

#include <app/InteractionModelEngine.h>
#include <app/OperationalDeviceProxy.h>
#include <app/util/error-mapping.h>
#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/NodeId.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

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
using namespace chip::Transport;
using namespace chip::Credentials;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

using namespace chip::Encoding;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
using namespace chip::Protocols::UserDirectedCommissioning;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

constexpr uint32_t kSessionEstablishmentTimeout = 40 * kMillisecondsPerSecond;

DeviceController::DeviceController()
{
    mState                    = State::NotInitialized;
    mStorageDelegate          = nullptr;
    mPairedDevicesInitialized = false;
}

CHIP_ERROR DeviceController::Init(ControllerInitParams params)
{
    VerifyOrReturnError(mState == State::NotInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(params.systemState != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(params.systemState->SystemLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.systemState->UDPEndPointManager() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStorageDelegate = params.storageDelegate;
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(params.systemState->BleLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#endif

    VerifyOrReturnError(params.systemState->TransportMgr() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    ReturnErrorOnFailure(mDNSResolver.Init(params.systemState->UDPEndPointManager()));
    mDNSResolver.SetResolverDelegate(this);
    RegisterDeviceAddressUpdateDelegate(params.deviceAddressUpdateDelegate);
    RegisterDeviceDiscoveryDelegate(params.deviceDiscoveryDelegate);
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    VerifyOrReturnError(params.operationalCredentialsDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mOperationalCredentialsDelegate = params.operationalCredentialsDelegate;

    mVendorId = params.controllerVendorId;
    if (params.operationalKeypair != nullptr || !params.controllerNOC.empty() || !params.controllerRCAC.empty())
    {
        ReturnErrorOnFailure(ProcessControllerNOCChain(params));
    }

    DeviceProxyInitParams deviceInitParams = {
        .sessionManager = params.systemState->SessionMgr(),
        .exchangeMgr    = params.systemState->ExchangeMgr(),
        .idAllocator    = &mIDAllocator,
        .fabricTable    = params.systemState->Fabrics(),
        .clientPool     = &mCASEClientPool,
        .mrpLocalConfig = Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig()),
    };

    CASESessionManagerConfig sessionManagerConfig = {
        .sessionInitParams = deviceInitParams,
        .dnsCache          = &mDNSCache,
        .devicePool        = &mDevicePool,
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
        .dnsResolver = &mDNSResolver,
#else
        .dnsResolver = nullptr,
#endif
    };

    mCASESessionManager = chip::Platform::New<CASESessionManager>(sessionManagerConfig);
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_NO_MEMORY);

    mSystemState = params.systemState->Retain();
    mState       = State::Initialized;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::ProcessControllerNOCChain(const ControllerInitParams & params)
{
    FabricInfo newFabric;
    constexpr uint32_t chipCertAllocatedLen = kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> chipCert;
    Credentials::P256PublicKeySpan rootPublicKey;
    FabricId fabricId;

    ReturnErrorOnFailure(newFabric.SetOperationalKeypair(params.operationalKeypair));
    newFabric.SetVendorId(params.controllerVendorId);

    ReturnErrorCodeIf(!chipCert.Alloc(chipCertAllocatedLen), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan chipCertSpan(chipCert.Get(), chipCertAllocatedLen);

    ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerRCAC, chipCertSpan));
    ReturnErrorOnFailure(newFabric.SetRootCert(chipCertSpan));

    if (params.controllerICAC.empty())
    {
        ChipLogProgress(Controller, "Intermediate CA is not needed");
    }
    else
    {
        chipCertSpan = MutableByteSpan(chipCert.Get(), chipCertAllocatedLen);

        ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerICAC, chipCertSpan));
        ReturnErrorOnFailure(newFabric.SetICACert(chipCertSpan));
    }

    chipCertSpan = MutableByteSpan(chipCert.Get(), chipCertAllocatedLen);

    ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerNOC, chipCertSpan));
    ReturnErrorOnFailure(newFabric.SetNOCCert(chipCertSpan));
    ReturnErrorOnFailure(ExtractFabricIdFromCert(chipCertSpan, &fabricId));

    ReturnErrorOnFailure(newFabric.GetRootPubkey(rootPublicKey));
    mFabricInfo = params.systemState->Fabrics()->FindFabric(rootPublicKey, fabricId);
    if (mFabricInfo != nullptr)
    {
        ReturnErrorOnFailure(mFabricInfo->SetFabricInfo(newFabric));
    }
    else
    {
        FabricIndex fabricIndex;
        ReturnErrorOnFailure(params.systemState->Fabrics()->AddNewFabric(newFabric, &fabricIndex));
        mFabricInfo = params.systemState->Fabrics()->FindFabricWithIndex(fabricIndex);
        ReturnErrorCodeIf(mFabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);
    }

    mLocalId  = mFabricInfo->GetPeerId();
    mFabricId = mFabricInfo->GetFabricId();

    ChipLogProgress(Controller, "Joined the fabric at index %d. Compressed fabric ID is: 0x" ChipLogFormatX64,
                    mFabricInfo->GetFabricIndex(), ChipLogValueX64(GetCompressedFabricId()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the controller");

    mState = State::NotInitialized;

    mStorageDelegate = nullptr;

    if (mFabricInfo != nullptr)
    {
        mFabricInfo->Reset();
    }
    mSystemState->Release();
    mSystemState = nullptr;

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    mDNSResolver.Shutdown();
    mDeviceAddressUpdateDelegate = nullptr;
    mDeviceDiscoveryDelegate     = nullptr;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    chip::Platform::Delete(mCASESessionManager);
    mCASESessionManager = nullptr;

    return CHIP_NO_ERROR;
}

bool DeviceController::DoesDevicePairingExist(const PeerId & deviceId)
{
    if (InitializePairedDeviceList() == CHIP_NO_ERROR)
    {
        return mPairedDevices.Contains(deviceId.GetNodeId());
    }

    return false;
}

void DeviceController::ReleaseOperationalDevice(NodeId remoteDeviceId)
{
    VerifyOrReturn(mState == State::Initialized && mFabricInfo != nullptr,
                   ChipLogError(Controller, "ReleaseOperationalDevice was called in incorrect state"));
    mCASESessionManager->ReleaseSession(mFabricInfo->GetPeerIdForNode(remoteDeviceId));
}

void DeviceController::OnFirstMessageDeliveryFailed(const SessionHandle & session)
{
    VerifyOrReturn(mState == State::Initialized,
                   ChipLogError(Controller, "OnFirstMessageDeliveryFailed was called in incorrect state"));
    VerifyOrReturn(session->GetSessionType() == Transport::Session::SessionType::kSecure);
    CHIP_ERROR err = UpdateDevice(session->AsSecureSession()->GetPeerNodeId());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller,
                     "OnFirstMessageDeliveryFailed was called, but UpdateDevice did not succeed (%" CHIP_ERROR_FORMAT ")",
                     err.Format());
    }
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
        ChipLogError(Controller, "Failed to initialize the device list with error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR DeviceController::SetPairedDeviceList(ByteSpan serialized)
{
    CHIP_ERROR err = mPairedDevices.Deserialize(serialized);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to recreate the device list with buffer %.*s\n", static_cast<int>(serialized.size()),
                     serialized.data());
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
        uint16_t nextKeyID = mIDAllocator.Peek();
        mStorageDelegate->SyncSetKeyValue(kNextAvailableKeyID, &nextKeyID, sizeof(nextKeyID));
    }
}

CHIP_ERROR DeviceController::GetPeerAddressAndPort(PeerId peerId, Inet::IPAddress & addr, uint16_t & port)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    Transport::PeerAddress peerAddr;
    ReturnErrorOnFailure(mCASESessionManager->GetPeerAddress(peerId, peerAddr));
    addr = peerAddr.GetIPAddress();
    port = peerAddr.GetPort();
    return CHIP_NO_ERROR;
}

void DeviceController::OnPIDReadResponse(void * context, uint16_t value)
{
    ChipLogProgress(Controller, "Received PID for the device. Value %d", value);
    DeviceController * controller       = static_cast<DeviceController *>(context);
    controller->mSetupPayload.productID = value;

    if (controller->OpenCommissioningWindowInternal() != CHIP_NO_ERROR)
    {
        OnOpenPairingWindowFailureResponse(context, CHIP_NO_ERROR);
    }
}

void DeviceController::OnVIDReadResponse(void * context, VendorId value)
{
    ChipLogProgress(Controller, "Received VID for the device. Value %d", to_underlying(value));

    DeviceController * controller = static_cast<DeviceController *>(context);

    controller->mSetupPayload.vendorID = value;

    OperationalDeviceProxy * device =
        controller->mCASESessionManager->FindExistingSession(controller->GetPeerIdWithCommissioningWindowOpen());
    if (device == nullptr)
    {
        ChipLogError(Controller, "Could not find device for opening commissioning window");
        OnOpenPairingWindowFailureResponse(context, CHIP_NO_ERROR);
        return;
    }

    constexpr EndpointId kBasicClusterEndpoint = 0;
    chip::Controller::BasicCluster cluster;
    cluster.Associate(device, kBasicClusterEndpoint);

    if (cluster.ReadAttribute<app::Clusters::Basic::Attributes::ProductID::TypeInfo>(context, OnPIDReadResponse,
                                                                                     OnVIDPIDReadFailureResponse) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Could not read PID for opening commissioning window");
        OnOpenPairingWindowFailureResponse(context, CHIP_NO_ERROR);
    }
}

void DeviceController::OnVIDPIDReadFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Failed to read VID/PID for the device. error %" CHIP_ERROR_FORMAT, error.Format());
    OnOpenPairingWindowFailureResponse(context, error);
}

void DeviceController::OnOpenPairingWindowSuccessResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    ChipLogProgress(Controller, "Successfully opened pairing window on the device");
    DeviceController * controller = static_cast<DeviceController *>(context);
    if (controller->mCommissioningWindowCallback != nullptr)
    {
        controller->mCommissioningWindowCallback->mCall(controller->mCommissioningWindowCallback->mContext,
                                                        controller->mDeviceWithCommissioningWindowOpen, CHIP_NO_ERROR,
                                                        controller->mSetupPayload);
    }
}

void DeviceController::OnOpenPairingWindowFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogError(Controller, "Failed to open pairing window on the device. Status %s", chip::ErrorStr(error));
    DeviceController * controller = static_cast<DeviceController *>(context);
    if (controller->mCommissioningWindowCallback != nullptr)
    {
        controller->mCommissioningWindowCallback->mCall(controller->mCommissioningWindowCallback->mContext,
                                                        controller->mDeviceWithCommissioningWindowOpen, error, SetupPayload());
    }
}

CHIP_ERROR DeviceController::ComputePASEVerifier(uint32_t iterations, uint32_t setupPincode, const ByteSpan & salt,
                                                 PASEVerifier & outVerifier, PasscodeId & outPasscodeId)
{
    ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(outVerifier, iterations, salt, /* useRandomPIN= */ false, setupPincode));

    outPasscodeId = mPAKEVerifierID++;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::OpenCommissioningWindowWithCallback(NodeId deviceId, uint16_t timeout, uint32_t iteration,
                                                                 uint16_t discriminator, uint8_t option,
                                                                 chip::Callback::Callback<OnOpenCommissioningWindow> * callback,
                                                                 bool readVIDPIDAttributes)
{
    mSetupPayload = SetupPayload();

    mSetupPayload.version               = 0;
    mSetupPayload.discriminator         = discriminator;
    mSetupPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork);

    mCommissioningWindowCallback       = callback;
    mDeviceWithCommissioningWindowOpen = deviceId;
    mCommissioningWindowTimeout        = timeout;
    mCommissioningWindowIteration      = iteration;

    switch (option)
    {
    case 0:
        mCommissioningWindowOption = CommissioningWindowOption::kOriginalSetupCode;
        break;
    case 1:
        mCommissioningWindowOption = CommissioningWindowOption::kTokenWithRandomPIN;
        break;
    case 2:
        mCommissioningWindowOption = CommissioningWindowOption::kTokenWithProvidedPIN;
        mSetupPayload.setUpPINCode = mSuggestedSetUpPINCode;
        break;
    default:
        ChipLogError(Controller, "Invalid Pairing Window option");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (callback != nullptr && mCommissioningWindowOption != CommissioningWindowOption::kOriginalSetupCode && readVIDPIDAttributes)
    {
        OperationalDeviceProxy * device = mCASESessionManager->FindExistingSession(GetPeerIdWithCommissioningWindowOpen());
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        constexpr EndpointId kBasicClusterEndpoint = 0;
        chip::Controller::BasicCluster cluster;
        cluster.Associate(device, kBasicClusterEndpoint);

        return cluster.ReadAttribute<app::Clusters::Basic::Attributes::VendorID::TypeInfo>(this, OnVIDReadResponse,
                                                                                           OnVIDPIDReadFailureResponse);
    }

    return OpenCommissioningWindowInternal();
}

CHIP_ERROR DeviceController::OpenCommissioningWindowInternal()
{
    ChipLogProgress(Controller, "OpenCommissioningWindow for device ID %" PRIu64, mDeviceWithCommissioningWindowOpen);
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    OperationalDeviceProxy * device = mCASESessionManager->FindExistingSession(GetPeerIdWithCommissioningWindowOpen());
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    constexpr EndpointId kAdministratorCommissioningClusterEndpoint = 0;

    chip::Controller::AdministratorCommissioningCluster cluster;
    cluster.Associate(device, kAdministratorCommissioningClusterEndpoint);

    if (mCommissioningWindowOption != CommissioningWindowOption::kOriginalSetupCode)
    {
        // TODO: Salt should be provided as an input or it should be randomly generated when
        // the PIN is randomly generated.
        const char kSpake2pKeyExchangeSalt[] = "SPAKE2P Key Salt";
        ByteSpan salt(Uint8::from_const_char(kSpake2pKeyExchangeSalt), sizeof(kSpake2pKeyExchangeSalt));
        bool randomSetupPIN = (mCommissioningWindowOption == CommissioningWindowOption::kTokenWithRandomPIN);
        PASEVerifier verifier;

        ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(verifier, mCommissioningWindowIteration, salt, randomSetupPIN,
                                                               mSetupPayload.setUpPINCode));

        chip::PASEVerifierSerialized serializedVerifier;
        MutableByteSpan serializedVerifierSpan(serializedVerifier);
        ReturnErrorOnFailure(verifier.Serialize(serializedVerifierSpan));

        AdministratorCommissioning::Commands::OpenCommissioningWindow::Type request;
        request.commissioningTimeout = mCommissioningWindowTimeout;
        request.PAKEVerifier         = serializedVerifierSpan;
        request.discriminator        = mSetupPayload.discriminator;
        request.iterations           = mCommissioningWindowIteration;
        request.salt                 = salt;
        request.passcodeID           = mPAKEVerifierID++;

        // TODO: What should the timed invoke timeout here be?
        uint16_t timedInvokeTimeoutMs = 10000;
        ReturnErrorOnFailure(cluster.InvokeCommand(request, this, OnOpenPairingWindowSuccessResponse,
                                                   OnOpenPairingWindowFailureResponse, MakeOptional(timedInvokeTimeoutMs)));

        char payloadBuffer[QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength];

        MutableCharSpan manualCode(payloadBuffer);
        ReturnErrorOnFailure(ManualSetupPayloadGenerator(mSetupPayload).payloadDecimalStringRepresentation(manualCode));
        ChipLogProgress(Controller, "Manual pairing code: [%s]", payloadBuffer);

        MutableCharSpan QRCode(payloadBuffer);
        ReturnErrorOnFailure(QRCodeBasicSetupPayloadGenerator(mSetupPayload).payloadBase38Representation(QRCode));
        ChipLogProgress(Controller, "SetupQRCode: [%s]", payloadBuffer);
    }
    else
    {
        AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::Type request;
        request.commissioningTimeout = mCommissioningWindowTimeout;
        // TODO: What should the timed invoke timeout here be?
        uint16_t timedInvokeTimeoutMs = 10000;
        ReturnErrorOnFailure(cluster.InvokeCommand(request, this, OnOpenPairingWindowSuccessResponse,
                                                   OnOpenPairingWindowFailureResponse, MakeOptional(timedInvokeTimeoutMs)));
    }

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD

void DeviceController::OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnNodeIdResolved was called in incorrect state"));
    mCASESessionManager->OnNodeIdResolved(nodeData);
    if (mDeviceAddressUpdateDelegate != nullptr)
    {
        mDeviceAddressUpdateDelegate->OnAddressUpdateComplete(nodeData.mPeerId.GetNodeId(), CHIP_NO_ERROR);
    }
};

void DeviceController::OnNodeIdResolutionFailed(const chip::PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
    VerifyOrReturn(mState == State::Initialized,
                   ChipLogError(Controller, "OnNodeIdResolutionFailed was called in incorrect state"));
    mCASESessionManager->OnNodeIdResolutionFailed(peer, error);

    if (mDeviceAddressUpdateDelegate != nullptr)
    {
        mDeviceAddressUpdateDelegate->OnAddressUpdateComplete(peer.GetNodeId(), error);
    }
};

#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

ControllerDeviceInitParams DeviceController::GetControllerDeviceInitParams()
{
    return ControllerDeviceInitParams{
        .transportMgr       = mSystemState->TransportMgr(),
        .sessionManager     = mSystemState->SessionMgr(),
        .exchangeMgr        = mSystemState->ExchangeMgr(),
        .udpEndPointManager = mSystemState->UDPEndPointManager(),
        .storageDelegate    = mStorageDelegate,
        .idAllocator        = &mIDAllocator,
        .fabricsTable       = mSystemState->Fabrics(),
    };
}

DeviceCommissioner::DeviceCommissioner() :
    mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this),
    mDeviceAttestationInformationVerificationCallback(OnDeviceAttestationInformationVerification, this),
    mDeviceNOCChainCallback(OnDeviceNOCChainGeneration, this), mSetUpCodePairer(this)
{
    mPairingDelegate         = nullptr;
    mPairedDevicesUpdated    = false;
    mDeviceBeingCommissioned = nullptr;
}

CHIP_ERROR DeviceCommissioner::Init(CommissionerInitParams params)
{
    ReturnErrorOnFailure(DeviceController::Init(params));

    params.systemState->SessionMgr()->RegisterRecoveryDelegate(*this);

#if 0 //
      // We cannot reinstantiate session ID allocator state from each fabric-scoped commissioner
      // individually because the session ID allocator space is and must be shared for all users
      // of the Session Manager. Disable persistence for now. #12821 tracks a proper fix this issue.
      //

    uint16_t nextKeyID = 0;
    uint16_t size      = sizeof(nextKeyID);
    CHIP_ERROR error   = mStorageDelegate->SyncGetKeyValue(kNextAvailableKeyID, &nextKeyID, size);
    if ((error != CHIP_NO_ERROR) || (size != sizeof(nextKeyID)))
    {
        nextKeyID = 0;
    }
    ReturnErrorOnFailure(mIDAllocator.ReserveUpTo(nextKeyID));
#endif

    mPairingDelegate = params.pairingDelegate;
    if (params.defaultCommissioner != nullptr)
    {
        mDefaultCommissioner = params.defaultCommissioner;
    }
    else
    {
        mDefaultCommissioner = &mAutoCommissioner;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    mUdcTransportMgr = chip::Platform::New<DeviceIPTransportMgr>();
    ReturnErrorOnFailure(mUdcTransportMgr->Init(Transport::UdpListenParameters(mSystemState->UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv6)
                                                    .SetListenPort((uint16_t)(mUdcListenPort))
#if INET_CONFIG_ENABLE_IPV4
                                                    ,
                                                Transport::UdpListenParameters(mSystemState->UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv4)
                                                    .SetListenPort((uint16_t)(mUdcListenPort))
#endif // INET_CONFIG_ENABLE_IPV4
                                                    ));

    mUdcServer = chip::Platform::New<UserDirectedCommissioningServer>();
    mUdcTransportMgr->SetSessionManager(mUdcServer);

    mUdcServer->SetInstanceNameResolver(this);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

#if CONFIG_NETWORK_LAYER_BLE
    mSetUpCodePairer.SetBleLayer(mSystemState->BleLayer());
#endif // CONFIG_NETWORK_LAYER_BLE
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the commissioner");

    mSystemState->SessionMgr()->UnregisterRecoveryDelegate(*this);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    if (mUdcTransportMgr != nullptr)
    {
        chip::Platform::Delete(mUdcTransportMgr);
        mUdcTransportMgr = nullptr;
    }
    if (mUdcServer != nullptr)
    {
        mUdcServer->SetInstanceNameResolver(nullptr);
        chip::Platform::Delete(mUdcServer);
        mUdcServer = nullptr;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    DeviceController::Shutdown();
    return CHIP_NO_ERROR;
}

CommissioneeDeviceProxy * DeviceCommissioner::FindCommissioneeDevice(const SessionHandle & session)
{
    CommissioneeDeviceProxy * foundDevice = nullptr;
    mCommissioneeDevicePool.ForEachActiveObject([&](auto * deviceProxy) {
        if (deviceProxy->MatchesSession(session))
        {
            foundDevice = deviceProxy;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    return foundDevice;
}

CommissioneeDeviceProxy * DeviceCommissioner::FindCommissioneeDevice(NodeId id)
{
    CommissioneeDeviceProxy * foundDevice = nullptr;
    mCommissioneeDevicePool.ForEachActiveObject([&](auto * deviceProxy) {
        if (deviceProxy->GetDeviceId() == id)
        {
            foundDevice = deviceProxy;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    return foundDevice;
}

void DeviceCommissioner::ReleaseCommissioneeDevice(CommissioneeDeviceProxy * device)
{
    mCommissioneeDevicePool.ReleaseObject(device);
}

CHIP_ERROR DeviceCommissioner::GetDeviceBeingCommissioned(NodeId deviceId, CommissioneeDeviceProxy ** out_device)
{
    VerifyOrReturnError(out_device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CommissioneeDeviceProxy * device = FindCommissioneeDevice(deviceId);

    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    *out_device = device;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::GetConnectedDevice(NodeId deviceId, chip::Callback::Callback<OnDeviceConnected> * onConnection,
                                                  chip::Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    return DeviceController::GetConnectedDevice(deviceId, onConnection, onFailure);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, const char * setUpCode)
{
    return mSetUpCodePairer.PairDevice(remoteDeviceId, setUpCode);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & params)
{
    CommissioningParameters commissioningParams;
    return PairDevice(remoteDeviceId, params, commissioningParams);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & rendezvousParams,
                                          CommissioningParameters & commissioningParams)
{
    ReturnErrorOnFailure(EstablishPASEConnection(remoteDeviceId, rendezvousParams));
    return Commission(remoteDeviceId, commissioningParams);
}

CHIP_ERROR DeviceCommissioner::EstablishPASEConnection(NodeId remoteDeviceId, RendezvousParameters & params)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device   = nullptr;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    Messaging::ExchangeContext * exchangeCtxt = nullptr;
    Optional<SessionHandle> session;

    uint16_t keyID = 0;

    VerifyOrExit(mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingCommissioned == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = InitializePairedDeviceList();
    SuccessOrExit(err);

    // TODO(#13940): We need to specify the peer address for BLE transport in bindings.
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

    device = mCommissioneeDevicePool.CreateObject();
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mDeviceBeingCommissioned = device;

    mIsIPRendezvous = (params.GetPeerAddress().GetTransportType() != Transport::Type::kBle);

    {
        FabricIndex fabricIndex = mFabricInfo != nullptr ? mFabricInfo->GetFabricIndex() : kUndefinedFabricIndex;
        device->Init(GetControllerDeviceInitParams(), remoteDeviceId, peerAddress, fabricIndex);
    }

    if (params.GetPeerAddress().GetTransportType() != Transport::Type::kBle)
    {
        device->SetAddress(params.GetPeerAddress().GetIPAddress());
    }
#if CONFIG_NETWORK_LAYER_BLE
    else
    {
        if (params.HasConnectionObject())
        {
            SuccessOrExit(err = mSystemState->BleLayer()->NewBleConnectionByObject(params.GetConnectionObject()));
        }
        else if (params.HasDiscriminator())
        {
            SuccessOrExit(err = mSystemState->BleLayer()->NewBleConnectionByDiscriminator(params.GetDiscriminator()));
        }
        else
        {
            ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
#endif
    // TODO: In some cases like PASE over IP, CRA and CRI values from commissionable node service should be used
    session = mSystemState->SessionMgr()->CreateUnauthenticatedSession(params.GetPeerAddress(), device->GetMRPConfig());
    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NO_MEMORY);

    err = mIDAllocator.Allocate(keyID);
    SuccessOrExit(err);

    // TODO - Remove use of SetActive/IsActive from CommissioneeDeviceProxy
    device->SetActive(true);

    // Allocate the exchange immediately before calling PASESession::Pair.
    //
    // PASESession::Pair takes ownership of the exchange and will free it on
    // error, but can only do this if it is actually called.  Allocating the
    // exchange context right before calling Pair ensures that if allocation
    // succeeds, PASESession has taken ownership.
    exchangeCtxt = mSystemState->ExchangeMgr()->NewContext(session.Value(), &device->GetPairing());
    VerifyOrExit(exchangeCtxt != nullptr, err = CHIP_ERROR_INTERNAL);

    err = device->GetPairing().Pair(params.GetPeerAddress(), params.GetSetupPINCode(), keyID,
                                    Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig()), exchangeCtxt, this);
    SuccessOrExit(err);

    // Immediately persist the updated mNextKeyID value
    // TODO maybe remove FreeRendezvousSession() since mNextKeyID is always persisted immediately
    //
    // Disabling session ID persistence (see previous comment in Init() about persisting key ids)
    //
    // PersistNextKeyId();

exit:
    if (err != CHIP_NO_ERROR)
    {
        // Delete the current rendezvous session only if a device is not currently being paired.
        if (mDeviceBeingCommissioned == nullptr)
        {
            FreeRendezvousSession();
        }

        if (device != nullptr)
        {
            ReleaseCommissioneeDevice(device);
            mDeviceBeingCommissioned = nullptr;
        }
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::Commission(NodeId remoteDeviceId, CommissioningParameters & params)
{
    // TODO(cecille): Can we get rid of mDeviceBeingCommissioned and use the remote id instead? Would require storing the
    // commissioning stage in the device.
    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;
    if (device == nullptr || device->GetDeviceId() != remoteDeviceId ||
        (!device->IsSecureConnected() && !device->IsSessionSetupInProgress()))
    {
        ChipLogError(Controller, "Invalid device for commissioning " ChipLogFormatX64, ChipLogValueX64(remoteDeviceId));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mCommissioningStage != CommissioningStage::kSecurePairing)
    {
        ChipLogError(Controller, "Commissioning already in progress - not restarting");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (!params.GetWiFiCredentials().HasValue() && !params.GetThreadOperationalDataset().HasValue() && !mIsIPRendezvous)
    {
        ChipLogError(Controller, "Network commissioning parameters are required for BLE auto commissioning.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ChipLogProgress(Controller, "Commission called for node ID 0x" ChipLogFormatX64, ChipLogValueX64(remoteDeviceId));

    mSystemState->SystemLayer()->StartTimer(chip::System::Clock::Milliseconds32(kSessionEstablishmentTimeout),
                                            OnSessionEstablishmentTimeoutCallback, this);

    mDefaultCommissioner->SetOperationalCredentialsDelegate(mOperationalCredentialsDelegate);
    ReturnErrorOnFailure(mDefaultCommissioner->SetCommissioningParameters(params));
    if (device->IsSecureConnected())
    {
        mDefaultCommissioner->StartCommissioning(this, device);
    }
    else
    {
        mRunCommissioningAfterConnection = true;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::GetAttestationChallenge(ByteSpan & attestationChallenge)
{
    Optional<SessionHandle> secureSessionHandle;

    VerifyOrReturnError(mDeviceBeingCommissioned != nullptr, CHIP_ERROR_INCORRECT_STATE);

    secureSessionHandle = mDeviceBeingCommissioned->GetSecureSession();
    VerifyOrReturnError(secureSessionHandle.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    attestationChallenge = secureSessionHandle.Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::StopPairing(NodeId remoteDeviceId)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    CommissioneeDeviceProxy * device = FindCommissioneeDevice(remoteDeviceId);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    FreeRendezvousSession();

    ReleaseCommissioneeDevice(device);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::UnpairDevice(NodeId remoteDeviceId)
{
    // TODO: Send unpairing message to the remote device.
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::FreeRendezvousSession()
{
    PersistNextKeyId();
}

void DeviceCommissioner::RendezvousCleanup(CHIP_ERROR status)
{
    FreeRendezvousSession();

    if (mDeviceBeingCommissioned != nullptr)
    {
        // Release the commissionee device. For BLE, this is stored,
        // for IP commissioning, we have taken a reference to the
        // operational node to send the completion command.
        ReleaseCommissioneeDevice(mDeviceBeingCommissioned);
        mDeviceBeingCommissioned = nullptr;
    }

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(status);
    }
}

void DeviceCommissioner::OnSessionEstablishmentError(CHIP_ERROR err)
{
    // PASE session establishment failure.
    mSystemState->SystemLayer()->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingFailed);
    }

    RendezvousCleanup(err);
}

void DeviceCommissioner::OnSessionEstablished()
{
    // PASE session established.
    VerifyOrReturn(mDeviceBeingCommissioned != nullptr, OnSessionEstablishmentError(CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR));

    PASESession * pairing = &mDeviceBeingCommissioned->GetPairing();

    // TODO: the session should know which peer we are trying to connect to when started
    pairing->SetPeerNodeId(mDeviceBeingCommissioned->GetDeviceId());

    CHIP_ERROR err = mDeviceBeingCommissioned->SetConnected();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake");

    // TODO: Add code to receive CSR from the device, and process the signing request
    // For IP rendezvous, this is sent as part of the state machine.
    if (mRunCommissioningAfterConnection)
    {
        mRunCommissioningAfterConnection = false;
        mDefaultCommissioner->StartCommissioning(this, mDeviceBeingCommissioned);
    }
    else
    {
        ChipLogProgress(Controller, "OnPairingComplete");
        mPairingDelegate->OnPairingComplete(CHIP_NO_ERROR);
    }
}

CHIP_ERROR DeviceCommissioner::SendCertificateChainRequestCommand(DeviceProxy * device,
                                                                  Credentials::CertificateType certificateType)
{
    ChipLogDetail(Controller, "Sending Certificate Chain request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::CertificateChainRequest::Type request;
    request.certificateType = certificateType;
    return SendCommand<OperationalCredentialsCluster>(device, request, OnCertificateChainResponse,
                                                      OnCertificateChainFailureResponse);

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCertificateChainFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Device failed to receive the Certificate Chain request Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnCertificateChainResponse(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType & response)
{
    ChipLogProgress(Controller, "Received certificate chain from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<RequestedCertificate>(RequestedCertificate(response.certificate));

    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

CHIP_ERROR DeviceCommissioner::SendAttestationRequestCommand(DeviceProxy * device, const ByteSpan & attestationNonce)
{
    ChipLogDetail(Controller, "Sending Attestation request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::AttestationRequest::Type request;
    request.attestationNonce = attestationNonce;

    ReturnErrorOnFailure(
        SendCommand<OperationalCredentialsCluster>(device, request, OnAttestationResponse, OnAttestationFailureResponse));
    ChipLogDetail(Controller, "Sent Attestation request, waiting for the Attestation Information");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnAttestationFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Device failed to receive the Attestation Information Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnAttestationResponse(void * context,
                                               const OperationalCredentials::Commands::AttestationResponse::DecodableType & data)
{
    ChipLogProgress(Controller, "Received Attestation Information from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<AttestationResponse>(AttestationResponse(data.attestationElements, data.signature));
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

void DeviceCommissioner::OnDeviceAttestationInformationVerification(void * context, AttestationVerificationResult result)
{
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    if (result != AttestationVerificationResult::kSuccess)
    {
        CommissioningDelegate::CommissioningReport report;
        report.Set<AdditionalErrorInfo>(result);
        if (result == AttestationVerificationResult::kNotImplemented)
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device due to default "
                         "DeviceAttestationVerifier Class not being overridden by a real implementation.");
            commissioner->CommissioningStageComplete(CHIP_ERROR_NOT_IMPLEMENTED, report);
            return;
        }
        else
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device: err %hu. Look at "
                         "AttestationVerificationResult enum to understand the errors",
                         static_cast<uint16_t>(result));
            // Go look at AttestationVerificationResult enum in src/credentials/DeviceAttestationVerifier.h to understand the
            // errors.
            commissioner->CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
            return;
        }
    }

    ChipLogProgress(Controller, "Successfully validated 'Attestation Information' command received from the device.");
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

CHIP_ERROR DeviceCommissioner::ValidateAttestationInfo(const ByteSpan & attestationElements, const ByteSpan & signature,
                                                       const ByteSpan & attestationNonce, const ByteSpan & pai,
                                                       const ByteSpan & dac, VendorId remoteVendorId, uint16_t remoteProductId,
                                                       DeviceProxy * proxy)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(proxy != nullptr, CHIP_ERROR_INCORRECT_STATE);

    DeviceAttestationVerifier * dac_verifier = GetDeviceAttestationVerifier();

    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        proxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    dac_verifier->VerifyAttestationInformation(attestationElements, attestationChallenge, signature, pai, dac, attestationNonce,
                                               remoteVendorId, remoteProductId, &mDeviceAttestationInformationVerificationCallback);

    // TODO: Validate Firmware Information

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificateSigningRequestCommand(DeviceProxy * device, const ByteSpan & csrNonce)
{
    ChipLogDetail(Controller, "Sending CSR request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::CSRRequest::Type request;
    request.CSRNonce = csrNonce;

    ReturnErrorOnFailure(
        SendCommand<OperationalCredentialsCluster>(device, request, OnOperationalCertificateSigningRequest, OnCSRFailureResponse));
    ChipLogDetail(Controller, "Sent CSR request, waiting for the CSR");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCSRFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Device failed to receive the CSR request Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnOperationalCertificateSigningRequest(
    void * context, const OperationalCredentials::Commands::CSRResponse::DecodableType & data)
{
    ChipLogProgress(Controller, "Received certificate signing request from the device");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<AttestationResponse>(AttestationResponse(data.NOCSRElements, data.attestationSignature));
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

void DeviceCommissioner::OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                                    const ByteSpan & rcac, Optional<AesCcm128KeySpan> ipk,
                                                    Optional<NodeId> adminSubject)
{
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    // TODO(#13825): If not passed by the signer, the commissioner should
    // provide its current IPK to the commissionee in the AddNOC command.
    const uint8_t placeHolderIpk[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    ChipLogProgress(Controller, "Received callback from the CA for NOC Chain generation. Status %s", ErrorStr(status));
    if (commissioner->mState != State::Initialized)
    {
        status = CHIP_ERROR_INCORRECT_STATE;
    }
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in generating device's operational credentials. Error %s", ErrorStr(status));
    }

    // TODO - Verify that the generated root cert matches with commissioner's root cert
    CommissioningDelegate::CommissioningReport report;
    report.Set<NocChain>(NocChain(noc, icac, rcac, ipk.HasValue() ? ipk.Value() : AesCcm128KeySpan(placeHolderIpk),
                                  adminSubject.HasValue() ? adminSubject.Value() : commissioner->GetNodeId()));
    commissioner->CommissioningStageComplete(status, report);
}

CHIP_ERROR DeviceCommissioner::ProcessCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements,
                                          const ByteSpan & AttestationSignature, ByteSpan dac, ByteSpan csrNonce)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "Getting certificate chain for the device from the issuer");

    DeviceAttestationVerifier * dacVerifier = GetDeviceAttestationVerifier();

    P256PublicKey dacPubkey;
    ReturnErrorOnFailure(ExtractPubkeyFromX509Cert(dac, dacPubkey));

    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        proxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    // The operational CA should also verify this on its end during NOC generation, if end-to-end attestation is desired.
    ReturnErrorOnFailure(dacVerifier->VerifyNodeOperationalCSRInformation(NOCSRElements, attestationChallenge, AttestationSignature,
                                                                          dacPubkey, csrNonce));

    mOperationalCredentialsDelegate->SetNodeIdForNextNOCRequest(proxy->GetDeviceId());

    if (mFabricInfo != nullptr)
    {
        mOperationalCredentialsDelegate->SetFabricIdForNextNOCRequest(mFabricInfo->GetFabricId());
    }

    return mOperationalCredentialsDelegate->GenerateNOCChain(NOCSRElements, AttestationSignature, dac, ByteSpan(), ByteSpan(),
                                                             &mDeviceNOCChainCallback);
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificate(DeviceProxy * device, const ByteSpan & nocCertBuf,
                                                          const ByteSpan & icaCertBuf, const AesCcm128KeySpan ipk,
                                                          const NodeId adminSubject)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::AddNOC::Type request;
    request.NOCValue      = nocCertBuf;
    request.ICACValue     = chip::Optional<ByteSpan>(icaCertBuf);
    request.IPKValue      = ipk;
    request.caseAdminNode = adminSubject;
    request.adminVendorId = mVendorId;

    ReturnErrorOnFailure(SendCommand<OperationalCredentialsCluster>(mDeviceBeingCommissioned, request,
                                                                    OnOperationalCertificateAddResponse, OnAddNOCFailureResponse));

    ChipLogProgress(Controller, "Sent operational certificate to the device");

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::ConvertFromOperationalCertStatus(OperationalCredentials::OperationalCertStatus err)
{
    using OperationalCredentials::OperationalCertStatus;
    switch (err)
    {
    case OperationalCertStatus::kSuccess:
        return CHIP_NO_ERROR;
    case OperationalCertStatus::kInvalidPublicKey:
        return CHIP_ERROR_INVALID_PUBLIC_KEY;
    case OperationalCertStatus::kInvalidNodeOpId:
        return CHIP_ERROR_WRONG_NODE_ID;
    case OperationalCertStatus::kInvalidNOC:
        return CHIP_ERROR_UNSUPPORTED_CERT_FORMAT;
    case OperationalCertStatus::kMissingCsr:
        return CHIP_ERROR_INCORRECT_STATE;
    case OperationalCertStatus::kTableFull:
        return CHIP_ERROR_NO_MEMORY;
    case OperationalCertStatus::kInsufficientPrivilege:
    case OperationalCertStatus::kFabricConflict:
    case OperationalCertStatus::kLabelConflict:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case OperationalCertStatus::kInvalidFabricIndex:
        return CHIP_ERROR_INVALID_FABRIC_ID;
    }

    return CHIP_ERROR_CERT_LOAD_FAILED;
}

void DeviceCommissioner::OnAddNOCFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Device failed to receive the operational certificate Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnOperationalCertificateAddResponse(
    void * context, const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    ChipLogProgress(Controller, "Device returned status %d on receiving the NOC", to_underlying(data.statusCode));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err                   = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device = nullptr;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(commissioner->mDeviceBeingCommissioned != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = ConvertFromOperationalCertStatus(data.statusCode);
    SuccessOrExit(err);

    device = commissioner->mDeviceBeingCommissioned;

    err = commissioner->OnOperationalCredentialsProvisioningCompletion(device);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Add NOC failed with error %s", ErrorStr(err));
        commissioner->CommissioningStageComplete(err);
    }
}

CHIP_ERROR DeviceCommissioner::SendTrustedRootCertificate(DeviceProxy * device, const ByteSpan & rcac)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(Controller, "Sending root certificate to the device");

    OperationalCredentials::Commands::AddTrustedRootCertificate::Type request;
    request.rootCertificate = rcac;
    ReturnErrorOnFailure(
        SendCommand<OperationalCredentialsCluster>(device, request, OnRootCertSuccessResponse, OnRootCertFailureResponse));

    ChipLogProgress(Controller, "Sent root certificate to the device");

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnRootCertSuccessResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    ChipLogProgress(Controller, "Device confirmed that it has received the root certificate");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnRootCertFailureResponse(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Device failed to receive the root certificate Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

CHIP_ERROR DeviceCommissioner::OnOperationalCredentialsProvisioningCompletion(CommissioneeDeviceProxy * device)
{
    ChipLogProgress(Controller, "Operational credentials provisioned on device %p", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSystemState->SystemLayer()->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

    mPairedDevices.Insert(device->GetDeviceId());
    mPairedDevicesUpdated = true;

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
    }
    CommissioningStageComplete(CHIP_NO_ERROR);

    return CHIP_NO_ERROR;
}

#if CONFIG_NETWORK_LAYER_BLE
CHIP_ERROR DeviceCommissioner::CloseBleConnection()
{
    // It is fine since we can only commission one device at the same time.
    // We should be able to distinguish different BLE connections if we want
    // to commission multiple devices at the same time over BLE.
    return mSystemState->BleLayer()->CloseAllBleConnections();
}
#endif

void DeviceCommissioner::OnSessionEstablishmentTimeout()
{
    VerifyOrReturn(mState == State::Initialized);
    VerifyOrReturn(mDeviceBeingCommissioned != nullptr);

    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;
    StopPairing(device->GetDeviceId());

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(CHIP_ERROR_TIMEOUT);
    }
}

void DeviceCommissioner::OnSessionEstablishmentTimeoutCallback(System::Layer * aLayer, void * aAppState)
{
    static_cast<DeviceCommissioner *>(aAppState)->OnSessionEstablishmentTimeout();
}

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
CHIP_ERROR DeviceCommissioner::DiscoverCommissionableNodes(Dnssd::DiscoveryFilter filter)
{
    ReturnErrorOnFailure(SetUpNodeDiscovery());
    return mDNSResolver.FindCommissionableNodes(filter);
}

const Dnssd::DiscoveredNodeData * DeviceCommissioner::GetDiscoveredDevice(int idx)
{
    return GetDiscoveredNode(idx);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable

CHIP_ERROR DeviceCommissioner::SetUdcListenPort(uint16_t listenPort)
{
    if (mState == State::Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mUdcListenPort = listenPort;
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::FindCommissionableNode(char * instanceName)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kInstanceName, instanceName);
    DiscoverCommissionableNodes(filter);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD

void DeviceCommissioner::OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    if (mUdcServer != nullptr)
    {
        mUdcServer->OnCommissionableNodeFound(nodeData);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    AbstractDnssdDiscoveryController::OnNodeDiscoveryComplete(nodeData);
    mSetUpCodePairer.NotifyCommissionableDeviceDiscovered(nodeData);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

void OnBasicFailure(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Received failure response %s\n", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::CommissioningStageComplete(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    if (mCommissioningDelegate == nullptr)
    {
        return;
    }
    report.stageCompleted = mCommissioningStage;
    CHIP_ERROR status     = mCommissioningDelegate->CommissioningStepFinished(err, report);
    if (status != CHIP_NO_ERROR)
    {
        // Commissioning delegate will only return error if it failed to perform the appropriate commissioning step.
        // In this case, we should call back the commissioning complete and call session error
        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnCommissioningComplete(mDeviceBeingCommissioned->GetDeviceId(), status);
        }
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
void DeviceCommissioner::OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "OperationalDiscoveryComplete for device ID 0x" ChipLogFormatX64,
                    ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
    VerifyOrReturn(mState == State::Initialized);

    mDNSCache.Insert(nodeData);

    mCASESessionManager->FindOrEstablishSession(nodeData.mPeerId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    DeviceController::OnNodeIdResolved(nodeData);
}

void DeviceCommissioner::OnNodeIdResolutionFailed(const chip::PeerId & peer, CHIP_ERROR error)
{
    if (mDeviceBeingCommissioned != nullptr)
    {
        CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;
        if (device->GetDeviceId() == peer.GetNodeId() && mCommissioningStage == CommissioningStage::kFindOperational)
        {
            CommissioningStageComplete(error);
        }
    }
    DeviceController::OnNodeIdResolutionFailed(peer, error);
}

#endif

void DeviceCommissioner::OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device)
{
    // CASE session established.
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    VerifyOrReturn(commissioner != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    if (commissioner->mCommissioningStage == CommissioningStage::kFindOperational)
    {
        if (commissioner->mDeviceBeingCommissioned != nullptr &&
            commissioner->mDeviceBeingCommissioned->GetDeviceId() == device->GetDeviceId())
        {
            // Let's release the device that's being paired, if pairing was successful,
            // and the device is available on the operational network.
            commissioner->ReleaseCommissioneeDevice(commissioner->mDeviceBeingCommissioned);
            commissioner->mDeviceBeingCommissioned = nullptr;
            if (commissioner->mCommissioningDelegate != nullptr)
            {
                CommissioningDelegate::CommissioningReport report;
                report.Set<OperationalNodeFoundData>(OperationalNodeFoundData(device));
                commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
            }
        }
    }
    else
    {
        VerifyOrReturn(commissioner->mPairingDelegate != nullptr,
                       ChipLogProgress(Controller, "Device connected callback with null pairing delegate. Ignoring"));
        commissioner->mPairingDelegate->OnPairingComplete(CHIP_NO_ERROR);
    }
}

void DeviceCommissioner::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error)
{
    // CASE session establishment failed.
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    ChipLogProgress(Controller, "Device connection failed. Error %s", ErrorStr(error));
    VerifyOrReturn(commissioner != nullptr,
                   ChipLogProgress(Controller, "Device connection failure callback with null context. Ignoring"));
    VerifyOrReturn(commissioner->mPairingDelegate != nullptr,
                   ChipLogProgress(Controller, "Device connection failure callback with null pairing delegate. Ignoring"));

    commissioner->mCASESessionManager->ReleaseSession(peerId);
    if (commissioner->mCommissioningStage == CommissioningStage::kFindOperational &&
        commissioner->mCommissioningDelegate != nullptr)
    {
        commissioner->CommissioningStageComplete(error);
    }
    else
    {
        commissioner->mPairingDelegate->OnPairingComplete(error);
    }
}

void DeviceCommissioner::SetupCluster(ClusterBase & base, DeviceProxy * proxy, EndpointId endpoint,
                                      Optional<System::Clock::Timeout> timeout)
{
    base.Associate(proxy, endpoint);
    base.SetCommandTimeout(timeout);
}

// AttributeCache::Callback impl
void DeviceCommissioner::OnDone()
{
    CHIP_ERROR err;
    CHIP_ERROR return_err = CHIP_NO_ERROR;
    ReadCommissioningInfo info;

    // Using ForEachAttribute because this attribute can be queried on any endpoint.
    err = mAttributeCache->ForEachAttribute(
        app::Clusters::GeneralCommissioning::Id, [this, &info](const app::ConcreteAttributePath & path) {
            if (path.mAttributeId != app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::Id)
            {
                return CHIP_NO_ERROR;
            }
            app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::TypeInfo::DecodableType basicInfo;
            ReturnErrorOnFailure(
                this->mAttributeCache->Get<app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::TypeInfo>(
                    path, basicInfo));
            info.general.recommendedFailsafe = basicInfo.failSafeExpiryLengthSeconds;
            return CHIP_NO_ERROR;
        });

    // Try to parse as much as we can here before returning, even if this is an error.
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    err = mAttributeCache->ForEachAttribute(app::Clusters::Basic::Id, [this, &info](const app::ConcreteAttributePath & path) {
        if (path.mAttributeId != app::Clusters::Basic::Attributes::VendorID::Id &&
            path.mAttributeId != app::Clusters::Basic::Attributes::ProductID::Id &&
            path.mAttributeId != app::Clusters::Basic::Attributes::SoftwareVersion::Id)
        {
            // Continue on
            return CHIP_NO_ERROR;
        }

        switch (path.mAttributeId)
        {
        case app::Clusters::Basic::Attributes::VendorID::Id:
            return this->mAttributeCache->Get<app::Clusters::Basic::Attributes::VendorID::TypeInfo>(path, info.basic.vendorId);
        case app::Clusters::Basic::Attributes::ProductID::Id:
            return this->mAttributeCache->Get<app::Clusters::Basic::Attributes::ProductID::TypeInfo>(path, info.basic.productId);
        case app::Clusters::Basic::Attributes::SoftwareVersion::Id:
            return this->mAttributeCache->Get<app::Clusters::Basic::Attributes::SoftwareVersion::TypeInfo>(
                path, info.basic.softwareVersion);
        default:
            return CHIP_NO_ERROR;
        }
    });
    // Try to parse as much as we can here before returning, even if this is an error.
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    err = mAttributeCache->ForEachAttribute(
        app::Clusters::NetworkCommissioning::Id, [this, &info](const app::ConcreteAttributePath & path) {
            if (path.mAttributeId != app::Clusters::NetworkCommissioning::Attributes::FeatureMap::Id)
            {
                return CHIP_NO_ERROR;
            }
            TLV::TLVReader reader;
            if (this->mAttributeCache->Get(path, reader) == CHIP_NO_ERROR)
            {
                BitFlags<app::Clusters::NetworkCommissioning::NetworkCommissioningFeature> features;
                if (app::DataModel::Decode(reader, features) == CHIP_NO_ERROR)
                {
                    if (features.Has(app::Clusters::NetworkCommissioning::NetworkCommissioningFeature::kWiFiNetworkInterface))
                    {
                        info.network.wifi = path.mEndpointId;
                    }
                    else if (features.Has(
                                 app::Clusters::NetworkCommissioning::NetworkCommissioningFeature::kThreadNetworkInterface))
                    {
                        info.network.thread = path.mEndpointId;
                    }
                    else if (features.Has(
                                 app::Clusters::NetworkCommissioning::NetworkCommissioningFeature::kEthernetNetworkInterface))
                    {
                        info.network.eth = path.mEndpointId;
                    }
                    else
                    {
                        // TODO: Gross workaround for the empty feature map on all clusters. Remove.
                        if (info.network.thread == kInvalidEndpointId)
                        {
                            info.network.thread = path.mEndpointId;
                        }
                        if (info.network.wifi == kInvalidEndpointId)
                        {
                            info.network.wifi = path.mEndpointId;
                        }
                    }
                }
            }
            return CHIP_NO_ERROR;
        });
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    if (return_err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error parsing commissioning information");
    }
    mAttributeCache = nullptr;
    mReadClient     = nullptr;
    CommissioningDelegate::CommissioningReport report;
    report.Set<ReadCommissioningInfo>(info);
    CommissioningStageComplete(return_err, report);
}
void DeviceCommissioner::OnArmFailSafe(void * context,
                                       const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    // TODO: Use errorCode
    ChipLogProgress(Controller, "Received ArmFailSafe response");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnSetRegulatoryConfigResponse(
    void * context, const GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data)
{
    // TODO: Use errorCode
    ChipLogProgress(Controller, "Received SetRegulatoryConfig response");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnNetworkConfigResponse(void * context,
                                                 const NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    // TODO: Use networkingStatus
    ChipLogProgress(Controller, "Received NetworkConfig response");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnConnectNetworkResponse(
    void * context, const NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
{
    // TODO: Use networkingStatus
    ChipLogProgress(Controller, "Received ConnectNetwork response");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    // TODO: Use errorCode
    ChipLogProgress(Controller, "Received CommissioningComplete response");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::PerformCommissioningStep(DeviceProxy * proxy, CommissioningStage step, CommissioningParameters & params,
                                                  CommissioningDelegate * delegate, EndpointId endpoint,
                                                  Optional<System::Clock::Timeout> timeout)
{
    // For now, we ignore errors coming in from the device since not all commissioning clusters are implemented on the device
    // side.
    mCommissioningStage    = step;
    mCommissioningDelegate = delegate;
    // TODO: Extend timeouts to the DAC and Opcert requests.

    // TODO(cecille): We probably want something better than this for breadcrumbs.
    uint64_t breadcrumb = static_cast<uint64_t>(step);

    // TODO(cecille): This should be customized per command.
    constexpr uint32_t kCommandTimeoutMs = 3000;

    switch (step)
    {
    case CommissioningStage::kArmFailsafe: {
        GeneralCommissioning::Commands::ArmFailSafe::Type request;
        request.expiryLengthSeconds = params.GetFailsafeTimerSeconds().ValueOr(kDefaultFailsafeTimeout);
        request.breadcrumb          = breadcrumb;
        request.timeoutMs           = kCommandTimeoutMs;
        ChipLogProgress(Controller, "Arming failsafe (%u seconds)", request.expiryLengthSeconds);
        SendCommand<GeneralCommissioningCluster>(proxy, request, OnArmFailSafe, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kReadCommissioningInfo: {
        ChipLogProgress(Controller, "Sending request for commissioning information");
        app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
        app::ReadPrepareParams readParams(proxy->GetSecureSession().Value());

        app::AttributePathParams readPaths[5];
        // Read all the feature maps for all the networking clusters on any endpoint to determine what is supported
        readPaths[0] = app::AttributePathParams(app::Clusters::NetworkCommissioning::Id,
                                                app::Clusters::NetworkCommissioning::Attributes::FeatureMap::Id);
        // Get the basic commissioning info from the general commissioning cluster on this endpoint (recommended failsafe time)
        readPaths[1] = app::AttributePathParams(endpoint, app::Clusters::GeneralCommissioning::Id,
                                                app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::Id);
        // Read attributes from the basic info cluster (vendor id / product id / software version)
        readPaths[2] = app::AttributePathParams(endpoint, app::Clusters::Basic::Id, app::Clusters::Basic::Attributes::VendorID::Id);
        readPaths[3] =
            app::AttributePathParams(endpoint, app::Clusters::Basic::Id, app::Clusters::Basic::Attributes::ProductID::Id);
        readPaths[4] =
            app::AttributePathParams(endpoint, app::Clusters::Basic::Id, app::Clusters::Basic::Attributes::SoftwareVersion::Id);

        readParams.mpAttributePathParamsList    = readPaths;
        readParams.mAttributePathParamsListSize = 5;
        if (timeout.HasValue())
        {
            readParams.mTimeout = timeout.Value();
        }
        auto attributeCache = Platform::MakeUnique<app::AttributeCache>(*this);
        auto readClient     = chip::Platform::MakeUnique<app::ReadClient>(
            engine, proxy->GetExchangeManager(), attributeCache->GetBufferedCallback(), app::ReadClient::InteractionType::Read);
        CHIP_ERROR err = readClient->SendRequest(readParams);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed to send read request for networking clusters");
            CommissioningStageComplete(err);
            return;
        }
        mAttributeCache = std::move(attributeCache);
        mReadClient     = std::move(readClient);
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
        uint8_t regulatoryLocation = to_underlying(app::Clusters::GeneralCommissioning::RegulatoryLocationType::kOutdoor);
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
        status = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to find country code, defaulting to WW");
        }
        chip::CharSpan countryCode(countryCodeStr, actualCountryCodeSize);

        GeneralCommissioning::Commands::SetRegulatoryConfig::Type request;
        request.location    = static_cast<GeneralCommissioning::RegulatoryLocationType>(regulatoryLocation);
        request.countryCode = countryCode;
        request.breadcrumb  = breadcrumb;
        request.timeoutMs   = kCommandTimeoutMs;
        SendCommand<GeneralCommissioningCluster>(proxy, request, OnSetRegulatoryConfigResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kSendPAICertificateRequest:
        ChipLogProgress(Controller, "Sending request for PAI certificate");
        SendCertificateChainRequestCommand(proxy, CertificateType::kPAI);
        break;
    case CommissioningStage::kSendDACCertificateRequest:
        ChipLogProgress(Controller, "Sending request for DAC certificate");
        SendCertificateChainRequestCommand(proxy, CertificateType::kDAC);
        break;
    case CommissioningStage::kSendAttestationRequest:
        ChipLogProgress(Controller, "Sending Attestation Request to the device.");
        if (!params.GetAttestationNonce().HasValue())
        {
            ChipLogError(Controller, "No attestation nonce found");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        SendAttestationRequestCommand(proxy, params.GetAttestationNonce().Value());
        break;
    case CommissioningStage::kAttestationVerification:
        ChipLogProgress(Controller, "Verifying attestation");
        if (!params.GetAttestationElements().HasValue() || !params.GetAttestationSignature().HasValue() ||
            !params.GetAttestationNonce().HasValue() || !params.GetDAC().HasValue() || !params.GetPAI().HasValue() ||
            !params.GetRemoteVendorId().HasValue() || !params.GetRemoteProductId().HasValue())
        {
            ChipLogError(Controller, "Missing attestation information");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        if (ValidateAttestationInfo(params.GetAttestationElements().Value(), params.GetAttestationSignature().Value(),
                                    params.GetAttestationNonce().Value(), params.GetPAI().Value(), params.GetDAC().Value(),
                                    params.GetRemoteVendorId().Value(), params.GetRemoteProductId().Value(),
                                    proxy) != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Error validating attestation information");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        break;
    case CommissioningStage::kSendOpCertSigningRequest:
        if (!params.GetCSRNonce().HasValue())
        {
            ChipLogError(Controller, "No CSR nonce found");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        SendOperationalCertificateSigningRequestCommand(proxy, params.GetCSRNonce().Value());
        break;
    case CommissioningStage::kGenerateNOCChain: {
        if (!params.GetNOCChainGenerationParameters().HasValue() || !params.GetDAC().HasValue() || !params.GetCSRNonce().HasValue())
        {
            ChipLogError(Controller, "Unable to generate NOC chain parameters");
            return CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
        }
        CHIP_ERROR err = ProcessCSR(proxy, params.GetNOCChainGenerationParameters().Value().nocsrElements,
                                    params.GetNOCChainGenerationParameters().Value().signature, params.GetDAC().Value(),
                                    params.GetCSRNonce().Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to process Op CSR");
            // Handle error, and notify session failure to the commissioner application.
            ChipLogError(Controller, "Failed to process the certificate signing request");
            // TODO: Map error status to correct error code
            CommissioningStageComplete(err);
            return;
        }
    }
    break;
    case CommissioningStage::kSendTrustedRootCert: {
        if (!params.GetRootCert().HasValue() || !params.GetNoc().HasValue())
        {
            ChipLogError(Controller, "No trusted root cert or NOC specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        CHIP_ERROR err = SendTrustedRootCertificate(proxy, params.GetRootCert().Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Error sending trusted root certificate: %s", err.AsString());
            CommissioningStageComplete(err);
            return;
        }
        err = proxy->SetPeerId(params.GetRootCert().Value(), params.GetNoc().Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Error setting peer id: %s", err.AsString());
            CommissioningStageComplete(err);
            return;
        }
        if (!IsOperationalNodeId(proxy->GetDeviceId()))
        {
            ChipLogError(Controller, "Given node ID is not an operational node ID");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
    }
    break;
    case CommissioningStage::kSendNOC:
        if (!params.GetNoc().HasValue() || !params.GetIcac().HasValue() || !params.GetIpk().HasValue() ||
            !params.GetAdminSubject().HasValue())
        {
            ChipLogError(Controller, "AddNOC contents not specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        ChipLogProgress(Controller, "Sending operational certificate chain to the device");
        SendOperationalCertificate(proxy, params.GetNoc().Value(), params.GetIcac().Value(), params.GetIpk().Value(),
                                   params.GetAdminSubject().Value());
        break;
    case CommissioningStage::kConfigACL:
        // TODO: Implement
        break;
    case CommissioningStage::kWiFiNetworkSetup: {
        if (!params.GetWiFiCredentials().HasValue())
        {
            ChipLogError(Controller, "No wifi credentials specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }

        ChipLogProgress(Controller, "Adding wifi network");
        NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type request;
        request.ssid        = params.GetWiFiCredentials().Value().ssid;
        request.credentials = params.GetWiFiCredentials().Value().credentials;
        request.breadcrumb  = breadcrumb;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnNetworkConfigResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kThreadNetworkSetup: {
        if (!params.GetThreadOperationalDataset().HasValue())
        {
            ChipLogError(Controller, "No thread credentials specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        ChipLogProgress(Controller, "Adding thread network");
        NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type request;
        request.operationalDataset = params.GetThreadOperationalDataset().Value();
        request.breadcrumb         = breadcrumb;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnNetworkConfigResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kWiFiNetworkEnable: {
        if (!params.GetWiFiCredentials().HasValue())
        {
            ChipLogError(Controller, "No wifi credentials specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        ChipLogProgress(Controller, "Enabling wifi network");
        NetworkCommissioning::Commands::ConnectNetwork::Type request;
        request.networkID  = params.GetWiFiCredentials().Value().ssid;
        request.breadcrumb = breadcrumb;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnConnectNetworkResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kThreadNetworkEnable: {
        ByteSpan extendedPanId;
        chip::Thread::OperationalDataset operationalDataset;
        if (!params.GetThreadOperationalDataset().HasValue() ||
            operationalDataset.Init(params.GetThreadOperationalDataset().Value()) != CHIP_NO_ERROR ||
            operationalDataset.GetExtendedPanIdAsByteSpan(extendedPanId) != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to get extended pan ID for thread operational dataset\n");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        ChipLogProgress(Controller, "Enabling thread network");
        NetworkCommissioning::Commands::ConnectNetwork::Type request;
        request.networkID  = extendedPanId;
        request.breadcrumb = breadcrumb;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnConnectNetworkResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kFindOperational: {
        CHIP_ERROR err = UpdateDevice(proxy->GetDeviceId());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to proceed to operational discovery\n");
            CommissioningStageComplete(err);
            return;
        }
    }
    break;
    case CommissioningStage::kSendComplete: {
        ChipLogProgress(Controller, "Calling commissioning complete");
        GeneralCommissioning::Commands::CommissioningComplete::Type request;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnCommissioningCompleteResponse, OnBasicFailure, endpoint,
                                                 timeout);
    }
    break;
    case CommissioningStage::kCleanup:
        ChipLogProgress(Controller, "Rendezvous cleanup");
        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnCommissioningComplete(proxy->GetDeviceId(), params.GetCompletionStatus());
        }
        CommissioningStageComplete(CHIP_NO_ERROR);
        mCommissioningStage = CommissioningStage::kSecurePairing;
        break;
    case CommissioningStage::kError:
        mCommissioningStage = CommissioningStage::kSecurePairing;
        break;
    case CommissioningStage::kSecurePairing:
        break;
    }
} // namespace Controller

} // namespace Controller
} // namespace chip
