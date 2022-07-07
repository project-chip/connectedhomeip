/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      Implementation of the native methods expected by the Python
 *      version of Chip Device Manager.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <type_traits>
#include <unistd.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#include <inttypes.h>
#include <net/if.h>

#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Dnssd.h>
#include <controller/AutoCommissioner.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioningDelegate.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>

#include <controller/python/ChipDeviceController-ScriptDevicePairingDelegate.h>
#include <controller/python/ChipDeviceController-StorageDelegate.h>
#include <controller/python/chip/interaction_model/Delegate.h>

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <inet/IPAddress.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <system/SystemClock.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/PlatformManager.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

using namespace chip;
using namespace chip::Ble;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);
typedef void (*DeviceAvailableFunc)(DeviceProxy * device, ChipError::StorageType err);
typedef void (*ChipThreadTaskRunnerFunct)(intptr_t context);
}

namespace {
chip::Platform::ScopedMemoryBuffer<uint8_t> sSsidBuf;
chip::Platform::ScopedMemoryBuffer<uint8_t> sCredsBuf;
chip::Platform::ScopedMemoryBuffer<uint8_t> sThreadBuf;
chip::Controller::CommissioningParameters sCommissioningParameters;

} // namespace

chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
chip::Controller::Python::StorageAdapter * sStorageAdapter = nullptr;
chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
chip::Credentials::PersistentStorageOpCertStore sPersistentStorageOpCertStore;

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kDefaultLocalDeviceId = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId       = chip::kTestDeviceNodeId;

extern "C" {
ChipError::StorageType pychip_DeviceController_StackInit(uint32_t bluetoothAdapterId);
ChipError::StorageType pychip_DeviceController_StackShutdown();

ChipError::StorageType pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                                   chip::NodeId localDeviceId, bool useTestCommissioner);
ChipError::StorageType pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl);
ChipError::StorageType pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl,
                                                                 chip::NodeId nodeId, char * outAddress, uint64_t maxAddressLen,
                                                                 uint16_t * outPort);
ChipError::StorageType pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint64_t * outFabricId);
ChipError::StorageType pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId);
ChipError::StorageType pychip_DeviceController_GetNodeId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outNodeId);

// Rendezvous
ChipError::StorageType pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                                          uint32_t setupPINCode, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                                         uint32_t setupPINCode, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_ConnectWithCode(chip::Controller::DeviceCommissioner * devCtrl,
                                                               const char * onboardingPayload, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_SetThreadOperationalDataset(const char * threadOperationalDataset, uint32_t size);
ChipError::StorageType pychip_DeviceController_SetWiFiCredentials(const char * ssid, const char * credentials);
ChipError::StorageType pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_EstablishPASESessionIP(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      const char * peerAddrStr, uint32_t setupPINCode,
                                                                      chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_Commission(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid);

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint16_t long_discriminator);
ChipError::StorageType pychip_DeviceController_DiscoverAllCommissionableNodes(chip::Controller::DeviceCommissioner * devCtrl);

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      uint16_t short_discriminator);
ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesVendor(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                 uint16_t vendor);
ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesDeviceType(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                     uint16_t device_type);
ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl);
ChipError::StorageType pychip_DeviceController_PostTaskOnChipThread(ChipThreadTaskRunnerFunct callback, void * pythonContext);

ChipError::StorageType pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl,
                                                                       chip::NodeId nodeid, uint16_t timeout, uint32_t iteration,
                                                                       uint16_t discriminator, uint8_t optionInt);

void pychip_DeviceController_PrintDiscoveredDevices(chip::Controller::DeviceCommissioner * devCtrl);
bool pychip_DeviceController_GetIPForDiscoveredDevice(chip::Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len);

// Pairing Delegate
ChipError::StorageType
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback);

ChipError::StorageType pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnCommissioningCompleteFunct callback);

ChipError::StorageType pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback(
    chip::Controller::DeviceCommissioner * devCtrl,
    chip::Controller::DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback);

// BLE
ChipError::StorageType pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl);

uint8_t pychip_DeviceController_GetLogFilter();
void pychip_DeviceController_SetLogFilter(uint8_t category);

const char * pychip_Stack_ErrorToString(ChipError::StorageType err);
const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void pychip_Stack_SetLogFunct(LogMessageFunct logFunct);

