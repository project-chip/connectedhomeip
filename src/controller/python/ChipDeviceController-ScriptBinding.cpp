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
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#include <inttypes.h>
#include <net/if.h>

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "ChipDeviceController-StorageDelegate.h"

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceController_deprecated.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Ble;
using namespace chip::DeviceController;

extern "C" {
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);

typedef void (*OnConnectFunct)(chip::DeviceController::ChipDeviceController * dc,
                               const chip::Transport::PeerConnectionState * state, void * appReqState);
typedef void (*OnErrorFunct)(chip::DeviceController::ChipDeviceController * dc, void * appReqState, CHIP_ERROR err,
                             const chip::Inet::IPPacketInfo * pi);
typedef void (*OnMessageFunct)(chip::DeviceController::ChipDeviceController * dc, void * appReqState,
                               chip::System::PacketBufferHandle buffer);
}

static chip::Controller::PythonPersistentStorageDelegate sStorageDelegate;

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kLocalDeviceId  = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId = chip::kTestDeviceNodeId;

extern "C" {
// Trampolined callback types
CHIP_ERROR pychip_DeviceController_DriveIO(uint32_t sleepTimeMS);

CHIP_ERROR pychip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl);
CHIP_ERROR pychip_DeviceController_DeleteDeviceController(chip::DeviceController::ChipDeviceController * devCtrl);

// Rendezvous
CHIP_ERROR pychip_DeviceController_ConnectBLE(chip::DeviceController::ChipDeviceController * devCtrl, uint16_t discriminator,
                                              uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                              OnErrorFunct onError);
CHIP_ERROR pychip_DeviceController_ConnectIP(chip::DeviceController::ChipDeviceController * devCtrl, const char * peerAddrStr,
                                             uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                             OnErrorFunct onError);

// Network Provisioning
CHIP_ERROR
pychip_ScriptDevicePairingDelegate_NewPairingDelegate(chip::DeviceController::ScriptDevicePairingDelegate ** pairingDelegate);
CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetWifiCredential(chip::DeviceController::ScriptDevicePairingDelegate * pairingDelegate,
                                                     const char * ssid, const char * password);
CHIP_ERROR pychip_DeviceController_SetDevicePairingDelegate(chip::DeviceController::ChipDeviceController * devCtrl,
                                                            chip::Controller::DevicePairingDelegate * pairingDelegate);

bool pychip_DeviceController_IsConnected(chip::DeviceController::ChipDeviceController * devCtrl);
void pychip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl);
uint8_t pychip_DeviceController_GetLogFilter();
void pychip_DeviceController_SetLogFilter(uint8_t category);

CHIP_ERROR pychip_Stack_Init();
CHIP_ERROR pychip_Stack_Shutdown();
const char * pychip_Stack_ErrorToString(CHIP_ERROR err);
const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void pychip_Stack_SetLogFunct(LogMessageFunct logFunct);

CHIP_ERROR pychip_GetDeviceByNodeId(chip::DeviceController::ChipDeviceController * devCtrl, chip::NodeId nodeId,
                                    chip::Controller::Device ** device);
}

CHIP_ERROR pychip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    *outDevCtrl = new chip::DeviceController::ChipDeviceController();
    VerifyOrExit(*outDevCtrl != NULL, err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = (*outDevCtrl)->Init(kLocalDeviceId, nullptr, nullptr, nullptr, &sStorageDelegate));
    SuccessOrExit(err = (*outDevCtrl)->ServiceEvents());

exit:
    if (err != CHIP_NO_ERROR && *outDevCtrl != NULL)
    {
        delete *outDevCtrl;
        *outDevCtrl = NULL;
    }
    return err;
}

CHIP_ERROR pychip_DeviceController_DeleteDeviceController(chip::DeviceController::ChipDeviceController * devCtrl)
{
    if (devCtrl != NULL)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }
    return CHIP_NO_ERROR;
}

void pychip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl) {}

bool pychip_DeviceController_IsConnected(chip::DeviceController::ChipDeviceController * devCtrl)
{
    return devCtrl->IsConnected();
}

const char * pychip_DeviceController_ErrorToString(CHIP_ERROR err)
{
    return chip::ErrorStr(err);
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

CHIP_ERROR pychip_DeviceController_Connect(chip::DeviceController::ChipDeviceController * devCtrl, BLE_CONNECTION_OBJECT connObj,
                                           uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                           OnErrorFunct onError)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR pychip_DeviceController_ConnectBLE(chip::DeviceController::ChipDeviceController * devCtrl, uint16_t discriminator,
                                              uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                              OnErrorFunct onError)
{
    return devCtrl->ConnectDevice(kRemoteDeviceId,
                                  chip::RendezvousParameters()
                                      .SetPeerAddress(Transport::PeerAddress(Transport::Type::kBle))
                                      .SetSetupPINCode(setupPINCode)
                                      .SetDiscriminator(discriminator),
                                  (void *) devCtrl, onConnect, onMessage, onError);
}

CHIP_ERROR pychip_DeviceController_ConnectIP(chip::DeviceController::ChipDeviceController * devCtrl, const char * peerAddrStr,
                                             uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                             OnErrorFunct onError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Inet::IPAddress peerAddr;
    chip::Transport::PeerAddress addr;
    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);

    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr), err = CHIP_ERROR_INVALID_ARGUMENT);
    // TODO: IP rendezvous should use TCP connection.
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr);
    params.SetPeerAddress(addr).SetDiscriminator(0);
    return devCtrl->ConnectDevice(kRemoteDeviceId, params, (void *) devCtrl, onConnect, onMessage, onError);
}

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_NewPairingDelegate(chip::DeviceController::ScriptDevicePairingDelegate ** pairingDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(pairingDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    *pairingDelegate = new chip::DeviceController::ScriptDevicePairingDelegate();

exit:
    if (err != CHIP_NO_ERROR && pairingDelegate != nullptr && *pairingDelegate != nullptr)
    {
        delete *pairingDelegate;
        *pairingDelegate = nullptr;
    }
    return err;
}

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetWifiCredential(chip::DeviceController::ScriptDevicePairingDelegate * pairingDelegate,
                                                     const char * ssid, const char * password)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(pairingDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ssid != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(password != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    pairingDelegate->SetWifiCredential(ssid, password);

exit:
    return err;
}
CHIP_ERROR pychip_DeviceController_SetDevicePairingDelegate(chip::DeviceController::ChipDeviceController * devCtrl,
                                                            chip::Controller::DevicePairingDelegate * pairingDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(devCtrl != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pairingDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    devCtrl->SetDevicePairingDelegate(pairingDelegate);

exit:
    return err;
}

CHIP_ERROR pychip_Stack_Init()
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

    return err;
}

CHIP_ERROR pychip_Stack_Shutdown()
{
    return CHIP_NO_ERROR;
}

const char * pychip_Stack_ErrorToString(CHIP_ERROR err)
{
    return chip::ErrorStr(err);
}

const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return NULL;
}

CHIP_ERROR pychip_GetDeviceByNodeId(chip::DeviceController::ChipDeviceController * devCtrl, chip::NodeId nodeId,
                                    chip::Controller::Device ** device)
{
    return devCtrl->GetDeviceController()->GetDevice(nodeId, device);
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
