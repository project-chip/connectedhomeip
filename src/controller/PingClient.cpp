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

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

bool PingClient::EchoIntervalExpired(void)
{
    uint64_t now = System::Timer::GetCurrentEpoch();

    return (now >= mLastEchoTimeMillis + mEchoIntervalMillis);
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

CHIP_ERROR PingClient::SendEchoRequest(NodeId remoteDeviceId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::SendFlags sendFlags;
    System::PacketBufferHandle payloadBuf;

    payloadBuf = MessagePacketBuffer::New(mEchoReqSize);
    VerifyOrExit(!payloadBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

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

    ChipLogDetail(Controller, "Send echo request message with payload size: %d bytes to Node: 0x" ChipLogFormatX64, mEchoReqSize,
                  ChipLogValueX64(remoteDeviceId));

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

exit:
    return err;
}

CHIP_ERROR PingClient::PingDevice(NodeId remoteDeviceId, uint16_t maxCount, Messaging::ExchangeManager * exchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: temprary create a SecureSessionHandle from node id to unblock end-to-end test. Complete solution is tracked in
    // issue:4451
    ReturnErrorOnFailure(mEchoClient.Init(exchangeMgr, { remoteDeviceId, 0, 0 }));

    // Arrange to get a callback whenever an Echo Response is received.
    mEchoClient.SetDelegate(this);

    // Connection has been established. Now send the EchoRequests.
    for (unsigned int i = 0; i < maxCount; i++)
    {
        err = SendEchoRequest(remoteDeviceId);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Send request failed: %s", ErrorStr(err));
            break;
        }

        // Wait for response until the Echo interval.
        while (!EchoIntervalExpired())
        {
            // We can use condition_varible to suspend the current thread and wake it up when response arrive after
            // condition_varible are supported on all embedded platforms.
            sleep(1);
        }

        // Check if expected response was received.
        if (mWaitingForEchoResp)
        {
            ChipLogError(Controller, "No response received");
            mWaitingForEchoResp = false;
        }
    }

    mEchoClient.Shutdown();

    if ((err != CHIP_NO_ERROR))
    {
        ChipLogError(Controller, "Ping failed with error: %s", ErrorStr(err));
    }

    return err;
}

} // namespace Controller
} // namespace chip