ChipError::StorageType pychip_GetConnectedDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                         DeviceAvailableFunc callback);
ChipError::StorageType pychip_GetDeviceBeingCommissioned(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                         CommissioneeDeviceProxy ** proxy);
uint64_t pychip_GetCommandSenderHandle(chip::DeviceProxy * device);

chip::ChipError::StorageType pychip_InteractionModel_ShutdownSubscription(SubscriptionId subscriptionId);

//
// Storage
//
void pychip_Storage_InitializeStorageAdapter(chip::Controller::Python::PyObject * context,
                                             chip::Controller::Python::SyncSetKeyValueCb setCb,
                                             chip::Controller::Python::SetGetKeyValueCb getCb,
                                             chip::Controller::Python::SyncDeleteKeyValueCb deleteCb);
void pychip_Storage_ShutdownAdapter();
}

void pychip_Storage_InitializeStorageAdapter(chip::Controller::Python::PyObject * context,
                                             chip::Controller::Python::SyncSetKeyValueCb setCb,
                                             chip::Controller::Python::SetGetKeyValueCb getCb,
                                             chip::Controller::Python::SyncDeleteKeyValueCb deleteCb)
{
    sStorageAdapter = new chip::Controller::Python::StorageAdapter(context, setCb, getCb, deleteCb);
}

void pychip_Storage_ShutdownAdapter()
{
    delete sStorageAdapter;
}

chip::Controller::Python::StorageAdapter * pychip_Storage_GetStorageAdapter()
{
    return sStorageAdapter;
}

ChipError::StorageType pychip_DeviceController_StackInit(uint32_t bluetoothAdapterId)
{
    VerifyOrDie(sStorageAdapter != nullptr);

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    CHIP_ERROR err =
        chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(/* BLE adapter ID */ bluetoothAdapterId, /* BLE central */ true);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());
#endif

    FactoryInitParams factoryParams;
    factoryParams.fabricIndependentStorage = sStorageAdapter;

    sGroupDataProvider.SetStorageDelegate(sStorageAdapter);
    ReturnErrorOnFailure(sGroupDataProvider.Init().AsInteger());
    factoryParams.groupDataProvider = &sGroupDataProvider;

    ReturnErrorOnFailure(sPersistentStorageOpCertStore.Init(sStorageAdapter).AsInteger());
    factoryParams.opCertStore = &sPersistentStorageOpCertStore;

    factoryParams.enableServerInteractions = true;

    // Hack needed due to the fact that DnsSd server uses the CommissionableDataProvider even
    // when never starting commissionable advertising. This will not be used but prevents
    // null pointer dereferences.
    static chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
    chip::DeviceLayer::SetCommissionableDataProvider(&TestOnlyCommissionableDataProvider);

    ReturnErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryParams).AsInteger());

    //
    // In situations where all the controller instances get shutdown, the entire stack is then also
    // implicitly shutdown. In the REPL, users can create such a situation by manually shutting down
    // controllers (for example, when they call ChipReplStartup::LoadFabricAdmins multiple times). In
    // that situation, momentarily, the stack gets de-initialized. This results in further interactions with
    // the stack being dangerous (and in fact, causes crashes).
    //
    // This retain call ensures the stack doesn't get de-initialized in the REPL.
    //
    DeviceControllerFactory::GetInstance().RetainSystemState();

    //
    // Finally, start up the main Matter thread. Any further interactions with the stack
    // will now need to happen on the Matter thread, OR protected with the stack lock.
    //
    ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgr().StartEventLoopTask().AsInteger());

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_StackShutdown()
{
    ChipLogError(Controller, "Shutting down the stack...");

    //
    // Let's stop the Matter thread, and wait till the event loop has stopped.
    //
    ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask().AsInteger());

    //
    // There is the symmetric call to match the Retain called at stack initialization
    // time. This will release all resources (if there are no other controllers active).
    //
    DeviceControllerFactory::GetInstance().ReleaseSystemState();

    DeviceControllerFactory::GetInstance().Shutdown();

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl,
                                                                 chip::NodeId nodeId, char * outAddress, uint64_t maxAddressLen,
                                                                 uint16_t * outPort)
{
    Inet::IPAddress address;
    ReturnErrorOnFailure(
        devCtrl
            ->GetPeerAddressAndPort(PeerId().SetCompressedFabricId(devCtrl->GetCompressedFabricId()).SetNodeId(nodeId), address,
                                    *outPort)
            .AsInteger());
    VerifyOrReturnError(address.ToString(outAddress, maxAddressLen), CHIP_ERROR_BUFFER_TOO_SMALL.AsInteger());

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetCompressedFabricId();
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetFabricId();
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetNodeId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outNodeId)
{
    *outNodeId = devCtrl->GetNodeId();
    return CHIP_NO_ERROR.AsInteger();
}

