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

#include <app/server/Dnssd.h>

#include <app/InteractionModelEngine.h>
#include <app/OperationalSessionSetup.h>
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
#include <platform/LockTracker.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <trace/trace.h>

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

DeviceController::DeviceController()
{
    mState = State::NotInitialized;
}

CHIP_ERROR DeviceController::Init(ControllerInitParams params)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mState == State::NotInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(params.systemState != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(params.systemState->SystemLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.systemState->UDPEndPointManager() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(params.systemState->BleLayer() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#endif

    VerifyOrReturnError(params.systemState->TransportMgr() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(mDNSResolver.Init(params.systemState->UDPEndPointManager()));
    mDNSResolver.SetCommissioningDelegate(this);
    RegisterDeviceDiscoveryDelegate(params.deviceDiscoveryDelegate);

    VerifyOrReturnError(params.operationalCredentialsDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mOperationalCredentialsDelegate = params.operationalCredentialsDelegate;

    mVendorId = params.controllerVendorId;
    if (params.operationalKeypair != nullptr || !params.controllerNOC.empty() || !params.controllerRCAC.empty())
    {
        ReturnErrorOnFailure(InitControllerNOCChain(params));

        if (params.enableServerInteractions)
        {
            //
            // Advertise our operational identity on the network to facilitate discovery by clients that look to
            // establish CASE with a controller that is also offering server-side capabilities (e.g an OTA provider).
            //
            app::DnssdServer::Instance().AdvertiseOperational();
        }
    }

    mSystemState = params.systemState->Retain();
    mState       = State::Initialized;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::InitControllerNOCChain(const ControllerInitParams & params)
{
    FabricInfo newFabric;
    constexpr uint32_t chipCertAllocatedLen = kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcacBuf;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icacBuf;
    chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
    Credentials::P256PublicKeySpan rootPublicKeySpan;
    FabricId fabricId;
    NodeId nodeId;
    bool hasExternallyOwnedKeypair                   = false;
    Crypto::P256Keypair * externalOperationalKeypair = nullptr;
    VendorId newFabricVendorId                       = params.controllerVendorId;

    // There are three possibilities here in terms of what happens with our
    // operational key:
    // 1) We have an externally owned operational keypair.
    // 2) We have an operational keypair that the fabric table should clone via
    //    serialize/deserialize.
    // 3) We have no keypair at all, and the fabric table has been initialized
    //    with a key store.
    if (params.operationalKeypair != nullptr)
    {
        hasExternallyOwnedKeypair  = params.hasExternallyOwnedOperationalKeypair;
        externalOperationalKeypair = params.operationalKeypair;
    }

    ReturnErrorCodeIf(!rcacBuf.Alloc(chipCertAllocatedLen), CHIP_ERROR_NO_MEMORY);
    ReturnErrorCodeIf(!icacBuf.Alloc(chipCertAllocatedLen), CHIP_ERROR_NO_MEMORY);
    ReturnErrorCodeIf(!nocBuf.Alloc(chipCertAllocatedLen), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan rcacSpan(rcacBuf.Get(), chipCertAllocatedLen);

    ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerRCAC, rcacSpan));
    ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(rcacSpan, rootPublicKeySpan));
    Crypto::P256PublicKey rootPublicKey{ rootPublicKeySpan };

    MutableByteSpan icacSpan;
    if (params.controllerICAC.empty())
    {
        ChipLogProgress(Controller, "Intermediate CA is not needed");
    }
    else
    {
        icacSpan = MutableByteSpan(icacBuf.Get(), chipCertAllocatedLen);
        ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerICAC, icacSpan));
    }

    MutableByteSpan nocSpan = MutableByteSpan(nocBuf.Get(), chipCertAllocatedLen);

    ReturnErrorOnFailure(ConvertX509CertToChipCert(params.controllerNOC, nocSpan));
    ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(nocSpan, &nodeId, &fabricId));

    auto * fabricTable            = params.systemState->Fabrics();
    const FabricInfo * fabricInfo = nullptr;

    //
    // When multiple controllers are permitted on the same fabric, we need to find fabrics with
    // nodeId as an extra discriminant since we can have multiple FabricInfo objects that all
    // collide on the same fabric. Not doing so may result in a match with an existing FabricInfo
    // instance that matches the fabric in the provided NOC but is associated with a different NodeId
    // that is already in use by another active controller instance. That will effectively cause it
    // to change its identity inadvertently, which is not acceptable.
    //
    // TODO: Figure out how to clean up unreclaimed FabricInfos restored from persistent
    //       storage that are not in use by active DeviceController instances. Also, figure out
    //       how to reclaim FabricInfo slots when a DeviceController instance is deleted.
    //
    if (params.permitMultiControllerFabrics)
    {
        fabricInfo = fabricTable->FindIdentity(rootPublicKey, fabricId, nodeId);
    }
    else
    {
        fabricInfo = fabricTable->FindFabric(rootPublicKey, fabricId);
    }

    bool fabricFoundInTable = (fabricInfo != nullptr);

    FabricIndex fabricIndex = fabricFoundInTable ? fabricInfo->GetFabricIndex() : kUndefinedFabricIndex;

    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // We permit colliding fabrics when multiple controllers are present on the same logical fabric
    // since each controller is associated with a unique FabricInfo 'identity' object and consequently,
    // a unique FabricIndex.
    //
    // This sets a flag that will be cleared automatically when the fabric is committed/reverted later
    // in this function.
    //
    if (params.permitMultiControllerFabrics)
    {
        fabricTable->PermitCollidingFabrics();
    }

    // We have 4 cases to handle legacy usage of direct operational key injection
    if (externalOperationalKeypair)
    {
        // Cases 1 and 2: Injected operational keys

        // CASE 1: Fabric update with injected key
        if (fabricFoundInTable)
        {
            err = fabricTable->UpdatePendingFabricWithProvidedOpKey(fabricIndex, nocSpan, icacSpan, externalOperationalKeypair,
                                                                    hasExternallyOwnedKeypair);
        }
        else
        // CASE 2: New fabric with injected key
        {
            err = fabricTable->AddNewPendingTrustedRootCert(rcacSpan);
            if (err == CHIP_NO_ERROR)
            {
                err = fabricTable->AddNewPendingFabricWithProvidedOpKey(
                    nocSpan, icacSpan, newFabricVendorId, externalOperationalKeypair, hasExternallyOwnedKeypair, &fabricIndex);
            }
        }
    }
    else
    {
        // Cases 3 and 4: OperationalKeystore has the keys

        // CASE 3: Fabric update with operational keystore
        if (fabricFoundInTable)
        {
            VerifyOrReturnError(fabricTable->HasOperationalKeyForFabric(fabricIndex), CHIP_ERROR_KEY_NOT_FOUND);

            err = fabricTable->UpdatePendingFabricWithOperationalKeystore(fabricIndex, nocSpan, icacSpan);
        }
        else
        // CASE 4: New fabric with operational keystore
        {
            err = fabricTable->AddNewPendingTrustedRootCert(rcacSpan);
            if (err == CHIP_NO_ERROR)
            {
                err = fabricTable->AddNewPendingFabricWithOperationalKeystore(nocSpan, icacSpan, newFabricVendorId, &fabricIndex);
            }

            if (err == CHIP_NO_ERROR)
            {
                // Now that we know our planned fabric index, verify that the
                // keystore has a key for it.
                if (!fabricTable->HasOperationalKeyForFabric(fabricIndex))
                {
                    err = CHIP_ERROR_KEY_NOT_FOUND;
                }
            }
        }
    }

    // Commit after setup, error-out on failure.
    if (err == CHIP_NO_ERROR)
    {
        // No need to revert on error: CommitPendingFabricData reverts internally on *any* error.
        err = fabricTable->CommitPendingFabricData();
    }
    else
    {
        fabricTable->RevertPendingFabricData();
    }

    ReturnErrorOnFailure(err);
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INTERNAL);

    mFabricIndex = fabricIndex;

    ChipLogProgress(Controller, "Joined the fabric at index %d. Compressed fabric ID is: 0x" ChipLogFormatX64, GetFabricIndex(),
                    ChipLogValueX64(GetCompressedFabricId()));

    return CHIP_NO_ERROR;
}

void DeviceController::Shutdown()
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mState != State::NotInitialized);

    ChipLogDetail(Controller, "Shutting down the controller");

    mState = State::NotInitialized;

    if (mFabricIndex != kUndefinedFabricIndex)
    {
        // Shut down any subscription clients for this fabric.
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mFabricIndex);

        // Shut down any ongoing CASE session activity we have.  We're going to
        // assume that all sessions for our fabric belong to us here.
        mSystemState->CASESessionMgr()->ReleaseSessionsForFabric(mFabricIndex);

        // TODO: The CASE session manager does not shut down existing CASE
        // sessions.  It just shuts down any ongoing CASE session establishment
        // we're in the middle of as initiator.  Maybe it should shut down
        // existing sessions too?
        mSystemState->SessionMgr()->ExpireAllSessionsForFabric(mFabricIndex);

        FabricTable * fabricTable = mSystemState->Fabrics();
        if (fabricTable != nullptr)
        {
            fabricTable->Forget(mFabricIndex);
        }
    }

    mSystemState->Release();
    mSystemState = nullptr;

    mDNSResolver.Shutdown();
    mDeviceDiscoveryDelegate = nullptr;
}

