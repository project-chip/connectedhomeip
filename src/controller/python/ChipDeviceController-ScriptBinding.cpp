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
#include <controller/CurrentFabricRemover.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <controller/SetUpCodePairer.h>

#include <controller/python/ChipDeviceController-ScriptDevicePairingDelegate.h>
#include <controller/python/ChipDeviceController-ScriptPairingDeviceDiscoveryDelegate.h>
#include <controller/python/ChipDeviceController-StorageDelegate.h>
#include <controller/python/chip/interaction_model/Delegate.h>
#include <controller/python/chip/native/PyChipError.h>

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <crypto/RawKeySessionKeystore.h>
#include <inet/IPAddress.h>
#include <lib/core/TLV.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
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

extern "C" {
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);
typedef void (*DeviceAvailableFunc)(chip::Controller::Python::PyObject * context, DeviceProxy * device, PyChipError err);
typedef void (*ChipThreadTaskRunnerFunct)(intptr_t context);
typedef void (*DeviceUnpairingCompleteFunct)(uint64_t nodeId, PyChipError error);
}

namespace {
chip::Platform::ScopedMemoryBuffer<uint8_t> sSsidBuf;
chip::Platform::ScopedMemoryBuffer<uint8_t> sCredsBuf;
chip::Platform::ScopedMemoryBuffer<uint8_t> sThreadBuf;
chip::Platform::ScopedMemoryBuffer<char> sDefaultNTPBuf;
app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type sDSTBuf;
app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type sTimeZoneBuf;
chip::Platform::ScopedMemoryBuffer<char> sTimeZoneNameBuf;
chip::Controller::CommissioningParameters sCommissioningParameters;

} // namespace

chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
chip::Controller::ScriptPairingDeviceDiscoveryDelegate sPairingDeviceDiscoveryDelegate;
chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
chip::Credentials::PersistentStorageOpCertStore sPersistentStorageOpCertStore;
chip::Crypto::RawKeySessionKeystore sSessionKeystore;

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kDefaultLocalDeviceId = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId       = chip::kTestDeviceNodeId;

extern "C" {
PyChipError pychip_DeviceController_StackInit(Controller::Python::StorageAdapter * storageAdapter, bool enableServerInteractions);
PyChipError pychip_DeviceController_StackShutdown();

PyChipError pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                        chip::NodeId localDeviceId, bool useTestCommissioner);
PyChipError pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl);
PyChipError pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                      char * outAddress, uint64_t maxAddressLen, uint16_t * outPort);
PyChipError pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId);
PyChipError pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId);
PyChipError pychip_DeviceController_GetNodeId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outNodeId);

// Rendezvous
PyChipError pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                               uint32_t setupPINCode, chip::NodeId nodeid);
PyChipError pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                              uint32_t setupPINCode, chip::NodeId nodeid);
PyChipError pychip_DeviceController_ConnectWithCode(chip::Controller::DeviceCommissioner * devCtrl, const char * onboardingPayload,
                                                    chip::NodeId nodeid, uint8_t discoveryType);
PyChipError pychip_DeviceController_UnpairDevice(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId remoteDeviceId,
                                                 DeviceUnpairingCompleteFunct callback);
PyChipError pychip_DeviceController_SetThreadOperationalDataset(const char * threadOperationalDataset, uint32_t size);
PyChipError pychip_DeviceController_SetWiFiCredentials(const char * ssid, const char * credentials);
PyChipError pychip_DeviceController_SetTimeZone(int32_t offset, uint64_t validAt, const char * name);
PyChipError pychip_DeviceController_SetDSTOffset(int32_t offset, uint64_t validStarting, uint64_t validUntil);
PyChipError pychip_DeviceController_SetDefaultNtp(const char * defaultNTP);
PyChipError pychip_DeviceController_SetTrustedTimeSource(chip::NodeId nodeId, chip::EndpointId endpoint);
PyChipError pychip_DeviceController_SetCheckMatchingFabric(bool check);
PyChipError pychip_DeviceController_ResetCommissioningParameters();
PyChipError pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid);
PyChipError pychip_DeviceController_EstablishPASESessionIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                                           uint32_t setupPINCode, chip::NodeId nodeid, uint16_t port);
PyChipError pychip_DeviceController_EstablishPASESessionBLE(chip::Controller::DeviceCommissioner * devCtrl, uint32_t setupPINCode,
                                                            uint16_t discriminator, chip::NodeId nodeid);
