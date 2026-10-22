/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/power-source-server/PowerSourceCluster.h>
#include <clusters/PowerSource/AttributeIds.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

class WiredPowerSource : public SingleEndpoint
{
    using Feature                                      = Clusters::PowerSource::Feature;
    constexpr static auto kWiredCurrentTypeAttributeId = Clusters::PowerSource::Attributes::WiredCurrentType::Id;

public:
    using SimpleWiredPowerSourceCluster = Clusters::PowerSourceCluster<BitFlags<Feature>(Feature::kWired).Raw(),
                                                                       OptionalAttributeSet<kWiredCurrentTypeAttributeId>::All()>;

    WiredPowerSource(CharSpan description, Clusters::PowerSource::WiredCurrentTypeEnum wiredCurrentType, uint8_t order = 0);
    ~WiredPowerSource() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    SimpleWiredPowerSourceCluster & WiredPowerSourceCluster();

protected:
    CharSpan mDescription;
    Clusters::PowerSource::WiredCurrentTypeEnum mWiredCurrentType;
    const uint8_t mOrder;
    EndpointId mEndpointList[1] = { kInvalidEndpointId };

    LazyRegisteredServerCluster<SimpleWiredPowerSourceCluster> mWiredPowerSourceCluster;
};

} // namespace chip::app
