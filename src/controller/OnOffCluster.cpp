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

constexpr uint16_t kMaxOnOffMessageLength = 64;

CHIP_ERROR OnOffCluster::On(Callback::Callback<> * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * message = nullptr;
    SEND_CLUSTER_COMMAND(err, mDevice, kMaxOnOffMessageLength, message,
                         encodeOnOffClusterOnCommand(message->Start(), message->AvailableDataLength(), mEndpoint), onCompletion);
    return err;
}

CHIP_ERROR OnOffCluster::Off(Callback::Callback<> * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * message = nullptr;
    SEND_CLUSTER_COMMAND(err, mDevice, kMaxOnOffMessageLength, message,
                         encodeOnOffClusterOffCommand(message->Start(), message->AvailableDataLength(), mEndpoint), onCompletion);
    return err;
}

CHIP_ERROR OnOffCluster::Toggle(Callback::Callback<> * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * message = nullptr;
    SEND_CLUSTER_COMMAND(err, mDevice, kMaxOnOffMessageLength, message,
                         encodeOnOffClusterToggleCommand(message->Start(), message->AvailableDataLength(), mEndpoint),
                         onCompletion);
    return err;
}

CHIP_ERROR OnOffCluster::IsOn(Callback::Callback<> * onCompletion)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * message = nullptr;
    SEND_CLUSTER_COMMAND(err, mDevice, kMaxOnOffMessageLength, message,
                         encodeOnOffClusterReadOnOffAttribute(message->Start(), message->AvailableDataLength(), mEndpoint),
                         onCompletion);
    return err;
}

CHIP_ERROR OnOffCluster::ReportAttributeOnOff(uint16_t minInterval, uint16_t maxInterval, Callback::Callback<> * onChange)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * message = nullptr;
    REQUEST_CLUSTER_REPORT(err, mDevice, kMaxOnOffMessageLength, message,
                           encodeOnOffClusterReportOnOffAttribute(message->Start(), message->AvailableDataLength(), mEndpoint,
                                                                  minInterval, maxInterval),
                           onChange);
    return err;
}

} // namespace Controller
} // namespace chip