PyChipError pychip_DeviceController_EstablishPASESession(chip::Controller::DeviceCommissioner * devCtrl, const char * setUpCode,
                                                         chip::NodeId nodeid);
PyChipError pychip_DeviceController_Commission(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid);

PyChipError pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                 uint16_t long_discriminator);
PyChipError pychip_DeviceController_DiscoverAllCommissionableNodes(chip::Controller::DeviceCommissioner * devCtrl);

PyChipError pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                  uint16_t short_discriminator);
PyChipError pychip_DeviceController_DiscoverCommissionableNodesVendor(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      uint16_t vendor);
PyChipError pychip_DeviceController_DiscoverCommissionableNodesDeviceType(chip::Controller::DeviceCommissioner * devCtrl,
                                                                          uint16_t device_type);
PyChipError pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl);

PyChipError pychip_DeviceController_OnNetworkCommission(chip::Controller::DeviceCommissioner * devCtrl, uint64_t nodeId,
                                                        uint32_t setupPasscode, const uint8_t filterType, const char * filterParam,
                                                        uint32_t discoveryTimeoutMsec);

PyChipError pychip_DeviceController_PostTaskOnChipThread(ChipThreadTaskRunnerFunct callback, void * pythonContext);

PyChipError pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid,
                                                            uint16_t timeout, uint32_t iteration, uint16_t discriminator,
                                                            uint8_t optionInt);

void pychip_DeviceController_PrintDiscoveredDevices(chip::Controller::DeviceCommissioner * devCtrl);
bool pychip_DeviceController_GetIPForDiscoveredDevice(chip::Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len);

// Pairing Delegate
PyChipError
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback);

PyChipError pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnCommissioningCompleteFunct callback);

PyChipError pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback(
    chip::Controller::DeviceCommissioner * devCtrl,
    chip::Controller::DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback);
PyChipError
pychip_ScriptDevicePairingDelegate_SetFabricCheckCallback(chip::Controller::DevicePairingDelegate_OnFabricCheckFunct callback);
PyChipError pychip_ScriptDevicePairingDelegate_SetOpenWindowCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnWindowOpenCompleteFunct callback);

// BLE
PyChipError pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl);

const char * pychip_Stack_ErrorToString(ChipError::StorageType err);
const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void pychip_Stack_SetLogFunct(LogMessageFunct logFunct);

PyChipError pychip_GetConnectedDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                              chip::Controller::Python::PyObject * context, DeviceAvailableFunc callback);
PyChipError pychip_FreeOperationalDeviceProxy(chip::OperationalDeviceProxy * deviceProxy);
PyChipError pychip_GetLocalSessionId(chip::OperationalDeviceProxy * deviceProxy, uint16_t * localSessionId);
PyChipError pychip_GetNumSessionsToPeer(chip::OperationalDeviceProxy * deviceProxy, uint32_t * numSessions);
PyChipError pychip_GetAttestationChallenge(chip::OperationalDeviceProxy * deviceProxy, uint8_t * buf, size_t * size);
PyChipError pychip_GetDeviceBeingCommissioned(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                              CommissioneeDeviceProxy ** proxy);
PyChipError pychip_ExpireSessions(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId);

uint64_t pychip_GetCommandSenderHandle(chip::DeviceProxy * device);

PyChipError pychip_InteractionModel_ShutdownSubscription(SubscriptionId subscriptionId);

//
// Storage
//
void * pychip_Storage_InitializeStorageAdapter(chip::Controller::Python::PyObject * context,
                                               chip::Controller::Python::SyncSetKeyValueCb setCb,
                                               chip::Controller::Python::SetGetKeyValueCb getCb,
                                               chip::Controller::Python::SyncDeleteKeyValueCb deleteCb);
void pychip_Storage_ShutdownAdapter(chip::Controller::Python::StorageAdapter * storageAdapter);
}

