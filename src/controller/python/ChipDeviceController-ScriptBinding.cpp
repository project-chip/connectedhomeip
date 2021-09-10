/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include "ChipDeviceController-ScriptDeviceAddressUpdateDelegate.h"
#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "ChipDeviceController-StorageDelegate.h"

#include "chip/interaction_model/Delegate.h"

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <inet/IPAddress.h>
#include <lib/mdns/Resolver.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

using namespace chip;
using namespace chip::Ble;
using namespace chip::Controller;
using namespace chip::DeviceLayer;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);
typedef void (*DeviceAvailableFunc)(Device * device, ChipError::StorageType err);
typedef void (*ChipThreadTaskRunnerFunct)(intptr_t context);
}

namespace {
chip::Controller::PythonPersistentStorageDelegate sStorageDelegate;
chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
chip::Controller::ScriptDeviceAddressUpdateDelegate sDeviceAddressUpdateDelegate;
chip::Controller::ExampleOperationalCredentialsIssuer sOperationalCredentialsIssuer;
} // namespace

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kDefaultLocalDeviceId = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId       = chip::kTestDeviceNodeId;

extern "C" {
ChipError::StorageType pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                                   chip::NodeId localDeviceId);
ChipError::StorageType pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl);
ChipError::StorageType pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl,
                                                                 chip::NodeId nodeId, char * outAddress, uint64_t maxAddressLen,
                                                                 uint16_t * outPort);
ChipError::StorageType pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint64_t * outFabricId);
ChipError::StorageType pychip_DeviceController_CommissioningComplete(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     chip::NodeId nodeId);
ChipError::StorageType pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId);

// Rendezvous
ChipError::StorageType pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                                          uint32_t setupPINCode, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                                         uint32_t setupPINCode, chip::NodeId nodeid);
ChipError::StorageType pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid);

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
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl,
                                                                        uint16_t enabled);
ChipError::StorageType pychip_DeviceController_PostTaskOnChipThread(ChipThreadTaskRunnerFunct callback, void * pythonContext);

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabledFromCommand(chip::Controller::DeviceCommissioner * devCtrl);

ChipError::StorageType pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl,
                                                                       chip::NodeId nodeid, uint16_t timeout, uint16_t iteration,
                                                                       uint16_t discriminator, uint8_t option);

void pychip_DeviceController_PrintDiscoveredDevices(chip::Controller::DeviceCommissioner * devCtrl);
bool pychip_DeviceController_GetIPForDiscoveredDevice(chip::Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len);

// Pairing Delegate
ChipError::StorageType
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback);

ChipError::StorageType pychip_ScriptDevicePairingDelegate_SetCommissioningCompleteCallback(
    chip::Controller::DeviceCommissioner * devCtrl, chip::Controller::DevicePairingDelegate_OnCommissioningCompleteFunct callback);

void pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete(
    chip::Controller::DeviceAddressUpdateDelegate_OnUpdateComplete callback);
ChipError::StorageType pychip_Resolver_ResolveNode(uint64_t fabricid, chip::NodeId nodeid);

// BLE
ChipError::StorageType pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl);

uint8_t pychip_DeviceController_GetLogFilter();
void pychip_DeviceController_SetLogFilter(uint8_t category);

ChipError::StorageType pychip_Stack_Init();
ChipError::StorageType pychip_Stack_Shutdown();
const char * pychip_Stack_ErrorToString(ChipError::StorageType err);
const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void pychip_Stack_SetLogFunct(LogMessageFunct logFunct);

ChipError::StorageType pychip_GetDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                chip::Controller::Device ** device);
ChipError::StorageType pychip_GetConnectedDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                         DeviceAvailableFunc callback);
uint64_t pychip_GetCommandSenderHandle(chip::Controller::Device * device);
// CHIP Stack objects
ChipError::StorageType pychip_BLEMgrImpl_ConfigureBle(uint32_t bluetoothAdapterId);
}

