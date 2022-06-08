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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/logging/CHIPLogging.h>

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/KeyValueStoreManager.h>

#if defined(PW_RPC_ENABLED)
#include <CommonRpc.h>
#endif

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#include <signal.h>

#include "CommissionerMain.h"

using namespace chip;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::app::Clusters;

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

DeviceCommissioner gCommissioner;
CommissionerDiscoveryController gCommissionerDiscoveryController;
MyCommissionerCallback gCommissionerCallback;
MyServerStorageDelegate gServerStorage;
ExampleOperationalCredentialsIssuer gOpCredsIssuer;
NodeId gLocalId = kMaxOperationalNodeId;
Credentials::GroupDataProviderImpl gGroupDataProvider;

CHIP_ERROR InitCommissioner(uint16_t commissionerPort, uint16_t udcListenPort)
{
    Controller::FactoryInitParams factoryParams;
    Controller::SetupParams params;

    // use a different listen port for the commissioner than the default used by chip-tool.
    factoryParams.listenPort               = commissionerPort;
    factoryParams.fabricIndependentStorage = &gServerStorage;
    factoryParams.fabricTable              = &Server::GetInstance().GetFabricTable();

    gGroupDataProvider.SetStorageDelegate(&gServerStorage);
    ReturnErrorOnFailure(gGroupDataProvider.Init());
    factoryParams.groupDataProvider = &gGroupDataProvider;

    params.operationalCredentialsDelegate = &gOpCredsIssuer;

    ReturnErrorOnFailure(gOpCredsIssuer.Initialize(gServerStorage));

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
    ReturnErrorOnFailure(ephemeralKey.Initialize());

    ReturnErrorOnFailure(gOpCredsIssuer.GenerateNOCChainAfterValidation(gLocalId, /* fabricId = */ 1, chip::kUndefinedCATs,
                                                                        ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));

    params.operationalKeypair = &ephemeralKey;
    params.controllerRCAC     = rcacSpan;
    params.controllerICAC     = icacSpan;
    params.controllerNOC      = nocSpan;

    auto & factory = Controller::DeviceControllerFactory::GetInstance();
    ReturnErrorOnFailure(factory.Init(factoryParams));
    ReturnErrorOnFailure(factory.SetupCommissioner(params, gCommissioner));

    chip::FabricInfo * fabricInfo = gCommissioner.GetFabricInfo();
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INTERNAL);

    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    MutableByteSpan compressedFabricIdSpan(compressedFabricId);
    ReturnErrorOnFailure(fabricInfo->GetCompressedId(compressedFabricIdSpan));
    ChipLogProgress(Support, "Setting up group data for Fabric Index %u with Compressed Fabric ID:",
                    static_cast<unsigned>(fabricInfo->GetFabricIndex()));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    // TODO: Once ExampleOperationalCredentialsIssuer has support, set default IPK on it as well so
    // that commissioned devices get the IPK set from real values rather than "test-only" internal hookups.
    ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    ReturnLogErrorOnFailure(chip::Credentials::SetSingleIpkEpochKey(&gGroupDataProvider, fabricInfo->GetFabricIndex(), defaultIpk,
                                                                    compressedFabricIdSpan));

    gCommissionerDiscoveryController.SetUserDirectedCommissioningServer(gCommissioner.GetUserDirectedCommissioningServer());
    gCommissionerDiscoveryController.SetCommissionerCallback(&gCommissionerCallback);

    // advertise operational since we are an admin
    app::DnssdServer::Instance().AdvertiseOperational();

    ChipLogProgress(Support, "InitCommissioner nodeId=0x" ChipLogFormatX64 " fabricIndex=%d",
                    ChipLogValueX64(gCommissioner.GetNodeId()), fabricInfo->GetFabricIndex());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ShutdownCommissioner()
{
    UserDirectedCommissioningServer * udcServer = gCommissioner.GetUserDirectedCommissioningServer();
    if (udcServer != nullptr)
    {
        udcServer->SetUserConfirmationProvider(nullptr);
    }

    gCommissioner.Shutdown();
    return CHIP_NO_ERROR;
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

private:
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

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

        gCommissioner.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
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

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

void PairingCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    ChipLogProgress(Controller, "OnDeviceConnectedFn");
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();

    if (device == nullptr)
    {
        ChipLogProgress(AppServer, "No OperationalDeviceProxy returned from OnDeviceConnectedFn");
        if (cdc != nullptr)
        {
            cdc->CommissioningFailed(CHIP_ERROR_INCORRECT_STATE);
        }
        return;
    }

    if (cdc != nullptr)
    {
        // TODO: get from DAC!
        UDCClientState * udc = cdc->GetUDCClientState();
        uint16_t vendorId    = (udc == nullptr ? 0 : udc->GetVendorId());
        uint16_t productId   = (udc == nullptr ? 0 : udc->GetProductId());
        cdc->CommissioningSucceeded(vendorId, productId, gRemoteId, device);
    }
}

void PairingCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR err)
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

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
