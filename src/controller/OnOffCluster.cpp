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

CHIP_ERROR OnOffCluster::On(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterOnCommand(mEndpoint);
    return SendCommand(payload, onCompletion);
}

CHIP_ERROR OnOffCluster::Off(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterOffCommand(mEndpoint);
    return SendCommand(payload, onCompletion);
}

CHIP_ERROR OnOffCluster::Toggle(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterToggleCommand(mEndpoint);
    return SendCommand(payload, onCompletion);
}

CHIP_ERROR OnOffCluster::ReadAttributeOnOff(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReadOnOffAttribute(mEndpoint);
    return SendCommand(payload, onCompletion);
}

CHIP_ERROR OnOffCluster::ReportAttributeOnOff(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                              uint16_t minInterval, uint16_t maxInterval)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReportOnOffAttribute(mEndpoint, minInterval, maxInterval);
    return RequestAttributeReporting(payload, onCompletion, onChange);
}

CHIP_ERROR OnOffCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(payload, onCompletion);
}

} // namespace Controller
} // namespace chip
