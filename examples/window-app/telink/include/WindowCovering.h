/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "LEDWidget.h"
#include "PWMDevice.h"

#include <app/clusters/window-covering-server/window-covering-delegate.h>
#include <app/clusters/window-covering-server/window-covering-server.h>

#include <cstdint>

using namespace chip::app::Clusters::WindowCovering;

class WindowCovering
{
public:
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

    void StartMove(WindowCoveringType aMoveType);
    void SetSingleStepTarget(OperationalState aDirection);
    void SetMoveType(WindowCoveringType aMoveType) { mCurrentUIMoveType = aMoveType; }
    WindowCoveringType GetMoveType() { return mCurrentUIMoveType; }
    void PositionLEDUpdate(WindowCoveringType aMoveType);

    static void SchedulePostAttributeChange(chip::EndpointId aEndpoint, chip::AttributeId aAttributeId);
    static constexpr chip::EndpointId Endpoint() { return 1; };

private:
    void SetBrightness(WindowCoveringType aMoveType, uint16_t aPosition);
    void SetTargetPosition(OperationalState aDirection, chip::Percent100ths aPosition);
    uint8_t PositionToBrightness(uint16_t aPosition, WindowCoveringType aMoveType);

    static void UpdateOperationalStatus(WindowCoveringType aMoveType, OperationalState aDirection);
    static bool TargetCompleted(WindowCoveringType aMoveType, NPercent100ths aCurrent, NPercent100ths aTarget);
    static void StartTimer(WindowCoveringType aMoveType, uint32_t aTimeoutMs);
    static chip::Percent100ths CalculateSingleStep(WindowCoveringType aMoveType);
    static void DriveCurrentLiftPosition(intptr_t);
    static void DriveCurrentTiltPosition(intptr_t);
    static void MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState);
    static void DoPostAttributeChange(intptr_t aArg);

    WindowCoveringType mCurrentUIMoveType;

    PWMDevice mLiftIndicator;
    PWMDevice mTiltIndicator;
    bool mInLiftMove{ false };
    bool mInTiltMove{ false };
};