void * pychip_Storage_InitializeStorageAdapter(chip::Controller::Python::PyObject * context,
                                               chip::Controller::Python::SyncSetKeyValueCb setCb,
                                               chip::Controller::Python::SetGetKeyValueCb getCb,
                                               chip::Controller::Python::SyncDeleteKeyValueCb deleteCb)
{
    auto ptr = new chip::Controller::Python::StorageAdapter(context, setCb, getCb, deleteCb);
    return ptr;
}

void pychip_Storage_ShutdownAdapter(chip::Controller::Python::StorageAdapter * storageAdapter)
{
    delete storageAdapter;
}

PyChipError pychip_DeviceController_StackInit(Controller::Python::StorageAdapter * storageAdapter, bool enableServerInteractions)
{
    VerifyOrDie(storageAdapter != nullptr);

    FactoryInitParams factoryParams;

    factoryParams.fabricIndependentStorage = storageAdapter;
    factoryParams.sessionKeystore          = &sSessionKeystore;

    sGroupDataProvider.SetStorageDelegate(storageAdapter);
    sGroupDataProvider.SetSessionKeystore(factoryParams.sessionKeystore);
    PyReturnErrorOnFailure(ToPyChipError(sGroupDataProvider.Init()));
    Credentials::SetGroupDataProvider(&sGroupDataProvider);
    factoryParams.groupDataProvider = &sGroupDataProvider;

    PyReturnErrorOnFailure(ToPyChipError(sPersistentStorageOpCertStore.Init(storageAdapter)));
    factoryParams.opCertStore = &sPersistentStorageOpCertStore;

    factoryParams.enableServerInteractions = enableServerInteractions;

    // Hack needed due to the fact that DnsSd server uses the CommissionableDataProvider even
    // when never starting commissionable advertising. This will not be used but prevents
    // null pointer dereferences.
    static chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
    chip::DeviceLayer::SetCommissionableDataProvider(&TestOnlyCommissionableDataProvider);

    PyReturnErrorOnFailure(ToPyChipError(DeviceControllerFactory::GetInstance().Init(factoryParams)));

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
    PyReturnErrorOnFailure(ToPyChipError(chip::DeviceLayer::PlatformMgr().StartEventLoopTask()));

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_StackShutdown()
{
    ChipLogError(Controller, "Shutting down the stack...");

    //
    // Let's stop the Matter thread, and wait till the event loop has stopped.
    //
    PyReturnErrorOnFailure(ToPyChipError(chip::DeviceLayer::PlatformMgr().StopEventLoopTask()));

    //
    // There is the symmetric call to match the Retain called at stack initialization
    // time. This will release all resources (if there are no other controllers active).
    //
    DeviceControllerFactory::GetInstance().ReleaseSystemState();

    DeviceControllerFactory::GetInstance().Shutdown();

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                      char * outAddress, uint64_t maxAddressLen, uint16_t * outPort)
{
    Inet::IPAddress address;
    PyReturnErrorOnFailure(ToPyChipError(devCtrl->GetPeerAddressAndPort(nodeId, address, *outPort)));
    VerifyOrReturnError(address.ToString(outAddress, static_cast<uint32_t>(maxAddressLen)),
                        ToPyChipError(CHIP_ERROR_BUFFER_TOO_SMALL));

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetCompressedFabricId();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetFabricId();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_GetNodeId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outNodeId)
{
    *outNodeId = devCtrl->GetNodeId();
    return ToPyChipError(CHIP_NO_ERROR);
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

PyChipError pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                               uint32_t setupPINCode, chip::NodeId nodeid)
{
    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->PairDevice(nodeid,
                                             chip::RendezvousParameters()
                                                 .SetPeerAddress(Transport::PeerAddress(Transport::Type::kBle))
                                                 .SetSetupPINCode(setupPINCode)
                                                 .SetDiscriminator(discriminator),
                                             sCommissioningParameters));
}

PyChipError pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                              uint32_t setupPINCode, chip::NodeId nodeid)
{
    chip::Inet::IPAddress peerAddr;
    chip::Inet::InterfaceId ifaceOutput;
    chip::Transport::PeerAddress addr;
    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);

    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr, ifaceOutput),
                        ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    // TODO: IP rendezvous should use TCP connection.
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr).SetInterface(ifaceOutput);
    params.SetPeerAddress(addr).SetDiscriminator(0);

    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->PairDevice(nodeid, params, sCommissioningParameters));
}

