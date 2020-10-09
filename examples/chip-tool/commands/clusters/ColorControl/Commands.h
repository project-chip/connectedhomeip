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
    MoveToHue(const uint16_t clusterId) : ModelCommand("move-to-hue", clusterId)
    {
        AddArgument("hue", 0, UINT8_MAX, &mHue);
        AddArgument("direction", 0, UINT16_MAX, &mDirection);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToHueCommand(buffer->Start(), bufferSize, endPointId, mHue, mDirection, mTransitionTime, mOptionsMask,
                                      mOptionsOverride);
    }

private:
    uint8_t mHue;
    uint16_t mDirection;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveHue : public ModelCommand
{
public:
    MoveHue(const uint16_t clusterId) : ModelCommand("move-hue", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mMoveMode);
        AddArgument("rate", 0, UINT8_MAX, &mRate);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveHueCommand(buffer->Start(), bufferSize, endPointId, mMoveMode, mRate, mOptionsMask, mOptionsOverride);
    }

private:
    uint8_t mMoveMode;
    uint8_t mRate;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class StepHue : public ModelCommand
{
public:
    StepHue(const uint16_t clusterId) : ModelCommand("step-hue", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mStepMode);
        AddArgument("size", 0, UINT8_MAX, &mStepSize);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStepHueCommand(buffer->Start(), bufferSize, endPointId, mStepMode, mStepSize, mTransitionTime, mOptionsMask,
                                    mOptionsOverride);
    }