ChipError::StorageType pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                                   chip::NodeId localDeviceId)
{
    *outDevCtrl = new chip::Controller::DeviceCommissioner();
    VerifyOrReturnError(*outDevCtrl != NULL, CHIP_ERROR_NO_MEMORY.AsInteger());

    if (localDeviceId == chip::kUndefinedNodeId)
    {
        localDeviceId = kDefaultLocalDeviceId;
    }

    CHIP_ERROR err = sOperationalCredentialsIssuer.Initialize(sStorageDelegate);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    chip::Crypto::P256Keypair ephemeralKey;
    err = ephemeralKey.Initialize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    ReturnErrorCodeIf(!icac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan icacSpan(icac.Get(), kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    err = sOperationalCredentialsIssuer.GenerateNOCChainAfterValidation(localDeviceId, 0, ephemeralKey.Pubkey(), rcacSpan, icacSpan,
                                                                        nocSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    CommissionerInitParams initParams;
    initParams.storageDelegate                = &sStorageDelegate;
    initParams.mDeviceAddressUpdateDelegate   = &sDeviceAddressUpdateDelegate;
    initParams.pairingDelegate                = &sPairingDelegate;
    initParams.operationalCredentialsDelegate = &sOperationalCredentialsIssuer;
    initParams.imDelegate                     = &PythonInteractionModelDelegate::Instance();
    initParams.ephemeralKeypair               = &ephemeralKey;
    initParams.controllerRCAC                 = rcacSpan;
    initParams.controllerICAC                 = icacSpan;
    initParams.controllerNOC                  = nocSpan;

    (*outDevCtrl)->SetUdpListenPort(0);
    err = (*outDevCtrl)->Init(initParams);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl)
{
    if (devCtrl != NULL)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetAddressAndPort(chip::Controller::DeviceCommissioner * devCtrl,
                                                                 chip::NodeId nodeId, char * outAddress, uint64_t maxAddressLen,
                                                                 uint16_t * outPort)
{
    Device * device;
    CHIP_ERROR err = devCtrl->GetDevice(nodeId, &device);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    Inet::IPAddress address;
    VerifyOrReturnError(device->GetAddress(address, *outPort), CHIP_ERROR_INCORRECT_STATE.AsInteger());
    VerifyOrReturnError(address.ToString(outAddress, maxAddressLen), CHIP_ERROR_BUFFER_TOO_SMALL.AsInteger());

    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetCompressedFabricId(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetCompressedFabricId();
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_DeviceController_CommissioningComplete(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     chip::NodeId nodeId)
{
    return devCtrl->CommissioningComplete(nodeId).AsInteger();
}

ChipError::StorageType pychip_DeviceController_GetFabricId(chip::Controller::DeviceCommissioner * devCtrl, uint64_t * outFabricId)
{
    *outFabricId = devCtrl->GetFabricId();
    return CHIP_NO_ERROR.AsInteger();
}

const char * pychip_DeviceController_ErrorToString(ChipError::StorageType err)
{
    return chip::ErrorStr(CHIP_ERROR(err));
}

const char * pychip_DeviceController_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return NULL;
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
                         .SetDiscriminator(discriminator))
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
    return devCtrl->PairDevice(nodeid, params).AsInteger();
}

ChipError::StorageType pychip_DeviceController_CloseSession(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeid)
{
    Device * device;
    CHIP_ERROR err = devCtrl->GetDevice(nodeid, &device);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    return device->CloseSession().AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverAllCommissionableNodes(chip::Controller::DeviceCommissioner * devCtrl)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kNone, static_cast<uint16_t>(0));
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesLongDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                     uint16_t long_discriminator)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kLong, long_discriminator);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesShortDiscriminator(chip::Controller::DeviceCommissioner * devCtrl,
                                                                      uint16_t short_discriminator)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kShort, short_discriminator);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesVendor(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                 uint16_t vendor)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kVendor, vendor);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_DiscoverCommissionableNodesDeviceType(chip::Controller::DeviceCommissioner * devCtrl,
                                                                                     uint16_t device_type)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kDeviceType, device_type);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabled(chip::Controller::DeviceCommissioner * devCtrl,
                                                                        uint16_t enabled)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kCommissioningMode, enabled);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType
pychip_DeviceController_DiscoverCommissionableNodesCommissioningEnabledFromCommand(chip::Controller::DeviceCommissioner * devCtrl)
{
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kCommissioningModeFromCommand, 1);
    return devCtrl->DiscoverCommissionableNodes(filter).AsInteger();
}

ChipError::StorageType pychip_DeviceController_OpenCommissioningWindow(chip::Controller::DeviceCommissioner * devCtrl,
                                                                       chip::NodeId nodeid, uint16_t timeout, uint16_t iteration,
                                                                       uint16_t discriminator, uint8_t option)
{
    return devCtrl->OpenCommissioningWindow(nodeid, timeout, iteration, discriminator, option).AsInteger();
}

