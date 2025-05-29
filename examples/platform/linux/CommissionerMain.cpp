/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "CommissionerMain.h"

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/RawKeySessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/NodeId.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <controller/CHIPDeviceControllerFactory.h>         // nogncheck
#include <controller/ExampleOperationalCredentialsIssuer.h> // nogncheck

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#include <signal.h>
#include <string>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::app::Clusters;
using namespace chip::Protocols::UserDirectedCommissioning;

using namespace ::chip::Messaging;
using namespace ::chip::Controller;

class MyServerStorageDelegate : public PersistentStorageDelegate
{
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        ChipLogProgress(AppServer, "Retrieving value from server storage.");
        size_t bytesRead = 0;
        CHIP_ERROR err   = PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, &bytesRead);

        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Retrieved value from server storage.");
        }
        size = static_cast<uint16_t>(bytesRead);
        return err;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        ChipLogProgress(AppServer, "Stored value in server storage");
        return PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        ChipLogProgress(AppServer, "Delete value in server storage");
        return PersistedStorage::KeyValueStoreMgr().Delete(key);
    }
};

class MyCommissionerCallback : public CommissionerCallback
{
    void ReadyForCommissioning(uint32_t pincode, uint16_t longDiscriminator, PeerAddress peerAddress) override
    {
        CommissionerPairOnNetwork(pincode, longDiscriminator, peerAddress);
    }
};

AutoCommissioner gAutoCommissioner;

DeviceCommissioner gCommissioner;
CommissionerDiscoveryController gCommissionerDiscoveryController;
MyCommissionerCallback gCommissionerCallback;
MyServerStorageDelegate gServerStorage;
ExampleOperationalCredentialsIssuer gOpCredsIssuer;
NodeId gLocalId = kMaxOperationalNodeId;
Credentials::GroupDataProviderImpl gGroupDataProvider;
Crypto::RawKeySessionKeystore gSessionKeystore;

CHIP_ERROR InitCommissioner(uint16_t commissionerPort, uint16_t udcListenPort, FabricId fabricId)
{
    Controller::FactoryInitParams factoryParams;
    Controller::SetupParams params;

    // use a different listen port for the commissioner than the default used by chip-tool.
    factoryParams.listenPort               = commissionerPort;
    factoryParams.fabricIndependentStorage = &gServerStorage;
    factoryParams.fabricTable              = &Server::GetInstance().GetFabricTable();
    factoryParams.sessionKeystore          = &gSessionKeystore;
    factoryParams.dataModelProvider        = chip::app::CodegenDataModelProviderInstance(&gServerStorage);

    gGroupDataProvider.SetStorageDelegate(&gServerStorage);
    gGroupDataProvider.SetSessionKeystore(factoryParams.sessionKeystore);
    ReturnErrorOnFailure(gGroupDataProvider.Init());
    factoryParams.groupDataProvider = &gGroupDataProvider;

    params.operationalCredentialsDelegate = &gOpCredsIssuer;
    uint16_t vendorId;
    DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vendorId);
    ChipLogProgress(Support, " ----- Commissioner using vendorId 0x%04X", vendorId);
    params.controllerVendorId = static_cast<VendorId>(vendorId);

    ReturnErrorOnFailure(gOpCredsIssuer.Initialize(gServerStorage));
    if (fabricId != kUndefinedFabricId)
    {
        gOpCredsIssuer.SetFabricIdForNextNOCRequest(fabricId);
    }

    // No need to explicitly set the UDC port since we will use default
    ChipLogProgress(Support, " ----- UDC listening on port %d", udcListenPort);
    ReturnErrorOnFailure(gCommissioner.SetUdcListenPort(udcListenPort));

    // Initialize device attestation verifier
    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const Credentials::AttestationTrustStore * testingRootStore = Credentials::GetTestAttestationTrustStore();
    SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    Crypto::P256Keypair ephemeralKey;
    ReturnErrorOnFailure(ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA));

    ReturnErrorOnFailure(gOpCredsIssuer.GenerateNOCChainAfterValidation(gLocalId, /* fabricId = */ 1, chip::kUndefinedCATs,
                                                                        ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));

    params.operationalKeypair = &ephemeralKey;
    params.controllerRCAC     = rcacSpan;
    params.controllerICAC     = icacSpan;
    params.controllerNOC      = nocSpan;

    params.defaultCommissioner      = &gAutoCommissioner;
    params.enableServerInteractions = true;

    // assign prefered feature settings
    CommissioningParameters commissioningParams = gAutoCommissioner.GetCommissioningParameters();
    commissioningParams.SetCheckForMatchingFabric(true);
    gAutoCommissioner.SetCommissioningParameters(commissioningParams);

    auto & factory = Controller::DeviceControllerFactory::GetInstance();
    ReturnErrorOnFailure(factory.Init(factoryParams));
    ReturnErrorOnFailure(factory.SetupCommissioner(params, gCommissioner));

    FabricIndex fabricIndex = gCommissioner.GetFabricIndex();
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INTERNAL);

    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    MutableByteSpan compressedFabricIdSpan(compressedFabricId);
    ReturnErrorOnFailure(gCommissioner.GetCompressedFabricIdBytes(compressedFabricIdSpan));
    ChipLogProgress(Support,
                    "Setting up group data for Fabric Index %u with Compressed Fabric ID:", static_cast<unsigned>(fabricIndex));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    // TODO: Once ExampleOperationalCredentialsIssuer has support, set default IPK on it as well so
    // that commissioned devices get the IPK set from real values rather than "test-only" internal hookups.
    ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    ReturnLogErrorOnFailure(
        chip::Credentials::SetSingleIpkEpochKey(&gGroupDataProvider, fabricIndex, defaultIpk, compressedFabricIdSpan));

    gCommissionerDiscoveryController.SetUserDirectedCommissioningServer(gCommissioner.GetUserDirectedCommissioningServer());
    gCommissionerDiscoveryController.SetCommissionerCallback(&gCommissionerCallback);

    // advertise operational since we are an admin
    ReturnLogErrorOnFailure(app::DnssdServer::Instance().AdvertiseOperational());

    ChipLogProgress(Support,
                    "InitCommissioner nodeId=0x" ChipLogFormatX64 " fabric.fabricId=0x" ChipLogFormatX64 " fabricIndex=0x%x",
                    ChipLogValueX64(gCommissioner.GetNodeId()), ChipLogValueX64(fabricId), static_cast<unsigned>(fabricIndex));

    return CHIP_NO_ERROR;
}

