/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    PWMDevice & GetLiftIndicator() { return mLiftIndicator; }
    PWMDevice & GetTiltIndicator() { return mTiltIndicator; }

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
    static chip::Percent100ths CalculateNextPosition(MoveType aMoveType);
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
