/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#if CONFIG_NETWORK_LAYER_BLE
#include "ChipDeviceController-BleApplicationDelegate.h"
#include "ChipDeviceController-BlePlatformDelegate.h"
#endif /* CONFIG_NETWORK_LAYER_BLE */

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "ChipDeviceController-StorageDelegate.h"

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
typedef void * (*GetBleEventCBFunct)(void);
typedef void (*ConstructBytesArrayFunct)(const uint8_t * dataBuf, uint32_t dataLen);
typedef void (*LogMessageFunct)(uint64_t time, uint64_t timeUS, const char * moduleName, uint8_t category, const char * msg);

typedef void (*OnConnectFunct)(chip::DeviceController::ChipDeviceController * dc,
                               const chip::Transport::PeerConnectionState * state, void * appReqState);
typedef void (*OnErrorFunct)(chip::DeviceController::ChipDeviceController * dc, void * appReqState, CHIP_ERROR err,
                             const chip::Inet::IPPacketInfo * pi);
typedef void (*OnMessageFunct)(chip::DeviceController::ChipDeviceController * dc, void * appReqState,
                               chip::System::PacketBufferHandle buffer);
}

enum BleEventType
{
    kBleEventType_Rx         = 1,
    kBleEventType_Tx         = 2,
    kBleEventType_Subscribe  = 3,
    kBleEventType_Disconnect = 4
};

enum BleSubscribeOperation
{
    kBleSubOp_Subscribe   = 1,
    kBleSubOp_Unsubscribe = 2
};

class BleEventBase
{
public:
    int32_t eventType;
};

class BleRxEvent : public BleEventBase
{
public:
    void * connObj;
    void * svcId;
    void * charId;
    void * buffer;
    uint16_t length;
};

class BleTxEvent : public BleEventBase
{
public:
    void * connObj;
    void * svcId;
    void * charId;
    bool status;
};

class BleSubscribeEvent : public BleEventBase
{
public:
    void * connObj;
    void * svcId;
    void * charId;
    int32_t operation;
    bool status;
};

class BleDisconnectEvent : public BleEventBase
{
public:
    void * connObj;
    int32_t error;
};

static chip::System::Layer sSystemLayer;
static chip::Inet::InetLayer sInetLayer;
static chip::Controller::PythonPersistentStorageDelegate sStorageDelegate;

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kLocalDeviceId  = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId = chip::kTestDeviceNodeId;

#if CONFIG_NETWORK_LAYER_BLE
static BleLayer sBle;
static BLEEndPoint * spBleEndPoint = NULL;
static DeviceController_BlePlatformDelegate sBlePlatformDelegate(&sBle);
static DeviceController_BleApplicationDelegate sBleApplicationDelegate;

static volatile GetBleEventCBFunct GetBleEventCB = NULL;

static int BleWakePipe[2];

union
{
    CompleteHandler General;
} sOnComplete;

ErrorHandler sOnError;

#endif /* CONFIG_NETWORK_LAYER_BLE */

extern "C" {
// Trampolined callback types
CHIP_ERROR nl_Chip_DeviceController_DriveIO(uint32_t sleepTimeMS);

#if CONFIG_NETWORK_LAYER_BLE
CHIP_ERROR nl_Chip_DeviceController_WakeForBleIO();
CHIP_ERROR nl_Chip_DeviceController_SetBleEventCB(GetBleEventCBFunct getBleEventCB);
CHIP_ERROR nl_Chip_DeviceController_SetBleWriteCharacteristic(WriteBleCharacteristicCBFunct writeBleCharacteristicCB);
CHIP_ERROR nl_Chip_DeviceController_SetBleSubscribeCharacteristic(SubscribeBleCharacteristicCBFunct subscribeBleCharacteristicCB);
CHIP_ERROR nl_Chip_DeviceController_SetBleClose(CloseBleCBFunct closeBleCB);
#endif /* CONFIG_NETWORK_LAYER_BLE */

CHIP_ERROR nl_Chip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl);
CHIP_ERROR nl_Chip_DeviceController_DeleteDeviceController(chip::DeviceController::ChipDeviceController * devCtrl);
void nl_Chip_DeviceController_GetDeviceConrollerNewApi(chip::DeviceController::ChipDeviceController * devCtrl,
                                                       chip::Controller::DeviceController ** out);