void ShutdownCommissioner()
{
    UserDirectedCommissioningServer * udcServer = gCommissioner.GetUserDirectedCommissioningServer();
    if (udcServer != nullptr)
    {
        udcServer->SetUserConfirmationProvider(nullptr);
    }

    gCommissioner.Shutdown();
}

class PairingCommand : public Controller::DevicePairingDelegate
{
public:
    PairingCommand() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this){};

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) override;

    void OnReadCommissioningInfo(const ReadCommissioningInfo & info) override;
    void OnFabricCheck(NodeId matchingNodeId) override;

private:
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
};

PairingCommand gPairingCommand;
NodeId gRemoteId = kTestDeviceNodeId;

void PairingCommand::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(AppServer, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(AppServer, "Secure Pairing Failed");
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->CommissioningFailed(CHIP_ERROR_CONNECTION_ABORTED);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        break;
    }
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Pairing Success");
    }
    else
    {
        ChipLogProgress(AppServer, "Pairing Failure: %s", ErrorStr(err));
    }
}

void PairingCommand::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(AppServer, "Pairing Deleted Failure: %s", ErrorStr(err));
    }
}

void PairingCommand::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ChipLogProgress(AppServer, "Device commissioning completed with success - getting OperationalDeviceProxy");

        gCommissioner.GetConnectedDevice(gAutoCommissioner.GetCommissioningParameters().GetRemoteNodeId().ValueOr(nodeId),
                                         &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
#else  // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ChipLogProgress(AppServer, "Device commissioning completed with success");
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    }
    else
    {
        ChipLogProgress(AppServer, "Device commissioning Failure: %s", ErrorStr(err));
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->CommissioningFailed(err);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    }
}

void PairingCommand::OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error)
{
    ChipLogProgress(AppServer, "OnCommissioningStatusUpdate - stageCompleted='%s' error='%s'", StageToString(stageCompleted),
                    ErrorStr(error));

    // if we have successfully finished attestation AND this device already has a NodeId on our fabric
    // then stop commissioning and attempt to connect to it.
    if (stageCompleted == CommissioningStage::kAttestationVerification && error == CHIP_NO_ERROR &&
        gAutoCommissioner.GetCommissioningParameters().GetRemoteNodeId().HasValue())
    {
        gAutoCommissioner.StopCommissioning();
    }
}

