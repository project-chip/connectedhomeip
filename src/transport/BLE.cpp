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

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>

using namespace chip::Ble;
using namespace chip::System;

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

CHIP_ERROR BLE::Init(RendezvousSessionDelegate * delegate, const RendezvousParameters & params)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    BleLayer * bleLayer = params.GetBleLayer();

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(bleLayer, err = CHIP_ERROR_INCORRECT_STATE);

    mDelegate = delegate;

    mBleLayer                           = bleLayer;
    mBleLayer->mAppState                = reinterpret_cast<void *>(this);
    mBleLayer->OnChipBleConnectReceived = OnNewConnection;

    if (params.HasDiscriminator())
    {
        err = DelegateConnection(params.GetDiscriminator());
    }
    else if (params.HasConnectionObject())
    {
        err = InitInternal(params.GetConnectionObject());
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR BLE::InitInternal(BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mBleLayer->NewBleEndPoint(&mBleEndPoint, connObj, kBleRole_Central, true);
    SuccessOrExit(err);

    // Initiate CHIP over BLE protocol connection.
    SetupEvents(mBleEndPoint);
    err = mBleEndPoint->StartConnect();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (mBleEndPoint)
        {
            mBleEndPoint->Close();
            mBleEndPoint = nullptr;
        }
    }
    return err;
}

CHIP_ERROR BLE::SetEndPoint(Ble::BLEEndPoint * endPoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(endPoint->mState == BLEEndPoint::kState_Connected, err = CHIP_ERROR_INVALID_ARGUMENT);

    mBleEndPoint = endPoint;
    SetupEvents(mBleEndPoint);

    // Manually trigger the OnConnectComplete callback.
    OnBleEndPointConnectionComplete(endPoint, err);

exit:
    return err;
}

void BLE::SetupEvents(Ble::BLEEndPoint * endPoint)
{
    endPoint->mAppState          = reinterpret_cast<void *>(this);
    endPoint->OnMessageReceived  = OnBleEndPointReceive;
    endPoint->OnConnectComplete  = OnBleEndPointConnectionComplete;
    endPoint->OnConnectionClosed = OnBleEndPointConnectionClosed;
}

CHIP_ERROR BLE::DelegateConnection(const uint16_t connDiscriminator)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mBleLayer->NewBleConnection(reinterpret_cast<void *>(this), connDiscriminator, OnBleConnectionComplete,
                                      OnBleConnectionError);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR BLE::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address, System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    const uint16_t headerSize = header.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize;

    VerifyOrExit(address.GetTransportType() == Type::kBle, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mBleEndPoint != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);

    err = header.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);

    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err = mBleEndPoint->Send(std::move(msgBuf));
    SuccessOrExit(err);

exit:
    return err;
}

void BLE::OnBleConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BLE * ble      = reinterpret_cast<BLE *>(appState);

    err = ble->InitInternal(connObj);
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

    if (ble->mDelegate)
    {
        ble->mDelegate->OnRendezvousError(err);
    }
}

void BLE::OnBleEndPointReceive(BLEEndPoint * endPoint, PacketBufferHandle buffer)
{
    BLE * ble      = reinterpret_cast<BLE *>(endPoint->mAppState);
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (ble->mDelegate)
    {
        uint16_t headerSize = 0;

        PacketHeader header;
        err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
        SuccessOrExit(err);

        buffer->ConsumeHead(headerSize);
        ble->mDelegate->OnRendezvousMessageReceived(header, Transport::PeerAddress(Transport::Type::kBle), std::move(buffer));
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive BLE message: %s", ErrorStr(err));
    }
}

void BLE::OnBleEndPointConnectionComplete(BLEEndPoint * endPoint, BLE_ERROR err)
{
    BLE * ble   = reinterpret_cast<BLE *>(endPoint->mAppState);
    ble->mState = State::kInitialized;

    if (ble->mDelegate)
    {
        if (err != BLE_NO_ERROR)
        {
            ble->mDelegate->OnRendezvousError(err);
        }
        else
        {
            ble->mDelegate->OnRendezvousConnectionOpened();
        }
    }
}

void BLE::OnBleEndPointConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    BLE * ble   = reinterpret_cast<BLE *>(endPoint->mAppState);
    ble->mState = State::kNotReady;

    // Already closed, avoid closing again in our destructor.
    ble->mBleEndPoint = nullptr;

    if (ble->mDelegate)
    {
        if (err != BLE_NO_ERROR)
        {
            ble->mDelegate->OnRendezvousError(err);
        } else {
            // OnRendezvousError may delete |ble|; don't call both callbacks.
            ble->mDelegate->OnRendezvousConnectionClosed();
        }
    }
}

void BLE::OnNewConnection(BLEEndPoint * endPoint)
{
    BLE * ble      = reinterpret_cast<BLE *>(endPoint->mAppState);
    CHIP_ERROR err = ble->SetEndPoint(endPoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Transport::BLE Init failure: %s", ErrorStr(err));
    }
}

} // namespace Transport
} // namespace chip