void pychip_DeviceController_PrintDiscoveredDevices(chip::Controller::DeviceCommissioner * devCtrl)
{
    for (int i = 0; i < devCtrl->GetMaxCommissionableNodesSupported(); ++i)
    {
        const chip::Mdns::DiscoveredNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(i);
        if (dnsSdInfo == nullptr)
        {
            continue;
        }
        char rotatingId[chip::Mdns::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(dnsSdInfo->rotatingId, dnsSdInfo->rotatingIdLen, rotatingId, sizeof(rotatingId));

        ChipLogProgress(Discovery, "Commissionable Node %d", i);
        ChipLogProgress(Discovery, "\tHost name:\t\t%s", dnsSdInfo->hostName);
        ChipLogProgress(Discovery, "\tLong discriminator:\t%u", dnsSdInfo->longDiscriminator);
        ChipLogProgress(Discovery, "\tVendor ID:\t\t%u", dnsSdInfo->vendorId);
        ChipLogProgress(Discovery, "\tProduct ID:\t\t%u", dnsSdInfo->productId);
        ChipLogProgress(Discovery, "\tAdditional Pairing\t%u", dnsSdInfo->additionalPairing);
        ChipLogProgress(Discovery, "\tCommissioning Mode\t%u", dnsSdInfo->commissioningMode);
        ChipLogProgress(Discovery, "\tDevice Type\t\t%u", dnsSdInfo->deviceType);
        ChipLogProgress(Discovery, "\tDevice Name\t\t%s", dnsSdInfo->deviceName);
        ChipLogProgress(Discovery, "\tRotating Id\t\t%s", rotatingId);
        ChipLogProgress(Discovery, "\tPairing Instruction\t%s", dnsSdInfo->pairingInstruction);
        ChipLogProgress(Discovery, "\tPairing Hint\t\t0x%x", dnsSdInfo->pairingHint);
        for (int j = 0; j < dnsSdInfo->numIPs; ++j)
        {
            char buf[chip::Inet::kMaxIPAddressStringLength];
            dnsSdInfo->ipAddress[j].ToString(buf);
            ChipLogProgress(Discovery, "\tAddress %d:\t\t%s", j, buf);
        }
    }
}

bool pychip_DeviceController_GetIPForDiscoveredDevice(chip::Controller::DeviceCommissioner * devCtrl, int idx, char * addrStr,
                                                      uint32_t len)
{
    const chip::Mdns::DiscoveredNodeData * dnsSdInfo = devCtrl->GetDiscoveredDevice(idx);
    if (dnsSdInfo == nullptr)
    {
        return false;
    }
    // TODO(cecille): Select which one we actually want.
    if (dnsSdInfo->ipAddress[0].ToString(addrStr, len) == addrStr)
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

void pychip_ScriptDeviceAddressUpdateDelegate_SetOnAddressUpdateComplete(
    chip::Controller::DeviceAddressUpdateDelegate_OnUpdateComplete callback)
{
    sDeviceAddressUpdateDelegate.SetOnAddressUpdateComplete(callback);
}

ChipError::StorageType pychip_Resolver_ResolveNode(uint64_t fabricid, chip::NodeId nodeid)
{
    return Mdns::Resolver::Instance()
        .ResolveNodeId(PeerId().SetNodeId(nodeid).SetCompressedFabricId(fabricid), Inet::kIPAddressType_Any)
        .AsInteger();
}

ChipError::StorageType pychip_Stack_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

#if !CHIP_SYSTEM_CONFIG_USE_SOCKETS

    ExitNow(err = CHIP_ERROR_NOT_IMPLEMENTED);

#else /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */

#endif /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */

exit:
    if (err != CHIP_NO_ERROR)
        pychip_Stack_Shutdown();

    return err.AsInteger();
}

ChipError::StorageType pychip_Stack_Shutdown()
{
    return CHIP_NO_ERROR.AsInteger();
}

const char * pychip_Stack_ErrorToString(ChipError::StorageType err)
{
    return chip::ErrorStr(CHIP_ERROR(err));
}

const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return NULL;
}

ChipError::StorageType pychip_GetDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                                chip::Controller::Device ** device)
{
    VerifyOrReturnError(devCtrl != nullptr, CHIP_ERROR_INVALID_ARGUMENT.AsInteger());
    return devCtrl->GetDevice(nodeId, device).AsInteger();
}

namespace {
struct GetDeviceCallbacks
{
    GetDeviceCallbacks(DeviceAvailableFunc callback) :
        mOnSuccess(OnDeviceConnectedFn, this), mOnFailure(OnConnectionFailureFn, this), mCallback(callback)
    {}

    static void OnDeviceConnectedFn(void * context, Device * device)
    {
        auto * self = static_cast<GetDeviceCallbacks *>(context);
        self->mCallback(device, CHIP_NO_ERROR.AsInteger());
        delete self;
    }

    static void OnConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
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

ChipError::StorageType pychip_DeviceCommissioner_CloseBleConnection(chip::Controller::DeviceCommissioner * devCtrl)
{
#if CONFIG_NETWORK_LAYER_BLE
    return devCtrl->CloseBleConnection().AsInteger();
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.AsInteger();
#endif
}

uint64_t pychip_GetCommandSenderHandle(chip::Controller::Device * device)
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