void PairingCommand::OnReadCommissioningInfo(const Controller::ReadCommissioningInfo & info)
{
    ChipLogProgress(AppServer, "OnReadCommissioningInfo - vendorId=0x%04X productId=0x%04X", info.basic.vendorId,
                    info.basic.productId);

    // The string in CharSpan received from the device is not null-terminated, we use std::string here for coping and
    // appending a numm-terminator at the end of the string.
    std::string userActiveModeTriggerInstruction;

    // Note: the callback doesn't own the buffer, should make a copy if it will be used it later.
    if (info.icd.userActiveModeTriggerInstruction.size() != 0)
    {
        userActiveModeTriggerInstruction =
            std::string(info.icd.userActiveModeTriggerInstruction.data(), info.icd.userActiveModeTriggerInstruction.size());
    }

    if (info.icd.userActiveModeTriggerHint.HasAny())
    {
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerHint=0x%08x",
                        info.icd.userActiveModeTriggerHint.Raw());
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerInstruction=%s",
                        userActiveModeTriggerInstruction.c_str());
    }

    ChipLogProgress(AppServer, "OnReadCommissioningInfo ICD - IdleModeDuration=%u activeModeDuration=%u activeModeThreshold=%u",
                    info.icd.idleModeDuration, info.icd.activeModeDuration, info.icd.activeModeThreshold);
}

void PairingCommand::OnFabricCheck(NodeId matchingNodeId)
{
    if (matchingNodeId != kUndefinedNodeId)
    {
        ChipLogProgress(AppServer, "ALREADY ON FABRIC WITH nodeId=0x" ChipLogFormatX64, ChipLogValueX64(matchingNodeId));
        // wait until attestation verification before cancelling so we can validate vid/pid
    }
}

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

void PairingCommand::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                         const SessionHandle & sessionHandle)
{
    ChipLogProgress(Controller, "OnDeviceConnectedFn");
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();

    if (cdc != nullptr)
    {
        uint16_t vendorId  = gAutoCommissioner.GetCommissioningParameters().GetRemoteVendorId().Value();
        uint16_t productId = gAutoCommissioner.GetCommissioningParameters().GetRemoteProductId().Value();
        ChipLogProgress(Support, " ----- AutoCommissioner -- Commissionee vendorId=0x%04X productId=0x%04X", vendorId, productId);

        cdc->CommissioningSucceeded(vendorId, productId, gRemoteId, exchangeMgr, sessionHandle);
    }
}

void PairingCommand::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn - attempt to get OperationalDeviceProxy failed");
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
    {
        cdc->CommissioningFailed(err);
    }
}

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

CHIP_ERROR CommissionerPairOnNetwork(uint32_t pincode, uint16_t disc, Transport::PeerAddress address)
{
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(pincode).SetDiscriminator(disc).SetPeerAddress(address);

    gOpCredsIssuer.GetRandomOperationalNodeId(&gRemoteId);
    gCommissioner.RegisterPairingDelegate(&gPairingCommand);
    gCommissioner.PairDevice(gRemoteId, params);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index)
{
    UDCClientState * state = gCommissioner.GetUserDirectedCommissioningServer()->GetUDCClients().GetUDCClientState(index);
    if (state == nullptr)
    {
        ChipLogProgress(AppServer, "udc client[%ld] null \r\n", static_cast<long>(index));
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    else
    {
        Transport::PeerAddress peerAddress = state->GetPeerAddress();

        state->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningNode);

        return CommissionerPairOnNetwork(pincode, state->GetLongDiscriminator(), peerAddress);
    }
}

DeviceCommissioner * GetDeviceCommissioner()
{
    return &gCommissioner;
}

CommissionerDiscoveryController * GetCommissionerDiscoveryController()
{
    return &gCommissionerDiscoveryController;
}

Crypto::SessionKeystore * GetSessionKeystore()
{
    return &gSessionKeystore;
}

PersistentStorageDelegate * GetPersistentStorageDelegate()
{
    return &gServerStorage;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
