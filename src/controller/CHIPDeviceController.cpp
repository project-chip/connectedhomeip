/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <controller-clusters/zap-generated/CHIPClusters.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#endif

#include <app/InteractionModelEngine.h>
#include <app/OperationalDeviceProxy.h>
#include <app/util/DataModelHandler.h>
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
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
using namespace chip::Protocols::UserDirectedCommissioning;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

constexpr uint32_t kSessionEstablishmentTimeout = 40 * kMillisecondsPerSecond;

DeviceController::DeviceController() :
    mOpenPairingSuccessCallback(OnOpenPairingWindowSuccessResponse, this),
    mOpenPairingFailureCallback(OnOpenPairingWindowFailureResponse, this)
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
    VerifyOrReturnError(params.systemState->InetLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStorageDelegate = params.storageDelegate;
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(params.systemState->BleLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#endif

    VerifyOrReturnError(params.systemState->TransportMgr() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    Dnssd::Resolver::Instance().Init(params.systemState->InetLayer());
    Dnssd::Resolver::Instance().SetResolverDelegate(this);
    RegisterDeviceAddressUpdateDelegate(params.deviceAddressUpdateDelegate);
    RegisterDeviceDiscoveryDelegate(params.deviceDiscoveryDelegate);
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    InitDataModelHandler(params.systemState->ExchangeMgr());

    VerifyOrReturnError(params.operationalCredentialsDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mOperationalCredentialsDelegate = params.operationalCredentialsDelegate;

    ReturnErrorOnFailure(ProcessControllerNOCChain(params));

    DeviceProxyInitParams deviceInitParams = {
        .sessionManager = params.systemState->SessionMgr(),
        .exchangeMgr    = params.systemState->ExchangeMgr(),
        .idAllocator    = &mIDAllocator,
        .fabricInfo     = params.systemState->Fabrics()->FindFabricWithIndex(mFabricIndex),
        .imDelegate     = params.systemState->IMDelegate(),
    };

    CASESessionManagerConfig sessionManagerConfig = {
        .sessionInitParams = deviceInitParams,
        .dnsCache          = &mDNSCache,
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

    ReturnErrorCodeIf(params.ephemeralKeypair == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    newFabric.SetEphemeralKey(params.ephemeralKeypair);

    constexpr uint32_t chipCertAllocatedLen = kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> chipCert;

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
    newFabric.SetVendorId(params.controllerVendorId);

    FabricInfo * fabric = params.systemState->Fabrics()->FindFabricWithIndex(mFabricIndex);
    ReturnErrorCodeIf(fabric == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(fabric->SetFabricInfo(newFabric));
    mLocalId  = fabric->GetPeerId();
    mVendorId = fabric->GetVendorId();

    mFabricId = fabric->GetFabricId();

    ChipLogProgress(Controller, "Joined the fabric at index %d. Compressed fabric ID is: 0x" ChipLogFormatX64, mFabricIndex,
                    ChipLogValueX64(GetCompressedFabricId()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::Shutdown()
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Controller, "Shutting down the controller");

    mState = State::NotInitialized;

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    Dnssd::Resolver::Instance().Shutdown();
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    mStorageDelegate = nullptr;

    mSystemState->Fabrics()->ReleaseFabricIndex(mFabricIndex);
    mSystemState->Release();
    mSystemState = nullptr;

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    Dnssd::Resolver::Instance().SetResolverDelegate(nullptr);
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
    VerifyOrReturn(mState == State::Initialized,
                   ChipLogError(Controller, "ReleaseOperationalDevice was called in incorrect state"));
    mCASESessionManager->ReleaseSession(remoteDeviceId);
}

void DeviceController::OnSessionReleased(SessionHandle session)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnConnectionExpired was called in incorrect state"));
    mCASESessionManager->OnSessionReleased(session);
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
    ReturnErrorOnFailure(mCASESessionManager->GetPeerAddress(peerId.GetNodeId(), peerAddr));
    addr = peerAddr.GetIPAddress();
    port = peerAddr.GetPort();
    return CHIP_NO_ERROR;
}

void DeviceController::OnOpenPairingWindowSuccessResponse(void * context)
{
    ChipLogProgress(Controller, "Successfully opened pairing window on the device");
    DeviceController * controller = reinterpret_cast<DeviceController *>(context);
    if (controller->mCommissioningWindowCallback != nullptr)
    {
        controller->mCommissioningWindowCallback->mCall(controller->mCommissioningWindowCallback->mContext,
                                                        controller->mDeviceWithCommissioningWindowOpen, CHIP_NO_ERROR,
                                                        controller->mSetupPayload);
    }
}

void DeviceController::OnOpenPairingWindowFailureResponse(void * context, uint8_t status)
{
    ChipLogError(Controller, "Failed to open pairing window on the device. Status %d", status);
    DeviceController * controller = reinterpret_cast<DeviceController *>(context);
    if (controller->mCommissioningWindowCallback != nullptr)
    {
        CHIP_ERROR error = CHIP_ERROR_INVALID_PASE_PARAMETER;
        // TODO - Use cluster enum chip::app::Clusters::AdministratorCommissioning::StatusCode::kBusy
        if (status == 1)
        {
            error = CHIP_ERROR_ANOTHER_COMMISSIONING_IN_PROGRESS;
        }
        controller->mCommissioningWindowCallback->mCall(controller->mCommissioningWindowCallback->mContext,
                                                        controller->mDeviceWithCommissioningWindowOpen, error, SetupPayload());
    }
}

CHIP_ERROR DeviceController::ComputePASEVerifier(uint32_t iterations, uint32_t setupPincode, const ByteSpan & salt,
                                                 PASEVerifier & outVerifier, uint32_t & outPasscodeId)
{
    ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(outVerifier, iterations, salt, /* useRandomPIN= */ false, setupPincode));

    outPasscodeId = mPAKEVerifierID++;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::OpenCommissioningWindowWithCallback(NodeId deviceId, uint16_t timeout, uint16_t iteration,
                                                                 uint16_t discriminator, uint8_t option,
                                                                 Callback::Callback<OnOpenCommissioningWindow> * callback)
{
    ChipLogProgress(Controller, "OpenCommissioningWindow for device ID %" PRIu64, deviceId);
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    OperationalDeviceProxy * device = mCASESessionManager->FindExistingSession(deviceId);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    std::string QRCode;
    std::string manualPairingCode;
    SetupPayload payload;
    CommissioningWindowOption commissioningWindowOption;
    ByteSpan salt(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt));

    payload.discriminator = discriminator;

    switch (option)
    {
    case 0:
        commissioningWindowOption = CommissioningWindowOption::kOriginalSetupCode;
        break;
    case 1:
        commissioningWindowOption = CommissioningWindowOption::kTokenWithRandomPIN;
        break;
    case 2:
        commissioningWindowOption = CommissioningWindowOption::kTokenWithProvidedPIN;
        break;
    default:
        ChipLogError(Controller, "Invalid Pairing Window option");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    constexpr EndpointId kAdministratorCommissioningClusterEndpoint = 0;

    chip::Controller::AdministratorCommissioningCluster cluster;
    cluster.Associate(device, kAdministratorCommissioningClusterEndpoint);

    Callback::Cancelable * successCallback = mOpenPairingSuccessCallback.Cancel();
    Callback::Cancelable * failureCallback = mOpenPairingFailureCallback.Cancel();

    payload.version               = 0;
    payload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork);

    mCommissioningWindowCallback = callback;
    if (commissioningWindowOption != CommissioningWindowOption::kOriginalSetupCode)
    {
        bool randomSetupPIN = (commissioningWindowOption == CommissioningWindowOption::kTokenWithRandomPIN);
        PASEVerifier verifier;

        ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(verifier, iteration, salt, randomSetupPIN, payload.setUpPINCode));

        uint8_t serializedVerifier[2 * kSpake2p_WS_Length];
        VerifyOrReturnError(sizeof(serializedVerifier) == sizeof(verifier), CHIP_ERROR_INTERNAL);

        memcpy(serializedVerifier, verifier.mW0, kSpake2p_WS_Length);
        memcpy(&serializedVerifier[kSpake2p_WS_Length], verifier.mL, kSpake2p_WS_Length);

        ReturnErrorOnFailure(cluster.OpenCommissioningWindow(successCallback, failureCallback, timeout,
                                                             ByteSpan(serializedVerifier, sizeof(serializedVerifier)),
                                                             payload.discriminator, iteration, salt, mPAKEVerifierID++));

        ReturnErrorOnFailure(ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode));
        ChipLogProgress(Controller, "Manual pairing code: [%s]", manualPairingCode.c_str());

        ReturnErrorOnFailure(QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(QRCode));
        ChipLogProgress(Controller, "SetupQRCode: [%s]", QRCode.c_str());
    }
    else
    {
        ReturnErrorOnFailure(cluster.OpenBasicCommissioningWindow(successCallback, failureCallback, timeout));
    }

    mSetupPayload                      = payload;
    mDeviceWithCommissioningWindowOpen = deviceId;

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
Transport::PeerAddress DeviceController::ToPeerAddress(const chip::Dnssd::ResolvedNodeData & nodeData) const
{
    Inet::InterfaceId interfaceId;

    // Only use the mDNS resolution's InterfaceID for addresses that are IPv6 LLA.
    // For all other addresses, we should rely on the device's routing table to route messages sent.
    // Forcing messages down an InterfaceId might fail. For example, in bridged networks like Thread,
    // mDNS advertisements are not usually received on the same interface the peer is reachable on.
    // TODO: Right now, just use addr0, but we should really push all the addresses and interfaces to
    // the device and allow it to make a proper decision about which addresses are preferred and reachable.
    if (nodeData.mAddress[0].IsIPv6LinkLocal())
    {
        interfaceId = nodeData.mInterfaceId;
    }

    return Transport::PeerAddress::UDP(nodeData.mAddress[0], nodeData.mPort, interfaceId);
}

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
        .transportMgr    = mSystemState->TransportMgr(),
        .sessionManager  = mSystemState->SessionMgr(),
        .exchangeMgr     = mSystemState->ExchangeMgr(),
        .inetLayer       = mSystemState->InetLayer(),
        .storageDelegate = mStorageDelegate,
        .idAllocator     = &mIDAllocator,
        .fabricsTable    = mSystemState->Fabrics(),
        .imDelegate      = mSystemState->IMDelegate(),
    };
}

DeviceCommissioner::DeviceCommissioner() :
    mSuccess(BasicSuccess, this), mFailure(BasicFailure, this), mCertificateChainResponseCallback(OnCertificateChainResponse, this),
    mAttestationResponseCallback(OnAttestationResponse, this), mOpCSRResponseCallback(OnOperationalCertificateSigningRequest, this),
    mNOCResponseCallback(OnOperationalCertificateAddResponse, this), mRootCertResponseCallback(OnRootCertSuccessResponse, this),
    mOnCertificateChainFailureCallback(OnCertificateChainFailureResponse, this),
    mOnAttestationFailureCallback(OnAttestationFailureResponse, this), mOnCSRFailureCallback(OnCSRFailureResponse, this),
    mOnCertFailureCallback(OnAddNOCFailureResponse, this), mOnRootCertFailureCallback(OnRootCertFailureResponse, this),
    mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this),
    mDeviceNOCChainCallback(OnDeviceNOCChainGeneration, this), mSetUpCodePairer(this)
{
    mPairingDelegate         = nullptr;
    mPairedDevicesUpdated    = false;
    mDeviceBeingCommissioned = nullptr;
}

CHIP_ERROR DeviceCommissioner::Init(CommissionerInitParams params)
{
    ReturnErrorOnFailure(DeviceController::Init(params));

    params.systemState->SessionMgr()->RegisterCreationDelegate(*this);
    params.systemState->SessionMgr()->RegisterReleaseDelegate(*this);

    uint16_t nextKeyID = 0;
    uint16_t size      = sizeof(nextKeyID);
    CHIP_ERROR error   = mStorageDelegate->SyncGetKeyValue(kNextAvailableKeyID, &nextKeyID, size);
    if ((error != CHIP_NO_ERROR) || (size != sizeof(nextKeyID)))
    {
        nextKeyID = 0;
    }
    ReturnErrorOnFailure(mIDAllocator.ReserveUpTo(nextKeyID));
    mPairingDelegate = params.pairingDelegate;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    mUdcTransportMgr = chip::Platform::New<DeviceIPTransportMgr>();
    ReturnErrorOnFailure(mUdcTransportMgr->Init(Transport::UdpListenParameters(mSystemState->InetLayer())
                                                    .SetAddressType(Inet::IPAddressType::kIPv6)
                                                    .SetListenPort((uint16_t)(mUdcListenPort))
#if INET_CONFIG_ENABLE_IPV4
                                                    ,
                                                Transport::UdpListenParameters(mSystemState->InetLayer())
                                                    .SetAddressType(Inet::IPAddressType::kIPv4)
                                                    .SetListenPort((uint16_t)(mUdcListenPort))
#endif // INET_CONFIG_ENABLE_IPV4
                                                    ));

    mUdcServer = chip::Platform::New<UserDirectedCommissioningServer>();
    mUdcTransportMgr->SetSessionManager(mUdcServer);

    mUdcServer->SetInstanceNameResolver(this);
    mUdcServer->SetUserConfirmationProvider(this);
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

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    if (mUdcTransportMgr != nullptr)
    {
        chip::Platform::Delete(mUdcTransportMgr);
        mUdcTransportMgr = nullptr;
    }
    if (mUdcServer != nullptr)
    {
        mUdcServer->SetInstanceNameResolver(nullptr);
        mUdcServer->SetUserConfirmationProvider(nullptr);
        chip::Platform::Delete(mUdcServer);
        mUdcServer = nullptr;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    DeviceController::Shutdown();
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnNewSession(SessionHandle session)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnNewConnection was called in incorrect state"));

    CommissioneeDeviceProxy * device =
        FindCommissioneeDevice(mSystemState->SessionMgr()->GetSecureSession(session)->GetPeerNodeId());
    VerifyOrReturn(device != nullptr, ChipLogDetail(Controller, "OnNewConnection was called for unknown device, ignoring it."));

    device->OnNewConnection(session);
}

void DeviceCommissioner::OnSessionReleased(SessionHandle session)
{
    VerifyOrReturn(mState == State::Initialized, ChipLogError(Controller, "OnConnectionExpired was called in incorrect state"));

    CommissioneeDeviceProxy * device = FindCommissioneeDevice(session);
    VerifyOrReturn(device != nullptr, ChipLogDetail(Controller, "OnConnectionExpired was called for unknown device, ignoring it."));

    device->OnSessionReleased(session);
}

CommissioneeDeviceProxy * DeviceCommissioner::FindCommissioneeDevice(SessionHandle session)
{
    CommissioneeDeviceProxy * foundDevice = nullptr;
    mCommissioneeDevicePool.ForEachActiveObject([&](auto * deviceProxy) {
        if (deviceProxy->MatchesSession(session))
        {
            foundDevice = deviceProxy;
            return false;
        }
        return true;
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
            return false;
        }
        return true;
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

CHIP_ERROR DeviceCommissioner::GetConnectedDevice(NodeId deviceId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                  Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    if (mDeviceBeingCommissioned != nullptr && mDeviceBeingCommissioned->GetDeviceId() == deviceId &&
        mDeviceBeingCommissioned->IsSecureConnected())
    {
        onConnection->mCall(onConnection->mContext, mDeviceBeingCommissioned);
        return CHIP_NO_ERROR;
    }
    return DeviceController::GetConnectedDevice(deviceId, onConnection, onFailure);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, const char * setUpCode)
{
    return mSetUpCodePairer.PairDevice(remoteDeviceId, setUpCode);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & params)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device   = nullptr;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);
    uint32_t mrpIdleInterval, mrpActiveInterval;

    Messaging::ExchangeContext * exchangeCtxt = nullptr;
    Optional<SessionHandle> session;

    uint16_t keyID = 0;

    FabricInfo * fabric = mSystemState->Fabrics()->FindFabricWithIndex(mFabricIndex);

    VerifyOrExit(IsOperationalNodeId(remoteDeviceId), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceBeingCommissioned == nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = InitializePairedDeviceList();
    SuccessOrExit(err);

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

    device = mCommissioneeDevicePool.CreateObject();
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mDeviceBeingCommissioned = device;

    // If the CSRNonce is passed in, using that else using a random one..
    if (params.HasCSRNonce())
    {
        ReturnErrorOnFailure(device->SetCSRNonce(params.GetCSRNonce().Value()));
    }
    else
    {
        uint8_t mCSRNonce[kOpCSRNonceLength];
        Crypto::DRBG_get_bytes(mCSRNonce, sizeof(mCSRNonce));
        ReturnErrorOnFailure(device->SetCSRNonce(ByteSpan(mCSRNonce)));
    }

    // If the AttestationNonce is passed in, using that else using a random one..
    if (params.HasAttestationNonce())
    {
        ReturnErrorOnFailure(device->SetAttestationNonce(params.GetAttestationNonce().Value()));
    }
    else
    {
        uint8_t mAttestationNonce[kAttestationNonceLength];
        Crypto::DRBG_get_bytes(mAttestationNonce, sizeof(mAttestationNonce));
        ReturnErrorOnFailure(device->SetAttestationNonce(ByteSpan(mAttestationNonce)));
    }

    mIsIPRendezvous = (params.GetPeerAddress().GetTransportType() != Transport::Type::kBle);

    device->Init(GetControllerDeviceInitParams(), remoteDeviceId, peerAddress, fabric->GetFabricIndex());

    err = device->GetPairing().MessageDispatch().Init(mSystemState->SessionMgr());
    SuccessOrExit(err);

    mSystemState->SystemLayer()->StartTimer(chip::System::Clock::Milliseconds32(kSessionEstablishmentTimeout),
                                            OnSessionEstablishmentTimeoutCallback, this);
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
    session = mSystemState->SessionMgr()->CreateUnauthenticatedSession(params.GetPeerAddress());
    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NO_MEMORY);

    // TODO: In some cases like PASE over IP, CRA and CRI values from commissionable node service should be used
    device->GetMRPIntervals(mrpIdleInterval, mrpActiveInterval);
    session.Value().GetUnauthenticatedSession()->SetMRPIntervals(mrpIdleInterval, mrpActiveInterval);

    exchangeCtxt = mSystemState->ExchangeMgr()->NewContext(session.Value(), &device->GetPairing());
    VerifyOrExit(exchangeCtxt != nullptr, err = CHIP_ERROR_INTERNAL);

    err = mIDAllocator.Allocate(keyID);
    SuccessOrExit(err);

    // TODO - Remove use of SetActive/IsActive from CommissioneeDeviceProxy
    device->SetActive(true);

    err = device->GetPairing().Pair(params.GetPeerAddress(), params.GetSetupPINCode(), keyID, exchangeCtxt, this);
    SuccessOrExit(err);

    // Immediately persist the updted mNextKeyID value
    // TODO maybe remove FreeRendezvousSession() since mNextKeyID is always persisted immediately
    PersistNextKeyId();

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

    if (mDeviceBeingCommissioned != nullptr && mIsIPRendezvous)
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
    mSystemState->SystemLayer()->CancelTimer(OnSessionEstablishmentTimeoutCallback, this);

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingFailed);
    }

    RendezvousCleanup(err);

    if (mDeviceBeingCommissioned != nullptr)
    {
        ReleaseCommissioneeDevice(mDeviceBeingCommissioned);
        mDeviceBeingCommissioned = nullptr;
    }
}

