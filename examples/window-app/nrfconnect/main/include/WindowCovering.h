/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include "LEDWidget.h"
#include "PWMDevice.h"

#include <app/clusters/window-covering-server/window-covering-server.h>

#include <cstdint>

using namespace chip::app::Clusters::WindowCovering;

class WindowCovering
{
public:
    enum class MoveType : uint8_t
    {
        LIFT = 0,
        TILT,
        NONE
    };

    struct AttributeUpdateData
    {
        chip::EndpointId mEndpoint;
        chip::AttributeId mAttributeId;
    };

    WindowCovering();
    static WindowCovering & Instance()
    {
        static WindowCovering sInstance;
        return sInstance;
    }

    void StartMove(MoveType aMoveType);
    void SetSingleStepTarget(OperationalState aDirection);
    void SetMoveType(MoveType aMoveType) { mCurrentUIMoveType = aMoveType; }
    MoveType GetMoveType() { return mCurrentUIMoveType; }
    void PositionLEDUpdate(MoveType aMoveType);

    static void SchedulePostAttributeChange(chip::EndpointId aEndpoint, chip::AttributeId aAttributeId);
    static constexpr chip::EndpointId Endpoint() { return 1; };

private:
    void SetBrightness(MoveType aMoveType, uint16_t aPosition);
    void SetTargetPosition(OperationalState aDirection, chip::Percent100ths aPosition);
    uint8_t PositionToBrightness(uint16_t aPosition, MoveType aMoveType);

    static void UpdateOperationalStatus(MoveType aMoveType, OperationalState aDirection);
    static bool TargetCompleted(MoveType aMoveType, NPercent100ths aCurrent, NPercent100ths aTarget);
    static void StartTimer(MoveType aMoveType, uint32_t aTimeoutMs);
    static chip::Percent100ths CalculateSingleStep(MoveType aMoveType);
    static void DriveCurrentLiftPosition(intptr_t);
    static void DriveCurrentTiltPosition(intptr_t);
    static void MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState);
    static void DoPostAttributeChange(intptr_t aArg);

    MoveType mCurrentUIMoveType;
    LEDWidget mLiftLED;
    LEDWidget mTiltLED;
    PWMDevice mLiftIndicator;
    PWMDevice mTiltIndicator;
    bool mInLiftMove{ false };
    bool mInTiltMove{ false };
};
