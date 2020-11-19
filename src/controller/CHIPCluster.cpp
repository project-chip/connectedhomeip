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
 *  @file
 *    This file contains definitions for a base Cluster class. This class will
 *    be derived by various ZCL clusters supported by CHIP. The objects of the
 *    ZCL cluster class will be used by Controller applications to interact with
 *    the CHIP device.
 */

#include <controller/CHIPCluster.h>

namespace chip {
namespace Controller {

CHIP_ERROR ClusterBase::Associate(Device * device, EndpointId endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Check if the device supports mCluster at the requested endpoint

    mDevice   = device;
    mEndpoint = endpoint;
    return err;
}

void ClusterBase::Dissociate()
{
    mDevice = nullptr;
}

CHIP_ERROR ClusterBase::SendCommand(CommandEncoder commandEncoder, uint16_t maxCmdLen, Callback::Callback<> * responseHandler)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint16_t encodedLength = 0;
    System::PacketBufferHandle message;

    VerifyOrExit(commandEncoder != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::NewWithAvailableSize(maxCmdLen);
    VerifyOrExit(!message.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    encodedLength = commandEncoder(message->Start(), message->AvailableDataLength(), mEndpoint);
    VerifyOrExit(encodedLength != 0, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(encodedLength <= maxCmdLen, err = CHIP_ERROR_INTERNAL);

    message->SetDataLength(encodedLength);
    VerifyOrExit(message->DataLength() >= encodedLength, err = CHIP_ERROR_NO_MEMORY);

    err = mDevice->SendMessage(std::move(message));
    SuccessOrExit(err);

    if (responseHandler != nullptr)
    {
        mDevice->AddResponseHandler(mEndpoint, mClusterId, responseHandler);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in sending cluster command. Err %d", err);
    }

    return err;
}

CHIP_ERROR ClusterBase::RequestAttributeReporting(RequestEncoder requestEncoder, uint16_t maxCmdLen, uint16_t minInterval,
                                                  uint16_t maxInterval, Callback::Callback<> * reportHandler)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint16_t encodedLength = 0;
    System::PacketBufferHandle message;

    VerifyOrExit(requestEncoder != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::NewWithAvailableSize(maxCmdLen);
    VerifyOrExit(!message.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    encodedLength = requestEncoder(message->Start(), message->AvailableDataLength(), mEndpoint, minInterval, maxInterval);
    VerifyOrExit(encodedLength != 0, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(encodedLength <= maxCmdLen, err = CHIP_ERROR_INTERNAL);

    message->SetDataLength(encodedLength);
    VerifyOrExit(message->DataLength() >= encodedLength, err = CHIP_ERROR_NO_MEMORY);

    err = mDevice->SendMessage(std::move(message));
    SuccessOrExit(err);

    if (reportHandler != nullptr)
    {
        mDevice->AddReportHandler(mEndpoint, mClusterId, reportHandler);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in requesting attribute reporting. Err %d", err);
    }

    return err;
}

} // namespace Controller
} // namespace chip
