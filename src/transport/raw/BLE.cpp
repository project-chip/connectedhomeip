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

#include <transport/raw/BLE.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>

using namespace chip::Ble;
using namespace chip::System;

namespace chip {
namespace Transport {

BLEBase::~BLEBase()
{
    ClearState();
}

void BLEBase::ClearState()
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

CHIP_ERROR BLEBase::Init(const BleListenParameters & param)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    BleLayer * bleLayer = param.GetBleLayer();

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(bleLayer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mBleLayer                           = bleLayer;
    mBleLayer->mBleTransport            = this;
    mBleLayer->OnChipBleConnectReceived = OnNewConnection;

    mState = State::kInitialized;

    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR BLEBase::SetEndPoint(Ble::BLEEndPoint * endPoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(endPoint->mState == BLEEndPoint::kState_Connected, err = CHIP_ERROR_INVALID_ARGUMENT);

    mBleEndPoint = endPoint;
    SetupEvents(mBleEndPoint);

    // Manually trigger the OnConnectComplete callback.
    OnEndPointConnectComplete(endPoint, err);

exit:
    return err;
}

void BLEBase::SetupEvents(Ble::BLEEndPoint * endPoint)
{
    endPoint->mAppState = reinterpret_cast<void *>(this);
}

CHIP_ERROR BLEBase::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                                System::PacketBufferHandle msgBuf)
{
    ReturnErrorCodeIf(address.GetTransportType() != Type::kBle, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    // ReturnErrorCodeIf(mBleEndPoint == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(header.EncodeBeforeData(msgBuf));

    if (mState == State::kConnected)
    {
        ReturnErrorOnFailure(mBleEndPoint->Send(std::move(msgBuf)));
    }
    else
    {
        ReturnErrorOnFailure(SendAfterConnect(std::move(msgBuf)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEBase::SendAfterConnect(System::PacketBufferHandle msg)
{
    // This will initiate a connection to the specified peer
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    // Iterate through the ENTIRE array. If a pending packet for
    // the address already exists, this means a connection is pending and
    // does NOT need to be re-established.
    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].IsNull())
        {
            ChipLogDetail(Inet, "Message appended to send queue");
            mPendingPackets[i] = std::move(msg);
            err                = CHIP_NO_ERROR;
            break;
        }
    }

    return err;
}

void BLEBase::OnBleConnectionComplete(Ble::BLEEndPoint * endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogDetail(Inet, "BleConnectionComplete");

    mBleEndPoint = endpoint;

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
        ChipLogError(Ble, "Failed to setup ble endpoint: %s", ErrorStr(err));
    }
}

void BLEBase::OnBleConnectionError(BLE_ERROR err)
{
    ChipLogDetail(Inet, "BleConnection Error: %s", ErrorStr(err));
}

void BLEBase::OnEndPointMessageReceived(BLEEndPoint * endPoint, PacketBufferHandle buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PacketHeader header;
    if ((err = header.DecodeAndConsume(buffer)) != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive BLE message: %s", ErrorStr(err));
        return;
    }

    HandleMessageReceived(header, Transport::PeerAddress(Transport::Type::kBle), std::move(buffer));
}

void BLEBase::OnEndPointConnectComplete(BLEEndPoint * endPoint, BLE_ERROR err)
{
    mState = State::kConnected;

    if (err != BLE_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to establish BLE connection: %s", ErrorStr(err));
    }
    else
    {
        for (size_t i = 0; i < mPendingPacketsSize; i++)
        {
            if (!mPendingPackets[i].IsNull())
            {
                endPoint->Send(std::move(mPendingPackets[i]));
            }
        }
        ChipLogDetail(Inet, "BLE EndPoint Connection Complete");
    }
}

void BLEBase::OnEndPointConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    mState = State::kInitialized;
}

void BLEBase::OnNewConnection(BLEEndPoint * endPoint)
{
    CHIP_ERROR err = endPoint->mBleTransport->SetEndPoint(endPoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Transport::BLE Init failure: %s", ErrorStr(err));
    }
}

} // namespace Transport
} // namespace chip