CHIP_ERROR DeviceController::GetPeerAddressAndPort(NodeId peerId, Inet::IPAddress & addr, uint16_t & port)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    Transport::PeerAddress peerAddr;
    ReturnErrorOnFailure(mSystemState->CASESessionMgr()->GetPeerAddress(GetPeerScopedId(peerId), peerAddr));
    addr = peerAddr.GetIPAddress();
    port = peerAddr.GetPort();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::GetPeerAddress(NodeId nodeId, Transport::PeerAddress & addr)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mSystemState->CASESessionMgr()->GetPeerAddress(GetPeerScopedId(nodeId), addr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceController::ComputePASEVerifier(uint32_t iterations, uint32_t setupPincode, const ByteSpan & salt,
                                                 Spake2pVerifier & outVerifier)
{
    ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(outVerifier, iterations, salt, /* useRandomPIN= */ false, setupPincode));

    return CHIP_NO_ERROR;
}

ControllerDeviceInitParams DeviceController::GetControllerDeviceInitParams()
{
    return ControllerDeviceInitParams{
        .sessionManager = mSystemState->SessionMgr(),
        .exchangeMgr    = mSystemState->ExchangeMgr(),
    };
}

DeviceCommissioner::DeviceCommissioner() :
    mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this),
    mDeviceAttestationInformationVerificationCallback(OnDeviceAttestationInformationVerification, this),
    mDeviceNOCChainCallback(OnDeviceNOCChainGeneration, this), mSetUpCodePairer(this)
{
    mPairingDelegate           = nullptr;
    mDeviceBeingCommissioned   = nullptr;
    mDeviceInPASEEstablishment = nullptr;
}

