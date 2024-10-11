/*
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * This file contains backwards-compatibility enum name definitions.  This file
 * is meant to be included at the end of cluster-enums.h, after all the normal
 * enum definitions are available.
 */
#pragma once

#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {
namespace Clusters {

namespace OtaSoftwareUpdateProvider {
// https://github.com/project-chip/connectedhomeip/pull/29459 renamed these
// enums to follow the spec names.
using OTAApplyUpdateAction = ApplyUpdateActionEnum;
using OTADownloadProtocol  = DownloadProtocolEnum;
using OTAQueryStatus       = StatusEnum;
} // namespace OtaSoftwareUpdateProvider

namespace MediaPlayback {
using MediaPlaybackStatusEnum = StatusEnum;
}

namespace OtaSoftwareUpdateRequestor {
// https://github.com/project-chip/connectedhomeip/pull/29459 renamed these
// enums to follow the spec names.
using OTAAnnouncementReason = AnnouncementReasonEnum;
using OTAChangeReasonEnum   = ChangeReasonEnum;
using OTAUpdateStateEnum    = UpdateStateEnum;
} // namespace OtaSoftwareUpdateRequestor

namespace ContentLauncher {
// https://github.com/project-chip/connectedhomeip/pull/30176 renamed these
// enums to follow the spec names.
using SupportedStreamingProtocol = SupportedProtocolsBitmap;
using ContentLaunchStatusEnum    = StatusEnum;
} // namespace ContentLauncher

namespace ApplicationLauncher {
// https://github.com/project-chip/connectedhomeip/pull/30134 renamed this
using ApplicationLauncherStatusEnum = StatusEnum;
} // namespace ApplicationLauncher

namespace KeypadInput {
// https://github.com/project-chip/connectedhomeip/pull/30316 renamed this
using KeypadInputStatusEnum = StatusEnum;
// https://github.com/project-chip/connectedhomeip/pull/30497 renamed this
using CecKeyCode = CECKeyCodeEnum;
} // namespace KeypadInput

namespace Channel {
using ChannelStatusEnum = StatusEnum;
} // namespace Channel

namespace TargetNavigator {
// https://github.com/project-chip/connectedhomeip/pull/30322 renamed this
using TargetNavigatorStatusEnum = StatusEnum;
} // namespace TargetNavigator

namespace LevelControl {
// https://github.com/project-chip/connectedhomeip/pull/30417 renamed/updated these
using MoveMode            = MoveModeEnum;
using StepMode            = StepModeEnum;
using LevelControlOptions = OptionsBitmap;
} // namespace LevelControl

namespace ColorControl {
// https://github.com/project-chip/connectedhomeip/pull/33612 renamed this
enum class ColorMode : uint8_t
{
    kCurrentHueAndCurrentSaturation = to_underlying(ColorModeEnum::kCurrentHueAndCurrentSaturation),
    kCurrentXAndCurrentY            = to_underlying(ColorModeEnum::kCurrentXAndCurrentY),
    kColorTemperature               = to_underlying(ColorModeEnum::kColorTemperatureMireds),
    kUnknownEnumValue               = to_underlying(ColorModeEnum::kUnknownEnumValue)
};

enum class HueDirection : uint8_t
{
    ShortestDistance  = to_underlying(DirectionEnum::kShortest),
    LongestDistance   = to_underlying(DirectionEnum::kLongest),
    Up                = to_underlying(DirectionEnum::kUp),
    Down              = to_underlying(DirectionEnum::kDown),
    kUnknownEnumValue = to_underlying(DirectionEnum::kUnknownEnumValue)
};

enum class ColorCapabilities : uint16_t
{
    ColorLoopSupported        = to_underlying(ColorCapabilitiesBitmap::kColorLoop),
    ColorTemperatureSupported = to_underlying(ColorCapabilitiesBitmap::kColorTemperature),
    EnhancedHueSupported      = to_underlying(ColorCapabilitiesBitmap::kEnhancedHue),
    HueSaturationSupported    = to_underlying(ColorCapabilitiesBitmap::kHueSaturation),
    XYAttributesSupported     = to_underlying(ColorCapabilitiesBitmap::kXy)
};

enum class ColorLoopDirection : uint8_t
{
    DecrementHue      = to_underlying(ColorLoopDirectionEnum::kDecrement),
    IncrementHue      = to_underlying(ColorLoopDirectionEnum::kIncrement),
    kUnknownEnumValue = to_underlying(ColorLoopDirectionEnum::kUnknownEnumValue)
};

using EnhancedColorMode    = EnhancedColorModeEnum;
using ColorLoopUpdateFlags = UpdateFlagsBitmap;
using ColorLoopAction      = ColorLoopActionEnum;
using HueMoveMode          = MoveModeEnum;
using HueStepMode          = StepModeEnum;
using SaturationMoveMode   = MoveModeEnum;
using SaturationStepMode   = StepModeEnum;
} // namespace ColorControl

namespace RefrigeratorAlarm {
// https://github.com/project-chip/connectedhomeip/pull/31517 renamed this
using AlarmMap = AlarmBitmap;
} // namespace RefrigeratorAlarm

namespace DishwasherAlarm {
// https://github.com/project-chip/connectedhomeip/pull/31517 renamed this
using AlarmMap = AlarmBitmap;
} // namespace DishwasherAlarm

} // namespace Clusters
} // namespace app
} // namespace chip