PyChipError pychip_DeviceController_ConnectWithCode(chip::Controller::DeviceCommissioner * devCtrl, const char * onboardingPayload,
                                                    chip::NodeId nodeid, uint8_t discoveryType)
{
    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->PairDevice(nodeid, onboardingPayload, sCommissioningParameters,
                                             static_cast<chip::Controller::DiscoveryType>(discoveryType)));
}

namespace {
struct UnpairDeviceCallback
{
    UnpairDeviceCallback(DeviceUnpairingCompleteFunct callback, chip::Controller::CurrentFabricRemover * remover) :
        mOnCurrentFabricRemove(OnCurrentFabricRemoveFn, this), mCallback(callback), mRemover(remover)
    {}

    static void OnCurrentFabricRemoveFn(void * context, chip::NodeId nodeId, CHIP_ERROR error)
    {
        auto * self = static_cast<UnpairDeviceCallback *>(context);
        self->mCallback(nodeId, ToPyChipError(error));
        delete self->mRemover;
        delete self;
    }

    Callback::Callback<OnCurrentFabricRemove> mOnCurrentFabricRemove;
    DeviceUnpairingCompleteFunct mCallback;
    chip::Controller::CurrentFabricRemover * mRemover;
};
} // anonymous namespace

PyChipError pychip_DeviceController_UnpairDevice(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid,
                                                 DeviceUnpairingCompleteFunct callback)
{
    // Create a new CurrentFabricRemover instance
    auto * fabricRemover = new chip::Controller::CurrentFabricRemover(devCtrl);

    auto * callbacks = new UnpairDeviceCallback(callback, fabricRemover);

    // Pass the callback and nodeid to the RemoveCurrentFabric function
    CHIP_ERROR err = fabricRemover->RemoveCurrentFabric(nodeid, &callbacks->mOnCurrentFabricRemove);
    if (err != CHIP_NO_ERROR)
    {
        delete fabricRemover;
        delete callbacks;
    }
    // Else will clean up when the callback is called.
    return ToPyChipError(err);
}

