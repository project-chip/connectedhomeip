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

#include <device/types/window-covering/WindowCoveringVariants.h>

namespace chip {
namespace app {
namespace {

// Number tags tell the variants apart when several run on one node (e.g. --device "*").
const EndpointComposition::SemanticTag kBlindTag[]   = { { .namespaceID = CommonNamespace::kNumberId, .tag = 1 } };
const EndpointComposition::SemanticTag kCurtainTag[] = { { .namespaceID = CommonNamespace::kNumberId, .tag = 2 } };
const EndpointComposition::SemanticTag kShutterTag[] = { { .namespaceID = CommonNamespace::kNumberId, .tag = 3 } };
const EndpointComposition::SemanticTag kAwningTag[]  = { { .namespaceID = CommonNamespace::kNumberId, .tag = 4 } };
const EndpointComposition::SemanticTag kPergolaTag[] = { { .namespaceID = CommonNamespace::kNumberId, .tag = 5 } };

WindowCovering::Config BlindConfig()
{
    WindowCovering::Config config{
        .features = BitFlags<Clusters::WindowCovering::Feature>(
            Clusters::WindowCovering::Feature::kLift, Clusters::WindowCovering::Feature::kPositionAwareLift,
            Clusters::WindowCovering::Feature::kTilt, Clusters::WindowCovering::Feature::kPositionAwareTilt),
        .type           = Clusters::WindowCovering::Type::kTiltBlindLiftAndTilt,
        .endProductType = Clusters::WindowCovering::EndProductType::kInteriorBlind,
        .tags           = Span<const EndpointComposition::SemanticTag>(kBlindTag),
    };
    config.optionalAttributes.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
        .Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
    return config;
}

WindowCovering::Config CurtainConfig()
{
    WindowCovering::Config config{
        .features       = BitFlags<Clusters::WindowCovering::Feature>(Clusters::WindowCovering::Feature::kLift,
                                                                Clusters::WindowCovering::Feature::kPositionAwareLift),
        .type           = Clusters::WindowCovering::Type::kDrapery,
        .endProductType = Clusters::WindowCovering::EndProductType::kCentralCurtain,
        .tags           = Span<const EndpointComposition::SemanticTag>(kCurtainTag),
    };
    config.optionalAttributes.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
    return config;
}

WindowCovering::Config ShutterConfig()
{
    WindowCovering::Config config{
        .features       = BitFlags<Clusters::WindowCovering::Feature>(Clusters::WindowCovering::Feature::kTilt,
                                                                Clusters::WindowCovering::Feature::kPositionAwareTilt),
        .type           = Clusters::WindowCovering::Type::kShutter,
        .endProductType = Clusters::WindowCovering::EndProductType::kSwingingShutter,
        .tags           = Span<const EndpointComposition::SemanticTag>(kShutterTag),
    };
    config.optionalAttributes.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
    return config;
}

WindowCovering::Config AwningConfig()
{
    WindowCovering::Config config{
        .features       = BitFlags<Clusters::WindowCovering::Feature>(Clusters::WindowCovering::Feature::kLift,
                                                                Clusters::WindowCovering::Feature::kPositionAwareLift),
        .type           = Clusters::WindowCovering::Type::kAwning,
        .endProductType = Clusters::WindowCovering::EndProductType::kAwningVerticalScreen,
        .tags           = Span<const EndpointComposition::SemanticTag>(kAwningTag),
    };
    config.optionalAttributes.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsLift::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
    return config;
}

WindowCovering::Config PergolaConfig()
{
    WindowCovering::Config config{
        .features       = BitFlags<Clusters::WindowCovering::Feature>(Clusters::WindowCovering::Feature::kTilt,
                                                                Clusters::WindowCovering::Feature::kPositionAwareTilt),
        .type           = Clusters::WindowCovering::Type::kShutter,
        .endProductType = Clusters::WindowCovering::EndProductType::kTiltOnlyPergola,
        .tags           = Span<const EndpointComposition::SemanticTag>(kPergolaTag),
    };
    config.optionalAttributes.Set<Clusters::WindowCovering::Attributes::NumberOfActuationsTilt::Id>()
        .Set<Clusters::WindowCovering::Attributes::CurrentPositionTiltPercentage::Id>()
        .Set<Clusters::WindowCovering::Attributes::SafetyStatus::Id>();
    return config;
}

// Zephyr sizes its device array per creator: keep ALL_DEVICES_ENABLED_DEVICE_COUNT in
// examples/all-devices-app/zephyr/CMakeLists.txt in sync when adding or removing entries.
constexpr WindowCoveringVariant kVariants[] = {
    { "window-covering", &BlindConfig },           { "window-covering-curtain", &CurtainConfig },
    { "window-covering-shutter", &ShutterConfig }, { "window-covering-awning", &AwningConfig },
    { "window-covering-pergola", &PergolaConfig },
};

} // namespace

Span<const WindowCoveringVariant> WindowCoveringVariants()
{
    return Span<const WindowCoveringVariant>(kVariants);
}

} // namespace app
} // namespace chip
