/*
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

#include "LoggingIrrigationSystem.h"
#include "device/api/Interface.h"

#include <clusters/shared/Enums.h>
#include <cstdint>

namespace chip::app {
namespace {

using namespace Clusters;
constexpr uint8_t kLocationZoneTag = 0x04;

// Zone 1: level-controlled valve, no default duration (runs until closed).
const ValveConfigurationAndControlCluster::StartupConfiguration kConfig1{ DataModel::NullNullable, 100, 1 };

// Zone 2: level-controlled valve opening to 50% in steps of 10, auto-closes after 10 minutes by default.
const ValveConfigurationAndControlCluster::StartupConfiguration kConfig2{ DataModel::MakeNullable<uint32_t>(600), 50, 10 };

// Zone 3: simple on/off valve (no Level feature), auto-closes after 5 minutes by default.
const ValveConfigurationAndControlCluster::StartupConfiguration kConfig3{ DataModel::MakeNullable<uint32_t>(300),
                                                                          ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                          ValveConfigurationAndControlCluster::kDefaultLevelStep };
const ValveConfigurationAndControlCluster::ValveContext kValveContext1 = {
    .features             = BitFlags<ValveConfigurationAndControl::Feature>(ValveConfigurationAndControl::Feature::kLevel),
    .optionalAttributeSet = {},
    .config               = kConfig1,
    .tsTracker            = nullptr,
    .delegate             = nullptr,
};

const ValveConfigurationAndControlCluster::ValveContext kValveContext2 = {
    .features             = BitFlags<ValveConfigurationAndControl::Feature>(ValveConfigurationAndControl::Feature::kLevel),
    .optionalAttributeSet = ValveConfigurationAndControlCluster::OptionalAttributeSet()
                                .Set<ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id>()
                                .Set<ValveConfigurationAndControl::Attributes::LevelStep::Id>(),
    .config    = kConfig2,
    .tsTracker = nullptr,
    .delegate  = nullptr,
};
const ValveConfigurationAndControlCluster::ValveContext kValveContext3 = {
    .features             = {},
    .optionalAttributeSet = {},
    .config               = kConfig3,
    .tsTracker            = nullptr,
    .delegate             = nullptr,
};
const EndpointComposition::SemanticTag kValve1[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = kLocationZoneTag },
    { .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
const EndpointComposition::SemanticTag kValve2[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = kLocationZoneTag },
    { .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};
const EndpointComposition::SemanticTag kValve3[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = kLocationZoneTag },
    { .namespaceID = CommonNamespace::kNumberId, .tag = 3 },
};

} // namespace

std::vector<IrrigationSystem::ValveList> LoggingIrrigationSystem::ValveConfiguration()
{
    return std::vector<IrrigationSystem::ValveList>{
        IrrigationSystem::ValveList{
            .startupConfiguration = kConfig1,
            .valveContext         = kValveContext1,
            .tags                 = Span<const EndpointComposition::SemanticTag>(kValve1),
        },
        IrrigationSystem::ValveList{
            .startupConfiguration = kConfig2,
            .valveContext         = kValveContext2,
            .tags                 = Span<const EndpointComposition::SemanticTag>(kValve2),
        },
        IrrigationSystem::ValveList{
            .startupConfiguration = kConfig3,
            .valveContext         = kValveContext3,
            .tags                 = Span<const EndpointComposition::SemanticTag>(kValve3),
        },

    };
}

} // namespace chip::app
