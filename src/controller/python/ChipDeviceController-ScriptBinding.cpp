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
CHIP_ERROR nl_Chip_DeviceController_DriveIO(uint32_t sleepTimeMS);

CHIP_ERROR nl_Chip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl);
CHIP_ERROR nl_Chip_DeviceController_DeleteDeviceController(chip::DeviceController::ChipDeviceController * devCtrl);

// Rendezvous
CHIP_ERROR nl_Chip_DeviceController_ConnectBLE(chip::DeviceController::ChipDeviceController * devCtrl, uint16_t discriminator,
                                               uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                               OnErrorFunct onError);
CHIP_ERROR nl_Chip_DeviceController_ConnectIP(chip::DeviceController::ChipDeviceController * devCtrl, const char * peerAddrStr,
                                              uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                              OnErrorFunct onError);

// Network Provisioning
CHIP_ERROR
nl_Chip_ScriptDevicePairingDelegate_NewPairingDelegate(chip::DeviceController::ScriptDevicePairingDelegate ** pairingDelegate);
CHIP_ERROR
nl_Chip_ScriptDevicePairingDelegate_SetWifiCredential(chip::DeviceController::ScriptDevicePairingDelegate * pairingDelegate,
                                                      const char * ssid, const char * password);
CHIP_ERROR nl_Chip_DeviceController_SetDevicePairingDelegate(chip::DeviceController::ChipDeviceController * devCtrl,
                                                             chip::Controller::DevicePairingDelegate * pairingDelegate);

bool nl_Chip_DeviceController_IsConnected(chip::DeviceController::ChipDeviceController * devCtrl);
void nl_Chip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl);
uint8_t nl_Chip_DeviceController_GetLogFilter();
void nl_Chip_DeviceController_SetLogFilter(uint8_t category);

CHIP_ERROR nl_Chip_Stack_Init();
CHIP_ERROR nl_Chip_Stack_Shutdown();
const char * nl_Chip_Stack_ErrorToString(CHIP_ERROR err);
const char * nl_Chip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void nl_Chip_Stack_SetLogFunct(LogMessageFunct logFunct);

CHIP_ERROR nl_Chip_GetDeviceByNodeId(chip::DeviceController::ChipDeviceController * devCtrl, chip::NodeId nodeId,
                                     chip::Controller::Device ** device);
}

CHIP_ERROR nl_Chip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl)
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

CHIP_ERROR nl_Chip_DeviceController_DeleteDeviceController(chip::DeviceController::ChipDeviceController * devCtrl)
{
    if (devCtrl != NULL)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }
    return CHIP_NO_ERROR;
}

void nl_Chip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl) {}

bool nl_Chip_DeviceController_IsConnected(chip::DeviceController::ChipDeviceController * devCtrl)
{
    return devCtrl->IsConnected();
}

const char * nl_Chip_DeviceController_ErrorToString(CHIP_ERROR err)
{
    return chip::ErrorStr(err);
}

const char * nl_Chip_DeviceController_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return NULL;
}

uint8_t nl_Chip_DeviceController_GetLogFilter()
{
#if _CHIP_USE_LOGGING
    return chip::Logging::GetLogFilter();
#else
    return chip::Logging::kLogCategory_None;
#endif
}

void nl_Chip_DeviceController_SetLogFilter(uint8_t category)
{
#if _CHIP_USE_LOGGING
    chip::Logging::SetLogFilter(category);
#endif
}

