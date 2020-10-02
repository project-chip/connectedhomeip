/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef __CHIPTOOL_COLORCONTROL_COMMANDS_H__
#define __CHIPTOOL_COLORCONTROL_COMMANDS_H__

#include "../../common/ModelCommand.h"

class MoveToHue : public ModelCommand
{
public:
    MoveToHue(const uint16_t clusterId) : ModelCommand("move-to-hue", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t hue             = 30;
        uint8_t direction       = 2;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveToHueCommand(buffer->Start(), bufferSize, endPointId, hue, direction, transitionTime, optionsMask,
                                      optionsOverride);
    }
};

class MoveHue : public ModelCommand
{
public:
    MoveHue(const uint16_t clusterId) : ModelCommand("move-hue", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t moveMode        = 2;
        uint8_t rate            = 127;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveHueCommand(buffer->Start(), bufferSize, endPointId, moveMode, rate, optionsMask, optionsOverride);
    }
};

class StepHue : public ModelCommand
{
public:
    StepHue(const uint16_t clusterId) : ModelCommand("step-hue", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t stepMode        = 2;
        uint8_t stepSize        = 127;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeStepHueCommand(buffer->Start(), bufferSize, endPointId, stepMode, stepSize, transitionTime, optionsMask,
                                    optionsOverride);
    }
};

class MoveToSaturation : public ModelCommand
{
public:
    MoveToSaturation(const uint16_t clusterId) : ModelCommand("move-to-saturation", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t saturation      = 30;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveToSaturationCommand(buffer->Start(), bufferSize, endPointId, saturation, transitionTime, optionsMask,
                                             optionsOverride);
    }
};

class MoveSaturation : public ModelCommand
{
public:
    MoveSaturation(const uint16_t clusterId) : ModelCommand("move-saturation", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t moveMode        = 30;
        uint8_t rate            = 100;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveSaturationCommand(buffer->Start(), bufferSize, endPointId, moveMode, rate, optionsMask, optionsOverride);
    }
};

class StepSaturation : public ModelCommand
{
public:
    StepSaturation(const uint16_t clusterId) : ModelCommand("step-saturation", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t stepMode        = 30;
        uint8_t stepSize        = 100;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeStepSaturationCommand(buffer->Start(), bufferSize, endPointId, stepMode, stepSize, transitionTime, optionsMask,
                                           optionsOverride);
    }
};

class MoveToHueSaturation : public ModelCommand
{
public:
    MoveToHueSaturation(const uint16_t clusterId) : ModelCommand("move-to-hue-saturation", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t hue             = 30;
        uint8_t saturation      = 100;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveToHueSaturationCommand(buffer->Start(), bufferSize, endPointId, hue, saturation, transitionTime,
                                                optionsMask, optionsOverride);
    }
};

class MoveToColor : public ModelCommand
{
public:
    MoveToColor(const uint16_t clusterId) : ModelCommand("move-to-color", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint16_t colorX         = 30;
        uint16_t colorY         = 30;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveToColorCommand(buffer->Start(), bufferSize, endPointId, colorX, colorY, transitionTime, optionsMask,
                                        optionsOverride);
    }
};

class MoveColor : public ModelCommand
{
public:
    MoveColor(const uint16_t clusterId) : ModelCommand("move-color", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint16_t rateX          = 30;
        uint16_t rateY          = 30;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeMoveColorCommand(buffer->Start(), bufferSize, endPointId, rateX, rateY, optionsMask, optionsOverride);
    }
};

class StepColor : public ModelCommand
{
public:
    StepColor(const uint16_t clusterId) : ModelCommand("step-color", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint16_t stepX          = 30;
        uint16_t stepY          = 30;
        uint16_t transitionTime = 10;
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeStepColorCommand(buffer->Start(), bufferSize, endPointId, stepX, stepY, transitionTime, optionsMask,
                                      optionsOverride);
    }
};

class MoveToColorTemperature : public ModelCommand
{
public:
    MoveToColorTemperature(const uint16_t clusterId) : ModelCommand("move-to-color-temperature", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint16_t colorTemperature = 50000;
        uint16_t transitionTime   = 50;
        uint8_t optionsMask       = 0;
        uint8_t optionsOverride   = 0;
        return encodeMoveToColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, colorTemperature, transitionTime,
                                                   optionsMask, optionsOverride);
    }
};

class MoveColorTemperature : public ModelCommand
{
public:
    MoveColorTemperature(const uint16_t clusterId) : ModelCommand("move-color-temperature", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t moveMode             = 4;
        uint16_t rate                = 100;
        uint16_t colorTemperatureMin = 1000;
        uint16_t colorTemperatureMax = 5000;
        uint8_t optionsMask          = 0;
        uint8_t optionsOverride      = 0;
        return encodeMoveColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, moveMode, rate, colorTemperatureMin,
                                                 colorTemperatureMax, optionsMask, optionsOverride);
    }
};

class StepColorTemperature : public ModelCommand
{
public:
    StepColorTemperature(const uint16_t clusterId) : ModelCommand("step-color-temperature", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t stepMode             = 4;
        uint16_t stepSize            = 100;
        uint16_t transitionTime      = 10;
        uint16_t colorTemperatureMin = 1000;
        uint16_t colorTemperatureMax = 5000;
        uint8_t optionsMask          = 0;
        uint8_t optionsOverride      = 0;
        return encodeStepColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, stepMode, stepSize, colorTemperatureMin,
                                                 colorTemperatureMax, transitionTime, optionsMask, optionsOverride);
    }
};

class StopMoveStep : public ModelCommand
{
public:
    StopMoveStep(const uint16_t clusterId) : ModelCommand("stop-move-step", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        uint8_t optionsMask     = 0;
        uint8_t optionsOverride = 0;
        return encodeStopMoveStepCommand(buffer->Start(), bufferSize, endPointId, optionsMask, optionsOverride);
    }
};

void registerClusterColorControl(Commands & commands)
{
    const char * clusterName = "ColorControl";
    const uint16_t clusterId = 0x0300;

    commands_list clusterCommands = {
        make_unique<MoveToHue>(clusterId),
        make_unique<MoveHue>(clusterId),
        make_unique<StepHue>(clusterId),
        make_unique<MoveToSaturation>(clusterId),
        make_unique<MoveSaturation>(clusterId),
        make_unique<StepSaturation>(clusterId),
        make_unique<MoveToHueSaturation>(clusterId),
        make_unique<MoveToColor>(clusterId),
        make_unique<MoveColor>(clusterId),
        make_unique<StepColor>(clusterId),
        make_unique<MoveToColorTemperature>(clusterId),
        make_unique<MoveColorTemperature>(clusterId),
        make_unique<StepColorTemperature>(clusterId),
        make_unique<StopMoveStep>(clusterId),
    };

    commands.Register(clusterName, clusterCommands);
}

#endif // __CHIPTOOL_COLORCONTROL_COMMANDS_H__