void DeviceCommissioner::OnSessionEstablished()
{
    VerifyOrReturn(mDeviceBeingCommissioned != nullptr, OnSessionEstablishmentError(CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR));

    PASESession * pairing = &mDeviceBeingCommissioned->GetPairing();

    // TODO: the session should know which peer we are trying to connect to when started
    pairing->SetPeerNodeId(mDeviceBeingCommissioned->GetDeviceId());

    CHIP_ERROR err = mSystemState->SessionMgr()->NewPairing(Optional<Transport::PeerAddress>::Value(pairing->GetPeerAddress()),
                                                            pairing->GetPeerNodeId(), pairing,
                                                            CryptoContext::SessionRole::kInitiator, mFabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake");

    // TODO: Add code to receive OpCSR from the device, and process the signing request
    // For IP rendezvous, this is sent as part of the state machine.
    bool usingLegacyFlowWithImmediateStart = !mIsIPRendezvous;

    if (usingLegacyFlowWithImmediateStart)
    {
        err = SendCertificateChainRequestCommand(mDeviceBeingCommissioned, CertificateType::kPAI);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Ble, "Failed in sending 'Certificate Chain request' command to the device: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
            return;
        }
    }
    else
    {
        AdvanceCommissioningStage(CHIP_NO_ERROR);
    }
}

