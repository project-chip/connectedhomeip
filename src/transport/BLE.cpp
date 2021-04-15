/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

// The largest supported value for Rendezvous discriminators
const uint16_t kMaxRendezvousDiscriminatorValue = 0xFFF;

BLE::~BLE()
{
    ClearState();
}

void BLE::ClearState()
{
    if (mBleLayer)
    {
        mBleLayer->CancelBleIncompleteConnection();
        mBleLayer->OnChipBleConnectReceived = nullptr;
        mBleLayer                           = nullptr;
    }

    if (mBleEndPoint)
    {
        mBleEndPoint->Close();
        mBleEndPoint = nullptr;
    }
}

CHIP_ERROR BLE::Init(RendezvousSessionDelegate * delegate, TransportMgrDelegate * transport, BleLayer * bleLayer,
                     uint16_t discriminator, BLE_CONNECTION_OBJECT connObj)
{
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(bleLayer, CHIP_ERROR_INCORRECT_STATE);

    mDelegate  = delegate;
    mTransport = transport;

    mBleLayer                           = bleLayer;
    mBleLayer->mAppState                = reinterpret_cast<void *>(this);
    mBleLayer->OnChipBleConnectReceived = OnNewConnection;

    if (discriminator <= kMaxRendezvousDiscriminatorValue)
    {
        return mBleLayer->NewBleConnection(reinterpret_cast<void *>(this), discriminator, OnBleConnectionComplete,
                                           OnBleConnectionError);
    }
    else if (connObj != 0)
    {
        return InitInternal(connObj);
    }

    return CHIP_NO_ERROR;
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
        ClearState();
    }
    return err;
}

CHIP_ERROR BLE::SetEndPoint(Ble::BLEEndPoint * endPoint)
{
    VerifyOrReturnError(endPoint->mState == BLEEndPoint::kState_Connected, CHIP_ERROR_INVALID_ARGUMENT);

    mBleEndPoint = endPoint;
    SetupEvents(mBleEndPoint);

    // Manually trigger the OnConnectComplete callback.
    OnBleEndPointConnectionComplete(endPoint, BLE_NO_ERROR);

    return CHIP_NO_ERROR;
}

void BLE::SetupEvents(Ble::BLEEndPoint * endPoint)
{
    endPoint->mAppState          = reinterpret_cast<void *>(this);
    endPoint->OnMessageReceived  = OnBleEndPointReceive;
    endPoint->OnConnectComplete  = OnBleEndPointConnectionComplete;
    endPoint->OnConnectionClosed = OnBleEndPointConnectionClosed;
}

CHIP_ERROR BLE::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address, System::PacketBufferHandle msgBuf)
{
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBleEndPoint != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(header.EncodeBeforeData(msgBuf));
    ReturnErrorOnFailure(mBleEndPoint->Send(std::move(msgBuf)));

    return CHIP_NO_ERROR;
}

void BLE::OnBleConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BLE * ble      = reinterpret_cast<BLE *>(appState);

    // TODO(#4547): On darwin, OnBleConnectionComplete is called multiple times for the same peripheral, this should become an error
    // in the future.
    VerifyOrExit(ble->mBleEndPoint == nullptr || !ble->mBleEndPoint->ConnectionObjectIs(connObj),
                 ChipLogError(Ble, "Warning: OnBleConnectionComplete is called multiple times for the same peripheral."));

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

    if (ble->mTransport)
    {
        PacketHeader header;
        err = header.DecodeAndConsume(buffer);
        SuccessOrExit(err);

        ble->mTransport->OnMessageReceived(header, Transport::PeerAddress(Transport::Type::kBle), std::move(buffer));
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
        }
        else
        {
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