CHIP_ERROR DeviceCommissioner::Init(CommissionerInitParams params)
{
    ReturnErrorOnFailure(DeviceController::Init(params));

    mPairingDelegate = params.pairingDelegate;

    // Configure device attestation validation
    mDeviceAttestationVerifier = params.deviceAttestationVerifier;
    if (mDeviceAttestationVerifier == nullptr)
    {
        mDeviceAttestationVerifier = Credentials::GetDeviceAttestationVerifier();
        if (mDeviceAttestationVerifier == nullptr)
        {
            ChipLogError(Controller,
                         "Missing DeviceAttestationVerifier configuration at DeviceCommissioner init and none set with "
                         "Credentials::SetDeviceAttestationVerifier()!");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // We fell back on a default from singleton accessor.
        ChipLogProgress(Controller,
                        "*** Missing DeviceAttestationVerifier configuration at DeviceCommissioner init: using global default, "
                        "consider passing one in CommissionerInitParams.");
    }

    if (params.defaultCommissioner != nullptr)
    {
        mDefaultCommissioner = params.defaultCommissioner;
    }
    else
    {
        mDefaultCommissioner = &mAutoCommissioner;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    mUdcTransportMgr = chip::Platform::New<UdcTransportMgr>();
    ReturnErrorOnFailure(mUdcTransportMgr->Init(Transport::UdpListenParameters(mSystemState->UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv6)
                                                    .SetListenPort(static_cast<uint16_t>(mUdcListenPort))
#if INET_CONFIG_ENABLE_IPV4
                                                    ,
                                                Transport::UdpListenParameters(mSystemState->UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv4)
                                                    .SetListenPort(static_cast<uint16_t>(mUdcListenPort))
#endif // INET_CONFIG_ENABLE_IPV4
                                                    ));

    mUdcServer = chip::Platform::New<UserDirectedCommissioningServer>();
    mUdcTransportMgr->SetSessionManager(mUdcServer);

    mUdcServer->SetInstanceNameResolver(this);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    mSetUpCodePairer.SetSystemLayer(mSystemState->SystemLayer());
#if CONFIG_NETWORK_LAYER_BLE
    mSetUpCodePairer.SetBleLayer(mSystemState->BleLayer());
#endif // CONFIG_NETWORK_LAYER_BLE

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::Shutdown()
{
    VerifyOrReturn(mState != State::NotInitialized);

    ChipLogDetail(Controller, "Shutting down the commissioner");

    mSetUpCodePairer.CommissionerShuttingDown();

    // Check to see if pairing in progress before shutting down
    CommissioneeDeviceProxy * device = mDeviceInPASEEstablishment;
    if (device != nullptr && device->IsSessionSetupInProgress())
    {
        ChipLogDetail(Controller, "Setup in progress, stopping setup before shutting down");
        OnSessionEstablishmentError(CHIP_ERROR_CONNECTION_ABORTED);
    }
    // TODO: If we have a commissioning step in progress, is there a way to cancel that callback?

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

    // Release everything from the commissionee device pool here.
    // Make sure to use ReleaseCommissioneeDevice so we don't keep dangling
    // pointers to the device objects.
    mCommissioneeDevicePool.ForEachActiveObject([this](auto * commissioneeDevice) {
        ReleaseCommissioneeDevice(commissioneeDevice);
        return Loop::Continue;
    });

    DeviceController::Shutdown();
}

CommissioneeDeviceProxy * DeviceCommissioner::FindCommissioneeDevice(NodeId id)
{
    MATTER_TRACE_EVENT_SCOPE("FindCommissioneeDevice", "DeviceCommissioner");
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

CommissioneeDeviceProxy * DeviceCommissioner::FindCommissioneeDevice(const Transport::PeerAddress & peerAddress)
{
    CommissioneeDeviceProxy * foundDevice = nullptr;
    mCommissioneeDevicePool.ForEachActiveObject([&](auto * deviceProxy) {
        if (deviceProxy->GetPeerAddress() == peerAddress)
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
#if CONFIG_NETWORK_LAYER_BLE
    if (mSystemState->BleLayer() != nullptr && device->GetDeviceTransportType() == Transport::Type::kBle)
    {
        // We only support one BLE connection, so if this is BLE, close it
        ChipLogProgress(Discovery, "Closing all BLE connections");
        mSystemState->BleLayer()->CloseAllBleConnections();
    }
#endif
    // Make sure that there will be no dangling pointer
    if (mDeviceInPASEEstablishment == device)
    {
        mDeviceInPASEEstablishment = nullptr;
    }
    if (mDeviceBeingCommissioned == device)
    {
        mDeviceBeingCommissioned = nullptr;
    }

    // Release the commissionee device after we have nulled out our pointers,
    // because that can call back in to us with error notifications as the
    // session is released.
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

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, const char * setUpCode, const CommissioningParameters & params,
                                          DiscoveryType discoveryType)
{
    MATTER_TRACE_EVENT_SCOPE("PairDevice", "DeviceCommissioner");
    if (mDefaultCommissioner == nullptr)
    {
        ChipLogError(Controller, "No default commissioner is specified");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    ReturnErrorOnFailure(mDefaultCommissioner->SetCommissioningParameters(params));

    return mSetUpCodePairer.PairDevice(remoteDeviceId, setUpCode, SetupCodePairerBehaviour::kCommission, discoveryType);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, const char * setUpCode, DiscoveryType discoveryType)
{
    MATTER_TRACE_EVENT_SCOPE("PairDevice", "DeviceCommissioner");
    return mSetUpCodePairer.PairDevice(remoteDeviceId, setUpCode, SetupCodePairerBehaviour::kCommission, discoveryType);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & params)
{
    MATTER_TRACE_EVENT_SCOPE("PairDevice", "DeviceCommissioner");
    ReturnErrorOnFailure(EstablishPASEConnection(remoteDeviceId, params));
    return Commission(remoteDeviceId);
}

CHIP_ERROR DeviceCommissioner::PairDevice(NodeId remoteDeviceId, RendezvousParameters & rendezvousParams,
                                          CommissioningParameters & commissioningParams)
{
    MATTER_TRACE_EVENT_SCOPE("PairDevice", "DeviceCommissioner");
    ReturnErrorOnFailure(EstablishPASEConnection(remoteDeviceId, rendezvousParams));
    return Commission(remoteDeviceId, commissioningParams);
}

CHIP_ERROR DeviceCommissioner::EstablishPASEConnection(NodeId remoteDeviceId, const char * setUpCode, DiscoveryType discoveryType)
{
    MATTER_TRACE_EVENT_SCOPE("EstablishPASEConnection", "DeviceCommissioner");
    return mSetUpCodePairer.PairDevice(remoteDeviceId, setUpCode, SetupCodePairerBehaviour::kPaseOnly, discoveryType);
}

CHIP_ERROR DeviceCommissioner::EstablishPASEConnection(NodeId remoteDeviceId, RendezvousParameters & params)
{
    MATTER_TRACE_EVENT_SCOPE("EstablishPASEConnection", "DeviceCommissioner");
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    CommissioneeDeviceProxy * device   = nullptr;
    CommissioneeDeviceProxy * current  = nullptr;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    Messaging::ExchangeContext * exchangeCtxt = nullptr;
    Optional<SessionHandle> session;

    VerifyOrExit(mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeviceInPASEEstablishment == nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // TODO(#13940): We need to specify the peer address for BLE transport in bindings.
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle ||
        params.GetPeerAddress().GetTransportType() == Transport::Type::kUndefined)
    {
#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
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

    current = FindCommissioneeDevice(peerAddress);
    if (current != nullptr)
    {
        if (current->GetDeviceId() == remoteDeviceId)
        {
            // We might be able to just reuse its connection if it has one or is
            // working on one.
            if (current->IsSecureConnected())
            {
                if (mPairingDelegate)
                {
                    // We already have an open secure session to this device, call the callback immediately and early return.
                    mPairingDelegate->OnPairingComplete(CHIP_NO_ERROR);
                }
                return CHIP_NO_ERROR;
            }
            if (current->IsSessionSetupInProgress())
            {
                // We're not connected yet, but we're in the process of connecting. Pairing delegate will get a callback when
                // connection completes
                return CHIP_NO_ERROR;
            }
        }

        // Either the consumer wants to assign a different device id to this
        // peer address now (so we can't reuse the commissionee device we have
        // already) or something has gone strange. Delete the old device, try
        // again.
        ChipLogError(Controller, "Found unconnected device, removing");
        ReleaseCommissioneeDevice(current);
    }

    device = mCommissioneeDevicePool.CreateObject();
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mDeviceInPASEEstablishment = device;
    device->Init(GetControllerDeviceInitParams(), remoteDeviceId, peerAddress);

#if CONFIG_NETWORK_LAYER_BLE
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
    {
        if (params.HasConnectionObject())
        {
            SuccessOrExit(err = mSystemState->BleLayer()->NewBleConnectionByObject(params.GetConnectionObject()));
        }
        else if (params.HasDiscriminator())
        {
            SetupDiscriminator discriminator;
            discriminator.SetLongValue(params.GetDiscriminator());
            SuccessOrExit(err = mSystemState->BleLayer()->NewBleConnectionByDiscriminator(discriminator));
        }
        else
        {
            ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
#endif
    // TODO: In some cases like PASE over IP, SAI and SII values from commissionable node service should be used
    session = mSystemState->SessionMgr()->CreateUnauthenticatedSession(params.GetPeerAddress(), device->GetRemoteMRPConfig());
    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NO_MEMORY);

    // Allocate the exchange immediately before calling PASESession::Pair.
    //
    // PASESession::Pair takes ownership of the exchange and will free it on
    // error, but can only do this if it is actually called.  Allocating the
    // exchange context right before calling Pair ensures that if allocation
    // succeeds, PASESession has taken ownership.
    exchangeCtxt = mSystemState->ExchangeMgr()->NewContext(session.Value(), &device->GetPairing());
    VerifyOrExit(exchangeCtxt != nullptr, err = CHIP_ERROR_INTERNAL);

    err = device->GetPairing().Pair(*mSystemState->SessionMgr(), params.GetSetupPINCode(), GetLocalMRPConfig(), exchangeCtxt, this);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (device != nullptr)
        {
            ReleaseCommissioneeDevice(device);
        }
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::Commission(NodeId remoteDeviceId, CommissioningParameters & params)
{
    if (mDefaultCommissioner == nullptr)
    {
        ChipLogError(Controller, "No default commissioner is specified");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    ReturnErrorOnFailure(mDefaultCommissioner->SetCommissioningParameters(params));
    return Commission(remoteDeviceId);
}

CHIP_ERROR DeviceCommissioner::Commission(NodeId remoteDeviceId)
{
    MATTER_TRACE_EVENT_SCOPE("Commission", "DeviceCommissioner");
    CommissioneeDeviceProxy * device = FindCommissioneeDevice(remoteDeviceId);
    if (device == nullptr || (!device->IsSecureConnected() && !device->IsSessionSetupInProgress()))
    {
        ChipLogError(Controller, "Invalid device for commissioning " ChipLogFormatX64, ChipLogValueX64(remoteDeviceId));
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (!device->IsSecureConnected() && device != mDeviceInPASEEstablishment)
    {
        // We should not end up in this state because we won't attempt to establish more than one connection at a time.
        ChipLogError(Controller, "Device is not connected and not being paired " ChipLogFormatX64, ChipLogValueX64(remoteDeviceId));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mCommissioningStage != CommissioningStage::kSecurePairing)
    {
        ChipLogError(Controller, "Commissioning already in progress - not restarting");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mDefaultCommissioner == nullptr)
    {
        ChipLogError(Controller, "No default commissioner is specified");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(Controller, "Commission called for node ID 0x" ChipLogFormatX64, ChipLogValueX64(remoteDeviceId));

    mDefaultCommissioner->SetOperationalCredentialsDelegate(mOperationalCredentialsDelegate);
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

CHIP_ERROR
DeviceCommissioner::ContinueCommissioningAfterDeviceAttestation(DeviceProxy * device,
                                                                Credentials::AttestationVerificationResult attestationResult)
{
    MATTER_TRACE_EVENT_SCOPE("continueCommissioningDevice", "DeviceCommissioner");
    if (device == nullptr || device != mDeviceBeingCommissioned)
    {
        ChipLogError(Controller, "Invalid device for commissioning %p", device);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    CommissioneeDeviceProxy * commissioneeDevice = FindCommissioneeDevice(device->GetDeviceId());
    if (commissioneeDevice == nullptr || !commissioneeDevice->IsSecureConnected() || commissioneeDevice != mDeviceBeingCommissioned)
    {
        ChipLogError(Controller, "Invalid device for commissioning after attestation failure: 0x" ChipLogFormatX64,
                     ChipLogValueX64(commissioneeDevice->GetDeviceId()));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mCommissioningStage != CommissioningStage::kAttestationVerification)
    {
        ChipLogError(Controller, "Commissioning is not attestation verification phase");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mDefaultCommissioner == nullptr)
    {
        ChipLogError(Controller, "No default commissioner is specified");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(Controller, "Continuing commissioning after attestation failure for device ID 0x" ChipLogFormatX64,
                    ChipLogValueX64(commissioneeDevice->GetDeviceId()));

    if (attestationResult != AttestationVerificationResult::kSuccess)
    {
        ChipLogError(Controller, "Client selected error: %u for failed 'Attestation Information' for device",
                     to_underlying(attestationResult));

        CommissioningDelegate::CommissioningReport report;
        report.Set<AttestationErrorInfo>(attestationResult);
        CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
    }
    else
    {
        ChipLogProgress(Controller, "Overriding attestation failure per client and continuing commissioning");
        CommissioningStageComplete(CHIP_NO_ERROR);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::StopPairing(NodeId remoteDeviceId)
{
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    CommissioneeDeviceProxy * device = FindCommissioneeDevice(remoteDeviceId);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    ReleaseCommissioneeDevice(device);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::UnpairDevice(NodeId remoteDeviceId)
{
    // TODO: Send unpairing message to the remote device.
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::RendezvousCleanup(CHIP_ERROR status)
{
    if (mDeviceInPASEEstablishment != nullptr)
    {
        // Release the commissionee device. For BLE, this is stored,
        // for IP commissioning, we have taken a reference to the
        // operational node to send the completion command.
        ReleaseCommissioneeDevice(mDeviceInPASEEstablishment);

        if (mPairingDelegate != nullptr)
        {
            mPairingDelegate->OnPairingComplete(status);
        }
    }
}

void DeviceCommissioner::OnSessionEstablishmentError(CHIP_ERROR err)
{
    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingFailed);
    }

    RendezvousCleanup(err);
}

void DeviceCommissioner::OnSessionEstablished(const SessionHandle & session)
{
    // PASE session established.
    CommissioneeDeviceProxy * device = mDeviceInPASEEstablishment;

    // We are in the callback for this pairing. Reset so we can pair another device.
    mDeviceInPASEEstablishment = nullptr;

    VerifyOrReturn(device != nullptr, OnSessionEstablishmentError(CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR));

    CHIP_ERROR err = device->SetConnected(session);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogDetail(Controller, "Remote device completed SPAKE2+ handshake");

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnPairingComplete(CHIP_NO_ERROR);
    }

    if (mRunCommissioningAfterConnection)
    {
        mRunCommissioningAfterConnection = false;
        mDefaultCommissioner->StartCommissioning(this, device);
    }
}

CHIP_ERROR DeviceCommissioner::SendCertificateChainRequestCommand(DeviceProxy * device,
                                                                  Credentials::CertificateType certificateType,
                                                                  Optional<System::Clock::Timeout> timeout)
{
    MATTER_TRACE_EVENT_SCOPE("SendCertificateChainRequestCommand", "DeviceCommissioner");
    ChipLogDetail(Controller, "Sending Certificate Chain request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::CertificateChainRequest::Type request;
    request.certificateType = certificateType;
    return SendCommand<OperationalCredentialsCluster>(device, request, OnCertificateChainResponse,
                                                      OnCertificateChainFailureResponse, timeout);

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCertificateChainFailureResponse(void * context, CHIP_ERROR error)
{
    MATTER_TRACE_EVENT_SCOPE("OnCertificateChainFailureResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device failed to receive the Certificate Chain request Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnCertificateChainResponse(
    void * context, const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType & response)
{
    MATTER_TRACE_EVENT_SCOPE("OnCertificateChainResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Received certificate chain from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<RequestedCertificate>(RequestedCertificate(response.certificate));

    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

CHIP_ERROR DeviceCommissioner::SendAttestationRequestCommand(DeviceProxy * device, const ByteSpan & attestationNonce,
                                                             Optional<System::Clock::Timeout> timeout)
{
    MATTER_TRACE_EVENT_SCOPE("SendAttestationRequestCommand", "DeviceCommissioner");
    ChipLogDetail(Controller, "Sending Attestation request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::AttestationRequest::Type request;
    request.attestationNonce = attestationNonce;

    ReturnErrorOnFailure(
        SendCommand<OperationalCredentialsCluster>(device, request, OnAttestationResponse, OnAttestationFailureResponse, timeout));
    ChipLogDetail(Controller, "Sent Attestation request, waiting for the Attestation Information");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnAttestationFailureResponse(void * context, CHIP_ERROR error)
{
    MATTER_TRACE_EVENT_SCOPE("OnAttestationFailureResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device failed to receive the Attestation Information Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnAttestationResponse(void * context,
                                               const OperationalCredentials::Commands::AttestationResponse::DecodableType & data)
{
    MATTER_TRACE_EVENT_SCOPE("OnAttestationResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Received Attestation Information from the device");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<AttestationResponse>(AttestationResponse(data.attestationElements, data.signature));
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

void DeviceCommissioner::OnDeviceAttestationInformationVerification(
    void * context, const Credentials::DeviceAttestationVerifier::AttestationInfo & info, AttestationVerificationResult result)
{
    MATTER_TRACE_EVENT_SCOPE("OnDeviceAttestationInformationVerification", "DeviceCommissioner");
    DeviceCommissioner * commissioner = reinterpret_cast<DeviceCommissioner *>(context);

    if (!commissioner->mDeviceBeingCommissioned)
    {
        ChipLogError(Controller, "Device attestation verification result received when we're not commissioning a device");
        return;
    }

    auto & params = commissioner->mDefaultCommissioner->GetCommissioningParameters();
    Credentials::DeviceAttestationDelegate * deviceAttestationDelegate = params.GetDeviceAttestationDelegate();

    if (result != AttestationVerificationResult::kSuccess)
    {
        CommissioningDelegate::CommissioningReport report;
        report.Set<AttestationErrorInfo>(result);
        if (result == AttestationVerificationResult::kNotImplemented)
        {
            ChipLogError(Controller,
                         "Failed in verifying 'Attestation Information' command received from the device due to default "
                         "DeviceAttestationVerifier Class not being overridden by a real implementation.");
            commissioner->CommissioningStageComplete(CHIP_ERROR_NOT_IMPLEMENTED, report);
            return;
        }

        ChipLogError(Controller,
                     "Failed in verifying 'Attestation Information' command received from the device: err %hu. Look at "
                     "AttestationVerificationResult enum to understand the errors",
                     static_cast<uint16_t>(result));
        // Go look at AttestationVerificationResult enum in src/credentials/attestation_verifier/DeviceAttestationVerifier.h to
        // understand the errors.

        // If a device attestation status delegate is installed, delegate handling of failure to the client and let them
        // decide on whether to proceed further or not.
        if (deviceAttestationDelegate)
        {
            commissioner->ExtendArmFailSafeForDeviceAttestation(info, result);
        }
        else
        {
            commissioner->CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
        }
    }
    else
    {
        if (deviceAttestationDelegate && deviceAttestationDelegate->ShouldWaitAfterDeviceAttestation())
        {
            commissioner->ExtendArmFailSafeForDeviceAttestation(info, result);
        }
        else
        {
            ChipLogProgress(Controller, "Successfully validated 'Attestation Information' command received from the device.");
            commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
        }
    }
}

void DeviceCommissioner::OnArmFailSafeExtendedForDeviceAttestation(
    void * context, const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    // If this function starts using "data", need to fix ExtendArmFailSafeForDeviceAttestation accordingly.
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    if (!commissioner->mDeviceBeingCommissioned)
    {
        return;
    }

    auto & params = commissioner->mDefaultCommissioner->GetCommissioningParameters();
    Credentials::DeviceAttestationDelegate * deviceAttestationDelegate = params.GetDeviceAttestationDelegate();
    if (deviceAttestationDelegate)
    {
        ChipLogProgress(Controller, "Device attestation completed, delegating continuation to client");
        deviceAttestationDelegate->OnDeviceAttestationCompleted(commissioner, commissioner->mDeviceBeingCommissioned,
                                                                *commissioner->mAttestationDeviceInfo,
                                                                commissioner->mAttestationResult);
    }
    else
    {
        ChipLogProgress(Controller, "Device attestation failed and no delegate set, failing commissioning");
        CommissioningDelegate::CommissioningReport report;
        report.Set<AttestationErrorInfo>(commissioner->mAttestationResult);
        commissioner->CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
    }
}

void DeviceCommissioner::OnFailedToExtendedArmFailSafeDeviceAttestation(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Failed to extend fail-safe timer to handle attestation failure %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<AttestationErrorInfo>(commissioner->mAttestationResult);
    commissioner->CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
}

void DeviceCommissioner::ExtendArmFailSafeForDeviceAttestation(const Credentials::DeviceAttestationVerifier::AttestationInfo & info,
                                                               Credentials::AttestationVerificationResult result)
{
    mAttestationResult = result;

    auto & params                                                      = mDefaultCommissioner->GetCommissioningParameters();
    Credentials::DeviceAttestationDelegate * deviceAttestationDelegate = params.GetDeviceAttestationDelegate();

    mAttestationDeviceInfo = Platform::MakeUnique<Credentials::DeviceAttestationVerifier::AttestationDeviceInfo>(info);

    auto expiryLengthSeconds = deviceAttestationDelegate->FailSafeExpiryTimeoutSecs();
    if (expiryLengthSeconds.HasValue())
    {
        GeneralCommissioning::Commands::ArmFailSafe::Type request;
        request.expiryLengthSeconds = expiryLengthSeconds.Value();
        request.breadcrumb          = mCommissioningStage;
        ChipLogProgress(Controller, "Changing fail-safe timer to %u seconds to handle DA failure", request.expiryLengthSeconds);
        // Per spec, anything we do with the fail-safe armed must not time out
        // in less than kMinimumCommissioningStepTimeout.
        SendCommand<GeneralCommissioningCluster>(mDeviceBeingCommissioned, request, OnArmFailSafeExtendedForDeviceAttestation,
                                                 OnFailedToExtendedArmFailSafeDeviceAttestation,
                                                 MakeOptional(kMinimumCommissioningStepTimeout));
    }
    else
    {
        ChipLogProgress(Controller, "Proceeding without changing fail-safe timer value as delegate has not set it");
        // Callee does not use data argument.
        const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType data;
        OnArmFailSafeExtendedForDeviceAttestation(this, data);
    }
}

CHIP_ERROR DeviceCommissioner::ValidateAttestationInfo(const Credentials::DeviceAttestationVerifier::AttestationInfo & info)
{
    MATTER_TRACE_EVENT_SCOPE("ValidateAttestationInfo", "DeviceCommissioner");
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceAttestationVerifier != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mDeviceAttestationVerifier->VerifyAttestationInformation(info, &mDeviceAttestationInformationVerificationCallback);

    // TODO: Validate Firmware Information

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::ValidateCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements,
                                           const ByteSpan & AttestationSignature, const ByteSpan & dac, const ByteSpan & csrNonce)
{
    MATTER_TRACE_EVENT_SCOPE("ValidateCSR", "DeviceCommissioner");
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDeviceAttestationVerifier != nullptr, CHIP_ERROR_INCORRECT_STATE);

    P256PublicKey dacPubkey;
    ReturnErrorOnFailure(ExtractPubkeyFromX509Cert(dac, dacPubkey));

    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        proxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    // The operational CA should also verify this on its end during NOC generation, if end-to-end attestation is desired.
    return mDeviceAttestationVerifier->VerifyNodeOperationalCSRInformation(NOCSRElements, attestationChallenge,
                                                                           AttestationSignature, dacPubkey, csrNonce);
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificateSigningRequestCommand(DeviceProxy * device, const ByteSpan & csrNonce,
                                                                               Optional<System::Clock::Timeout> timeout)
{
    MATTER_TRACE_EVENT_SCOPE("SendOperationalCertificateSigningRequestCommand", "DeviceCommissioner");
    ChipLogDetail(Controller, "Sending CSR request to %p device", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::CSRRequest::Type request;
    request.CSRNonce = csrNonce;

    ReturnErrorOnFailure(SendCommand<OperationalCredentialsCluster>(device, request, OnOperationalCertificateSigningRequest,
                                                                    OnCSRFailureResponse, timeout));
    ChipLogDetail(Controller, "Sent CSR request, waiting for the CSR");
    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnCSRFailureResponse(void * context, CHIP_ERROR error)
{
    MATTER_TRACE_EVENT_SCOPE("OnCSRFailureResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device failed to receive the CSR request Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnOperationalCertificateSigningRequest(
    void * context, const OperationalCredentials::Commands::CSRResponse::DecodableType & data)
{
    MATTER_TRACE_EVENT_SCOPE("OnOperationalCertificateSigningRequest", "DeviceCommissioner");
    ChipLogProgress(Controller, "Received certificate signing request from the device");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CommissioningDelegate::CommissioningReport report;
    report.Set<CSRResponse>(CSRResponse(data.NOCSRElements, data.attestationSignature));
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
}

void DeviceCommissioner::OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                                    const ByteSpan & rcac, Optional<AesCcm128KeySpan> ipk,
                                                    Optional<NodeId> adminSubject)
{
    MATTER_TRACE_EVENT_SCOPE("OnDeviceNOCChainGeneration", "DeviceCommissioner");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    // The placeholder IPK is not satisfactory, but is there to fill the NocChain struct on error. It will still fail.
    const uint8_t placeHolderIpk[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    if (!ipk.HasValue())
    {
        ChipLogError(Controller, "Did not have an IPK from the OperationalCredentialsIssuer! Cannot commission.");
        status = CHIP_ERROR_INVALID_ARGUMENT;
    }

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

CHIP_ERROR DeviceCommissioner::IssueNOCChain(const ByteSpan & NOCSRElements, NodeId nodeId,
                                             chip::Callback::Callback<OnNOCChainGeneration> * callback)
{
    MATTER_TRACE_EVENT_SCOPE("IssueNOCChain", "DeviceCommissioner");
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "Getting certificate chain for the device on fabric idx %u", static_cast<unsigned>(mFabricIndex));

    mOperationalCredentialsDelegate->SetNodeIdForNextNOCRequest(nodeId);

    if (mFabricIndex != kUndefinedFabricIndex)
    {
        mOperationalCredentialsDelegate->SetFabricIdForNextNOCRequest(GetFabricId());
    }

    // Note: we don't have attestationSignature, attestationChallenge, DAC, PAI so we are just providing an empty ByteSpan
    // for those arguments.
    return mOperationalCredentialsDelegate->GenerateNOCChain(NOCSRElements, ByteSpan(), ByteSpan(), ByteSpan(), ByteSpan(),
                                                             ByteSpan(), callback);
}

CHIP_ERROR DeviceCommissioner::ProcessCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements,
                                          const ByteSpan & AttestationSignature, const ByteSpan & dac, const ByteSpan & pai,
                                          const ByteSpan & csrNonce)
{
    MATTER_TRACE_EVENT_SCOPE("ProcessOpCSR", "DeviceCommissioner");
    VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "Getting certificate chain for the device from the issuer");

    P256PublicKey dacPubkey;
    ReturnErrorOnFailure(ExtractPubkeyFromX509Cert(dac, dacPubkey));

    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        proxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    mOperationalCredentialsDelegate->SetNodeIdForNextNOCRequest(proxy->GetDeviceId());

    if (mFabricIndex != kUndefinedFabricIndex)
    {
        mOperationalCredentialsDelegate->SetFabricIdForNextNOCRequest(GetFabricId());
    }

    return mOperationalCredentialsDelegate->GenerateNOCChain(NOCSRElements, csrNonce, AttestationSignature, attestationChallenge,
                                                             dac, pai, &mDeviceNOCChainCallback);
}

CHIP_ERROR DeviceCommissioner::SendOperationalCertificate(DeviceProxy * device, const ByteSpan & nocCertBuf,
                                                          const Optional<ByteSpan> & icaCertBuf, const AesCcm128KeySpan ipk,
                                                          const NodeId adminSubject, Optional<System::Clock::Timeout> timeout)
{
    MATTER_TRACE_EVENT_SCOPE("SendOperationalCertificate", "DeviceCommissioner");

    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    OperationalCredentials::Commands::AddNOC::Type request;
    request.NOCValue         = nocCertBuf;
    request.ICACValue        = icaCertBuf;
    request.IPKValue         = ipk;
    request.caseAdminSubject = adminSubject;
    request.adminVendorId    = mVendorId;

    ReturnErrorOnFailure(SendCommand<OperationalCredentialsCluster>(device, request, OnOperationalCertificateAddResponse,
                                                                    OnAddNOCFailureResponse, timeout));

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
    case OperationalCertStatus::kInvalidAdminSubject:
        return CHIP_ERROR_INVALID_ADMIN_SUBJECT;
    case OperationalCertStatus::kFabricConflict:
        return CHIP_ERROR_FABRIC_EXISTS;
    case OperationalCertStatus::kLabelConflict:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case OperationalCertStatus::kInvalidFabricIndex:
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    case OperationalCertStatus::kUnknownEnumValue:
        // Is this a reasonable value?
        return CHIP_ERROR_CERT_LOAD_FAILED;
    }

    return CHIP_ERROR_CERT_LOAD_FAILED;
}

void DeviceCommissioner::OnAddNOCFailureResponse(void * context, CHIP_ERROR error)
{
    MATTER_TRACE_EVENT_SCOPE("OnAddNOCFailureResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device failed to receive the operational certificate Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::OnOperationalCertificateAddResponse(
    void * context, const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    MATTER_TRACE_EVENT_SCOPE("OnOperationalCertificateAddResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device returned status %d on receiving the NOC", to_underlying(data.statusCode));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(commissioner->mState == State::Initialized, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(commissioner->mDeviceBeingCommissioned != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = ConvertFromOperationalCertStatus(data.statusCode);
    SuccessOrExit(err);

    err = commissioner->OnOperationalCredentialsProvisioningCompletion(commissioner->mDeviceBeingCommissioned);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Add NOC failed with error %s", ErrorStr(err));
        commissioner->CommissioningStageComplete(err);
    }
}

CHIP_ERROR DeviceCommissioner::SendTrustedRootCertificate(DeviceProxy * device, const ByteSpan & rcac,
                                                          Optional<System::Clock::Timeout> timeout)
{
    MATTER_TRACE_EVENT_SCOPE("SendTrustedRootCertificate", "DeviceCommissioner");
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(Controller, "Sending root certificate to the device");

    OperationalCredentials::Commands::AddTrustedRootCertificate::Type request;
    request.rootCertificate = rcac;
    ReturnErrorOnFailure(
        SendCommand<OperationalCredentialsCluster>(device, request, OnRootCertSuccessResponse, OnRootCertFailureResponse, timeout));

    ChipLogProgress(Controller, "Sent root certificate to the device");

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnRootCertSuccessResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    MATTER_TRACE_EVENT_SCOPE("OnRootCertSuccessResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device confirmed that it has received the root certificate");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);
}

void DeviceCommissioner::OnRootCertFailureResponse(void * context, CHIP_ERROR error)
{
    MATTER_TRACE_EVENT_SCOPE("OnRootCertFailureResponse", "DeviceCommissioner");
    ChipLogProgress(Controller, "Device failed to receive the root certificate Response: %s", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

CHIP_ERROR DeviceCommissioner::OnOperationalCredentialsProvisioningCompletion(DeviceProxy * device)
{
    MATTER_TRACE_EVENT_SCOPE("OnOperationalCredentialsProvisioningCompletion", "DeviceCommissioner");
    ChipLogProgress(Controller, "Operational credentials provisioned on device %p", device);
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnStatusUpdate(DevicePairingDelegate::SecurePairingSuccess);
    }
    CommissioningStageComplete(CHIP_NO_ERROR);

    return CHIP_NO_ERROR;
}

#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
void DeviceCommissioner::ConnectBleTransportToSelf()
{
    Transport::BLEBase & transport = std::get<Transport::BLE<1>>(mSystemState->TransportMgr()->GetTransport().GetTransports());
    if (!transport.IsBleLayerTransportSetToSelf())
    {
        transport.SetBleLayerTransportToSelf();
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

void DeviceCommissioner::CloseBleConnection()
{
    // It is fine since we can only commission one device at the same time.
    // We should be able to distinguish different BLE connections if we want
    // to commission multiple devices at the same time over BLE.
    mSystemState->BleLayer()->CloseAllBleConnections();
}
#endif

CHIP_ERROR DeviceCommissioner::DiscoverCommissionableNodes(Dnssd::DiscoveryFilter filter)
{
    ReturnErrorOnFailure(SetUpNodeDiscovery());
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

const Dnssd::DiscoveredNodeData * DeviceCommissioner::GetDiscoveredDevice(int idx)
{
    return GetDiscoveredNode(idx);
}

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

void DeviceCommissioner::OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    if (mUdcServer != nullptr)
    {
        mUdcServer->OnCommissionableNodeFound(nodeData);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    AbstractDnssdDiscoveryController::OnNodeDiscovered(nodeData);
    mSetUpCodePairer.NotifyCommissionableDeviceDiscovered(nodeData);
}

void OnBasicFailure(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Received failure response %s\n", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(error);
}

void DeviceCommissioner::CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus)
{
    commissioningCompletionStatus = completionStatus;

    if (completionStatus.err == CHIP_NO_ERROR)
    {

        CommissioneeDeviceProxy * commissionee = FindCommissioneeDevice(nodeId);
        if (commissionee != nullptr)
        {
            ReleaseCommissioneeDevice(commissionee);
        }
        // Send the callbacks, we're done.
        CommissioningStageComplete(CHIP_NO_ERROR);
        SendCommissioningCompleteCallbacks(nodeId, commissioningCompletionStatus);
    }
    else if (completionStatus.failedStage.HasValue() && completionStatus.failedStage.Value() >= kWiFiNetworkSetup)
    {
        // If we were already doing network setup, we need to retain the pase session and start again from network setup stage.
        // We do not need to reset the failsafe here because we want to keep everything on the device up to this point, so just
        // send the completion callbacks.
        CommissioningStageComplete(CHIP_NO_ERROR);
        SendCommissioningCompleteCallbacks(nodeId, commissioningCompletionStatus);
    }
    else
    {
        // If we've failed somewhere in the early stages (or we don't have a failedStage specified), we need to start from the
        // beginning. However, because some of the commands can only be sent once per arm-failsafe, we also need to force a reset on
        // the failsafe so we can start fresh on the next attempt.
        GeneralCommissioning::Commands::ArmFailSafe::Type request;
        request.expiryLengthSeconds = 0; // Expire immediately.
        request.breadcrumb          = 0;
        ChipLogProgress(Controller, "Expiring failsafe on proxy %p", proxy);
        mDeviceBeingCommissioned = proxy;
        // We actually want to do the same thing on success or failure because we're already in a failure state
        SendCommand<GeneralCommissioningCluster>(proxy, request, OnDisarmFailsafe, OnDisarmFailsafeFailure,
                                                 /* timeout = */ NullOptional);
    }
}

void DeviceCommissioner::OnDisarmFailsafe(void * context,
                                          const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    ChipLogProgress(Controller, "Failsafe disarmed");
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->DisarmDone();
}

void DeviceCommissioner::OnDisarmFailsafeFailure(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Received failure response  when disarming failsafe%s\n", chip::ErrorStr(error));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->DisarmDone();
}

void DeviceCommissioner::DisarmDone()
{
    // If someone nulled out our mDeviceBeingCommissioned, there's nothing else
    // to do here.
    VerifyOrReturn(mDeviceBeingCommissioned != nullptr);

    NodeId nodeId = mDeviceBeingCommissioned->GetDeviceId();
    // At this point, we also want to close off the pase session so we need to re-establish
    CommissioneeDeviceProxy * commissionee = FindCommissioneeDevice(nodeId);

    // Signal completion - this will reset mDeviceBeingCommissioned.
    CommissioningStageComplete(CHIP_NO_ERROR);
    SendCommissioningCompleteCallbacks(nodeId, commissioningCompletionStatus);

    // If we've disarmed the failsafe, it's because we're starting again, so kill the pase connection.
    if (commissionee != nullptr)
    {
        ReleaseCommissioneeDevice(commissionee);
    }
}

void DeviceCommissioner::SendCommissioningCompleteCallbacks(NodeId nodeId, const CompletionStatus & completionStatus)
{
    mCommissioningStage = CommissioningStage::kSecurePairing;
    if (mPairingDelegate == nullptr)
    {
        return;
    }
    mPairingDelegate->OnCommissioningComplete(nodeId, completionStatus.err);
    PeerId peerId(GetCompressedFabricId(), nodeId);
    if (completionStatus.err == CHIP_NO_ERROR)
    {
        mPairingDelegate->OnCommissioningSuccess(peerId);
    }
    else
    {
        // TODO: We should propogate detailed error information (commissioningError, networkCommissioningStatus) from
        // completionStatus.
        mPairingDelegate->OnCommissioningFailure(peerId, completionStatus.err, completionStatus.failedStage.ValueOr(kError),
                                                 completionStatus.attestationResult);
    }
}

void DeviceCommissioner::CommissioningStageComplete(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report)
{
    // Once this stage is complete, reset mDeviceBeingCommissioned - this will be reset when the delegate calls the next step.
    MATTER_TRACE_EVENT_SCOPE("CommissioningStageComplete", "DeviceCommissioner");
    if (mDeviceBeingCommissioned == nullptr)
    {
        // We are getting a stray callback (e.g. due to un-cancellable
        // operations) when we are not in fact commissioning anything.  Just
        // ignore it.
        return;
    }

    NodeId nodeId            = mDeviceBeingCommissioned->GetDeviceId();
    DeviceProxy * proxy      = mDeviceBeingCommissioned;
    mDeviceBeingCommissioned = nullptr;

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnCommissioningStatusUpdate(PeerId(GetCompressedFabricId(), nodeId), mCommissioningStage, err);
    }

    if (mCommissioningDelegate == nullptr)
    {
        return;
    }
    report.stageCompleted = mCommissioningStage;
    CHIP_ERROR status     = mCommissioningDelegate->CommissioningStepFinished(err, report);
    if (status != CHIP_NO_ERROR)
    {
        // Commissioning delegate will only return error if it failed to perform the appropriate commissioning step.
        // In this case, we should complete the commissioning for it.
        CompletionStatus completionStatus;
        completionStatus.err         = status;
        completionStatus.failedStage = MakeOptional(report.stageCompleted);
        mCommissioningStage          = CommissioningStage::kCleanup;
        mDeviceBeingCommissioned     = proxy;
        CleanupCommissioning(proxy, nodeId, completionStatus);
    }
}

void DeviceCommissioner::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             SessionHandle & sessionHandle)
{
    // CASE session established.
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    VerifyOrReturn(commissioner != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    if (commissioner->mCommissioningStage != CommissioningStage::kFindOperational)
    {
        // This call is definitely not us finding our commissionee device.
        // This is presumably us trying to re-establish CASE on MRP failure.
        return;
    }

    if (commissioner->mDeviceBeingCommissioned == nullptr ||
        commissioner->mDeviceBeingCommissioned->GetDeviceId() != sessionHandle->GetPeer().GetNodeId())
    {
        // Not the device we are trying to commission.
        return;
    }

    if (commissioner->mCommissioningDelegate != nullptr)
    {
        CommissioningDelegate::CommissioningReport report;
        report.Set<OperationalNodeFoundData>(OperationalNodeFoundData(OperationalDeviceProxy(&exchangeMgr, sessionHandle)));
        commissioner->CommissioningStageComplete(CHIP_NO_ERROR, report);
    }
}

void DeviceCommissioner::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    // CASE session establishment failed.
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    ChipLogProgress(Controller, "Device connection failed. Error %s", ErrorStr(error));
    VerifyOrReturn(commissioner != nullptr,
                   ChipLogProgress(Controller, "Device connection failure callback with null context. Ignoring"));

    // Ensure that commissioning stage advancement is done based on seeing an error.
    if (error == CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Device connection failed without a valid error code. Making one up.");
        error = CHIP_ERROR_INTERNAL;
    }

    if (commissioner->mDeviceBeingCommissioned == nullptr ||
        commissioner->mDeviceBeingCommissioned->GetDeviceId() != peerId.GetNodeId())
    {
        // Not the device we are trying to commission.
        return;
    }

    if (commissioner->mCommissioningStage == CommissioningStage::kFindOperational &&
        commissioner->mCommissioningDelegate != nullptr)
    {
        commissioner->CommissioningStageComplete(error);
    }
}

// ClusterStateCache::Callback impl
void DeviceCommissioner::OnDone(app::ReadClient *)
{
    CHIP_ERROR err;
    CHIP_ERROR return_err = CHIP_NO_ERROR;
    ReadCommissioningInfo info;

    // Using ForEachAttribute because this attribute can be queried on any endpoint.
    err = mAttributeCache->ForEachAttribute(
        app::Clusters::GeneralCommissioning::Id, [this, &info](const app::ConcreteAttributePath & path) {
            switch (path.mAttributeId)
            {
            case app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::Id: {
                app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::TypeInfo::DecodableType basicInfo;
                ReturnErrorOnFailure(
                    this->mAttributeCache->Get<app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::TypeInfo>(
                        path, basicInfo));
                info.general.recommendedFailsafe = basicInfo.failSafeExpiryLengthSeconds;
            }
            break;
            case app::Clusters::GeneralCommissioning::Attributes::RegulatoryConfig::Id: {
                ReturnErrorOnFailure(
                    this->mAttributeCache->Get<app::Clusters::GeneralCommissioning::Attributes::RegulatoryConfig::TypeInfo>(
                        path, info.general.currentRegulatoryLocation));
            }
            break;
            case app::Clusters::GeneralCommissioning::Attributes::LocationCapability::Id: {
                ReturnErrorOnFailure(
                    this->mAttributeCache->Get<app::Clusters::GeneralCommissioning::Attributes::LocationCapability::TypeInfo>(
                        path, info.general.locationCapability));
            }
            break;
            case app::Clusters::GeneralCommissioning::Attributes::Breadcrumb::Id: {
                ReturnErrorOnFailure(
                    this->mAttributeCache->Get<app::Clusters::GeneralCommissioning::Attributes::Breadcrumb::TypeInfo>(
                        path, info.general.breadcrumb));
            }
            break;
            default:
                return CHIP_NO_ERROR;
            }

            return CHIP_NO_ERROR;
        });

    // Try to parse as much as we can here before returning, even if this is an error.
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    err = mAttributeCache->ForEachAttribute(app::Clusters::Basic::Id, [this, &info](const app::ConcreteAttributePath & path) {
        if (path.mAttributeId != app::Clusters::Basic::Attributes::VendorID::Id &&
            path.mAttributeId != app::Clusters::Basic::Attributes::ProductID::Id)
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
        default:
            return CHIP_NO_ERROR;
        }
    });

    // Try to parse as much as we can here before returning, even if this is an error.
    return_err = (err == CHIP_NO_ERROR) ? return_err : err;

    err = mAttributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &info](const app::ConcreteAttributePath & path) {
        // this code is checking if the device is already on the commissioner's fabric.
        // if a matching fabric is found, then remember the nodeId so that the commissioner
        // can, if it decides to, cancel commissioning (before it fails in AddNoc) and know
        // the device's nodeId on its fabric.
        switch (path.mAttributeId)
        {
        case OperationalCredentials::Attributes::Fabrics::Id: {
            OperationalCredentials::Attributes::Fabrics::TypeInfo::DecodableType fabrics;
            ReturnErrorOnFailure(this->mAttributeCache->Get<OperationalCredentials::Attributes::Fabrics::TypeInfo>(path, fabrics));
            // this is a best effort attempt to find a matching fabric, so no error checking on iter
            auto iter = fabrics.begin();
            while (iter.Next())
            {
                auto & fabricDescriptor = iter.GetValue();
                ChipLogProgress(Controller,
                                "DeviceCommissioner::OnDone - fabric.vendorId=0x%04X fabric.fabricId=0x" ChipLogFormatX64
                                " fabric.nodeId=0x" ChipLogFormatX64,
                                fabricDescriptor.vendorId, ChipLogValueX64(fabricDescriptor.fabricId),
                                ChipLogValueX64(fabricDescriptor.nodeId));
                if (GetFabricId() == fabricDescriptor.fabricId)
                {
                    ChipLogProgress(Controller, "DeviceCommissioner::OnDone - found a matching fabric id");
                    chip::ByteSpan rootKeySpan = fabricDescriptor.rootPublicKey;
                    if (rootKeySpan.size() != Crypto::kP256_PublicKey_Length)
                    {
                        ChipLogError(Controller, "DeviceCommissioner::OnDone - fabric root key size mismatch %u != %u",
                                     static_cast<unsigned>(rootKeySpan.size()),
                                     static_cast<unsigned>(Crypto::kP256_PublicKey_Length));
                        continue;
                    }
                    P256PublicKeySpan rootPubKeySpan(rootKeySpan.data());
                    Crypto::P256PublicKey deviceRootPublicKey(rootPubKeySpan);

                    Crypto::P256PublicKey commissionerRootPublicKey;
                    if (CHIP_NO_ERROR != GetRootPublicKey(commissionerRootPublicKey))
                    {
                        ChipLogError(Controller, "DeviceCommissioner::OnDone - error reading commissioner root public key");
                    }
                    else if (commissionerRootPublicKey.Matches(deviceRootPublicKey))
                    {
                        ChipLogProgress(Controller, "DeviceCommissioner::OnDone - fabric root keys match");
                        info.nodeId = fabricDescriptor.nodeId;
                    }
                }
            }

            return CHIP_NO_ERROR;
        }
        default:
            return CHIP_NO_ERROR;
        }
    });

    // Try to parse as much as we can here before returning, even if this is an error.
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    // Set the network cluster endpoints first so we can match up the connection times.
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
                        ChipLogProgress(Controller, "----- NetworkCommissioning Features: has WiFi. endpointid = %u",
                                        path.mEndpointId);
                        info.network.wifi.endpoint = path.mEndpointId;
                    }
                    else if (features.Has(
                                 app::Clusters::NetworkCommissioning::NetworkCommissioningFeature::kThreadNetworkInterface))
                    {
                        ChipLogProgress(Controller, "----- NetworkCommissioning Features: has Thread. endpointid = %u",
                                        path.mEndpointId);
                        info.network.thread.endpoint = path.mEndpointId;
                    }
                    else if (features.Has(
                                 app::Clusters::NetworkCommissioning::NetworkCommissioningFeature::kEthernetNetworkInterface))
                    {
                        ChipLogProgress(Controller, "----- NetworkCommissioning Features: has Ethernet. endpointid = %u",
                                        path.mEndpointId);
                        info.network.eth.endpoint = path.mEndpointId;
                    }
                    else
                    {
                        ChipLogProgress(Controller, "----- NetworkCommissioning Features: no features.");
                        // TODO: Gross workaround for the empty feature map on all clusters. Remove.
                        if (info.network.thread.endpoint == kInvalidEndpointId)
                        {
                            info.network.thread.endpoint = path.mEndpointId;
                        }
                        if (info.network.wifi.endpoint == kInvalidEndpointId)
                        {
                            info.network.wifi.endpoint = path.mEndpointId;
                        }
                    }
                }
            }
            return CHIP_NO_ERROR;
        });
    return_err = err == CHIP_NO_ERROR ? return_err : err;

    err = mAttributeCache->ForEachAttribute(
        app::Clusters::NetworkCommissioning::Id, [this, &info](const app::ConcreteAttributePath & path) {
            if (path.mAttributeId != app::Clusters::NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::Id)
            {
                return CHIP_NO_ERROR;
            }
            app::Clusters::NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::TypeInfo::DecodableArgType time;
            ReturnErrorOnFailure(
                this->mAttributeCache->Get<app::Clusters::NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::TypeInfo>(path,
                                                                                                                             time));
            if (path.mEndpointId == info.network.wifi.endpoint)
            {
                info.network.wifi.minConnectionTime = time;
            }
            else if (path.mEndpointId == info.network.thread.endpoint)
            {
                info.network.thread.minConnectionTime = time;
            }
            else if (path.mEndpointId == info.network.eth.endpoint)
            {
                info.network.eth.minConnectionTime = time;
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

    if (mPairingDelegate != nullptr)
    {
        mPairingDelegate->OnReadCommissioningInfo(info);
    }

    CommissioningDelegate::CommissioningReport report;
    report.Set<ReadCommissioningInfo>(info);
    CommissioningStageComplete(return_err, report);
}

void DeviceCommissioner::OnArmFailSafe(void * context,
                                       const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received ArmFailSafe response errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningError::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
        report.Set<CommissioningErrorInfo>(data.errorCode);
    }

    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(err, report);
}

void DeviceCommissioner::OnSetRegulatoryConfigResponse(
    void * context, const GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received SetRegulatoryConfig response errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningError::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
        report.Set<CommissioningErrorInfo>(data.errorCode);
    }
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(err, report);
}

void DeviceCommissioner::OnScanNetworksFailure(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "Received ScanNetworks failure response %" CHIP_ERROR_FORMAT, error.Format());

    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    // advance to the kNeedsNetworkCreds waiting step
    // clear error so that we don't abort the commissioning when ScanNetworks fails
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);

    if (commissioner->GetPairingDelegate() != nullptr)
    {
        commissioner->GetPairingDelegate()->OnScanNetworksFailure(error);
    }
}

void DeviceCommissioner::OnScanNetworksResponse(void * context,
                                                const NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;

    ChipLogProgress(Controller, "Received ScanNetwork response, networkingStatus=%u debugText=%s",
                    to_underlying(data.networkingStatus),
                    (data.debugText.HasValue() ? std::string(data.debugText.Value().data(), data.debugText.Value().size()).c_str()
                                               : "none provided"));
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);

    // advance to the kNeedsNetworkCreds waiting step
    commissioner->CommissioningStageComplete(CHIP_NO_ERROR);

    if (commissioner->GetPairingDelegate() != nullptr)
    {
        commissioner->GetPairingDelegate()->OnScanNetworksSuccess(data);
    }
}

CHIP_ERROR DeviceCommissioner::NetworkCredentialsReady()
{
    ReturnErrorCodeIf(mCommissioningStage != CommissioningStage::kNeedsNetworkCreds, CHIP_ERROR_INCORRECT_STATE);

    // need to advance to next step
    CommissioningStageComplete(CHIP_NO_ERROR);

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::OnNetworkConfigResponse(void * context,
                                                 const NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received NetworkConfig response, networkingStatus=%u", to_underlying(data.networkingStatus));
    if (data.networkingStatus != NetworkCommissioning::NetworkCommissioningStatus::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
        report.Set<NetworkCommissioningStatusInfo>(data.networkingStatus);
    }
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(err, report);
}

void DeviceCommissioner::OnConnectNetworkResponse(
    void * context, const NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received ConnectNetwork response, networkingStatus=%u", to_underlying(data.networkingStatus));
    if (data.networkingStatus != NetworkCommissioning::NetworkCommissioningStatus::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
        report.Set<NetworkCommissioningStatusInfo>(data.networkingStatus);
    }
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(err, report);
}

void DeviceCommissioner::OnCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    CommissioningDelegate::CommissioningReport report;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "Received CommissioningComplete response, errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningError::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
        report.Set<CommissioningErrorInfo>(data.errorCode);
    }
    DeviceCommissioner * commissioner = static_cast<DeviceCommissioner *>(context);
    commissioner->CommissioningStageComplete(err, report);
}

void DeviceCommissioner::PerformCommissioningStep(DeviceProxy * proxy, CommissioningStage step, CommissioningParameters & params,
                                                  CommissioningDelegate * delegate, EndpointId endpoint,
                                                  Optional<System::Clock::Timeout> timeout)
{
    if (params.GetCompletionStatus().err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Performing next commissioning step '%s'", StageToString(step));
    }
    else
    {
        ChipLogProgress(Controller, "Performing next commissioning step '%s' with completion status = '%s'", StageToString(step),
                        params.GetCompletionStatus().err.AsString());
    }

    // For now, we ignore errors coming in from the device since not all commissioning clusters are implemented on the device
    // side.
    mCommissioningStage      = step;
    mCommissioningDelegate   = delegate;
    mDeviceBeingCommissioned = proxy;
    // TODO: Extend timeouts to the DAC and Opcert requests.

    // TODO(cecille): We probably want something better than this for breadcrumbs.
    uint64_t breadcrumb = static_cast<uint64_t>(step);

    switch (step)
    {
    case CommissioningStage::kArmFailsafe: {
        GeneralCommissioning::Commands::ArmFailSafe::Type request;
        request.expiryLengthSeconds = params.GetFailsafeTimerSeconds().ValueOr(kDefaultFailsafeTimeout);
        request.breadcrumb          = breadcrumb;
        ChipLogProgress(Controller, "Arming failsafe (%u seconds)", request.expiryLengthSeconds);
        SendCommand<GeneralCommissioningCluster>(proxy, request, OnArmFailSafe, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kReadCommissioningInfo: {
        ChipLogProgress(Controller, "Sending request for commissioning information");
        app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
        app::ReadPrepareParams readParams(proxy->GetSecureSession().Value());

        // NOTE: this array cannot have more than 9 entries, since the spec mandates that server only needs to support 9
        app::AttributePathParams readPaths[9];
        // Read all the feature maps for all the networking clusters on any endpoint to determine what is supported
        readPaths[0] = app::AttributePathParams(app::Clusters::NetworkCommissioning::Id,
                                                app::Clusters::NetworkCommissioning::Attributes::FeatureMap::Id);
        // Get required general commissioning attributes on this endpoint (recommended failsafe time, regulatory location
        // info, breadcrumb)
        readPaths[1] = app::AttributePathParams(endpoint, app::Clusters::GeneralCommissioning::Id,
                                                app::Clusters::GeneralCommissioning::Attributes::Breadcrumb::Id);
        readPaths[2] = app::AttributePathParams(endpoint, app::Clusters::GeneralCommissioning::Id,
                                                app::Clusters::GeneralCommissioning::Attributes::BasicCommissioningInfo::Id);
        readPaths[3] = app::AttributePathParams(endpoint, app::Clusters::GeneralCommissioning::Id,
                                                app::Clusters::GeneralCommissioning::Attributes::RegulatoryConfig::Id);
        readPaths[4] = app::AttributePathParams(endpoint, app::Clusters::GeneralCommissioning::Id,
                                                app::Clusters::GeneralCommissioning::Attributes::LocationCapability::Id);
        // Read attributes from the basic info cluster (vendor id / product id / software version)
        readPaths[5] = app::AttributePathParams(endpoint, app::Clusters::Basic::Id, app::Clusters::Basic::Attributes::VendorID::Id);
        readPaths[6] =
            app::AttributePathParams(endpoint, app::Clusters::Basic::Id, app::Clusters::Basic::Attributes::ProductID::Id);
        // Read the requested minimum connection times from all network commissioning clusters
        readPaths[7] = app::AttributePathParams(app::Clusters::NetworkCommissioning::Id,
                                                app::Clusters::NetworkCommissioning::Attributes::ConnectMaxTimeSeconds::Id);

        readParams.mpAttributePathParamsList    = readPaths;
        readParams.mAttributePathParamsListSize = 8;

        // Read the current fabrics
        if (params.GetCheckForMatchingFabric())
        {
            readParams.mAttributePathParamsListSize = 9;
            readPaths[8] = app::AttributePathParams(OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
        }

        readParams.mIsFabricFiltered = false;
        if (timeout.HasValue())
        {
            readParams.mTimeout = timeout.Value();
        }
        auto attributeCache = Platform::MakeUnique<app::ClusterStateCache>(*this);
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
    case CommissioningStage::kScanNetworks: {
        NetworkCommissioning::Commands::ScanNetworks::Type request;
        if (params.GetWiFiCredentials().HasValue())
        {
            request.ssid.Emplace(params.GetWiFiCredentials().Value().ssid);
        }
        request.breadcrumb.Emplace(breadcrumb);
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnScanNetworksResponse, OnScanNetworksFailure, endpoint, timeout);
        break;
    }
    case CommissioningStage::kNeedsNetworkCreds: {
        // nothing to do, the OnScanNetworksSuccess and OnScanNetworksFailure callbacks provide indication to the
        // DevicePairingDelegate that network credentials are needed.
        break;
    }
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
        auto capability =
            params.GetLocationCapability().ValueOr(app::Clusters::GeneralCommissioning::RegulatoryLocationType::kOutdoor);
        app::Clusters::GeneralCommissioning::RegulatoryLocationType regulatoryConfig;
        // Value is only switchable on the devices with indoor/outdoor capability
        if (capability == app::Clusters::GeneralCommissioning::RegulatoryLocationType::kIndoorOutdoor)
        {
            // If the device supports indoor and outdoor configs, use the setting from the commissioner, otherwise fall back to
            // the current device setting then to outdoor (most restrictive)
            if (params.GetDeviceRegulatoryLocation().HasValue())
            {
                regulatoryConfig = params.GetDeviceRegulatoryLocation().Value();
                ChipLogProgress(Controller, "Setting regulatory config to %u from commissioner override",
                                static_cast<uint8_t>(regulatoryConfig));
            }
            else if (params.GetDefaultRegulatoryLocation().HasValue())
            {
                regulatoryConfig = params.GetDefaultRegulatoryLocation().Value();
                ChipLogProgress(Controller, "No regulatory config supplied by controller, leaving as device default (%u)",
                                static_cast<uint8_t>(regulatoryConfig));
            }
            else
            {
                regulatoryConfig = app::Clusters::GeneralCommissioning::RegulatoryLocationType::kOutdoor;
                ChipLogProgress(Controller, "No overrride or device regulatory config supplied, setting to outdoor");
            }
        }
        else
        {
            ChipLogProgress(Controller, "Device does not support configurable regulatory location");
            regulatoryConfig = capability;
        }

        CharSpan countryCode;
        const auto & providedCountryCode = params.GetCountryCode();
        if (providedCountryCode.HasValue())
        {
            countryCode = providedCountryCode.Value();
        }
        else
        {
            // Default to "XX", for lack of anything better.
            countryCode = CharSpan::fromCharString("XX");
        }

        GeneralCommissioning::Commands::SetRegulatoryConfig::Type request;
        request.newRegulatoryConfig = regulatoryConfig;
        request.countryCode         = countryCode;
        request.breadcrumb          = breadcrumb;
        SendCommand<GeneralCommissioningCluster>(proxy, request, OnSetRegulatoryConfigResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kSendPAICertificateRequest:
        ChipLogProgress(Controller, "Sending request for PAI certificate");
        SendCertificateChainRequestCommand(proxy, CertificateType::kPAI, timeout);
        break;
    case CommissioningStage::kSendDACCertificateRequest:
        ChipLogProgress(Controller, "Sending request for DAC certificate");
        SendCertificateChainRequestCommand(proxy, CertificateType::kDAC, timeout);
        break;
    case CommissioningStage::kSendAttestationRequest:
        ChipLogProgress(Controller, "Sending Attestation Request to the device.");
        if (!params.GetAttestationNonce().HasValue())
        {
            ChipLogError(Controller, "No attestation nonce found");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        SendAttestationRequestCommand(proxy, params.GetAttestationNonce().Value(), timeout);
        break;
    case CommissioningStage::kAttestationVerification: {
        ChipLogProgress(Controller, "Verifying attestation");
        if (!params.GetAttestationElements().HasValue() || !params.GetAttestationSignature().HasValue() ||
            !params.GetAttestationNonce().HasValue() || !params.GetDAC().HasValue() || !params.GetPAI().HasValue() ||
            !params.GetRemoteVendorId().HasValue() || !params.GetRemoteProductId().HasValue())
        {
            ChipLogError(Controller, "Missing attestation information");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }

        DeviceAttestationVerifier::AttestationInfo info(
            params.GetAttestationElements().Value(),
            proxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge(),
            params.GetAttestationSignature().Value(), params.GetPAI().Value(), params.GetDAC().Value(),
            params.GetAttestationNonce().Value(), params.GetRemoteVendorId().Value(), params.GetRemoteProductId().Value());

        if (ValidateAttestationInfo(info) != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Error validating attestation information");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
    }
    break;
    case CommissioningStage::kSendOpCertSigningRequest:
        if (!params.GetCSRNonce().HasValue())
        {
            ChipLogError(Controller, "No CSR nonce found");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        SendOperationalCertificateSigningRequestCommand(proxy, params.GetCSRNonce().Value(), timeout);
        break;
    case CommissioningStage::kValidateCSR: {
        if (!params.GetNOCChainGenerationParameters().HasValue() || !params.GetDAC().HasValue() || !params.GetCSRNonce().HasValue())
        {
            ChipLogError(Controller, "Unable to validate CSR");
            return CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
        }
        // This is non-blocking, so send the callback immediately.
        CHIP_ERROR err = ValidateCSR(proxy, params.GetNOCChainGenerationParameters().Value().nocsrElements,
                                     params.GetNOCChainGenerationParameters().Value().signature, params.GetDAC().Value(),
                                     params.GetCSRNonce().Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to validate CSR");
        }
        CommissioningStageComplete(err);
    }
    break;
    case CommissioningStage::kGenerateNOCChain: {
        if (!params.GetNOCChainGenerationParameters().HasValue() || !params.GetDAC().HasValue() || !params.GetPAI().HasValue() ||
            !params.GetCSRNonce().HasValue())
        {
            ChipLogError(Controller, "Unable to generate NOC chain parameters");
            return CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
        }
        CHIP_ERROR err = ProcessCSR(proxy, params.GetNOCChainGenerationParameters().Value().nocsrElements,
                                    params.GetNOCChainGenerationParameters().Value().signature, params.GetDAC().Value(),
                                    params.GetPAI().Value(), params.GetCSRNonce().Value());
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
        CHIP_ERROR err = SendTrustedRootCertificate(proxy, params.GetRootCert().Value(), timeout);
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
        if (!params.GetNoc().HasValue() || !params.GetIpk().HasValue() || !params.GetAdminSubject().HasValue())
        {
            ChipLogError(Controller, "AddNOC contents not specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }
        SendOperationalCertificate(proxy, params.GetNoc().Value(), params.GetIcac(), params.GetIpk().Value(),
                                   params.GetAdminSubject().Value(), timeout);
        break;
    case CommissioningStage::kWiFiNetworkSetup: {
        if (!params.GetWiFiCredentials().HasValue())
        {
            ChipLogError(Controller, "No wifi credentials specified");
            CommissioningStageComplete(CHIP_ERROR_INVALID_ARGUMENT);
            return;
        }

        NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type request;
        request.ssid        = params.GetWiFiCredentials().Value().ssid;
        request.credentials = params.GetWiFiCredentials().Value().credentials;
        request.breadcrumb.Emplace(breadcrumb);
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
        NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type request;
        request.operationalDataset = params.GetThreadOperationalDataset().Value();
        request.breadcrumb.Emplace(breadcrumb);
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
        NetworkCommissioning::Commands::ConnectNetwork::Type request;
        request.networkID = params.GetWiFiCredentials().Value().ssid;
        request.breadcrumb.Emplace(breadcrumb);
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
        NetworkCommissioning::Commands::ConnectNetwork::Type request;
        request.networkID = extendedPanId;
        request.breadcrumb.Emplace(breadcrumb);
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnConnectNetworkResponse, OnBasicFailure, endpoint, timeout);
    }
    break;
    case CommissioningStage::kFindOperational: {
        // If there is an error, CommissioningStageComplete will be called from OnDeviceConnectionFailureFn.
        auto scopedPeerId = GetPeerScopedId(proxy->GetDeviceId());
        mSystemState->CASESessionMgr()->FindOrEstablishSession(scopedPeerId, &mOnDeviceConnectedCallback,
                                                               &mOnDeviceConnectionFailureCallback);
    }
    break;
    case CommissioningStage::kSendComplete: {
        GeneralCommissioning::Commands::CommissioningComplete::Type request;
        SendCommand<NetworkCommissioningCluster>(proxy, request, OnCommissioningCompleteResponse, OnBasicFailure, endpoint,
                                                 timeout);
    }
    break;
    case CommissioningStage::kCleanup:
        CleanupCommissioning(proxy, proxy->GetDeviceId(), params.GetCompletionStatus());
        break;
    case CommissioningStage::kError:
        mCommissioningStage = CommissioningStage::kSecurePairing;
        break;
    case CommissioningStage::kSecurePairing:
        break;
    }
}

CHIP_ERROR DeviceController::GetCompressedFabricIdBytes(MutableByteSpan & outBytes) const
{
    const auto * fabricInfo = GetFabricInfo();
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    return fabricInfo->GetCompressedFabricIdBytes(outBytes);
}

CHIP_ERROR DeviceController::GetRootPublicKey(Crypto::P256PublicKey & outRootPublicKey) const
{
    const auto * fabricTable = GetFabricTable();
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return fabricTable->FetchRootPubkey(mFabricIndex, outRootPublicKey);
}

} // namespace Controller
} // namespace chip