private:
    uint8_t mStepMode;
    uint8_t mStepSize;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveToSaturation : public ModelCommand
{
public:
    MoveToSaturation(const uint16_t clusterId) : ModelCommand("move-to-saturation", clusterId)
    {
        AddArgument("saturation", 0, UINT8_MAX, &mSaturation);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToSaturationCommand(buffer->Start(), bufferSize, endPointId, mSaturation, mTransitionTime, mOptionsMask,
                                             mOptionsOverride);
    }

private:
    uint8_t mSaturation;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveSaturation : public ModelCommand
{
public:
    MoveSaturation(const uint16_t clusterId) : ModelCommand("move-saturation", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mMoveMode);
        AddArgument("rate", 0, UINT8_MAX, &mRate);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveSaturationCommand(buffer->Start(), bufferSize, endPointId, mMoveMode, mRate, mOptionsMask,
                                           mOptionsOverride);
    }

private:
    uint8_t mMoveMode;
    uint8_t mRate;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class StepSaturation : public ModelCommand
{
public:
    StepSaturation(const uint16_t clusterId) : ModelCommand("step-saturation", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mStepMode);
        AddArgument("size", 0, UINT8_MAX, &mStepSize);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStepSaturationCommand(buffer->Start(), bufferSize, endPointId, mStepMode, mStepSize, mTransitionTime,
                                           mOptionsMask, mOptionsOverride);
    }

private:
    uint8_t mStepMode;
    uint8_t mStepSize;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveToHueSaturation : public ModelCommand
{
public:
    MoveToHueSaturation(const uint16_t clusterId) : ModelCommand("move-to-hue-saturation", clusterId)
    {
        AddArgument("hue", 0, UINT8_MAX, &mHue);
        AddArgument("saturation", 0, UINT8_MAX, &mSaturation);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToHueSaturationCommand(buffer->Start(), bufferSize, endPointId, mHue, mSaturation, mTransitionTime,
                                                mOptionsMask, mOptionsOverride);
    }

private:
    uint8_t mHue;
    uint8_t mSaturation;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveToColor : public ModelCommand
{
public:
    MoveToColor(const uint16_t clusterId) : ModelCommand("move-to-color", clusterId)
    {
        AddArgument("x", 0, UINT16_MAX, &mColorX);
        AddArgument("y", 0, UINT16_MAX, &mColorY);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToColorCommand(buffer->Start(), bufferSize, endPointId, mColorX, mColorY, mTransitionTime, mOptionsMask,
                                        mOptionsOverride);
    }

private:
    uint16_t mColorX;
    uint16_t mColorY;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveColor : public ModelCommand
{
public:
    MoveColor(const uint16_t clusterId) : ModelCommand("move-color", clusterId)
    {
        AddArgument("x", 0, UINT16_MAX, &mRateX);
        AddArgument("y", 0, UINT16_MAX, &mRateY);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveColorCommand(buffer->Start(), bufferSize, endPointId, mRateX, mRateY, mOptionsMask, mOptionsOverride);
    }

private:
    uint16_t mRateX;
    uint16_t mRateY;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class StepColor : public ModelCommand
{
public:
    StepColor(const uint16_t clusterId) : ModelCommand("step-color", clusterId)
    {
        AddArgument("x", 0, UINT16_MAX, &mStepX);
        AddArgument("y", 0, UINT16_MAX, &mStepY);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStepColorCommand(buffer->Start(), bufferSize, endPointId, mStepX, mStepY, mTransitionTime, mOptionsMask,
                                      mOptionsOverride);
    }

private:
    uint16_t mStepX;
    uint16_t mStepY;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveToColorTemperature : public ModelCommand
{
public:
    MoveToColorTemperature(const uint16_t clusterId) : ModelCommand("move-to-color-temperature", clusterId)
    {
        AddArgument("temperature", 0, UINT16_MAX, &mColorTemperature);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, mColorTemperature, mTransitionTime,
                                                   mOptionsMask, mOptionsOverride);
    }

private:
    uint16_t mColorTemperature;
    uint16_t mTransitionTime;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class MoveColorTemperature : public ModelCommand
{
public:
    MoveColorTemperature(const uint16_t clusterId) : ModelCommand("move-color-temperature", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mMoveMode);
        AddArgument("rate", 0, UINT16_MAX, &mRate);
        AddArgument("min", 0, UINT16_MAX, &mColorTemperatureMin);
        AddArgument("max", 0, UINT16_MAX, &mColorTemperatureMax);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, mMoveMode, mRate, mColorTemperatureMin,
                                                 mColorTemperatureMax, mOptionsMask, mOptionsOverride);
    }

private:
    uint8_t mMoveMode;
    uint16_t mRate;
    uint16_t mColorTemperatureMin;
    uint16_t mColorTemperatureMax;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class StepColorTemperature : public ModelCommand
{
public:
    StepColorTemperature(const uint16_t clusterId) : ModelCommand("step-color-temperature", clusterId)
    {
        AddArgument("mode", 0, UINT8_MAX, &mStepMode);
        AddArgument("size", 0, UINT8_MAX, &mStepSize);
        AddArgument("time", 0, UINT16_MAX, &mTransitionTime);
        AddArgument("min", 0, UINT16_MAX, &mColorTemperatureMin);
        AddArgument("max", 0, UINT16_MAX, &mColorTemperatureMax);
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStepColorTemperatureCommand(buffer->Start(), bufferSize, endPointId, mStepMode, mStepSize,
                                                 mColorTemperatureMin, mColorTemperatureMax, mTransitionTime, mOptionsMask,
                                                 mOptionsOverride);
    }

private:
    uint8_t mStepMode;
    uint8_t mStepSize;
    uint16_t mTransitionTime;
    uint16_t mColorTemperatureMin;
    uint16_t mColorTemperatureMax;
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
};

class StopMoveStep : public ModelCommand
{
public:
    StopMoveStep(const uint16_t clusterId) : ModelCommand("stop-move-step", clusterId)
    {
        AddArgument("optionsMask", 0, UINT8_MAX, &mOptionsMask);
        AddArgument("optionsOverride", 0, UINT8_MAX, &mOptionsOverride);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStopMoveStepCommand(buffer->Start(), bufferSize, endPointId, mOptionsMask, mOptionsOverride);
    }

private:
    uint8_t mOptionsMask;
    uint8_t mOptionsOverride;
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