CHIP_ERROR nl_Chip_Controller_NewDevice(chip::Controller::DeviceController * controller, chip::NodeId nodeId,
                                        chip::Controller::Device ** device);

// Rendezvous
CHIP_ERROR nl_Chip_DeviceController_Connect(chip::DeviceController::ChipDeviceController * devCtrl, BLE_CONNECTION_OBJECT connObj,
                                            uint32_t setupPinCode, OnConnectFunct onConnect, OnMessageFunct onMessage,
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

#if CONFIG_NETWORK_LAYER_BLE
CHIP_ERROR nl_Chip_DeviceController_ValidateBTP(chip::DeviceController::ChipDeviceController * devCtrl,
                                                BLE_CONNECTION_OBJECT connObj, CompleteHandler onComplete, ErrorHandler onError);
#endif /* CONFIG_NETWORK_LAYER_BLE */

bool nl_Chip_DeviceController_IsConnected(chip::DeviceController::ChipDeviceController * devCtrl);
void nl_Chip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl);
uint8_t nl_Chip_DeviceController_GetLogFilter();
void nl_Chip_DeviceController_SetLogFilter(uint8_t category);

CHIP_ERROR nl_Chip_Stack_Init();
CHIP_ERROR nl_Chip_Stack_Shutdown();
const char * nl_Chip_Stack_ErrorToString(CHIP_ERROR err);
const char * nl_Chip_Stack_StatusReportToString(uint32_t profileId, uint16_t statusCode);
void nl_Chip_Stack_SetLogFunct(LogMessageFunct logFunct);
}

CHIP_ERROR nl_Chip_DeviceController_NewDeviceController(chip::DeviceController::ChipDeviceController ** outDevCtrl)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    *outDevCtrl = new chip::DeviceController::ChipDeviceController();
    (*outDevCtrl)->SetUdpListenPort(11095);
    VerifyOrExit(*outDevCtrl != NULL, err = CHIP_ERROR_NO_MEMORY);

    err = (*outDevCtrl)->Init(kLocalDeviceId, &sSystemLayer, &sInetLayer, nullptr, &sStorageDelegate);
    SuccessOrExit(err);

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

// Old device controller is deprecated, get the new controller.
// When all functions are migrated to the new API, the new API will be return by defualt via NewDeviceController
void nl_Chip_DeviceController_GetDeviceConrollerNewApi(chip::DeviceController::ChipDeviceController * devCtrl,
                                                       chip::Controller::DeviceController ** out)
{
    *out = devCtrl->GetNewApi();
}

CHIP_ERROR nl_Chip_Controller_NewDevice(chip::Controller::DeviceController * controller, chip::NodeId nodeId,
                                        chip::Controller::Device ** device)
{
    return controller->NewDevice(nodeId, device);
}

CHIP_ERROR nl_Chip_DeviceController_DriveIO(uint32_t sleepTimeMS)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if !CHIP_SYSTEM_CONFIG_USE_SOCKETS

    ExitNow(err = CHIP_ERROR_NOT_IMPLEMENTED);

#else /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */
    struct timeval sleepTime;
    fd_set readFDs, writeFDs, exceptFDs;
    int maxFDs = 0;
#if CONFIG_NETWORK_LAYER_BLE
    uint8_t bleWakeByte;
    chip::System::PacketBufferHandle msgBuf;
    ChipBleUUID svcId, charId;
    union
    {
        const BleEventBase * ev;
        const BleTxEvent * txEv;
        const BleRxEvent * rxEv;
        const BleSubscribeEvent * subscribeEv;
        const BleDisconnectEvent * dcEv;
    } evu;
