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

    mState = State::kNotReady;
}

CHIP_ERROR BLEBase::Init(const BleListenParameters & param)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    BleLayer * bleLayer = param.GetBleLayer();

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(bleLayer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mBleLayer                           = bleLayer;
    mBleLayer->mBleTransport            = this;
    mBleLayer->OnChipBleConnectReceived = nullptr;

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

    // Manually trigger the OnConnectComplete callback.
    OnEndPointConnectComplete(endPoint, err);

exit:
    return err;
}

CHIP_ERROR BLEBase::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                                System::PacketBufferHandle msgBuf)
{
    ReturnErrorCodeIf(address.GetTransportType() != Type::kBle, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

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
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].IsNull())
        {
            ChipLogDetail(Inet, "Message appended to BLE send queue");
            mPendingPackets[i] = std::move(msg);
            err                = CHIP_NO_ERROR;
            break;
        }
    }

    return err;
}

void BLEBase::OnBleConnectionComplete(Ble::BLEEndPoint * endPoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogDetail(Inet, "BleConnectionComplete: endPoint %p", endPoint);

    mBleEndPoint = endPoint;

    // Initiate CHIP over BLE protocol connection.
    err = mBleEndPoint->StartConnect();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (mBleEndPoint != nullptr)
        {
            mBleEndPoint->Close();
            mBleEndPoint = nullptr;
        }
        ChipLogError(Inet, "Failed to setup BLE endPoint: %s", ErrorStr(err));
    }
}

void BLEBase::OnBleConnectionError(BLE_ERROR err)
{
    ClearPendingPackets();
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
        ClearPendingPackets();
        return;
    }

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (!mPendingPackets[i].IsNull())
        {
            endPoint->Send(std::move(mPendingPackets[i]));
        }
    }
    ChipLogDetail(Inet, "BLE EndPoint %p Connection Complete", endPoint);
}

void BLEBase::OnEndPointConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    mState       = State::kInitialized;
    mBleEndPoint = nullptr;
    ClearPendingPackets();
}

void BLEBase::ClearPendingPackets()
{
    ChipLogDetail(Inet, "Clearing BLE pending packets.");
    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        mPendingPackets[i] = nullptr;
    }
}

} // namespace Transport
} // namespace chip
