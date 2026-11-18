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

#include <device/types/closure/impl/ClosureVariants.h>

#include <clusters/shared/Enums.h>

namespace chip {
namespace app {
namespace {

using namespace Clusters;

// Spans over these are handed to the device, so they must outlive it.
const EndpointComposition::SemanticTag kCabinetTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure),
      .tag         = to_underlying(Globals::ClosureTag::kCabinet) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kNumber), .tag = 1 },
};
const EndpointComposition::SemanticTag kCabinetNoPsNoSpTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure),
      .tag         = to_underlying(Globals::ClosureTag::kCabinet) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kNumber), .tag = 3 },
};
const EndpointComposition::SemanticTag kLiftTag[]   = { { .namespaceID =
                                                              to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
                                                          .tag = to_underlying(Globals::ClosurePanelTag::kLift) } };
const EndpointComposition::SemanticTag kRotateTag[] = { { .namespaceID =
                                                              to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
                                                          .tag = to_underlying(Globals::ClosurePanelTag::kRotate) } };
const EndpointComposition::SemanticTag kSlideTag[]  = { { .namespaceID =
                                                              to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
                                                          .tag = to_underlying(Globals::ClosurePanelTag::kSliding) } };

constexpr Percent100ths kPanelResolution = 1;
constexpr Percent100ths kPanelStepValue  = 1;

ClosurePanel::Config LiftPanel()
{
    return {
        .withAccess = false,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::TranslationParams{ ClosureDimension::TranslationDirectionEnum::kBackward },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kMillimeter,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 2000 }),
                    },
            },
    };
}

ClosurePanel::Config ModulatingPanel()
{
    return {
        .withAccess = false,
        .motionLatching =
            BitFlags<ClosureDimension::LatchControlModesBitmap>(ClosureDimension::LatchControlModesBitmap::kRemoteLatching,
                                                                ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching),
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
        .withAccess = false,
        .motionLatching =
            BitFlags<ClosureDimension::LatchControlModesBitmap>(ClosureDimension::LatchControlModesBitmap::kRemoteLatching,
                                                                ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching),
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kLeft,
                                                        ClosureDimension::OverflowEnum::kTopInside },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kDegree,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 90 }),
                    },
            },
    };
}

const EndpointComposition::SemanticTag kDoorTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure), .tag = to_underlying(Globals::ClosureTag::kDoor) },
};

const EndpointComposition::SemanticTag kGarageDoorTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure),
      .tag         = to_underlying(Globals::ClosureTag::kGarageDoor) },
};

const EndpointComposition::SemanticTag kGateTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure), .tag = to_underlying(Globals::ClosureTag::kGate) },
};
// Both leaves use the Rotate tag, so each also carries a Position tag.
const EndpointComposition::SemanticTag kGateLeftLeafTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kRotate) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kPosition), .tag = to_underlying(Globals::PositionTag::kLeft) },
};
const EndpointComposition::SemanticTag kGateRightLeafTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kRotate) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kPosition),
      .tag         = to_underlying(Globals::PositionTag::kRight) },
};

// Tilt-and-Turn window: Tilt panel + Turn panel
const EndpointComposition::SemanticTag kWindowTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure), .tag = to_underlying(Globals::ClosureTag::kWindow) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosureWindow), .tag = 0x01 }, // Facade
};
const EndpointComposition::SemanticTag kTiltTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kTilt) },
};

// Drawer + Cabinet Door + Flap
const EndpointComposition::SemanticTag kStorageCabinetTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure),
      .tag         = to_underlying(Globals::ClosureTag::kCabinet) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kNumber),
      .tag         = 4 }, // closure-simulated and closure-no-ps-no-sp use Number 1 and 3
};
const EndpointComposition::SemanticTag kDrawerTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kSliding) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosureCabinet), .tag = 0x01 }, // Drawer
};
const EndpointComposition::SemanticTag kCabinetDoorTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kRotate) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosureCabinet), .tag = 0x00 }, // CabinetDoor
};
// Tilt instead of Rotate, so the flap does not share a 0x45 tag with the cabinet door.
const EndpointComposition::SemanticTag kFlapTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosurePanel),
      .tag         = to_underlying(Globals::ClosurePanelTag::kTilt) },
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosureCabinet), .tag = 0x02 }, // Flap
};

const EndpointComposition::SemanticTag kBarrierTag[] = {
    { .namespaceID = to_underlying(Globals::SemanticTagNamespaceId::kClosure),
      .tag         = to_underlying(Globals::ClosureTag::kBarrier) },
};

ClosurePanel::Config DoorPanel()
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
                                                        ClosureDimension::OverflowEnum::kInside },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kDegree,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 90 }),
                    },
            },
    };
}

