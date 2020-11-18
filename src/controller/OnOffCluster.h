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

#pragma once

#include <controller/CHIPCluster.h>
#include <core/CHIPCallback.h>

namespace chip {
namespace Controller {

constexpr ClusterId kOnOffClusterId = 0x0006;

class DLL_EXPORT OnOffCluster : public ClusterBase
{
public:
    OnOffCluster() : ClusterBase(kOnOffClusterId) {}
    ~OnOffCluster() {}

    CHIP_ERROR On(Callback::Callback<> * onCompletion);
    CHIP_ERROR Off(Callback::Callback<> * onCompletion);
    CHIP_ERROR Toggle(Callback::Callback<> * onCompletion);

    CHIP_ERROR ReadAttributeOnOff(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReportAttributeOnOff(Callback::Callback<> * onChange, uint16_t minInterval, uint16_t maxInterval);

    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

} // namespace Controller
} // namespace chip
