/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <controller/CHIPDeviceController_deprecated.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Controller;

namespace chip {
namespace DeviceController {

using namespace chip::Encoding;

ChipDeviceController::ChipDeviceController()
{
    mState                  = kState_NotInitialized;
    AppState                = nullptr;
    mCurReqMsg              = nullptr;
    mOnError                = nullptr;
    mOnNewConnection        = nullptr;
    mListenPort             = CHIP_PORT;
    mLocalDeviceId          = kUndefinedNodeId;
    mRemoteDeviceId         = kUndefinedNodeId;
    mDevice                 = nullptr;
    mPairingWithoutSecurity = false;
    CHIP_ZERO_AT(mOnComplete);
}

ChipDeviceController::~ChipDeviceController() {}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, DevicePairingDelegate * pairingDelegate,
                                      PersistentStorageDelegate * storageDelegate)
{
    return mCommissioner.Init(localNodeId, storageDelegate, pairingDelegate);
}

CHIP_ERROR ChipDeviceController::Init(NodeId localNodeId, System::Layer * systemLayer, InetLayer * inetLayer,
                                      DevicePairingDelegate * pairingDelegate, PersistentStorageDelegate * storageDelegate)
{
    return mCommissioner.Init(localNodeId, storageDelegate, pairingDelegate, systemLayer, inetLayer);
}

CHIP_ERROR ChipDeviceController::Shutdown()
{
    return mCommissioner.Shutdown();
}

CHIP_ERROR ChipDeviceController::ConnectDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState,
                                               NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                               ErrorHandler onError, uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    CHIP_ERROR err = mCommissioner.PairDevice(remoteDeviceId, params, devicePort, interfaceId);
    SuccessOrExit(err);

    mState           = kState_Initialized;
    mRemoteDeviceId  = remoteDeviceId;
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    // TODO: Should call mOnNewConnected when rendezvous completed
    mOnNewConnection(this, nullptr, mAppReqState);

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDeviceWithoutSecurePairing(NodeId remoteDeviceId, const IPAddress & deviceAddr,
                                                                   void * appReqState, NewConnectionHandler onConnected,
                                                                   MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                                                                   uint16_t devicePort, Inet::InterfaceId interfaceId)
{
    CHIP_ERROR err =
        mCommissioner.PairTestDeviceWithoutSecurity(remoteDeviceId, deviceAddr, mSerializedTestDevice, devicePort, interfaceId);
    SuccessOrExit(err);

    mPairingWithoutSecurity = true;

    mState           = kState_Initialized;
    mRemoteDeviceId  = remoteDeviceId;
    mAppReqState     = appReqState;
    mOnNewConnection = onConnected;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    if (mOnNewConnection)
    {
        mOnNewConnection(this, nullptr, mAppReqState);
    }

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::SetUdpListenPort(uint16_t listenPort)
{
    if (mState != kState_Initialized)
        return CHIP_ERROR_INCORRECT_STATE;
    mListenPort = listenPort;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceController::ServiceEvents()
{
    return mCommissioner.ServiceEvents();
}

CHIP_ERROR ChipDeviceController::ServiceEventSignal()
{
    return mCommissioner.ServiceEventSignal();
}

bool ChipDeviceController::IsConnected() const
{
    return mState == kState_Initialized;
}

bool ChipDeviceController::GetIpAddress(Inet::IPAddress & addr)
{
    if (!IsConnected())
        return false;

    if (mDevice == nullptr)
        InitDevice();

    return mDevice != nullptr && mDevice->GetIpAddress(addr);
}

CHIP_ERROR ChipDeviceController::DisconnectDevice()
{
    if (mDevice != nullptr)
    {
        mCommissioner.ReleaseDevice(mDevice);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBufferHandle buffer, NodeId peerDevice)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!buffer.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    mAppReqState = appReqState;

    if (peerDevice != kUndefinedNodeId)
    {
        mRemoteDeviceId = peerDevice;
    }
    VerifyOrExit(mRemoteDeviceId != kUndefinedNodeId, err = CHIP_ERROR_INCORRECT_STATE);

    if (mDevice == nullptr)
    {
        SuccessOrExit(InitDevice());
    }

    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mDevice->SetDelegate(this);

    err = mDevice->SendMessage(std::move(buffer));

exit:

    return err;
}

CHIP_ERROR ChipDeviceController::SetDevicePairingDelegate(DevicePairingDelegate * pairingDelegate)
{
    mCommissioner.SetDevicePairingDelegate(pairingDelegate);
    return CHIP_NO_ERROR;
}

void ChipDeviceController::OnMessage(System::PacketBufferHandle msgBuf)
{
    if (mOnComplete.Response != nullptr)
    {
        mOnComplete.Response(this, mAppReqState, std::move(msgBuf));
    }
}

CHIP_ERROR ChipDeviceController::InitDevice()
{
    return mPairingWithoutSecurity ? mCommissioner.GetDevice(mRemoteDeviceId, mSerializedTestDevice, &mDevice)
                                   : mCommissioner.GetDevice(mRemoteDeviceId, &mDevice);
}

} // namespace DeviceController
} // namespace chip
