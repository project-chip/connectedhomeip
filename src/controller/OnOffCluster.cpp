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

#include <app/chip-zcl-zpro-codec-api.h>
#include <controller/OnOffCluster.h>

namespace chip {
namespace Controller {

CHIP_ERROR OnOffCluster::On(DeviceCallback * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint16_t encodeStatus          = 0;
    System::PacketBuffer * message = nullptr;
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::New();
    VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);

    encodeStatus = encodeOnOffClusterOnCommand(message->Start(), message->AvailableDataLength(), mEndpoint);
    VerifyOrExit(encodeStatus != 0, err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(message);
    SuccessOrExit(err);

    mDevice->OnResponse(mEndpoint, mClusterId, onCompletion);

exit:
    return err;
}

CHIP_ERROR OnOffCluster::Off(DeviceCallback * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint16_t encodeStatus          = 0;
    System::PacketBuffer * message = nullptr;
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::New();
    VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);

    encodeStatus = encodeOnOffClusterOffCommand(message->Start(), message->AvailableDataLength(), mEndpoint);
    VerifyOrExit(encodeStatus != 0, err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(message);
    SuccessOrExit(err);

    mDevice->OnResponse(mEndpoint, mClusterId, onCompletion);

exit:
    return err;
}

CHIP_ERROR OnOffCluster::Toggle(DeviceCallback * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint16_t encodeStatus          = 0;
    System::PacketBuffer * message = nullptr;
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::New();
    VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);

    encodeStatus = encodeOnOffClusterToggleCommand(message->Start(), message->AvailableDataLength(), mEndpoint);
    VerifyOrExit(encodeStatus != 0, err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(message);
    SuccessOrExit(err);

    mDevice->OnResponse(mEndpoint, mClusterId, onCompletion);

exit:
    return err;
}

CHIP_ERROR OnOffCluster::IsOn(DeviceCallback * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint16_t encodeStatus          = 0;
    System::PacketBuffer * message = nullptr;
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::New();
    VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);

    encodeStatus = encodeOnOffClusterReadOnOffAttribute(message->Start(), message->AvailableDataLength(), mEndpoint);
    VerifyOrExit(encodeStatus != 0, err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(message);
    SuccessOrExit(err);

    mDevice->OnResponse(mEndpoint, mClusterId, onCompletion);

exit:
    return err;
}

CHIP_ERROR OnOffCluster::ReportAttributeOnOff(uint16_t minInterval, uint16_t maxInterval, DeviceCallback * onChange)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint16_t encodeStatus          = 0;
    System::PacketBuffer * message = nullptr;
    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    message = System::PacketBuffer::New();
    VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);

    encodeStatus = encodeOnOffClusterReportOnOffAttribute(message->Start(), message->AvailableDataLength(), mEndpoint, minInterval,
                                                          maxInterval);
    VerifyOrExit(encodeStatus != 0, err = CHIP_ERROR_INTERNAL);

    err = mDevice->SendMessage(message);
    SuccessOrExit(err);

    mDevice->OnReport(mEndpoint, mClusterId, onChange);

exit:
    return err;
}

} // namespace Controller
} // namespace chip
