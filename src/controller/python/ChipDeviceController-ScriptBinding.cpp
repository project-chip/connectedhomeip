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
#include <controller/CHIPDeviceController.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Ble;
using namespace chip::Controller;

extern "C" {
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);
}

namespace {
chip::Controller::PythonPersistentStorageDelegate sStorageDelegate;
chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
} // namespace

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kDefaultLocalDeviceId = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId       = chip::kTestDeviceNodeId;

extern "C" {
CHIP_ERROR pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                       chip::NodeId localDeviceId);
CHIP_ERROR pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl);

// Rendezvous
CHIP_ERROR pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                              uint32_t setupPINCode, chip::NodeId nodeid);
CHIP_ERROR pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                             uint32_t setupPINCode, chip::NodeId nodeid);

// Pairing Delegate
CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetWifiCredential(chip::Controller::DeviceCommissioner * devCtrl, const char * ssid,
                                                     const char * password);
CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetThreadCredential(chip::Controller::DeviceCommissioner * devCtrl, int channel, int panId,
                                                       const char * masterKey);

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback);

uint8_t pychip_DeviceController_GetLogFilter();
void pychip_DeviceController_SetLogFilter(uint8_t category);

CHIP_ERROR pychip_Stack_Init();
CHIP_ERROR pychip_Stack_Shutdown();
const char * pychip_Stack_ErrorToString(CHIP_ERROR err);
const char * pychip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void pychip_Stack_SetLogFunct(LogMessageFunct logFunct);

CHIP_ERROR pychip_GetDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                    chip::Controller::Device ** device);
}

CHIP_ERROR pychip_DeviceController_NewDeviceController(chip::Controller::DeviceCommissioner ** outDevCtrl,
                                                       chip::NodeId localDeviceId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    *outDevCtrl = new chip::Controller::DeviceCommissioner();
    VerifyOrExit(*outDevCtrl != NULL, err = CHIP_ERROR_NO_MEMORY);

    if (localDeviceId == chip::kUndefinedNodeId)
    {
        localDeviceId = kDefaultLocalDeviceId;
    }
    SuccessOrExit(err = (*outDevCtrl)->Init(localDeviceId, &sStorageDelegate, &sPairingDelegate));
    SuccessOrExit(err = (*outDevCtrl)->ServiceEvents());

exit:
    if (err != CHIP_NO_ERROR && *outDevCtrl != NULL)
    {
        delete *outDevCtrl;
        *outDevCtrl = NULL;
    }
    return err;
}

CHIP_ERROR pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl)
{
    if (devCtrl != NULL)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }
    return CHIP_NO_ERROR;
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

CHIP_ERROR pychip_DeviceController_ConnectBLE(chip::Controller::DeviceCommissioner * devCtrl, uint16_t discriminator,
                                              uint32_t setupPINCode, chip::NodeId nodeid)
{
    return devCtrl->PairDevice(nodeid,
                               chip::RendezvousParameters()
                                   .SetPeerAddress(Transport::PeerAddress(Transport::Type::kBle))
                                   .SetSetupPINCode(setupPINCode)
                                   .SetDiscriminator(discriminator));
}

CHIP_ERROR pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner * devCtrl, const char * peerAddrStr,
                                             uint32_t setupPINCode, chip::NodeId nodeid)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Inet::IPAddress peerAddr;
    chip::Transport::PeerAddress addr;
    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode);

    VerifyOrReturnError(chip::Inet::IPAddress::FromString(peerAddrStr, peerAddr), err = CHIP_ERROR_INVALID_ARGUMENT);
    // TODO: IP rendezvous should use TCP connection.
    addr.SetTransportType(chip::Transport::Type::kUdp).SetIPAddress(peerAddr);
    params.SetPeerAddress(addr).SetDiscriminator(0);
    return devCtrl->PairDevice(nodeid, params);
}

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetWifiCredential(chip::Controller::DeviceCommissioner * devCtrl, const char * ssid,
                                                     const char * password)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    (void) devCtrl;

    VerifyOrExit(ssid != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(password != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    sPairingDelegate.SetWifiCredential(ssid, password);

exit:
    return err;
}

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetThreadCredential(chip::Controller::DeviceCommissioner * devCtrl, int channel, int panId,
                                                       const char * masterKeyStr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t masterKey[chip::DeviceLayer::Internal::kThreadMasterKeyLength];
    (void) devCtrl;

    VerifyOrExit(strlen(masterKeyStr) == 2 * chip::DeviceLayer::Internal::kThreadMasterKeyLength,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    for (size_t i = 0; i < chip::DeviceLayer::Internal::kThreadMasterKeyLength; i++)
        VerifyOrExit(sscanf(&masterKeyStr[2 * i], "%2hhx", &masterKey[i]) == 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    sPairingDelegate.SetThreadCredential(channel, panId, masterKey);

exit:
    return err;
}

CHIP_ERROR
pychip_ScriptDevicePairingDelegate_SetKeyExchangeCallback(chip::Controller::DeviceCommissioner * devCtrl,
                                                          chip::Controller::DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    sPairingDelegate.SetKeyExchangeCallback(callback);
    return CHIP_NO_ERROR;
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

CHIP_ERROR pychip_GetDeviceByNodeId(chip::Controller::DeviceCommissioner * devCtrl, chip::NodeId nodeId,
                                    chip::Controller::Device ** device)
{
    return devCtrl->GetDevice(nodeId, device);
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
