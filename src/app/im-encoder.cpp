/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "im-encoder.h"

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/util/basic-types.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace cluster {

namespace BarrierControl {

/*----------------------------------------------------------------------------*\
| Cluster BarrierControl                                              | 0x0103 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * BarrierControlGoToPercent                                         |   0x00 |
| * BarrierControlStop                                                |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * BarrierMovingState                                                | 0x0001 |
| * BarrierSafetyStatus                                               | 0x0002 |
| * BarrierCapabilities                                               | 0x0003 |
| * BarrierPosition                                                   | 0x000A |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeBarrierControlGoToPercentCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, const uint8_t percentOpen)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, BarrierControl::kClusterId,
                                         BarrierControl::kBarrierControlGoToPercentCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // percentOpen: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), percentOpen));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeBarrierControlStopCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, BarrierControl::kClusterId,
                                         BarrierControl::kBarrierControlStopCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(BarrierControl::kClusterId, BarrierControl::kBarrierControlGoToPercentCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleBarrierControlGoToPercentCommandReceived);
    ime->RegisterClusterCommandHandler(BarrierControl::kClusterId, BarrierControl::kBarrierControlStopCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleBarrierControlStopCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(BarrierControl::kClusterId, BarrierControl::kBarrierControlGoToPercentCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(BarrierControl::kClusterId, BarrierControl::kBarrierControlStopCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace BarrierControl

namespace Basic {

/*----------------------------------------------------------------------------*\
| Cluster Basic                                                       | 0x0000 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * ResetToFactoryDefaults                                            |   0x00 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ZclVersion                                                        | 0x0000 |
| * PowerSource                                                       | 0x0007 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeResetToFactoryDefaultsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Basic::kClusterId, Basic::kResetToFactoryDefaultsCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(Basic::kClusterId, Basic::kResetToFactoryDefaultsCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleResetToFactoryDefaultsCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(Basic::kClusterId, Basic::kResetToFactoryDefaultsCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace Basic

namespace ColorControl {

/*----------------------------------------------------------------------------*\
| Cluster ColorControl                                                | 0x0300 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * MoveColor                                                         |   0x08 |
| * MoveColorTemperature                                              |   0x4B |
| * MoveHue                                                           |   0x01 |
| * MoveSaturation                                                    |   0x04 |
| * MoveToColor                                                       |   0x07 |
| * MoveToColorTemperature                                            |   0x0A |
| * MoveToHue                                                         |   0x00 |
| * MoveToHueAndSaturation                                            |   0x06 |
| * MoveToSaturation                                                  |   0x03 |
| * StepColor                                                         |   0x09 |
| * StepColorTemperature                                              |   0x4C |
| * StepHue                                                           |   0x02 |
| * StepSaturation                                                    |   0x05 |
| * StopMoveStep                                                      |   0x47 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * CurrentHue                                                        | 0x0000 |
| * CurrentSaturation                                                 | 0x0001 |
| * RemainingTime                                                     | 0x0002 |
| * CurrentX                                                          | 0x0003 |
| * CurrentY                                                          | 0x0004 |
| * DriftCompensation                                                 | 0x0005 |
| * CompensationText                                                  | 0x0006 |
| * ColorTemperature                                                  | 0x0007 |
| * ColorMode                                                         | 0x0008 |
| * ColorControlOptions                                               | 0x000F |
| * NumberOfPrimaries                                                 | 0x0010 |
| * Primary1X                                                         | 0x0011 |
| * Primary1Y                                                         | 0x0012 |
| * Primary1Intensity                                                 | 0x0013 |
| * Primary2X                                                         | 0x0015 |
| * Primary2Y                                                         | 0x0016 |
| * Primary2Intensity                                                 | 0x0017 |
| * Primary3X                                                         | 0x0019 |
| * Primary3Y                                                         | 0x001A |
| * Primary3Intensity                                                 | 0x001B |
| * Primary4X                                                         | 0x0020 |
| * Primary4Y                                                         | 0x0021 |
| * Primary4Intensity                                                 | 0x0022 |
| * Primary5X                                                         | 0x0024 |
| * Primary5Y                                                         | 0x0025 |
| * Primary5Intensity                                                 | 0x0026 |
| * Primary6X                                                         | 0x0028 |
| * Primary6Y                                                         | 0x0029 |
| * Primary6Intensity                                                 | 0x002A |
| * WhitePointX                                                       | 0x0030 |
| * WhitePointY                                                       | 0x0031 |
| * ColorPointRX                                                      | 0x0032 |
| * ColorPointRY                                                      | 0x0033 |
| * ColorPointRIntensity                                              | 0x0034 |
| * ColorPointGX                                                      | 0x0036 |
| * ColorPointGY                                                      | 0x0037 |
| * ColorPointGIntensity                                              | 0x0038 |
| * ColorPointBX                                                      | 0x003A |
| * ColorPointBY                                                      | 0x003B |
| * ColorPointBIntensity                                              | 0x003C |
| * EnhancedCurrentHue                                                | 0x4000 |
| * EnhancedColorMode                                                 | 0x4001 |
| * ColorLoopActive                                                   | 0x4002 |
| * ColorLoopDirection                                                | 0x4003 |
| * ColorLoopTime                                                     | 0x4004 |
| * ColorCapabilities                                                 | 0x400A |
| * ColorTempPhysicalMin                                              | 0x400B |
| * ColorTempPhysicalMax                                              | 0x400C |
| * CoupleColorTempToLevelMinMireds                                   | 0x400D |
| * StartUpColorTemperatureMireds                                     | 0x4010 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeMoveColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const int16_t rateX, const int16_t rateY, const uint8_t optionsMask,
                                  const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kMoveColorCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // rateX: int16s
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rateX));
    // rateY: int16s
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rateY));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t moveMode, const uint16_t rate,
                                             const uint16_t colorTemperatureMinimum, const uint16_t colorTemperatureMaximum,
                                             const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kMoveColorTemperatureCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // moveMode: hueMoveMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), moveMode));
    // rate: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rate));
    // colorTemperatureMinimum: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorTemperatureMinimum));
    // colorTemperatureMaximum: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorTemperatureMaximum));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint8_t moveMode, const uint8_t rate, const uint8_t optionsMask,
                                const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kMoveHueCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // moveMode: hueMoveMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), moveMode));
    // rate: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rate));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                       const uint8_t moveMode, const uint8_t rate, const uint8_t optionsMask,
                                       const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kMoveSaturationCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // moveMode: saturationMoveMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), moveMode));
    // rate: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rate));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t colorX, const uint16_t colorY, const uint16_t transitionTime,
                                    const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kMoveToColorCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // colorX: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorX));
    // colorY: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorY));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId, const uint16_t colorTemperature,
                                               const uint16_t transitionTime, const uint8_t optionsMask,
                                               const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kMoveToColorTemperatureCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // colorTemperature: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorTemperature));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint8_t hue, const uint8_t direction, const uint16_t transitionTime,
                                  const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kMoveToHueCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // hue: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), hue));
    // direction: hueDirection
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), direction));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToHueAndSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId, const uint8_t hue, const uint8_t saturation,
                                               const uint16_t transitionTime, const uint8_t optionsMask,
                                               const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kMoveToHueAndSaturationCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // hue: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), hue));
    // saturation: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), saturation));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                         const uint8_t saturation, const uint16_t transitionTime, const uint8_t optionsMask,
                                         const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kMoveToSaturationCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // saturation: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), saturation));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const int16_t stepX, const int16_t stepY, const uint16_t transitionTime,
                                  const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kStepColorCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepX: int16s
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepX));
    // stepY: int16s
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepY));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t stepMode, const uint16_t stepSize,
                                             const uint16_t transitionTime, const uint16_t colorTemperatureMinimum,
                                             const uint16_t colorTemperatureMaximum, const uint8_t optionsMask,
                                             const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kStepColorTemperatureCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepMode: hueStepMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepMode));
    // stepSize: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepSize));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // colorTemperatureMinimum: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorTemperatureMinimum));
    // colorTemperatureMaximum: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), colorTemperatureMaximum));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint8_t stepMode, const uint8_t stepSize, const uint8_t transitionTime,
                                const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kStepHueCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepMode: hueStepMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepMode));
    // stepSize: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepSize));
    // transitionTime: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                       const uint8_t stepMode, const uint8_t stepSize, const uint8_t transitionTime,
                                       const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId,
                                         ColorControl::kStepSaturationCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepMode: saturationStepMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepMode));
    // stepSize: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepSize));
    // transitionTime: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStopMoveStepCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                     const uint8_t optionsMask, const uint8_t optionsOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, ColorControl::kClusterId, ColorControl::kStopMoveStepCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // optionsMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsMask));
    // optionsOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionsOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveColorCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveColorCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveColorTemperatureCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveColorTemperatureCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveHueCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveHueCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveSaturationCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveSaturationCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToColorCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToColorCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToColorTemperatureCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToColorTemperatureCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToHueCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToHueCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToHueAndSaturationCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToHueAndSaturationCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToSaturationCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToSaturationCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepColorCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepColorCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepColorTemperatureCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepColorTemperatureCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepHueCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepHueCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepSaturationCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepSaturationCommandReceived);
    ime->RegisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStopMoveStepCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStopMoveStepCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveColorCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveColorTemperatureCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveHueCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveSaturationCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToColorCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToColorTemperatureCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToHueCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToHueAndSaturationCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kMoveToSaturationCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepColorCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepColorTemperatureCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepHueCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStepSaturationCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(ColorControl::kClusterId, ColorControl::kStopMoveStepCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace ColorControl

namespace DoorLock {

/*----------------------------------------------------------------------------*\
| Cluster DoorLock                                                    | 0x0101 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * ClearAllPins                                                      |   0x08 |
| * ClearAllRfids                                                     |   0x19 |
| * ClearHolidaySchedule                                              |   0x13 |
| * ClearPin                                                          |   0x07 |
| * ClearRfid                                                         |   0x18 |
| * ClearWeekdaySchedule                                              |   0x0D |
| * ClearYeardaySchedule                                              |   0x10 |
| * GetHolidaySchedule                                                |   0x12 |
| * GetLogRecord                                                      |   0x04 |
| * GetPin                                                            |   0x06 |
| * GetRfid                                                           |   0x17 |
| * GetUserType                                                       |   0x15 |
| * GetWeekdaySchedule                                                |   0x0C |
| * GetYeardaySchedule                                                |   0x0F |
| * LockDoor                                                          |   0x00 |
| * SetHolidaySchedule                                                |   0x11 |
| * SetPin                                                            |   0x05 |
| * SetRfid                                                           |   0x16 |
| * SetUserType                                                       |   0x14 |
| * SetWeekdaySchedule                                                |   0x0B |
| * SetYeardaySchedule                                                |   0x0E |
| * UnlockDoor                                                        |   0x01 |
| * UnlockWithTimeout                                                 |   0x03 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * LockState                                                         | 0x0000 |
| * LockType                                                          | 0x0001 |
| * ActuatorEnabled                                                   | 0x0002 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeClearAllPinsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearAllPinsCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearAllRfidsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearAllRfidsCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearHolidayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearPinCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearRfidCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearWeekdayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeClearYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kClearYeardayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetHolidayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetLogRecordCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                     const uint16_t logIndex)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetLogRecordCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // logIndex: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), logIndex));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                               const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetPinCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetRfidCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetUserTypeCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetUserTypeCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetWeekdayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kGetYeardayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeLockDoorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const char * pin)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kLockDoorCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // pin: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), pin));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint32_t localStartTime,
                                           const uint32_t localEndTime, const uint8_t operatingModeDuringHoliday)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetHolidayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // localStartTime: int32u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), localStartTime));
    // localEndTime: int32u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), localEndTime));
    // operatingModeDuringHoliday: enum8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), operatingModeDuringHoliday));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                               const uint16_t userId, const uint8_t userStatus, const uint8_t userType, const char * pin)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetPinCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));
    // userStatus: doorLockUserStatus
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userStatus));
    // userType: doorLockUserType
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userType));
    // pin: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), pin));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint16_t userId, const uint8_t userStatus, const uint8_t userType, const char * id)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetRfidCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));
    // userStatus: doorLockUserStatus
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userStatus));
    // userType: doorLockUserType
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userType));
    // id: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), id));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetUserTypeCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t userId, const uint8_t userType)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetUserTypeCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));
    // userType: doorLockUserType
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userType));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId,
                                           const uint8_t daysMask, const uint8_t startHour, const uint8_t startMinute,
                                           const uint8_t endHour, const uint8_t endMinute)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetWeekdayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));
    // daysMask: doorLockDayOfWeek
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), daysMask));
    // startHour: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), startHour));
    // startMinute: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), startMinute));
    // endHour: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), endHour));
    // endMinute: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), endMinute));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeSetYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId,
                                           const uint32_t localStartTime, const uint32_t localEndTime)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kSetYeardayScheduleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // scheduleId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), scheduleId));
    // userId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), userId));
    // localStartTime: int32u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), localStartTime));
    // localEndTime: int32u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), localEndTime));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeUnlockDoorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                   const char * pin)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kUnlockDoorCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // pin: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), pin));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeUnlockWithTimeoutCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                          const uint16_t timeoutInSeconds, const char * pin)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, DoorLock::kClusterId, DoorLock::kUnlockWithTimeoutCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // timeoutInSeconds: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), timeoutInSeconds));
    // pin: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), pin));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearAllPinsCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearAllPinsCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearAllRfidsCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearAllRfidsCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearHolidayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearHolidayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearPinCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearPinCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearRfidCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearRfidCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearWeekdayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearWeekdayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearYeardayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleClearYeardayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetHolidayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetHolidayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetLogRecordCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetLogRecordCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetPinCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleGetPinCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetRfidCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleGetRfidCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetUserTypeCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetUserTypeCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetWeekdayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetWeekdayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetYeardayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetYeardayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kLockDoorCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleLockDoorCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetHolidayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleSetHolidayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetPinCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleSetPinCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetRfidCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleSetRfidCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetUserTypeCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleSetUserTypeCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetWeekdayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleSetWeekdayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetYeardayScheduleCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleSetYeardayScheduleCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kUnlockDoorCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleUnlockDoorCommandReceived);
    ime->RegisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kUnlockWithTimeoutCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleUnlockWithTimeoutCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearAllPinsCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearAllRfidsCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearHolidayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearPinCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearRfidCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearWeekdayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kClearYeardayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetHolidayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetLogRecordCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetPinCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetRfidCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetUserTypeCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetWeekdayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kGetYeardayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kLockDoorCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetHolidayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetPinCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetRfidCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetUserTypeCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetWeekdayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kSetYeardayScheduleCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kUnlockDoorCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(DoorLock::kClusterId, DoorLock::kUnlockWithTimeoutCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace DoorLock

namespace Groups {

/*----------------------------------------------------------------------------*\
| Cluster Groups                                                      | 0x0004 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * AddGroup                                                          |   0x00 |
| * AddGroupIfIdentifying                                             |   0x05 |
| * GetGroupMembership                                                |   0x02 |
| * RemoveAllGroups                                                   |   0x04 |
| * RemoveGroup                                                       |   0x03 |
| * ViewGroup                                                         |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * NameSupport                                                       | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeAddGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t groupId, const char * groupName)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kAddGroupCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // groupName: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), groupName));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeAddGroupIfIdentifyingCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                              chip::GroupId ZCLgroupId, const uint16_t groupId, const char * groupName)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kAddGroupIfIdentifyingCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // groupName: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), groupName));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetGroupMembershipCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t groupCount, const uint16_t groupList)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kGetGroupMembershipCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupCount: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupCount));
    // groupList: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupList));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeRemoveAllGroupsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kRemoveAllGroupsCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeRemoveGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kRemoveGroupCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeViewGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t groupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Groups::kClusterId, Groups::kViewGroupCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kAddGroupCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleAddGroupCommandReceived);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kAddGroupIfIdentifyingCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleAddGroupIfIdentifyingCommandReceived);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kGetGroupMembershipCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetGroupMembershipCommandReceived);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kRemoveAllGroupsCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleRemoveAllGroupsCommandReceived);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kRemoveGroupCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleRemoveGroupCommandReceived);
    ime->RegisterClusterCommandHandler(Groups::kClusterId, Groups::kViewGroupCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleViewGroupCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kAddGroupCommandId, Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kAddGroupIfIdentifyingCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kGetGroupMembershipCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kRemoveAllGroupsCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kRemoveGroupCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Groups::kClusterId, Groups::kViewGroupCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace Groups

namespace IasZone {

/*----------------------------------------------------------------------------*\
| Cluster IasZone                                                     | 0x0500 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ZoneState                                                         | 0x0000 |
| * ZoneType                                                          | 0x0001 |
| * ZoneStatus                                                        | 0x0002 |
| * IasCieAddress                                                     | 0x0010 |
| * ZoneId                                                            | 0x0011 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

} // namespace IasZone

namespace Identify {

/*----------------------------------------------------------------------------*\
| Cluster Identify                                                    | 0x0003 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Identify                                                          |   0x00 |
| * IdentifyQuery                                                     |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * IdentifyTime                                                      | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeIdentifyCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t identifyTime)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Identify::kClusterId, Identify::kIdentifyCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // identifyTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), identifyTime));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeIdentifyQueryCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Identify::kClusterId, Identify::kIdentifyQueryCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(Identify::kClusterId, Identify::kIdentifyCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleIdentifyCommandReceived);
    ime->RegisterClusterCommandHandler(Identify::kClusterId, Identify::kIdentifyQueryCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleIdentifyQueryCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(Identify::kClusterId, Identify::kIdentifyCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Identify::kClusterId, Identify::kIdentifyQueryCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace Identify

namespace LevelControl {

/*----------------------------------------------------------------------------*\
| Cluster LevelControl                                                | 0x0008 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Move                                                              |   0x01 |
| * MoveToLevel                                                       |   0x00 |
| * MoveToLevelWithOnOff                                              |   0x04 |
| * MoveWithOnOff                                                     |   0x05 |
| * Step                                                              |   0x02 |
| * StepWithOnOff                                                     |   0x06 |
| * Stop                                                              |   0x03 |
| * StopWithOnOff                                                     |   0x07 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * CurrentLevel                                                      | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeMoveCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t moveMode, const uint8_t rate, const uint8_t optionMask, const uint8_t optionOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kMoveCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // moveMode: moveMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), moveMode));
    // rate: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rate));
    // optionMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionMask));
    // optionOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToLevelCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint8_t level, const uint16_t transitionTime, const uint8_t optionMask,
                                    const uint8_t optionOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kMoveToLevelCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // level: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), level));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionMask));
    // optionOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveToLevelWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t level, const uint16_t transitionTime)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId,
                                         LevelControl::kMoveToLevelWithOnOffCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // level: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), level));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeMoveWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                      const uint8_t moveMode, const uint8_t rate)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kMoveWithOnOffCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // moveMode: moveMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), moveMode));
    // rate: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), rate));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t stepMode, const uint8_t stepSize, const uint16_t transitionTime,
                             const uint8_t optionMask, const uint8_t optionOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kStepCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepMode: stepMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepMode));
    // stepSize: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepSize));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // optionMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionMask));
    // optionOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStepWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                      const uint8_t stepMode, const uint8_t stepSize, const uint16_t transitionTime)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kStepWithOnOffCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // stepMode: stepMode
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepMode));
    // stepSize: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), stepSize));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStopCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t optionMask, const uint8_t optionOverride)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kStopCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // optionMask: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionMask));
    // optionOverride: bitmap8
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), optionOverride));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStopWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, LevelControl::kClusterId, LevelControl::kStopWithOnOffCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveToLevelCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToLevelCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveToLevelWithOnOffCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveToLevelWithOnOffCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveWithOnOffCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleMoveWithOnOffCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStepCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStepWithOnOffCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStepWithOnOffCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStopCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStopCommandReceived);
    ime->RegisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStopWithOnOffCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleStopWithOnOffCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveToLevelCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveToLevelWithOnOffCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kMoveWithOnOffCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStepCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStepWithOnOffCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStopCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(LevelControl::kClusterId, LevelControl::kStopWithOnOffCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace LevelControl

namespace OnOff {

/*----------------------------------------------------------------------------*\
| Cluster OnOff                                                       | 0x0006 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Off                                                               |   0x00 |
| * On                                                                |   0x01 |
| * Toggle                                                            |   0x02 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * OnOff                                                             | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, OnOff::kClusterId, OnOff::kOffCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeOnCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, OnOff::kClusterId, OnOff::kOnCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeToggleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, OnOff::kClusterId, OnOff::kToggleCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    // Command takes no arguments.

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(OnOff::kClusterId, OnOff::kOffCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleOffCommandReceived);
    ime->RegisterClusterCommandHandler(OnOff::kClusterId, OnOff::kOnCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleOnCommandReceived);
    ime->RegisterClusterCommandHandler(OnOff::kClusterId, OnOff::kToggleCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleToggleCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(OnOff::kClusterId, OnOff::kOffCommandId, Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(OnOff::kClusterId, OnOff::kOnCommandId, Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(OnOff::kClusterId, OnOff::kToggleCommandId, Command::CommandRoleId::kCommandHandlerId);
}

} // namespace OnOff

namespace Scenes {

/*----------------------------------------------------------------------------*\
| Cluster Scenes                                                      | 0x0005 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * AddScene                                                          |   0x00 |
| * GetSceneMembership                                                |   0x06 |
| * RecallScene                                                       |   0x05 |
| * RemoveAllScenes                                                   |   0x03 |
| * RemoveScene                                                       |   0x02 |
| * StoreScene                                                        |   0x04 |
| * ViewScene                                                         |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * SceneCount                                                        | 0x0000 |
| * CurrentScene                                                      | 0x0001 |
| * CurrentGroup                                                      | 0x0002 |
| * SceneValid                                                        | 0x0003 |
| * NameSupport                                                       | 0x0004 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

CHIP_ERROR EncodeAddSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t groupId, const uint8_t sceneId, const uint16_t transitionTime,
                                 const char * sceneName, const chip::ClusterId clusterId, const uint8_t length, const uint8_t value)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kAddSceneCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // sceneId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), sceneId));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));
    // sceneName: char_string
    ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(argSeqNumber++), sceneName));
    // clusterId: cluster_id
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), clusterId));
    // length: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), length));
    // value: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), value));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeGetSceneMembershipCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint16_t groupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kGetSceneMembershipCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeRecallSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId, const uint8_t sceneId, const uint16_t transitionTime)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kRecallSceneCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // sceneId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), sceneId));
    // transitionTime: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), transitionTime));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeRemoveAllScenesCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                        const uint16_t groupId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kRemoveAllScenesCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeRemoveSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId, const uint8_t sceneId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kRemoveSceneCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // sceneId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), sceneId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeStoreSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                   const uint16_t groupId, const uint8_t sceneId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kStoreSceneCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // sceneId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), sceneId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeViewSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t groupId, const uint8_t sceneId)
{
    Command::CommandParams cmdParams = { ZCLendpointId, ZCLgroupId, Scenes::kClusterId, Scenes::kViewSceneCommandId,
                                         (chip::app::Command::kCommandPathFlag_EndpointIdValid) };
    TLV::TLVWriter writer            = ZCLcommand->CreateCommandDataElementTLVWriter();

    TLV::TLVType dummyType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, dummyType));

    uint8_t argSeqNumber = 0;
    // groupId: int16u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), groupId));
    // sceneId: int8u
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(argSeqNumber++), sceneId));

    ReturnErrorOnFailure(writer.EndContainer(dummyType));
    ReturnErrorOnFailure(writer.Finalize());
    ReturnErrorOnFailure(ZCLcommand->AddCommand(cmdParams));

    return CHIP_NO_ERROR;
}

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kAddSceneCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleAddSceneCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kGetSceneMembershipCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleGetSceneMembershipCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRecallSceneCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleRecallSceneCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRemoveAllScenesCommandId,
                                       Command::CommandRoleId::kCommandHandlerId, HandleRemoveAllScenesCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRemoveSceneCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleRemoveSceneCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kStoreSceneCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleStoreSceneCommandReceived);
    ime->RegisterClusterCommandHandler(Scenes::kClusterId, Scenes::kViewSceneCommandId, Command::CommandRoleId::kCommandHandlerId,
                                       HandleViewSceneCommandReceived);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kAddSceneCommandId, Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kGetSceneMembershipCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRecallSceneCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRemoveAllScenesCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kRemoveSceneCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kStoreSceneCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
    ime->DeregisterClusterCommandHandler(Scenes::kClusterId, Scenes::kViewSceneCommandId,
                                         Command::CommandRoleId::kCommandHandlerId);
}

} // namespace Scenes

namespace TemperatureMeasurement {

/*----------------------------------------------------------------------------*\
| Cluster TemperatureMeasurement                                      | 0x0402 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * MeasuredValue                                                     | 0x0000 |
| * MinMeasuredValue                                                  | 0x0001 |
| * MaxMeasuredValue                                                  | 0x0002 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

} // namespace TemperatureMeasurement

void InitClusters(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    BarrierControl::InitCluster(ime);
    Basic::InitCluster(ime);
    ColorControl::InitCluster(ime);
    DoorLock::InitCluster(ime);
    Groups::InitCluster(ime);
    IasZone::InitCluster(ime);
    Identify::InitCluster(ime);
    LevelControl::InitCluster(ime);
    OnOff::InitCluster(ime);
    Scenes::InitCluster(ime);
    TemperatureMeasurement::InitCluster(ime);
}

void ShutdownClusters(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    BarrierControl::ShutdownCluster(ime);
    Basic::ShutdownCluster(ime);
    ColorControl::ShutdownCluster(ime);
    DoorLock::ShutdownCluster(ime);
    Groups::ShutdownCluster(ime);
    IasZone::ShutdownCluster(ime);
    Identify::ShutdownCluster(ime);
    LevelControl::ShutdownCluster(ime);
    OnOff::ShutdownCluster(ime);
    Scenes::ShutdownCluster(ime);
    TemperatureMeasurement::ShutdownCluster(ime);
}

} // namespace cluster
} // namespace app
} // namespace chip
