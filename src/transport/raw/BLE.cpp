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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
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
        mBleLayer->mBleTransport = nullptr;
        mBleLayer                = nullptr;
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
    BleLayer * bleLayer = param.GetBleLayer();

    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(bleLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mBleLayer = bleLayer;
    if (mBleLayer->mBleTransport == nullptr || !param.PreserveExistingBleLayerTransport())
    {
        mBleLayer->mBleTransport = this;
        ChipLogDetail(Inet, "BLEBase::Init - setting/overriding transport");
    }
    else
    {
        ChipLogDetail(Inet, "BLEBase::Init - not overriding transport");
    }

    mState = State::kInitialized;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEBase::SetEndPoint(Ble::BLEEndPoint * endPoint)
{
    VerifyOrReturnError(endPoint->mState == BLEEndPoint::kState_Connected, CHIP_ERROR_INVALID_ARGUMENT);

    mBleEndPoint = endPoint;

    // Manually trigger the OnConnectComplete callback.
    OnEndPointConnectComplete(endPoint, CHIP_NO_ERROR);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    ReturnErrorCodeIf(address.GetTransportType() != Type::kBle, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mBleEndPoint == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

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

CHIP_ERROR BLEBase::SendAfterConnect(System::PacketBufferHandle && msg)
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
        ChipLogError(Inet, "Failed to setup BLE endPoint: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void BLEBase::OnBleConnectionError(CHIP_ERROR err)
{
    ClearPendingPackets();
    ChipLogDetail(Inet, "BleConnection Error: %" CHIP_ERROR_FORMAT, err.Format());
}

void BLEBase::OnEndPointMessageReceived(BLEEndPoint * endPoint, PacketBufferHandle && buffer)
{
    HandleMessageReceived(Transport::PeerAddress(Transport::Type::kBle), std::move(buffer));
}

void BLEBase::OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err)
{
    mState = State::kConnected;

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to establish BLE connection: %" CHIP_ERROR_FORMAT, err.Format());
        OnEndPointConnectionClosed(endPoint, err);
        return;
    }

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (!mPendingPackets[i].IsNull())
        {
            err = endPoint->Send(std::move(mPendingPackets[i]));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Inet, "Deferred sending failed: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }
    ChipLogDetail(Inet, "BLE EndPoint %p Connection Complete", endPoint);
}

void BLEBase::OnEndPointConnectionClosed(BLEEndPoint * endPoint, CHIP_ERROR err)
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