PyChipError pychip_DeviceController_OnNetworkCommission(chip::Controller::DeviceCommissioner * devCtrl, uint64_t nodeId,
                                                        uint32_t setupPasscode, const uint8_t filterType, const char * filterParam,
                                                        uint32_t discoveryTimeoutMsec)
{
    Dnssd::DiscoveryFilter filter(static_cast<Dnssd::DiscoveryFilterType>(filterType));
    switch (static_cast<Dnssd::DiscoveryFilterType>(filterType))
    {
    case chip::Dnssd::DiscoveryFilterType::kNone:
        break;
    case chip::Dnssd::DiscoveryFilterType::kShortDiscriminator:
    case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator:
    case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
    case chip::Dnssd::DiscoveryFilterType::kVendorId:
    case chip::Dnssd::DiscoveryFilterType::kDeviceType: {
        // For any numerical filter, convert the string to a filter value
        errno                               = 0;
        unsigned long long int numericalArg = strtoull(filterParam, nullptr, 0);
        if ((numericalArg == ULLONG_MAX) && (errno == ERANGE))
        {
            return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
        }
        filter.code = static_cast<uint64_t>(numericalArg);
        break;
    }
    case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
        break;
    case chip::Dnssd::DiscoveryFilterType::kCommissioner:
        filter.code = 1;
        break;
    case chip::Dnssd::DiscoveryFilterType::kInstanceName:
        filter.code         = 0;
        filter.instanceName = filterParam;
        break;
    default:
        return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
    }

    sPairingDelegate.SetExpectingPairingComplete(true);
    CHIP_ERROR err = sPairingDeviceDiscoveryDelegate.Init(nodeId, setupPasscode, sCommissioningParameters, &sPairingDelegate,
                                                          devCtrl, discoveryTimeoutMsec);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_SetThreadOperationalDataset(const char * threadOperationalDataset, uint32_t size)
{
    ReturnErrorCodeIf(!sThreadBuf.Alloc(size), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    memcpy(sThreadBuf.Get(), threadOperationalDataset, size);
    sCommissioningParameters.SetThreadOperationalDataset(ByteSpan(sThreadBuf.Get(), size));
    return ToPyChipError(CHIP_NO_ERROR);
}
PyChipError pychip_DeviceController_SetWiFiCredentials(const char * ssid, const char * credentials)
{
    size_t ssidSize = strlen(ssid);
    ReturnErrorCodeIf(!sSsidBuf.Alloc(ssidSize), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    memcpy(sSsidBuf.Get(), ssid, ssidSize);

    size_t credsSize = strlen(credentials);
    ReturnErrorCodeIf(!sCredsBuf.Alloc(credsSize), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    memcpy(sCredsBuf.Get(), credentials, credsSize);

    sCommissioningParameters.SetWiFiCredentials(
        chip::Controller::WiFiCredentials(ByteSpan(sSsidBuf.Get(), ssidSize), ByteSpan(sCredsBuf.Get(), credsSize)));
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_SetTimeZone(int32_t offset, uint64_t validAt, const char * name)
{
    sTimeZoneBuf.offset  = offset;
    sTimeZoneBuf.validAt = validAt;
    if (strcmp(name, "") == 0)
    {
        sTimeZoneNameBuf.Free();
        sTimeZoneBuf.name = NullOptional;
    }
    else
    {
        size_t len = strlen(name);
        ReturnErrorCodeIf(!sTimeZoneNameBuf.Alloc(len), ToPyChipError(CHIP_ERROR_NO_MEMORY));
        memcpy(sTimeZoneNameBuf.Get(), name, len);
        sTimeZoneBuf.name.SetValue(CharSpan(sTimeZoneNameBuf.Get(), len));
    }
    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type> list(&sTimeZoneBuf, 1);
    sCommissioningParameters.SetTimeZone(list);
    return ToPyChipError(CHIP_NO_ERROR);
}
PyChipError pychip_DeviceController_SetDSTOffset(int32_t offset, uint64_t validStarting, uint64_t validUntil)
{
    sDSTBuf.offset        = offset;
    sDSTBuf.validStarting = validStarting;
    sDSTBuf.validUntil    = chip::app::DataModel::MakeNullable(validUntil);
    app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type> list(&sDSTBuf, 1);
    sCommissioningParameters.SetDSTOffsets(list);
    return ToPyChipError(CHIP_NO_ERROR);
}
PyChipError pychip_DeviceController_SetDefaultNtp(const char * defaultNTP)
{
    size_t len = strlen(defaultNTP);
    ReturnErrorCodeIf(!sDefaultNTPBuf.Alloc(len), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    memcpy(sDefaultNTPBuf.Get(), defaultNTP, len);
    sCommissioningParameters.SetDefaultNTP(chip::app::DataModel::MakeNullable(CharSpan(sDefaultNTPBuf.Get(), len)));
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_SetTrustedTimeSource(chip::NodeId nodeId, chip::EndpointId endpoint)
{
    chip::app::Clusters::TimeSynchronization::Structs::FabricScopedTrustedTimeSourceStruct::Type timeSource = { .nodeID = nodeId,
                                                                                                                .endpoint =
                                                                                                                    endpoint };
    sCommissioningParameters.SetTrustedTimeSource(chip::app::DataModel::MakeNullable(timeSource));
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_SetCheckMatchingFabric(bool check)
{
    sCommissioningParameters.SetCheckForMatchingFabric(check);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_ResetCommissioningParameters()
{
    sCommissioningParameters = CommissioningParameters();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid)
{
    //
    // Since we permit multiple controllers per fabric and each is associated with a unique fabric index, closing a session
    // requires us to do so across all controllers on the same logical fabric.
    //
    devCtrl->SessionMgr()->ForEachMatchingSessionOnLogicalFabric(ScopedNodeId(nodeid, devCtrl->GetFabricIndex()),
                                                                 [](auto * session) {
                                                                     if (session->IsActiveSession())
                                                                     {
                                                                         session->MarkAsDefunct();
                                                                     }
                                                                 });

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_EstablishPASESessionIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                                           uint32_t setupPINCode, chip::NodeId nodeid, uint16_t port)
{
    chip::Inet::IPAddress peerAddr;
    chip::Inet::InterfaceId ifaceOutput;
    chip::Transport::PeerAddress addr;
    RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);
    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr, ifaceOutput),
                        ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr).SetInterface(ifaceOutput);
    if (port != 0)
    {
        addr.SetPort(port);
    }
    params.SetPeerAddress(addr).SetDiscriminator(0);
    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->EstablishPASEConnection(nodeid, params));
}

PyChipError pychip_DeviceController_EstablishPASESessionBLE(chip::Controller::DeviceCommissioner * devCtrl, uint32_t setupPINCode,
                                                            uint16_t discriminator, chip::NodeId nodeid)
{
    chip::Transport::PeerAddress addr;
    RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);
    addr.SetTransportType(chip::Transport::Type::kBle);
    params.SetPeerAddress(addr).SetDiscriminator(discriminator);
    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->EstablishPASEConnection(nodeid, params));
}

PyChipError pychip_DeviceController_EstablishPASESession(chip::Controller::DeviceCommissioner * devCtrl, const char * setUpCode,
                                                         chip::NodeId nodeid)
{
    sPairingDelegate.SetExpectingPairingComplete(true);
    return ToPyChipError(devCtrl->EstablishPASEConnection(nodeid, setUpCode));
}

PyChipError pychip_DeviceController_Commission(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid)
{
    CommissioningParameters params;
    return ToPyChipError(devCtrl->Commission(nodeid, params));
}

PyChipError pychip_DeviceController_DiscoverAllCommissionableNodes(chip::Controller::DeviceCommissioner * devCtrl)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, static_cast<uint64_t>(0));
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                 uint16_t long_discriminator)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kLongDiscriminator, long_discriminator);
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                  uint16_t short_discriminator)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kShortDiscriminator, short_discriminator);
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodesVendor(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      uint16_t vendor)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kVendorId, vendor);
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodesDeviceType(chip::Controller::DeviceCommissioner * devCtrl,
                                                                          uint16_t device_type)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kDeviceType, device_type);
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl)
{
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kCommissioningMode);
    return ToPyChipError(devCtrl->DiscoverCommissionableNodes(filter));
}