CHIP_ERROR DeviceCommissioner::SendCertificateChainRequestCommand(CommissioneeDeviceProxy * device,
                                                                  Credentials::CertificateType certificateType)
{
    ChipLogDetail(Controller, "Sending Certificate Chain request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    mCertificateTypeBeingRequested = certificateType;

    Callback::Cancelable * successCallback = mCertificateChainResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnCertificateChainFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.CertificateChainRequest(successCallback, failureCallback, certificateType));
    ChipLogDetail(Controller, "Sent Certificate Chain request, waiting for the DAC Certificate");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCertificateChainFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the Certificate Chain request Response: 0x%02x", status);
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->mCertificateChainResponseCallback.Cancel();
    commissioner->mOnCertificateChainFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnCertificateChainResponse(void * context, ByteSpan certificate)
{
    ChipLogProgress(Controller, "Received certificate chain from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    commissioner->mCertificateChainResponseCallback.Cancel();
    commissioner->mOnCertificateChainFailureCallback.Cancel();

    if (commissioner->ProcessCertificateChain(certificate) != CHIP_NO_ERROR)
    {
        // Handle error, and notify session failure to the commissioner application.
        ChipLogError(Controller, "Failed to process the certificate chain request");
        // TODO: Map error status to correct error code
        commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
    }
}

CHIP_ERROR DeviceCommissioner::ProcessCertificateChain(const ByteSpan & certificate)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceBeingCommissioned != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;

    // PAI is being requested first - If PAI is not present, DAC will be requested next anyway.
    switch (mCertificateTypeBeingRequested)
    {
    case CertificateType::kDAC: {
        device->SetDAC(certificate);
        break;
    }
    case CertificateType::kPAI: {
        device->SetPAI(certificate);
        break;
    }
    case CertificateType::kUnknown:
    default: {
        return CHIP_ERROR_INTERNAL;
    }
    }

    if (device->AreCredentialsAvailable())
    {
        ChipLogProgress(Controller, "Sending Attestation Request to the device.");
        ReturnErrorOnFailure(SendAttestationRequestCommand(device, device->GetAttestationNonce()));
    }
    else
    {
        CHIP_ERROR err = SendCertificateChainRequestCommand(device, CertificateType::kDAC);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in sending Certificate Chain request command to the device: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::SendAttestationRequestCommand(CommissioneeDeviceProxy * device, const ByteSpan & attestationNonce)
{
    ChipLogDetail(Controller, "Sending Attestation request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mAttestationResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnAttestationFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.AttestationRequest(successCallback, failureCallback, attestationNonce));
    ChipLogDetail(Controller, "Sent Attestation request, waiting for the Attestation Information");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnAttestationFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the Attestation Information Response: 0x%02x", status);
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->mAttestationResponseCallback.Cancel();
    commissioner->mOnAttestationFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnAttestationResponse(void * context, chip::ByteSpan attestationElements, chip::ByteSpan signature)
{
    ChipLogProgress(Controller, "Received Attestation Information from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    commissioner->mAttestationResponseCallback.Cancel();
    commissioner->mOnAttestationFailureCallback.Cancel();

    commissioner->HandleAttestationResult(commissioner->ValidateAttestationInfo(attestationElements, signature));
}

CHIP_ERROR DeviceCommissioner::ValidateAttestationInfo(const ByteSpan & attestationElements, const ByteSpan & signature)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceBeingCommissioned != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;

    DeviceAttestationVerifier * dac_verifier = GetDeviceAttestationVerifier();

    PASESession * pairing = &mDeviceBeingCommissioned->GetPairing();

    // Retrieve attestation challenge
    ByteSpan attestationChallenge = mSystemState->SessionMgr()
                                        ->GetSecureSession({ pairing->GetPeerNodeId(), pairing->GetLocalSessionId(),
                                                             pairing->GetPeerSessionId(), mFabricIndex })
                                        ->GetCryptoContext()
                                        .GetAttestationChallenge();

    AttestationVerificationResult result = dac_verifier->VerifyAttestationInformation(
        attestationElements, attestationChallenge, signature, device->GetPAI(), device->GetDAC(), device->GetAttestationNonce());
    if (result != AttestationVerificationResult::kSuccess)
    {
        if (result == AttestationVerificationResult::kNotImplemented)
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device due to default "
                         "DeviceAttestationVerifier Class not being overriden by a real implementation.");
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
        else
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device: err %hu. Look at "
                         "AttestationVerificationResult enum to understand the errors",
                         static_cast<uint16_t>(result));
            // Go look at AttestationVerificationResult enum in src/credentials/DeviceAttestationVerifier.h to understand the
            // errors.
            return CHIP_ERROR_INTERNAL;
        }
    }

    ChipLogProgress(Controller, "Successfully validated 'Attestation Information' command received from the device.");

    // TODO: Validate Firmware Information

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::HandleAttestationResult(CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR)
    {
        // Here we assume the Attestation Information validation always succeeds.
        // Spec mandates that commissioning shall continue despite attestation fails (in some cases).
        // TODO: Handle failure scenarios where commissioning may progress regardless.
        ChipLogError(Controller, "Failed to validate the Attestation Information");
    }

    VerifyOrReturn(mState == State::Initialized);
    VerifyOrReturn(mDeviceBeingCommissioned != nullptr);

    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;

    ChipLogProgress(Controller, "Sending 'CSR request' command to the device.");
    CHIP_ERROR error = SendOperationalCertificateSigningRequestCommand(device);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in sending 'CSR request' command to the device: err %s", ErrorStr(error));
        OnSessionEstablishmentError(error);
        return;
    }
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificateSigningRequestCommand(CommissioneeDeviceProxy * device)
{
    ChipLogDetail(Controller, "Sending OpCSR request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mOpCSRResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnCSRFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.OpCSRRequest(successCallback, failureCallback, device->GetCSRNonce()));

    ChipLogDetail(Controller, "Sent OpCSR request, waiting for the CSR");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCSRFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the CSR request Response: 0x%02x", status);
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCSRFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnOperationalCertificateSigningRequest(void * context, ByteSpan NOCSRElements,
                                                                ByteSpan AttestationSignature)
{
    ChipLogProgress(Controller, "Received certificate signing request from the device");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCSRFailureCallback.Cancel();

    if (commissioner->ProcessOpCSR(NOCSRElements, AttestationSignature) != CHIP_NO_ERROR)
    {
        // Handle error, and notify session failure to the commissioner application.
        ChipLogError(Controller, "Failed to process the certificate signing request");
        // TODO: Map error status to correct error code
        commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
    }
}

void DeviceCommissioner::OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                                    const ByteSpan & rcac)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    ChipLogProgress(Controller, "Received callback from the CA for NOC Chain generation. Status %s", ErrorStr(status));
    CommissioneeDeviceProxy * device = nullptr;
    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(commissioner->mDeviceBeingCommissioned != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Check if the callback returned a failure
    VerifyOrExit(status == CHIP_NO_ERROR, err = status);

    // TODO - Verify that the generated root cert matches with commissioner's root cert

    device = commissioner->mDeviceBeingCommissioned;

    {
        // Reuse NOC Cert buffer for temporary store Root Cert.
        MutableByteSpan rootCert = device->GetMutableNOCCert();

        err = ConvertX509CertToChipCert(rcac, rootCert);
        SuccessOrExit(err);

        err = commissioner->SendTrustedRootCertificate(device, rootCert);
        SuccessOrExit(err);
    }

    if (!icac.empty())
    {
        MutableByteSpan icaCert = device->GetMutableICACert();

        err = ConvertX509CertToChipCert(icac, icaCert);
        SuccessOrExit(err);

        err = device->SetICACertBufferSize(icaCert.size());
        SuccessOrExit(err);
    }

    {
        MutableByteSpan nocCert = device->GetMutableNOCCert();

        err = ConvertX509CertToChipCert(noc, nocCert);
        SuccessOrExit(err);

        err = device->SetNOCCertBufferSize(nocCert.size());
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in generating device's operational credentials. Error %s", ErrorStr(err));
        commissioner->OnSessionEstablishmentError(err);
    }
}

CHIP_ERROR DeviceCommissioner::ProcessOpCSR(const ByteSpan & NOCSRElements, const ByteSpan & AttestationSignature)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceBeingCommissioned != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;

    ChipLogProgress(Controller, "Getting certificate chain for the device from the issuer");

    mOperationalCredentialsDelegate->SetNodeIdForNextNOCRequest(device->GetDeviceId());

    FabricInfo * fabric = mSystemState->Fabrics()->FindFabricWithIndex(mFabricIndex);
    mOperationalCredentialsDelegate->SetFabricIdForNextNOCRequest(fabric->GetFabricId());

    return mOperationalCredentialsDelegate->GenerateNOCChain(NOCSRElements, AttestationSignature, ByteSpan(), ByteSpan(),
                                                             ByteSpan(), &mDeviceNOCChainCallback);
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificate(CommissioneeDeviceProxy * device, const ByteSpan & nocCertBuf,
                                                          const ByteSpan & icaCertBuf)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mNOCResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnCertFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.AddNOC(successCallback, failureCallback, nocCertBuf, icaCertBuf, ByteSpan(nullptr, 0),
                                        mLocalId.GetNodeId(), mVendorId));

    ChipLogProgress(Controller, "Sent operational certificate to the device");

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::ConvertFromNodeOperationalCertStatus(uint8_t err)
{
    switch (err)
    {
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS:
        return CHIP_NO_ERROR;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_PUBLIC_KEY:
        return CHIP_ERROR_INVALID_PUBLIC_KEY;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_NODE_OP_ID:
        return CHIP_ERROR_WRONG_NODE_ID;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_NOC:
        return CHIP_ERROR_CERT_LOAD_FAILED;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_MISSING_CSR:
        return CHIP_ERROR_INCORRECT_STATE;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_TABLE_FULL:
        return CHIP_ERROR_NO_MEMORY;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INSUFFICIENT_PRIVILEGE:
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_FABRIC_CONFLICT:
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_LABEL_CONFLICT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_FABRIC_INDEX:
        return CHIP_ERROR_INVALID_FABRIC_ID;
    }

    return CHIP_ERROR_CERT_LOAD_FAILED;
}

void DeviceCommissioner::OnAddNOCFailureResponse(void * context, uint8_t status)
{
    ChipLogProgress(Controller, "Device failed to receive the operational certificate Response: 0x%02x", status);
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCertFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
}

void DeviceCommissioner::OnOperationalCertificateAddResponse(void * context, uint8_t StatusCode, uint8_t FabricIndex,
                                                             CharSpan DebugText)
{
    ChipLogProgress(Controller, "Device returned status %d on receiving the NOC", StatusCode);
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err                   = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device = nullptr;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    commissioner->mOpCSRResponseCallback.Cancel();
    commissioner->mOnCertFailureCallback.Cancel();

    VerifyOrExit(commissioner->mDeviceBeingCommissioned != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = ConvertFromNodeOperationalCertStatus(StatusCode);
    SuccessOrExit(err);

    device = commissioner->mDeviceBeingCommissioned;

    err = commissioner->OnOperationalCredentialsProvisioningCompletion(device);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Add NOC failed with error %s", ErrorStr(err));
        commissioner->OnSessionEstablishmentError(err);
    }
}

CHIP_ERROR DeviceCommissioner::SendTrustedRootCertificate(CommissioneeDeviceProxy * device, const ByteSpan & rcac)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(Controller, "Sending root certificate to the device");

    chip::Controller::OperationalCredentialsCluster cluster;
    cluster.Associate(device, 0);

    Callback::Cancelable * successCallback = mRootCertResponseCallback.Cancel();
    Callback::Cancelable * failureCallback = mOnRootCertFailureCallback.Cancel();

    ReturnErrorOnFailure(cluster.AddTrustedRootCertificate(successCallback, failureCallback, rcac));

    ChipLogProgress(Controller, "Sent root certificate to the device");

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnRootCertSuccessResponse(void * context)
{
    ChipLogProgress(Controller, "Device confirmed that it has received the root certificate");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err                   = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device = nullptr;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    commissioner->mRootCertResponseCallback.Cancel();
    commissioner->mOnRootCertFailureCallback.Cancel();

    VerifyOrExit(commissioner->mDeviceBeingCommissioned != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    device = commissioner->mDeviceBeingCommissioned;

    ChipLogProgress(Controller, "Sending operational certificate chain to the device");
    err = commissioner->SendOperationalCertificate(device, device->GetNOCCert(), device->GetICACert());
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
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->mRootCertResponseCallback.Cancel();
    commissioner->mOnRootCertFailureCallback.Cancel();
    // TODO: Map error status to correct error code
    commissioner->OnSessionEstablishmentError(CHIP_ERROR_INTERNAL);
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
    if (mIsIPRendezvous)
    {
        AdvanceCommissioningStage(CHIP_NO_ERROR);
    }
    else
    {
        RendezvousCleanup(CHIP_NO_ERROR);
    }

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
    return chip::Dnssd::Resolver::Instance().FindCommissionableNodes(filter);
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

void DeviceCommissioner::OnUserDirectedCommissioningRequest(UDCClientState state)
{
    ChipLogDetail(Controller, "------PROMPT USER!! OnUserDirectedCommissioningRequest instance=%s deviceName=%s",
                  state.GetInstanceName(), state.GetDeviceName());

    if (mUdcServer != nullptr)
    {
        mUdcServer->PrintUDCClients();
    }

    ChipLogDetail(Controller, "------To Accept Enter: discover udc-commission <pincode> <udc-client-index>");
}

void DeviceCommissioner::OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    if (mUdcServer != nullptr)
    {
        mUdcServer->OnCommissionableNodeFound(nodeData);
    }

    AbstractDnssdDiscoveryController::OnNodeDiscoveryComplete(nodeData);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

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

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
void DeviceCommissioner::OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "OperationalDiscoveryComplete for device ID 0x" ChipLogFormatX64,
                    ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
    VerifyOrReturn(mState == State::Initialized);

    if (mDeviceBeingCommissioned != nullptr && mDeviceBeingCommissioned->GetDeviceId() == nodeData.mPeerId.GetNodeId())
    {
        // Let's release the device that's being paired, if pairing was successful,
        // and the device is available on the operational network.
        ReleaseCommissioneeDevice(mDeviceBeingCommissioned);
        mDeviceBeingCommissioned = nullptr;
    }

    mDNSCache.Insert(nodeData);

    mCASESessionManager->FindOrEstablishSession(nodeData.mPeerId.GetNodeId(), &mOnDeviceConnectedCallback,
                                                &mOnDeviceConnectionFailureCallback);
    DeviceController::OnNodeIdResolved(nodeData);
}

void DeviceCommissioner::OnNodeIdResolutionFailed(const chip::PeerId & peer, CHIP_ERROR error)
{
    if (mDeviceBeingCommissioned != nullptr)
    {
        CommissioneeDeviceProxy * device = mDeviceBeingCommissioned;
        if (device->GetDeviceId() == peer.GetNodeId() && mCommissioningStage == CommissioningStage::kFindOperational)
        {
            OnSessionEstablishmentError(error);
        }
    }
    DeviceController::OnNodeIdResolutionFailed(peer, error);
}

#endif

void DeviceCommissioner::OnDeviceConnectedFn(void * context, DeviceProxy * device)
{
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    VerifyOrReturn(commissioner != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    if (commissioner->mIsIPRendezvous)
    {
        if (commissioner->mCommissioningStage == CommissioningStage::kFindOperational)
        {
            commissioner->mDeviceOperational = device;
            commissioner->AdvanceCommissioningStage(CHIP_NO_ERROR);
        }
        else
        {
            commissioner->mPairingDelegate->OnPairingComplete(CHIP_NO_ERROR);
        }
        // For IP rendezvous, we don't want to call commissioning complete below because IP commissioning
        // has more steps currently.
        return;
    }

    VerifyOrReturn(commissioner->mPairingDelegate != nullptr,
                   ChipLogProgress(Controller, "Device connected callback with null pairing delegate. Ignoring"));
    commissioner->mPairingDelegate->OnCommissioningComplete(device->GetDeviceId(), CHIP_NO_ERROR);
}

void DeviceCommissioner::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    ChipLogProgress(Controller, "Device connection failed. Error %s", ErrorStr(error));
    VerifyOrReturn(commissioner != nullptr,
                   ChipLogProgress(Controller, "Device connection failure callback with null context. Ignoring"));
    VerifyOrReturn(commissioner->mPairingDelegate != nullptr,
                   ChipLogProgress(Controller, "Device connection failure callback with null pairing delegate. Ignoring"));
    commissioner->mPairingDelegate->OnCommissioningComplete(deviceId, error);
}

CommissioningStage DeviceCommissioner::GetNextCommissioningStage()
{
    switch (mCommissioningStage)
    {
    case CommissioningStage::kSecurePairing:
        return CommissioningStage::kArmFailsafe;
    case CommissioningStage::kArmFailsafe:
        return CommissioningStage::kConfigRegulatory;
    case CommissioningStage::kConfigRegulatory:
        return CommissioningStage::kDeviceAttestation;
    case CommissioningStage::kDeviceAttestation:
        // TODO(cecille): device attestation casues operational cert provisioinging to happen, This should be a separate stage.
        // For thread and wifi, this should go to network setup then enable. For on-network we can skip right to finding the
        // operational network because the provisioning of certificates will trigger the device to start operational advertising.
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
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
    case CommissioningStage::kNetworkEnable:
    case CommissioningStage::kScanNetworks:
    case CommissioningStage::kCheckCertificates:
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
    if (nextStage == CommissioningStage::kSendComplete || nextStage == CommissioningStage::kCleanup)
    {
        if (mDeviceOperational == nullptr)
        {
            ChipLogError(Controller, "Invalid operational device for commissioning");
            return;
        }
    }
    else
    {
        if (mDeviceBeingCommissioned == nullptr)
        {
            ChipLogError(Controller, "Invalid commissionee device for commissioning");
            return;
        }
    }

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
        // TODO: should get the endpoint information from the descriptor cluster.
        genCom.Associate(mDeviceBeingCommissioned, 0);
        // TODO(cecille): Make this a parameter
        uint16_t commissioningExpirySeconds = 60;
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
        chip::CharSpan countryCode(countryCodeStr, actualCountryCodeSize);

        GeneralCommissioningCluster genCom;
        genCom.Associate(mDeviceBeingCommissioned, 0);
        genCom.SetRegulatoryConfig(mSuccess.Cancel(), mFailure.Cancel(), static_cast<uint8_t>(regulatoryLocation), countryCode,
                                   breadcrumb, kCommandTimeoutMs);
    }
    break;
    case CommissioningStage::kDeviceAttestation: {
        ChipLogProgress(Controller, "Exchanging vendor certificates");
        CHIP_ERROR status = SendCertificateChainRequestCommand(mDeviceBeingCommissioned, CertificateType::kPAI);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in sending 'Certificate Chain Request' command to the device: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
            return;
        }
    }
    break;
    case CommissioningStage::kCheckCertificates: {
        ChipLogProgress(Controller, "Exchanging certificates");
        // TODO(cecille): Once this is implemented through the clusters, it should be moved to the proper stage and the callback
        // should advance the commissioning stage
        CHIP_ERROR status = SendOperationalCertificateSigningRequestCommand(mDeviceBeingCommissioned);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in sending 'CSR Request' command to the device: err %s", ErrorStr(err));
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
        // For on-network, this is a NO-OP becuase we now start operational advertising once credentials are provisioned.
        // This is a placeholder for thread and wifi networks once that is implemented.
    }
    break;
    case CommissioningStage::kFindOperational: {
        PeerId peerId = PeerId().SetCompressedFabricId(GetCompressedFabricId()).SetNodeId(mDeviceBeingCommissioned->GetDeviceId());
        RendezvousCleanup(CHIP_NO_ERROR);
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
        ChipLogProgress(Controller, "Finding node on operational network");
        Dnssd::Resolver::Instance().ResolveNodeId(peerId, Inet::IPAddressType::kAny);
#endif
    }
    break;
    case CommissioningStage::kSendComplete: {
        ChipLogProgress(Controller, "Calling commissioning complete");
        GeneralCommissioningCluster genCom;
        genCom.Associate(mDeviceOperational, 0);
        genCom.CommissioningComplete(mSuccess.Cancel(), mFailure.Cancel());
    }
    break;
    case CommissioningStage::kCleanup:
        ChipLogProgress(Controller, "Rendezvous cleanup");
        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnCommissioningComplete(mDeviceOperational->GetDeviceId(), CHIP_NO_ERROR);
        }
        mDeviceOperational = nullptr;
        break;
    case CommissioningStage::kSecurePairing:
    case CommissioningStage::kError:
        break;
    }
    mCommissioningStage = nextStage;
}

} // namespace Controller
} // namespace chip