ClosurePanel::Config GarageDoorPanel()
{
    return {
        .withAccess = true,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::TranslationParams{ ClosureDimension::TranslationDirectionEnum::kDownward },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kMillimeter,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 2400 }),
                    },
            },
    };
}

ClosurePanel::Config GateLeafPanel(ClosureDimension::RotationAxisEnum axis)
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
                .motion     = ClosurePanel::RotationParams{ axis, ClosureDimension::OverflowEnum::kInside },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kDegree,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 90 }),
                    },
            },
    };
}

ClosurePanel::Config WindowTiltPanel()
{
    return {
        .withAccess = true,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kBottom,
                                                        ClosureDimension::OverflowEnum::kTopInside },
            },
    };
}

ClosurePanel::Config WindowTurnPanel()
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
                                                        ClosureDimension::OverflowEnum::kRightInside },
            },
    };
}

ClosurePanel::Config DrawerPanel()
{
    return {
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::TranslationParams{ ClosureDimension::TranslationDirectionEnum::kBackward },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kMillimeter,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 500 }),
                    },
            },
    };
}

ClosurePanel::Config CabinetDoorPanel()
{
    return {
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kLeft,
                                                        ClosureDimension::OverflowEnum::kLeftOutside },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kDegree,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 90 }),
                    },
            },
    };
}

ClosurePanel::Config FlapPanel()
{
    return {
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kTop,
                                                        ClosureDimension::OverflowEnum::kBottomOutside },
            },
    };
}

ClosurePanel::Config BarrierArmPanel()
{
    return {
        .withAccess = true,
        .positioning =
            ClosurePanel::PositioningParams{
                .resolution = kPanelResolution,
                .stepValue  = kPanelStepValue,
                .motion     = ClosurePanel::RotationParams{ ClosureDimension::RotationAxisEnum::kLeftBarrier,
                                                        ClosureDimension::OverflowEnum::kNoOverflow },
                .unit =
                    ClosurePanel::UnitParams{
                        .unit  = ClosureDimension::ClosureUnitEnum::kDegree,
                        .range = DataModel::MakeNullable(ClosureDimension::Structs::UnitRangeStruct::Type{ .min = 0, .max = 90 }),
                    },
            },
    };
}

SimulatedClosure::Config ThreePanelCabinetClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kCabinetTag),
            .withPositioning = true,
            .motionLatching  = BitFlags<ClosureControl::LatchControlModesBitmap>(
                ClosureControl::LatchControlModesBitmap::kRemoteLatching,
                ClosureControl::LatchControlModesBitmap::kRemoteUnlatching),
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withVentilation            = true,
            .withPedestrian             = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = false, // ACC would make the !ACC test cases (CLCTRL_7_x, CLDIM_5_x/6_x) skip
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                MakeOptional(DataModel::MakeNullable(false)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ LiftPanel(), Span<const EndpointComposition::SemanticTag>(kLiftTag) },
            SimulatedClosure::PanelList{ ModulatingPanel(), Span<const EndpointComposition::SemanticTag>(kSlideTag) },
            SimulatedClosure::PanelList{ RotatingPanel(), Span<const EndpointComposition::SemanticTag>(kRotateTag) },
        },
    };
}

SimulatedClosure::Config ThreePanelCabinetClosureConfigNoPSNoSP()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kCabinetNoPsNoSpTag),
            .withPositioning = false,
            .motionLatching  = BitFlags<ClosureControl::LatchControlModesBitmap>(
                ClosureControl::LatchControlModesBitmap::kRemoteLatching,
                ClosureControl::LatchControlModesBitmap::kRemoteUnlatching),
            .withInstantaneous          = true,
            .withSpeed                  = false,
            .withVentilation            = false,
            .withPedestrian             = false,
            .withCalibration            = false,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = false, // ACC would make the !ACC test cases (CLCTRL_7_x, CLDIM_5_x/6_x) skip
            .withCountdownTime          = false,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(NullOptional,
                MakeOptional(DataModel::MakeNullable(false))))
        },
        .panels = {
            SimulatedClosure::PanelList{ LiftPanel(), Span<const EndpointComposition::SemanticTag>(kLiftTag) },
            SimulatedClosure::PanelList{ ModulatingPanel(), Span<const EndpointComposition::SemanticTag>(kSlideTag) },
            SimulatedClosure::PanelList{ RotatingPanel(), Span<const EndpointComposition::SemanticTag>(kRotateTag) },
        },
    };
}

SimulatedClosure::Config DoorClosureConfig()
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
            .withVentilation            = true,
            .withPedestrian             = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = true,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                MakeOptional(DataModel::MakeNullable(false)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ DoorPanel(), Span<const EndpointComposition::SemanticTag>(kRotateTag) },
        },
    };
}

