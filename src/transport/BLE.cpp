/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements the CHIP Connection object that maintains a BLE connection.
 *
 */

#include <transport/BLE.h>

#include <controller/CHIPDeviceController.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {

BLE::~BLE()
{
    if (mBleEndPoint)
    {
        // Ble endpoint is only non null if ble endpoint is initialized and connected
        mBleEndPoint->Close();
        mBleEndPoint = nullptr;
    }
}

CHIP_ERROR BLE::Init(BleConnectionParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(params.HasConnectionObject() || params.HasDiscriminator(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(params.GetBleLayer(), err = CHIP_ERROR_INVALID_ARGUMENT);

    if (params.HasConnectionObject())
    {
        err = InitInternal(params.GetBleLayer(), params.GetConnectionObject());
    }
    else
    {
        err = DelegateConnection(params.GetBleLayer(), params.GetDiscriminator());
    }
    SuccessOrExit(err);

    mBleCallbacks = params.GetCallbackHandler();
    mBleLayer     = params.GetBleLayer();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Failed to initialize Ble transport: %s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR BLE::InitInternal(Ble::BleLayer * bleLayer, BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = bleLayer->NewBleEndPoint(&mBleEndPoint, connObj, kBleRole_Central, true);
    SuccessOrExit(err);

    mBleEndPoint->mAppState          = reinterpret_cast<void *>(this);
    mBleEndPoint->OnMessageReceived  = OnBleEndPointReceive;
    mBleEndPoint->OnConnectComplete  = OnBleEndPointConnectionComplete;
    mBleEndPoint->OnConnectionClosed = OnBleEndPointConnectionClosed;

    // Initiate CHIP over BLE protocol connection.
    err = mBleEndPoint->StartConnect();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (mBleEndPoint)
        {
            mBleEndPoint = nullptr;
        }
    }
    return err;
}

CHIP_ERROR BLE::DelegateConnection(Ble::BleLayer * bleLayer, const uint16_t connDiscriminator)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = bleLayer->NewBleConnection(reinterpret_cast<void *>(this), connDiscriminator, OnBleConnectionComplete,
                                     OnBleConnectionError);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR BLE::SendMessage(const MessageHeader & header, const Transport::PeerAddress & address, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(address.GetTransportType() == Type::kBle, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mBleEndPoint != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err    = mBleEndPoint->Send(msgBuf);
    msgBuf = nullptr;
    SuccessOrExit(err);

exit:
    if (msgBuf != NULL)
    {
        System::PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

void BLE::OnBleConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BLE * ble      = reinterpret_cast<BLE *>(appState);

    err = ble->InitInternal(ble->mBleLayer, connObj);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ble->OnBleConnectionError(appState, err);
    }
}

void BLE::OnBleConnectionError(void * appState, BLE_ERROR err)
{
    BLE * ble = reinterpret_cast<BLE *>(appState);

    if (ble->mBleCallbacks)
    {
        ble->mBleCallbacks->OnBLEConnectionError(err);
    }
}

void BLE::OnBleEndPointReceive(BLEEndPoint * endPoint, PacketBuffer * buffer)
{
    BLE * ble = reinterpret_cast<BLE *>(endPoint->mAppState);

    if (ble->mBleCallbacks)
    {
        ble->mBleCallbacks->OnBLEPacketReceived(buffer);
    }
}

void BLE::OnBleEndPointConnectionComplete(BLEEndPoint * endPoint, BLE_ERROR err)
{
    BLE * ble   = reinterpret_cast<BLE *>(endPoint->mAppState);
    ble->mState = State::kInitialized;

    if (ble->mBleCallbacks)
    {
        ble->mBleCallbacks->OnBLEConnectionComplete(err);
    }
}

void BLE::OnBleEndPointConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    BLE * ble   = reinterpret_cast<BLE *>(endPoint->mAppState);
    ble->mState = State::kNotReady;

    if (ble->mBleCallbacks)
    {
        ble->mBleCallbacks->OnBLEConnectionClosed(err);
    }
}

} // namespace Transport
} // namespace chip
