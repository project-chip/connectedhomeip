/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "NetworkCommand.h"

using namespace ::chip;

constexpr uint16_t kWaitDurationInSeconds = 10;

// Make sure our buffer is big enough, but this will need a better setup!
constexpr uint16_t kMaxBufferSize = 1024;

CHIP_ERROR NetworkCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mCommissioner.Init(localId, &storage);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    err = RunInternal(remoteId);
    SuccessOrExit(err);

    VerifyOrExit(GetCommandExitStatus(), err = CHIP_ERROR_INTERNAL);

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
    return err;
}

CHIP_ERROR NetworkCommand::RunInternal(NodeId remoteId)
{
    ChipDevice * device;
    CHIP_ERROR err = mCommissioner.GetDevice(remoteId, &device);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Could not find a paired device. Are you sure it has been paired ?"));

    device->SetDelegate(this);

    err = RunCommandInternal(device);
    SuccessOrExit(err);

    UpdateWaitForResponse(true);
    WaitForResponse(kWaitDurationInSeconds);

exit:
    return err;
}

CHIP_ERROR NetworkCommand::RunCommandInternal(ChipDevice * device)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t payloadLen = 0;

    PacketBufferHandle buffer = PacketBuffer::NewWithAvailableSize(kMaxBufferSize);
    VerifyOrExit(!buffer.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    payloadLen = Encode(buffer, kMaxBufferSize);
    VerifyOrExit(payloadLen != 0, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    buffer->SetDataLength(payloadLen);

#ifdef DEBUG
    PrintBuffer(buffer);
#endif

    err = device->SendMessage(std::move(buffer));
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Failed to send message: %s", ErrorStr(err)));

exit:
    return err;
}

void NetworkCommand::OnMessage(PacketBufferHandle buffer)
{
    ChipLogDetail(chipTool, "OnMessage: Received %zu bytes", buffer->DataLength());

    SetCommandExitStatus(Decode(buffer));
    UpdateWaitForResponse(false);
}

void NetworkCommand::OnStatusChange(void)
{
    ChipLogProgress(chipTool, "DeviceStatusDelegate::OnStatusChange");
}

void NetworkCommand::PrintBuffer(PacketBufferHandle & buffer) const
{
    const size_t data_len = buffer->DataLength();

    fprintf(stderr, "SENDING: %zu ", data_len);
    for (size_t i = 0; i < data_len; ++i)
    {
        fprintf(stderr, "%d ", buffer->Start()[i]);
    }
    fprintf(stderr, "\n");
}