CHIP_ERROR nl_Chip_DeviceController_Connect(chip::DeviceController::ChipDeviceController * devCtrl, BLE_CONNECTION_OBJECT connObj,
                                            uint32_t setupPINCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
                                            OnErrorFunct onError)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR nl_Chip_DeviceController_ConnectBLE(chip::DeviceController::ChipDeviceController * devCtrl, uint16_t discriminator,
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

CHIP_ERROR nl_Chip_DeviceController_ConnectIP(chip::DeviceController::ChipDeviceController * devCtrl, const char * peerAddrStr,
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
nl_Chip_ScriptDevicePairingDelegate_NewPairingDelegate(chip::DeviceController::ScriptDevicePairingDelegate ** pairingDelegate)
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
nl_Chip_ScriptDevicePairingDelegate_SetWifiCredential(chip::DeviceController::ScriptDevicePairingDelegate * pairingDelegate,
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
CHIP_ERROR nl_Chip_DeviceController_SetDevicePairingDelegate(chip::DeviceController::ChipDeviceController * devCtrl,
                                                             chip::Controller::DevicePairingDelegate * pairingDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(devCtrl != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pairingDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    devCtrl->SetDevicePairingDelegate(pairingDelegate);

exit:
    return err;
}

CHIP_ERROR nl_Chip_Stack_Init()
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
        nl_Chip_Stack_Shutdown();

    return err;
}

CHIP_ERROR nl_Chip_Stack_Shutdown()
{
    return CHIP_NO_ERROR;
}

const char * nl_Chip_Stack_ErrorToString(CHIP_ERROR err)
{
    return chip::ErrorStr(err);
}

const char * nl_Chip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode)
{
    // return chip::StatusReportStr(profileId, statusCode);
    return NULL;
}

CHIP_ERROR nl_Chip_GetDeviceByNodeId(chip::DeviceController::ChipDeviceController * devCtrl, chip::NodeId nodeId,
                                     chip::Controller::Device ** device)
{
    return devCtrl->GetDeviceController()->GetDevice(nodeId, device);
}

#if _CHIP_USE_LOGGING && CHIP_LOG_ENABLE_DYNAMIC_LOGING_FUNCTION

// A pointer to the python logging function.
static LogMessageFunct sLogMessageFunct = NULL;

// This function is called by the Chip logging code whenever a developer message
// is logged.  It serves as glue to adapt the logging arguments to what is expected
// by the python code.
// NOTE that this function MUST be thread-safe.
static void LogMessageToPython(uint8_t module, uint8_t category, const char * msg, va_list ap)
{
    if (IsCategoryEnabled(category))
    {
        // Capture the timestamp of the log message.
        struct timeval tv;
        gettimeofday(&tv, NULL);

        // Get the module name
        char moduleName[chip::Logging::kMaxModuleNameLen + 1];
        ::chip:: ::Logging::GetModuleName(moduleName, sizeof(moduleName), module);

        // Format the log message into a dynamic memory buffer, growing the
        // buffer as needed to fit the message.
        char * msgBuf                    = NULL;
        size_t msgBufSize                = 0;
        size_t msgSize                   = 0;
        constexpr size_t kInitialBufSize = 120;
        do
        {
            va_list apCopy;
            va_copy(apCopy, ap);

            msgBufSize = max(msgSize + 1, kInitialBufSize);
            msgBuf     = (char *) realloc(msgBuf, msgBufSize);
            if (msgBuf == NULL)
            {
                return;
            }

            int res = vsnprintf(msgBuf, msgBufSize, msg, apCopy);
            if (res < 0)
            {
                return;
            }
            msgSize = (size_t) res;

            va_end(apCopy);
        } while (msgSize >= msgBufSize);

        // Call the configured python logging function.
        sLogMessageFunct((int64_t) tv.tv_sec, (int64_t) tv.tv_usec, moduleName, category, msgBuf);

        // Release the message buffer.
        free(msgBuf);
    }
}

void nl_Chip_Stack_SetLogFunct(LogMessageFunct logFunct)
{
    if (logFunct != NULL)
    {
        sLogMessageFunct = logFunct;
        ::chip::Logging::SetLogFunct(LogMessageToPython);
    }
    else
    {
        sLogMessageFunct = NULL;
        ::chip::Logging::SetLogFunct(NULL);
    }
}

#else // CHIP_LOG_ENABLE_DYNAMIC_LOGING_FUNCTION

void nl_Chip_Stack_SetLogFunct(LogMessageFunct logFunct) {}

#endif // CHIP_LOG_ENABLE_DYNAMIC_LOGING_FUNCTION
