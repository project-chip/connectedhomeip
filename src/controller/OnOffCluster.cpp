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

// TODO: Find a way to calculate maximum message length for clusters
//       https://github.com/project-chip/connectedhomeip/issues/965
constexpr uint16_t kMaxOnOffMessageLength = 64;

CHIP_ERROR OnOffCluster::On(Callback::Callback<> * onCompletion)
{
    return SendCommand(encodeOnOffClusterOnCommand, kMaxOnOffMessageLength, onCompletion);
}

CHIP_ERROR OnOffCluster::Off(Callback::Callback<> * onCompletion)
{
    return SendCommand(encodeOnOffClusterOffCommand, kMaxOnOffMessageLength, onCompletion);
}

CHIP_ERROR OnOffCluster::Toggle(Callback::Callback<> * onCompletion)
{
    return SendCommand(encodeOnOffClusterToggleCommand, kMaxOnOffMessageLength, onCompletion);
}

CHIP_ERROR OnOffCluster::ReadAttributeOnOff(Callback::Callback<> * onCompletion)
{
    return SendCommand(encodeOnOffClusterReadOnOffAttribute, kMaxOnOffMessageLength, onCompletion);
}

CHIP_ERROR OnOffCluster::ReportAttributeOnOff(Callback::Callback<> * onChange, uint16_t minInterval, uint16_t maxInterval)
{
    return RequestAttributeReporting(encodeOnOffClusterReportOnOffAttribute, kMaxOnOffMessageLength, minInterval, maxInterval,
                                     onChange);
}

CHIP_ERROR OnOffCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