#endif /* CONFIG_NETWORK_LAYER_BLE */

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    sleepTime.tv_sec  = sleepTimeMS / 1000;
    sleepTime.tv_usec = (sleepTimeMS % 1000) * 1000;

    if (sSystemLayer.State() == chip::System::kLayerState_Initialized)
        sSystemLayer.PrepareSelect(maxFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

    if (sInetLayer.State == chip::Inet::InetLayer::kState_Initialized)
        sInetLayer.PrepareSelect(maxFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

#if CONFIG_NETWORK_LAYER_BLE
    // Add read end of BLE wake pipe to readFDs.
    FD_SET(BleWakePipe[0], &readFDs);

    if (BleWakePipe[0] + 1 > maxFDs)
        maxFDs = BleWakePipe[0] + 1;
#endif /* CONFIG_NETWORK_LAYER_BLE */

    int selectRes = select(maxFDs, &readFDs, &writeFDs, &exceptFDs, &sleepTime);
    VerifyOrExit(selectRes >= 0, err = chip::System::MapErrorPOSIX(errno));

#if CONFIG_NETWORK_LAYER_BLE
    // Drive IO to InetLayer and/or BleLayer.
    if (FD_ISSET(BleWakePipe[0], &readFDs))
    {
        while (true)
        {
            if (read(BleWakePipe[0], &bleWakeByte, 1) == -1)
            {
                if (errno == EAGAIN)
                    break;
                else
                {
                    err = errno;
                    printf("bleWakePipe calling ExitNow()\n");
                    ExitNow();
                }
            }

            if (GetBleEventCB)
            {
                evu.ev = static_cast<const BleEventBase *>(GetBleEventCB());

                if (evu.ev)
                {
                    switch (evu.ev->eventType)
                    {
                    case kBleEventType_Rx:
                        // build a packet buffer from the rxEv and send to blelayer.
                        msgBuf = chip::System::PacketBuffer::New();
                        VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

                        memcpy(msgBuf->Start(), evu.rxEv->buffer, evu.rxEv->length);
                        msgBuf->SetDataLength(evu.rxEv->length);

                        // copy the svcId and charId from the event.
                        memcpy(svcId.bytes, evu.rxEv->svcId, sizeof(svcId.bytes));
                        memcpy(charId.bytes, evu.rxEv->charId, sizeof(charId.bytes));

                        sBle.HandleIndicationReceived(evu.txEv->connObj, &svcId, &charId, std::move(msgBuf));
                        break;

                    case kBleEventType_Tx:
                        // copy the svcId and charId from the event.
                        memcpy(svcId.bytes, evu.txEv->svcId, sizeof(svcId.bytes));
                        memcpy(charId.bytes, evu.txEv->charId, sizeof(charId.bytes));

                        sBle.HandleWriteConfirmation(evu.txEv->connObj, &svcId, &charId);
                        break;

                    case kBleEventType_Subscribe:
                        memcpy(svcId.bytes, evu.subscribeEv->svcId, sizeof(svcId.bytes));
                        memcpy(charId.bytes, evu.subscribeEv->charId, sizeof(charId.bytes));

                        switch (evu.subscribeEv->operation)
                        {
                        case kBleSubOp_Subscribe:
                            if (evu.subscribeEv->status)
                            {
                                sBle.HandleSubscribeComplete(evu.subscribeEv->connObj, &svcId, &charId);
                            }
                            else
                            {
                                sBle.HandleConnectionError(evu.subscribeEv->connObj, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
                            }
                            break;

                        case kBleSubOp_Unsubscribe:
                            if (evu.subscribeEv->status)
                            {
                                sBle.HandleUnsubscribeComplete(evu.subscribeEv->connObj, &svcId, &charId);
                            }
                            else
                            {
                                sBle.HandleConnectionError(evu.subscribeEv->connObj, BLE_ERROR_GATT_UNSUBSCRIBE_FAILED);
                            }
                            break;

                        default:
                            printf("Error: unhandled subscribe operation. Calling ExitNow()\n");
                            ExitNow();
                            break;
                        }
                        break;

                    case kBleEventType_Disconnect:
                        sBle.HandleConnectionError(evu.dcEv->connObj, evu.dcEv->error);
                        break;

                    default:
                        printf("Error: unhandled sBle EventType. Calling ExitNow()\n");
                        ExitNow();
                        break;
                    }
                }
                else
                {
                    printf("no event\n");
                }
            }
        }

        // Don't bother InetLayer if we only got BLE IO.
        selectRes--;
    }
#endif /* CONFIG_NETWORK_LAYER_BLE */

    if (sSystemLayer.State() == chip::System::kLayerState_Initialized)
        sSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);

    if (sInetLayer.State == chip::Inet::InetLayer::kState_Initialized)
        sInetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);

#endif /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */

exit:
    return err;
}