SimulatedClosure::Config GarageDoorClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags                       = Span<const EndpointComposition::SemanticTag>(kGarageDoorTag),
            .withPositioning            = true,
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withVentilation            = true,
            .withPedestrian             = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = true,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                NullOptional, MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ GarageDoorPanel(), Span<const EndpointComposition::SemanticTag>(kLiftTag) },
        },
    };
}

SimulatedClosure::Config GateClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kGateTag),
            .withPositioning = true,
            .motionLatching  = BitFlags<ClosureControl::LatchControlModesBitmap>(
                ClosureControl::LatchControlModesBitmap::kRemoteLatching,
                ClosureControl::LatchControlModesBitmap::kRemoteUnlatching),
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withPedestrian             = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withAccess                 = true,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                MakeOptional(DataModel::MakeNullable(false)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ GateLeafPanel(ClosureDimension::RotationAxisEnum::kLeft),
                       Span<const EndpointComposition::SemanticTag>(kGateLeftLeafTag) },
            SimulatedClosure::PanelList{ GateLeafPanel(ClosureDimension::RotationAxisEnum::kRight),
                       Span<const EndpointComposition::SemanticTag>(kGateRightLeafTag) },
        },
    };
}

SimulatedClosure::Config WindowClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kWindowTag),
            .withPositioning = true,
            .motionLatching  = BitFlags<ClosureControl::LatchControlModesBitmap>(
                ClosureControl::LatchControlModesBitmap::kRemoteLatching,
                ClosureControl::LatchControlModesBitmap::kRemoteUnlatching),
            .withVentilation            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = true,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                MakeOptional(DataModel::MakeNullable(false)))),
        },
        .panels = {
            SimulatedClosure::PanelList{ WindowTiltPanel(), Span<const EndpointComposition::SemanticTag>(kTiltTag) },
            SimulatedClosure::PanelList{ WindowTurnPanel(), Span<const EndpointComposition::SemanticTag>(kRotateTag) },
        },
    };
}

SimulatedClosure::Config CabinetClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags            = Span<const EndpointComposition::SemanticTag>(kStorageCabinetTag),
            .withPositioning = true,
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withAccess                 = false,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                NullOptional, MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ DrawerPanel(), Span<const EndpointComposition::SemanticTag>(kDrawerTag) },
            SimulatedClosure::PanelList{ CabinetDoorPanel(), Span<const EndpointComposition::SemanticTag>(kCabinetDoorTag) },
            SimulatedClosure::PanelList{ FlapPanel(), Span<const EndpointComposition::SemanticTag>(kFlapTag) },
        },
    };
}

SimulatedClosure::Config BarrierClosureConfig()
{
    return {
        .closure = Closure::Config{
            .tags                       = Span<const EndpointComposition::SemanticTag>(kBarrierTag),
            .withPositioning            = true,
            .withInstantaneous          = false, // mutually exclusive with Speed below
            .withSpeed                  = true,
            .withCalibration            = true,
            .withManuallyOperable       = true,
            .withProtection             = true,
            .withAccess                 = true,
            .withCountdownTime          = true,
            .initialOverallCurrentState = DataModel::MakeNullable(ClosureControl::GenericOverallCurrentState(
                MakeOptional(DataModel::MakeNullable(ClosureControl::CurrentPositionEnum::kFullyClosed)),
                NullOptional, MakeOptional(Globals::ThreeLevelAutoEnum::kAuto))),
        },
        .panels = {
            SimulatedClosure::PanelList{ BarrierArmPanel(), Span<const EndpointComposition::SemanticTag>(kTiltTag) },
        },
    };
}

// Zephyr sizes its device array per creator: keep ALL_DEVICES_ENABLED_DEVICE_COUNT in
// examples/all-devices-app/zephyr/CMakeLists.txt in sync when adding or removing entries.
constexpr ClosureVariant kVariants[] = {
    { "closure-simulated", &ThreePanelCabinetClosureConfig },
    { "closure-no-ps-no-sp-simulated", &ThreePanelCabinetClosureConfigNoPSNoSP },
    { "closure-door", &DoorClosureConfig },
    { "closure-garage-door", &GarageDoorClosureConfig },
    { "closure-gate", &GateClosureConfig },
    { "closure-window", &WindowClosureConfig },
    { "closure-cabinet", &CabinetClosureConfig },
    { "closure-barrier", &BarrierClosureConfig },
};

} // namespace

Span<const ClosureVariant> ClosureVariants()
{
    return Span<const ClosureVariant>(kVariants);
}

} // namespace app
} // namespace chip