const char * pychip_DeviceController_ErrorToString(ChipError::StorageType err)
{
    return chip::ErrorStr(CHIP_ERROR(err));
}

const char * pychip_DeviceController_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return nullptr;
}

uint8_t pychip_DeviceController_GetLogFilter()
{
#if _CHIP_USE_LOGGING
    return chip::Logging::GetLogFilter();
#else
    return chip::Logging::kLogCategory_None;
#endif
}

void pychip_DeviceController_SetLogFilter(uint8_t category)
{
#if _CHIP_USE_LOGGING
    chip::Logging::SetLogFilter(category);
#endif
}

ChipError::StorageType pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                                          uint32_t setupPINCode, chip::NodeId nodeid)
{
    return devCtrl
        ->PairDevice(nodeid,
                     chip::RendezvousParameters()
                         .SetPeerAddress(Transport::PeerAddress(Transport::Type::kBle))
                         .SetSetupPINCode(setupPINCode)
                         .SetDiscriminator(discriminator),
                     sCommissioningParameters)
        .AsInteger();
}

ChipError::StorageType pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                                         uint32_t setupPINCode, chip::NodeId nodeid)
{
    chip::Inet::IPAddress peerAddr;
    chip::Transport::PeerAddress addr;
    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);

    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr), CHIP_ERROR_INVALID_ARGUMENT.AsInteger());

    // TODO: IP rendezvous should use TCP connection.
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr);
    params.SetPeerAddress(addr).SetDiscriminator(0);

    devCtrl->ReleaseOperationalDevice(nodeid);

    return devCtrl->PairDevice(nodeid, params, sCommissioningParameters).AsInteger();
}

ChipError::StorageType pychip_DeviceController_ConnectWithCode(chip::Controller::DeviceCommissioner * devCtrl,
                                                               const char * onboardingPayload, chip::NodeId nodeid)
{
    return devCtrl->PairDevice(nodeid, onboardingPayload, sCommissioningParameters).AsInteger();
}

ChipError::StorageType pychip_DeviceController_SetThreadOperationalDataset(const char * threadOperationalDataset, uint32_t size)
{
    ReturnErrorCodeIf(!sThreadBuf.Alloc(size), CHIP_ERROR_NO_MEMORY.AsInteger());
    memcpy(sThreadBuf.Get(), threadOperationalDataset, size);
    sCommissioningParameters.SetThreadOperationalDataset(ByteSpan(sThreadBuf.Get(), size));
    return CHIP_NO_ERROR.AsInteger();
}
ChipError::StorageType pychip_DeviceController_SetWiFiCredentials(const char * ssid, const char * credentials)
{
    size_t ssidSize = strlen(ssid);
    ReturnErrorCodeIf(!sSsidBuf.Alloc(ssidSize), CHIP_ERROR_NO_MEMORY.AsInteger());
    memcpy(sSsidBuf.Get(), ssid, ssidSize);

    size_t credsSize = strlen(credentials);
    ReturnErrorCodeIf(!sCredsBuf.Alloc(credsSize), CHIP_ERROR_NO_MEMORY.AsInteger());
    memcpy(sCredsBuf.Get(), credentials, credsSize);

    sCommissioningParameters.SetWiFiCredentials(
        chip::Controller::WiFiCredentials(ByteSpan(sSsidBuf.Get(), ssidSize), ByteSpan(sCredsBuf.Get(), credsSize)));
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid)
{
    return devCtrl->DisconnectDevice(nodeid).AsInteger();
}

