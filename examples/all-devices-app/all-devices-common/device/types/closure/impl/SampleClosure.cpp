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

#include "SampleClosure.h"
#include <clusters/shared/Enums.h>

namespace chip::app {
namespace {

using namespace Clusters;

// Spans over these are handed to the device, so they must outlive it.
const EndpointComposition::SemanticTag kDoorTag[]   = { { .namespaceID = CommonNamespace::kClosureId,
                                                          .tag         = to_underlying(Globals::ClosureTag::kDoor) } };
const EndpointComposition::SemanticTag kLiftTag[]   = { { .namespaceID = CommonNamespace::kClosurePanelId,
                                                          .tag         = to_underlying(Globals::ClosurePanelTag::kLift) } };
const EndpointComposition::SemanticTag kRotateTag[] = { { .namespaceID = CommonNamespace::kClosurePanelId,
                                                          .tag         = to_underlying(Globals::ClosurePanelTag::kRotate) } };
const EndpointComposition::SemanticTag kSlideTag[]  = { { .namespaceID = CommonNamespace::kClosurePanelId,
                                                          .tag         = to_underlying(Globals::ClosurePanelTag::kSliding) } };

constexpr Percent100ths kPanelResolution = 1;
constexpr Percent100ths kPanelStepValue  = 1;

ClosurePanel::Config LiftPanel()
{
    return {
        .withAccess = true,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::TranslationParams{ ClosureDimension::TranslationDirectionEnum::kBackward },
            },
    };
}

ClosurePanel::Config ModulatingPanel()
{
    return {
        .withAccess = false,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::ModulationParams{ ClosureDimension::ModulationTypeEnum::kSlatsOpenwork },
            },
    };
}

ClosurePanel::Config RotatingPanel()
{
    return {
        .withAccess = true,
        .motionLatching =
            BitFlags<ClosureDimension::LatchControlModesBitmap>(ClosureDimension::LatchControlModesBitmap::kRemoteLatching,
                                                                ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching),
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kLeft,
                                                            ClosureDimension::OverflowEnum::kTopInside },
            },
    };
}

} // namespace

SampleClosure MakeSampleClosure()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kDoorTag),
            .withPositioning = true,
            .motionLatching  = BitFlags<ClosureControl::LatchControlModesBitmap>(
                ClosureControl::LatchControlModesBitmap::kRemoteLatching,
                ClosureControl::LatchControlModesBitmap::kRemoteUnlatching),
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withPedestrian             = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                MakeOptional(DataModel::MakeNullable(false)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            PanelList{ LiftPanel(), Span<const EndpointComposition::SemanticTag>(kLiftTag) },
            PanelList{ ModulatingPanel(), Span<const EndpointComposition::SemanticTag>(kSlideTag) },
            PanelList{ RotatingPanel(), Span<const EndpointComposition::SemanticTag>(kRotateTag) },
        },
    };
}

} // namespace chip::app