PyChipError pychip_ScriptDevicePairingDelegate_SetOpenWindowCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnWindowOpenCompleteFunct callback)
{
    sPairingDelegate.SetCommissioningWindowOpenCallback(callback);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid,
                                                            uint16_t timeout, uint32_t iteration, uint16_t discriminator,
                                                            uint8_t optionInt)
{
    const auto option = static_cast<Controller::CommissioningWindowOpener::CommissioningWindowOption>(optionInt);
    if (option == Controller::CommissioningWindowOpener::CommissioningWindowOption::kOriginalSetupCode)
    {
        return ToPyChipError(Controller::AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(
            devCtrl, nodeid, System::Clock::Seconds16(timeout)));
    }

    if (option == Controller::CommissioningWindowOpener::CommissioningWindowOption::kTokenWithRandomPIN)
    {
        SetupPayload payload;
        auto opener =
            Platform::New<Controller::CommissioningWindowOpener>(static_cast<chip::Controller::DeviceController *>(devCtrl));
        PyChipError err = ToPyChipError(opener->OpenCommissioningWindow(nodeid, System::Clock::Seconds16(timeout), iteration,
                                                                        discriminator, NullOptional, NullOptional,
                                                                        sPairingDelegate.GetOpenWindowCallback(opener), payload));
        return err;
    }

    return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
}