#if CONFIG_NETWORK_LAYER_BLE
CHIP_ERROR nl_Chip_DeviceController_WakeForBleIO()
{
    if (BleWakePipe[1] == 0)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    // Write a single byte to the BLE wake pipe. This wakes the IO thread's select loop for BLE input.
    if (write(BleWakePipe[1], "x", 1) == -1 && errno != EAGAIN)
    {
        return errno;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR nl_Chip_DeviceController_SetBleEventCB(GetBleEventCBFunct getBleEventCB)
{
    GetBleEventCB = getBleEventCB;

    return CHIP_NO_ERROR;
}

CHIP_ERROR nl_Chip_DeviceController_SetBleWriteCharacteristic(WriteBleCharacteristicCBFunct writeBleCharacteristicCB)
{
    sBlePlatformDelegate.SetWriteCharCB(writeBleCharacteristicCB);
    return CHIP_NO_ERROR;
}

CHIP_ERROR nl_Chip_DeviceController_SetBleSubscribeCharacteristic(SubscribeBleCharacteristicCBFunct subscribeBleCharacteristicCB)
{
    sBlePlatformDelegate.SetSubscribeCharCB(subscribeBleCharacteristicCB);
    return CHIP_NO_ERROR;
}

CHIP_ERROR nl_Chip_DeviceController_SetBleClose(CloseBleCBFunct closeBleCB)
{
    sBlePlatformDelegate.SetCloseCB(closeBleCB);
    return CHIP_NO_ERROR;
}

#endif /* CONFIG_NETWORK_LAYER_BLE */

void nl_Chip_DeviceController_Close(chip::DeviceController::ChipDeviceController * devCtrl)
{
#if CONFIG_NETWORK_LAYER_BLE
    if (spBleEndPoint != NULL)
    {
        spBleEndPoint->Close();
        spBleEndPoint = NULL;
    }
#endif
}

#if CONFIG_NETWORK_LAYER_BLE
static void HandleBleConnectComplete(BLEEndPoint * endPoint, BLE_ERROR err)
{
    if (err != BLE_NO_ERROR)
    {
        ChipLogError(MessageLayer, "ChipoBle con failed %d", err);
        spBleEndPoint->Abort();
        sOnError(NULL, NULL, err, NULL);
    }
    else
    {
        ChipLogProgress(Controller, "ChipoBle con complete\n");
        sOnComplete.General(NULL, NULL);
        spBleEndPoint->Close();
    }
    spBleEndPoint = NULL;
}

static void HandleBleConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    ChipLogProgress(Controller, "ChipoBle con close\n");
}

CHIP_ERROR nl_Chip_DeviceController_ValidateBTP(chip::DeviceController::ChipDeviceController * devCtrl,
                                                BLE_CONNECTION_OBJECT connObj, CompleteHandler onComplete, ErrorHandler onError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(Controller, "begin BTPConnect validation");

    err = sBle.NewBleEndPoint(&spBleEndPoint, connObj, kBleRole_Central, false);
    SuccessOrExit(err);

    spBleEndPoint->mAppState          = NULL;
    spBleEndPoint->OnConnectComplete  = HandleBleConnectComplete;
    spBleEndPoint->OnConnectionClosed = HandleBleConnectionClosed;

    sOnComplete.General = onComplete;
    sOnError            = onError;
    err                 = spBleEndPoint->StartConnect();
    SuccessOrExit(err);

exit:
    return err;
}
#endif /* CONFIG_NETWORK_LAYER_BLE */

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
    return devCtrl->ConnectDevice(kRemoteDeviceId,
                                  chip::RendezvousParameters()
                                      .SetPeerAddress(Transport::PeerAddress(Transport::Type::kBle))
                                      .SetSetupPINCode(setupPINCode)
                                      .SetConnectionObject(connObj)
                                      .SetBleLayer(&sBle),
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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CONFIG_NETWORK_LAYER_BLE
    int flags;
#endif /* CHIP_SYSTEM_CONFIG_USE_SOCKETS && CONFIG_NETWORK_LAYER_BLE */

#if !CHIP_SYSTEM_CONFIG_USE_SOCKETS

    ExitNow(err = CHIP_ERROR_NOT_IMPLEMENTED);

#else /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */

    if (sSystemLayer.State() == chip::System::kLayerState_Initialized)
        ExitNow();

    err = sSystemLayer.Init(NULL);
    SuccessOrExit(err);

    if (sInetLayer.State == chip::Inet::InetLayer::kState_Initialized)
        ExitNow();

    // Initialize the InetLayer object.
    err = sInetLayer.Init(sSystemLayer, NULL);
    SuccessOrExit(err);

#if CONFIG_NETWORK_LAYER_BLE
    // Initialize the BleLayer object. For now, assume Device Controller is always a central.
    err = sBle.Init(&sBlePlatformDelegate, nullptr, &sBleApplicationDelegate, &sSystemLayer);
    SuccessOrExit(err);

    // Create BLE wake pipe and make it non-blocking.
    if (pipe(BleWakePipe) == -1)
    {
        err = chip::System::MapErrorPOSIX(errno);
        ExitNow();
    }

    // Make read end non-blocking.
    flags = fcntl(BleWakePipe[0], F_GETFL);
    if (flags == -1)
    {
        err = chip::System::MapErrorPOSIX(errno);
        ExitNow();
    }

    flags |= O_NONBLOCK;
    if (fcntl(BleWakePipe[0], F_SETFL, flags) == -1)
    {
        err = chip::System::MapErrorPOSIX(errno);
        ExitNow();
    }

    // Make write end non-blocking.
    flags = fcntl(BleWakePipe[1], F_GETFL);
    if (flags == -1)
    {
        err = chip::System::MapErrorPOSIX(errno);
        ExitNow();
    }

    flags |= O_NONBLOCK;
    if (fcntl(BleWakePipe[1], F_SETFL, flags) == -1)
    {
        err = chip::System::MapErrorPOSIX(errno);
        ExitNow();
    }
#endif /* CONFIG_NETWORK_LAYER_BLE */
#endif /* CHIP_SYSTEM_CONFIG_USE_SOCKETS */

exit:
    if (err != CHIP_NO_ERROR)
        nl_Chip_Stack_Shutdown();

    return err;
}

CHIP_ERROR nl_Chip_Stack_Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (sInetLayer.State == chip::Inet::InetLayer::kState_NotInitialized)
        ExitNow();

    if (sSystemLayer.State() == chip::System::kLayerState_NotInitialized)
        ExitNow();

exit:
    return err;
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
        char moduleName[nlChipLoggingModuleNameLen + 1];
        ::chip:: ::Logging::GetModuleName(moduleName, module);

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