ChipError::StorageType pychip_DeviceController_EstablishPASESessionIP(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      const char * peerAddrStr, uint32_t setupPINCode,
                                                                      chip::NodeId nodeid)
{
    chip::Inet::IPAddress peerAddr;
    chip::Transport::PeerAddress addr;
    RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);
    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr), CHIP_ERROR_INVALID_ARGUMENT.AsInteger());
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr);
    params.SetPeerAddress(addr).SetDiscriminator(0);
    return devCtrl->EstablishPASEConnection(nodeid, params).AsInteger();
}
ChipError::StorageType pychip_DeviceController_Commission(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid)
{
    CommissioningParameters params;
    return devCtrl->Commission(nodeid, params).AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverAllCommissionableNodes(chip::Controller::DeviceCommissioner * devCtrl)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, static_cast<uint64_t>(0));
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint16_t long_discriminator)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kLongDiscriminator, long_discriminator);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      uint16_t short_discriminator)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kShortDiscriminator, short_discriminator);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesVendor(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                 uint16_t vendor)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kVendorId, vendor);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesDeviceType(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                     uint16_t device_type)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kDeviceType, device_type);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kCommissioningMode);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl,
                                                                       chip::NodeId nodeid, uint16_t timeout, uint32_t iteration,
                                                                       uint16_t discriminator, uint8_t optionInt)
{
    const auto option = static_cast<Controller::CommissioningWindowOpener::CommissioningWindowOption>(optionInt);
    if (option == Controller::CommissioningWindowOpener::CommissioningWindowOption::kOriginalSetupCode)
    {
        return Controller::AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(devCtrl, nodeid,
                                                                                       System::Clock::Seconds16(timeout))
            .AsInteger();
    }

    if (option == Controller::CommissioningWindowOpener::CommissioningWindowOption::kTokenWithRandomPIN)
    {
        SetupPayload payload;
        return Controller::AutoCommissioningWindowOpener::OpenCommissioningWindow(
                   devCtrl, nodeid, System::Clock::Seconds16(timeout), iteration, discriminator, NullOptional, NullOptional,
                   payload)
            .AsInteger();
    }

    return CHIP_ERROR_INVALID_ARGUMENT.AsInteger();
}

void pychip_DeviceController_PrintDiscoveredDevices(chip::Controller::DeviceCommissioner * devCtrl)
{
    for (int i = 0; i < devCtrl->GetMaxCommissionableNodesSupported(); ++i)
    {
        const chip::Dnssd::DiscoveredNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->commissionData.rotatingId, dnsSdInfo->commissionData.rotatingIdLen,
                                            rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissionable Node %d", i);
        ChipLogProgress(Discovery, "\tInstance name:\t\t%s", dnsSdInfo->commissionData.instanceName);
        ChipLogProgress(Discovery, "\tHost name:\t\t%s", dnsSdInfo->resolutionData.hostName);
        ChipLogProgress(Discovery, "\tPort:\t\t\t%u", dnsSdInfo->resolutionData.port);
        ChipLogProgress(Discovery, "\tLong discriminator:\t%u", dnsSdInfo->commissionData.longDiscriminator);
        ChipLogProgress(Discovery, "\tVendor ID:\t\t%u", dnsSdInfo->commissionData.vendorId);
        ChipLogProgress(Discovery, "\tProduct ID:\t\t%u", dnsSdInfo->commissionData.productId);
        ChipLogProgress(Discovery, "\tCommissioning Mode\t%u", dnsSdInfo->commissionData.commissioningMode);
        ChipLogProgress(Discovery, "\tDevice Type\t\t%u", dnsSdInfo->commissionData.deviceType);
        ChipLogProgress(Discovery, "\tDevice Name\t\t%s", dnsSdInfo->commissionData.deviceName);
        ChipLogProgress(Discovery, "\tRotating Id\t\t%s", rotatingId);
        ChipLogProgress(Discovery, "\tPairing Instruction\t%s", dnsSdInfo->commissionData.pairingInstruction);
        ChipLogProgress(Discovery, "\tPairing Hint\t\t%u", dnsSdInfo->commissionData.pairingHint);
        if (dnsSdInfo->resolutionData.GetMrpRetryIntervalIdle().HasValue())
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\t%u",
                            dnsSdInfo->resolutionData.GetMrpRetryIntervalIdle().Value().count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval idle\tNot present");
        }
        if (dnsSdInfo->resolutionData.GetMrpRetryIntervalActive().HasValue())
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\t%u",
                            dnsSdInfo->resolutionData.GetMrpRetryIntervalActive().Value().count());
        }
        else
        {
            ChipLogProgress(Discovery, "\tMrp Interval active\tNot present");
        }
        ChipLogProgress(Discovery, "\tSupports TCP\t\t%d", dnsSdInfo->resolutionData.supportsTcp);
        for (unsigned j = 0; j < dnsSdInfo->resolutionData.numIPs; ++j)
        {
            char buf[chip::Inet::IPAddress::kMaxStringLength];
            dnsSdInfo->resolutionData.ipAddress[j].ToString(buf);
            ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", j, buf);
        }
    }
}

