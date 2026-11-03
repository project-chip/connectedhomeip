/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/python/matter/im_capture/IMCapture.h>

#include <cstring>

#include <controller/python/matter/native/ChipMainLoopWork.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

static_assert(CONFIG_BUILD_FOR_HOST_UNIT_TEST,
              "IMCapture.cpp requires CONFIG_BUILD_FOR_HOST_UNIT_TEST; this file is only valid for the Python test build.");

namespace {

class ImReceivedMessageObserver : public chip::Messaging::TestOnlyReceivedMessageObserver
{
public:
    void OnMessageReceived(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader,
                           const chip::System::PacketBufferHandle & msgBuf) override
    {
        if (!payloadHeader.HasProtocol(chip::Protocols::InteractionModel::Id))
        {
            return;
        }

        using chip::Protocols::InteractionModel::MsgType;
        const auto opcode = static_cast<MsgType>(payloadHeader.GetMessageType());

        if (opcode == MsgType::InvokeCommandResponse)
        {
            mInvokeResponseCount++;
            mTotalImResponseCount++;
        }
        else if (opcode == MsgType::StatusResponse)
        {
            mStatusResponseCount++;
            mTotalImResponseCount++;
        }
        else if (opcode == MsgType::WriteResponse)
        {
            mWriteResponseCount++;
            mTotalImResponseCount++;
        }
    }

    void Reset()
    {
        mInvokeResponseCount  = 0;
        mStatusResponseCount  = 0;
        mWriteResponseCount   = 0;
        mTotalImResponseCount = 0;
    }

    PychipImCaptureSnapshot Snapshot() const
    {
        return PychipImCaptureSnapshot{
            .invokeResponseCount  = mInvokeResponseCount,
            .statusResponseCount  = mStatusResponseCount,
            .writeResponseCount   = mWriteResponseCount,
            .totalImResponseCount = mTotalImResponseCount,
        };
    }

private:
    uint32_t mInvokeResponseCount  = 0;
    uint32_t mStatusResponseCount  = 0;
    uint32_t mWriteResponseCount   = 0;
    uint32_t mTotalImResponseCount = 0;
};

ImReceivedMessageObserver gImObserver;

} // namespace

extern "C" {

PyChipError pychip_im_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl)
{
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    chip::MainLoopWork::ExecuteInMainLoop([devCtrl] { devCtrl->ExchangeMgr()->SetTestOnlyReceivedMessageObserver(&gImObserver); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_im_capture_reset(void)
{
    chip::MainLoopWork::ExecuteInMainLoop([] { gImObserver.Reset(); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_im_capture_get_snapshot(PychipImCaptureSnapshot * out)
{
    VerifyOrReturnError(out != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    chip::MainLoopWork::ExecuteInMainLoop([out] { *out = gImObserver.Snapshot(); });
    return ToPyChipError(CHIP_NO_ERROR);
}

} // extern "C"