PyChipError
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    sPairingDelegate.SetKeyExchangeCallback(callback);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnCommissioningCompleteFunct callback)
{
    sPairingDelegate.SetCommissioningCompleteCallback(callback);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_ScriptDevicePairingDelegate_SetCommissioningStatusUpdateCallback(
    chip::Controller::DeviceCommissioner * devCtrl,
    chip::Controller::DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback)
{
    sPairingDelegate.SetCommissioningStatusUpdateCallback(callback);
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError
pychip_ScriptDevicePairingDelegate_SetFabricCheckCallback(chip::Controller::DevicePairingDelegate_OnFabricCheckFunct callback)
{
    sPairingDelegate.SetFabricCheckCallback(callback);
    return ToPyChipError(CHIP_NO_ERROR);
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
    GetDeviceCallbacks(chip::Controller::Python::PyObject * context, DeviceAvailableFunc callback) :
        mOnSuccess(OnDeviceConnectedFn, this), mOnFailure(OnConnectionFailureFn, this), mContext(context), mCallback(callback)
    {}

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
    {
        auto * self                   = static_cast<GetDeviceCallbacks *>(context);
        auto * operationalDeviceProxy = new OperationalDeviceProxy(&exchangeMgr, sessionHandle);
        self->mCallback(self->mContext, operationalDeviceProxy, ToPyChipError(CHIP_NO_ERROR));
        delete self;
    }

    static void OnConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
    {
        auto * self = static_cast<GetDeviceCallbacks *>(context);
        self->mCallback(self->mContext, nullptr, ToPyChipError(error));
        delete self;
    }

    Callback::Callback<OnDeviceConnected> mOnSuccess;
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure;
    chip::Controller::Python::PyObject * const mContext;
    DeviceAvailableFunc mCallback;
};
} // anonymous namespace

PyChipError pychip_GetConnectedDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                              chip::Controller::Python::PyObject * context, DeviceAvailableFunc callback)
{
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
    auto * callbacks = new GetDeviceCallbacks(context, callback);
    return ToPyChipError(devCtrl->GetConnectedDevice(nodeId, &callbacks->mOnSuccess, &callbacks->mOnFailure));
}

PyChipError pychip_FreeOperationalDeviceProxy(chip::OperationalDeviceProxy * deviceProxy)
{
    if (deviceProxy != nullptr)
    {
        delete deviceProxy;
    }
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_GetLocalSessionId(chip::OperationalDeviceProxy * deviceProxy, uint16_t * localSessionId)
{
    VerifyOrReturnError(deviceProxy->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));
    VerifyOrReturnError(localSessionId != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    *localSessionId = deviceProxy->GetSecureSession().Value()->AsSecureSession()->GetLocalSessionId();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_GetNumSessionsToPeer(chip::OperationalDeviceProxy * deviceProxy, uint32_t * numSessions)
{
    VerifyOrReturnError(deviceProxy->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));
    VerifyOrReturnError(numSessions != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    *numSessions = 0;
    deviceProxy->GetExchangeManager()->GetSessionManager()->ForEachMatchingSession(
        deviceProxy->GetPeerScopedNodeId(), [numSessions](auto * session) { (*numSessions)++; });

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_GetAttestationChallenge(chip::OperationalDeviceProxy * deviceProxy, uint8_t * buf, size_t * size)
{
    VerifyOrReturnError(deviceProxy->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_MISSING_SECURE_SESSION));
    VerifyOrReturnError(buf != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    ByteSpan challenge = deviceProxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    VerifyOrReturnError(challenge.size() <= *size, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
    memcpy(buf, challenge.data(), challenge.size());
    *size = challenge.size();

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_GetDeviceBeingCommissioned(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                              CommissioneeDeviceProxy ** proxy)
{
    return ToPyChipError(devCtrl->GetDeviceBeingCommissioned(nodeId, proxy));
}

// This is a method called VERY seldom, just for RemoveFabric/UpdateNOC
PyChipError pychip_ExpireSessions(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId)
{
    VerifyOrReturnError((devCtrl != nullptr) && (devCtrl->SessionMgr() != nullptr), ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    //
    // Stop any active pairing sessions to this node.
    //
    devCtrl->StopPairing(nodeId);

    //
    // Since we permit multiple controllers on the same fabric each associated with a different fabric index, expiring a session
    // needs to correctly expire sessions on other controllers on matching fabrics as well.
    //
    devCtrl->SessionMgr()->ExpireAllSessionsOnLogicalFabric(ScopedNodeId(nodeId, devCtrl->GetFabricIndex()));
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl)
{
#if CONFIG_NETWORK_LAYER_BLE
    devCtrl->CloseBleConnection();
    return ToPyChipError(CHIP_NO_ERROR);
#else
    return ToPyChipError(CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
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

PyChipError pychip_DeviceController_PostTaskOnChipThread(ChipThreadTaskRunnerFunct callback, void * pythonContext)
{
    if (callback == nullptr || pythonContext == nullptr)
    {
        return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
    }
    PlatformMgr().ScheduleWork(callback, reinterpret_cast<intptr_t>(pythonContext));
    return ToPyChipError(CHIP_NO_ERROR);
}
