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
 *      that implements discovery, pairing and provisioning of Weave
 *      devices.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>

#include <core/CHIPCore.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <core/CHIPEncoding.h>

#include <controller/CHIPDeviceController.h>
#include <support/logging/CHIPLogging.h>
#include <support/ErrorStr.h>
#include <support/TimeUtils.h>

namespace chip {
namespace DeviceController {

using namespace chip::Encoding;

ChipDeviceController::ChipDeviceController()
{
    mState      = kState_NotInitialized;
    AppState    = NULL;
    mConState   = kConnectionState_NotConnected;
    mDeviceCon  = NULL;
    mCurReqMsg  = NULL;
    mOnError    = NULL;
    mDeviceAddr = IPAddress::Any;
    mDevicePort = CHIP_PORT;
    mDeviceId   = 0;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
}

CHIP_ERROR ChipDeviceController::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = new System::Layer();
    mInetLayer   = new Inet::InetLayer();

    // Initialize the CHIP System Layer.
    err = mSystemLayer->Init(NULL);
    if (err != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(Controller, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = mInetLayer->Init(*mSystemLayer, NULL);
    if (err != INET_NO_ERROR)
    {
        ChipLogError(Controller, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    mState = kState_Initialized;

exit:
    return err;
}

CHIP_ERROR ChipDeviceController::Shutdown()
{
    if (mState != kState_Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    mState         = kState_NotInitialized;

    if (mDeviceCon != NULL)
    {
        mDeviceCon->Close();
        delete mDeviceCon;
        mDeviceCon = NULL;
    }
    mSystemLayer->Shutdown();
    mInetLayer->Shutdown();
    delete mSystemLayer;
    delete mInetLayer;
    mSystemLayer = NULL;
    mInetLayer   = NULL;

    mConState = kConnectionState_NotConnected;
    memset(&mOnComplete, 0, sizeof(mOnComplete));
    mOnError = NULL;

    return err;
}

CHIP_ERROR ChipDeviceController::ConnectDevice(uint64_t deviceId, IPAddress deviceAddr, void * appReqState,
                                               MessageReceiveHandler onMessageReceived, ErrorHandler onError, uint16_t devicePort)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != kState_Initialized || mDeviceCon != NULL || mConState != kConnectionState_NotConnected)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mDeviceId    = deviceId;
    mDeviceAddr  = deviceAddr;
    mDevicePort  = devicePort;
    mAppReqState = appReqState;
    mDeviceCon   = new ChipConnection();

    mDeviceCon->Init(mInetLayer);
    err = mDeviceCon->Connect(mDeviceId, mDeviceAddr, mDevicePort);
    SuccessOrExit(err);

    mDeviceCon->OnMessageReceived = OnReceiveMessage;
    mDeviceCon->OnReceiveError    = OnReceiveError;
    mDeviceCon->AppState          = this;

    mOnComplete.Response = onMessageReceived;
    mOnError             = onError;

    mConState = kConnectionState_Connected;

exit:
    if (err != CHIP_NO_ERROR && mDeviceCon != NULL)
    {
        mDeviceCon->Close();
        delete mDeviceCon;
        mDeviceCon = NULL;
    }
    return err;
}

CHIP_ERROR ChipDeviceController::DisconnectDevice()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != kState_Initialized || mConState != kConnectionState_Connected)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    err = mDeviceCon->Close();
    delete mDeviceCon;
    mDeviceCon = NULL;
    mConState  = kConnectionState_NotConnected;
    return err;
};

CHIP_ERROR ChipDeviceController::SendMessage(void * appReqState, PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;

    mAppReqState = appReqState;
    if (mConState == kConnectionState_Connected)
    {
        err = mDeviceCon->SendMessage(buffer);
    }

    return err;
}

void ChipDeviceController::ServiceEvents()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (mState != kState_Initialized)
    {
        return;
    }
    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    static bool printed = false;

    if (!printed)
    {
        {
            ChipLogProgress(Controller, "CHIP node ready to service events; PID: %d; PPID: %d\n", getpid(), getppid());
            printed = true;
        }
    }
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (mSystemLayer->State() == System::kLayerState_Initialized)
        mSystemLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    if (mInetLayer->State == Inet::InetLayer::kState_Initialized)
        mInetLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        ChipLogError(Controller, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (mSystemLayer->State() == System::kLayerState_Initialized)
    {
        mSystemLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (mInetLayer->State == Inet::InetLayer::kState_Initialized)
    {
        mInetLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
#endif
}

void ChipDeviceController::ClearRequestState()
{
    if (mCurReqMsg != NULL)
    {
        PacketBuffer::Free(mCurReqMsg);
        mCurReqMsg = NULL;
    }
}

void ChipDeviceController::OnReceiveMessage(ChipConnection * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo)
{
    ChipDeviceController * mgr = (ChipDeviceController *) con->AppState;
    if (mgr->mConState == kConnectionState_Connected && mgr->mOnComplete.Response != NULL && pktInfo != NULL)
    {
        mgr->mOnComplete.Response(mgr, mgr->mAppReqState, msgBuf, pktInfo);
    }
}

void ChipDeviceController::OnReceiveError(ChipConnection * con, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    ChipDeviceController * mgr = (ChipDeviceController *) con->AppState;
    if (mgr->mConState == kConnectionState_Connected && mgr->mOnError != NULL && pktInfo != NULL)
    {
        mgr->mOnError(mgr, mgr->mAppReqState, err, pktInfo);
    }
}

} // namespace DeviceController
} // namespace chip