bool pychip_DeviceController_GetIPForDiscoveredDevice(chip::Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len)
{
    const chip::Dnssd::DiscoveredNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(idx);
    if (dnsSdInfo == nullptr)
    {
        return false;
    }
    // TODO(cecille): Select which one we actually want.
    if (dnsSdInfo->resolutionData.ipAddress[0].ToString(addrStr, len) == addrStr)
    {
        return true;
    }
    return false;
}

ChipError::StorageType
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    sPairingDelegate.SetKeyExchangeCallback(callback);
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnCommissioningCompleteFunct callback)
{
    sPairingDelegate.SetCommissioningCompleteCallback(callback);
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback(
    chip::Controller::DeviceCommissioner * devCtrl,
    chip::Controller::DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback)
{
    sPairingDelegate.SetCommissioningStatusUpdateCallback(callback);
    return CHIP_NO_ERROR.AsInteger();
}

const char * pychip_Stack_ErrorToString(ChipError::StorageType err)
{
    return chip::ErrorStr(CHIP_ERROR(err));
}

const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return nullptr;
}

namespace {
struct GetDeviceCallbacks
{
    GetDeviceCallbacks(DeviceAvailableFunc callback) :
        mOnSuccess(OnDeviceConnectedFn, this), mOnFailure(OnConnectionFailureFn, this), mCallback(callback)
    {}

    static void OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device)
    {
        auto * self = static_cast<GetDeviceCallbacks *>(context);
        self->mCallback(device, CHIP_NO_ERROR.AsInteger());
        delete self;
    }

    static void OnConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error)
    {
        auto * self = static_cast<GetDeviceCallbacks *>(context);
        self->mCallback(nullptr, error.AsInteger());
        delete self;
    }

    Callback::Callback<OnDeviceConnected> mOnSuccess;
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure;
    DeviceAvailableFunc mCallback;
};
} // anonymous namespace

ChipError::StorageType pychip_GetConnectedDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                         DeviceAvailableFunc callback)
{
    VerifyOrReturnError(devCtrl != nullptr, CHIP_ERROR_INVALID_ARGUMENT.AsInteger());
    auto * callbacks = new GetDeviceCallbacks(callback);
    return devCtrl->GetConnectedDevice(nodeId, &callbacks->mOnSuccess, &callbacks->mOnFailure).AsInteger();
}

ChipError::StorageType pychip_GetDeviceBeingCommissioned(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                         CommissioneeDeviceProxy ** proxy)
{
    return devCtrl->GetDeviceBeingCommissioned(nodeId, proxy).AsInteger();
}

ChipError::StorageType pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl)
{
#if CONFIG_NETWORK_LAYER_BLE
    devCtrl->CloseBleConnection();
    return CHIP_NO_ERROR.AsInteger();
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.AsInteger();
#endif
}

uint64_t pychip_GetCommandSenderHandle(chip::DeviceProxy * device)
{
    return 0;
}

void pychip_Stack_SetLogFunct(LogMessageFunct logFunct)
{
    // TODO: determine if log redirection is supposed to be functioning in CHIP
    //
    // Background: original log baseline supported 'redirect logs to this
    // function' however CHIP does not currently provide this.
    //
    // Ideally log redirection should work so that python code can do things
    // like using the log module.
}

ChipError::StorageType pychip_DeviceController_PostTaskOnChipThread(ChipThreadTaskRunnerFunct callback, void * pythonContext)
{
    if (callback == nullptr || pythonContext == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT.AsInteger();
    }
    PlatformMgr().ScheduleWork(callback, reinterpret_cast<intptr_t>(pythonContext));
    return CHIP_NO_ERROR.AsInteger();
}
