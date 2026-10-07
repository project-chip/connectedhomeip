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

#include "LoggingIrrigation.h"
#include "device/api/Interface.h"

#include <clusters/shared/Enums.h>

namespace chip::app {
namespace {

using namespace Clusters;

const ValveConfigurationAndControlCluster::StartupConfiguration defaultConfig{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };

const ValveConfigurationAndControlCluster::ValveContext defaultValveContext = {
        .features             = BitFlags<ValveConfigurationAndControl::Feature>(ValveConfigurationAndControl::Feature::kLevel),
        .optionalAttributeSet = {},
        .config                = defaultConfig,
        .tsTracker            = nullptr,
        .delegate              = nullptr,
    };


const EndpointComposition::SemanticTag kValve1[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = 1},
    { .namespaceID = CommonNamespace::kNumberId, .tag = 1 },
};
const EndpointComposition::SemanticTag kValve2[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = 2},
    { .namespaceID = CommonNamespace::kNumberId, .tag = 2 },
};
const EndpointComposition::SemanticTag kValve3[] = {
    { .namespaceID = CommonNamespace::kLocationId, .tag = 2},
    { .namespaceID = CommonNamespace::kNumberId, .tag = 3 },
};

} // namespace

std::vector<Irrigation::ValveList> LoggingIrrigation::ValveConfiguration()
{
    return  std::vector<Irrigation::ValveList>{
                    Irrigation::ValveList {
                                                .startupConfiguration = defaultConfig,
                                                .valveContext = defaultValveContext,
                                                .tags = Span<const EndpointComposition::SemanticTag>(kValve1),
                    },
                     Irrigation::ValveList {  .startupConfiguration = defaultConfig,
                                                    .valveContext = defaultValveContext,
                                                .tags = Span<const EndpointComposition::SemanticTag>(kValve2),
                    },
                    Irrigation::ValveList{
                                                .startupConfiguration = defaultConfig,
                                                .valveContext = defaultValveContext,
                                                .tags = Span<const EndpointComposition::SemanticTag>(kValve3),
                    },

        };
}

} // namespace chip::app
