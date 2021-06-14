/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements an object for a CHIP Ping client which using Echo
 *      Protocol to measure packet loss across network paths for a paired device.
 */

#include <controller/PingClient.h>

#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

void PingClient::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
{
    mSystemLayer = systemLayer;
    mExchangeMgr = exchangeMgr;
}

void PingClient::Reset()
{
    mEchoIntervalMillis = 1000;
    mLastEchoTimeMillis = 0;
    mRemoteDeviceId     = 0;
    mEchoReqCount       = 0;
    mEchoRespCount      = 0;
    mEchoMaxCount       = 3;
    mEchoReqSize        = 32;
    mWaitingForEchoResp = false;
    mUsingMRP           = false;
    mSystemLayer        = nullptr;
    mExchangeMgr        = nullptr;
}

void PingClient::HandlePingTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error error)
{
    PingClient * pingclient = static_cast<PingClient *>(appState);

    if (pingclient->mEchoRespCount != pingclient->mEchoReqCount)
    {
        ChipLogError(Controller, "No response received");

        // Reset mEchoRespCount for next iteration, if any
        pingclient->mEchoRespCount = pingclient->mEchoReqCount;
    }

    if (pingclient->mEchoReqCount < pingclient->mEchoMaxCount)
    {
        CHIP_ERROR err = pingclient->SendEchoRequest();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Send request failed with error: %s", ErrorStr(err));
            pingclient->Shutdown();
        }
    }
    else
    {
        pingclient->Shutdown();
    }
}

void PingClient::OnMessageReceived(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload)
{
    uint64_t respTime    = static_cast<uint64_t>(System::Timer::GetCurrentEpoch());
    uint64_t transitTime = respTime - mLastEchoTimeMillis;

    mWaitingForEchoResp = false;
    mEchoRespCount++;

    ChipLogProgress(Controller, "Echo Response: %" PRIu16 "/%" PRIu16 "(%.2f%%) len=%u time=%.3fms\n", mEchoRespCount,
                    mEchoReqCount, static_cast<double>(mEchoRespCount) * 100 / mEchoReqCount, payload->DataLength(),
                    static_cast<double>(transitTime) / 1000);
}

CHIP_ERROR PingClient::SendEchoRequest()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    System::PacketBufferHandle payloadBuf;

    payloadBuf = MessagePacketBuffer::New(mEchoReqSize);
    VerifyOrReturnError(!payloadBuf.IsNull(), CHIP_ERROR_NO_MEMORY);

    memset(payloadBuf->Start(), 0, mEchoReqSize);
    payloadBuf->SetDataLength(mEchoReqSize);

    if (mUsingMRP)
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNone);
    }
    else
    {
        sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    }

    mLastEchoTimeMillis = System::Timer::GetCurrentEpoch();

    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const chip::System::Error timerErr = mSystemLayer->StartTimer(mEchoIntervalMillis, HandlePingTimeout, this);
    VerifyOrReturnError(timerErr == CHIP_SYSTEM_NO_ERROR, CHIP_SYSTEM_ERROR_START_TIMER_FAILED);

    ChipLogDetail(Controller, "Send echo request message with payload size: %d bytes to Node: 0x" ChipLogFormatX64, mEchoReqSize,
                  ChipLogValueX64(mRemoteDeviceId));

    mWaitingForEchoResp = true;
    err                 = mEchoClient.SendEchoRequest(std::move(payloadBuf), sendFlags);

    if (err == CHIP_NO_ERROR)
    {
        mEchoReqCount++;
    }
    else
    {
        mWaitingForEchoResp = false;
    }

    return err;
}

CHIP_ERROR PingClient::PingDevice(NodeId remoteDeviceId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in
    // issue:4451
    ReturnErrorOnFailure(mEchoClient.Init(mExchangeMgr, { remoteDeviceId, 0, 0 }));

    // Arrange to get a callback whenever an Echo Response is received.
    mEchoClient.SetDelegate(this);

    mRemoteDeviceId = remoteDeviceId;

    err = SendEchoRequest();

    if ((err != CHIP_NO_ERROR))
    {
        ChipLogError(Controller, "Send request failed with error: %s", ErrorStr(err));
        Shutdown();
    }

    return err;
}

void PingClient::Shutdown()
{
    // Cancel any existing connect timer.
    if (mSystemLayer)
    {
        mSystemLayer->CancelTimer(HandlePingTimeout, this);
    }

    mEchoClient.Shutdown();
}

} // namespace Controller
} // namespace chip
