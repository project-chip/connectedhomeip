/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "BdxEndpoint.h"

#include <core/CHIPError.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <support/BitFlags.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {
namespace bdx {

CHIP_ERROR Endpoint::OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PacketHeader & packetHeader,
                                       const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle && payload)
{
    if (mExchangeCtx == nullptr)
    {
        mExchangeCtx = ec;
    }

    ChipLogDetail(BDX, "%s: message 0x%x protocol %u", __FUNCTION__, static_cast<uint8_t>(payloadHeader.GetMessageType()),
                  payloadHeader.GetProtocolID().GetProtocolId());
    CHIP_ERROR err =
        mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), System::Platform::Clock::GetMonotonicMilliseconds());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "failed to handle message: %s", ErrorStr(err));
    }
    return err;
}

void Endpoint::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogError(BDX, "%s, ec: %d", __FUNCTION__, ec->GetExchangeId());
    mExchangeCtx = nullptr;
    mTransfer.Reset();
}

void Endpoint::PollTimerHandler(chip::System::Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<chip::bdx::Endpoint *>(appState)->PollForOutput();
}

void Endpoint::PollForOutput()
{
    TransferSession::OutputEvent outEvent;
    mTransfer.PollOutput(outEvent, System::Platform::Clock::GetMonotonicMilliseconds());
    HandleTransferSessionOutput(outEvent);

#if CONFIG_DEVICE_LAYER
    DeviceLayer::SystemLayer.StartTimer(mPollFreqMs, PollTimerHandler, this);
#endif
}

void Endpoint::ScheduleImmediatePoll()
{
#if CONFIG_DEVICE_LAYER
    DeviceLayer::SystemLayer.StartTimer(kImmediatePollDelayMs, PollTimerHandler, this);
#endif
}

CHIP_ERROR Responder::PrepareForTransfer(TransferRole role, BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                         uint32_t timeoutMs, uint32_t pollFreqMs)
{
    mPollFreqMs = pollFreqMs;
    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, timeoutMs));
#if CONFIG_DEVICE_LAYER
    DeviceLayer::SystemLayer.StartTimer(mPollFreqMs, PollTimerHandler, this);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR Initiator::InitiateTransfer(TransferRole role, const TransferSession::TransferInitData & initData, uint32_t timeoutMs,
                                       uint32_t pollFreqMs)
{
    mPollFreqMs = pollFreqMs;
    ReturnErrorOnFailure(mTransfer.StartTransfer(role, initData, timeoutMs));
#if CONFIG_DEVICE_LAYER
    DeviceLayer::SystemLayer.StartTimer(mPollFreqMs, PollTimerHandler, this);
#endif
    return CHIP_NO_ERROR;
}

} // namespace bdx
} // namespace chip
