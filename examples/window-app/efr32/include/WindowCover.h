/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include <AppConfig.h>
#include <AppEvent.h>
#include <AppTimer.h>
#include <memory>
#include <stdint.h>
#include <vector>

class WindowCover
{
public:
    enum class CoverType
    {
        Rollershade                  = 0, // Lift
        Rollershade_2_motor          = 1, // Lift
        Rollershade_exterior         = 2, // Lift
        Rollershade_exterior_2_motor = 3, // Lift
        Drapery                      = 4, // Lift
        Awning                       = 5, // Lift
        Shutter                      = 6, // Tilt
        Tilt_blind                   = 7, // Tilt
        Tilt_Lift_blind              = 8, // Lift, Tilt
        Projector_screen             = 9  // Tilt
    };

    enum class CoverAction
    {
        None = 0,
        LiftDown,
        LiftUp,
        TiltDown,
        TiltUp,
    };

    static const char * TypeString(const CoverType type);

    WindowCover();
    WindowCover(CoverType type, uint16_t liftOpenLimit, uint16_t liftClosedLimit, uint16_t tiltOpenLimit, uint16_t tiltClosedLimit);

    // Status
    void StatusSet(uint8_t status);
    uint8_t StatusGet(void);
    // Type
    void TypeSet(CoverType type);
    void TypeCycle();
    CoverType TypeGet(void);
    // Lift
    uint16_t LiftOpenLimitGet(void);
    uint16_t LiftClosedLimitGet(void);
    void LiftSet(uint16_t lift);
    uint16_t LiftGet(void);
    void LiftPercentSet(uint8_t percentage);
    uint8_t LiftPercentGet(void);
    void LiftUp();
    void LiftDown();
    void LiftGotoValue(uint16_t lift);
    void LiftGotoPercent(uint8_t percentage);
    uint8_t LiftToPercent(uint16_t lift);
    uint16_t PercentToLift(uint8_t liftPercent);
    // Tilt
    uint16_t TiltOpenLimitGet(void);
    uint16_t TiltClosedLimitGet(void);
    void TiltSet(uint16_t tilt);
    uint16_t TiltGet(void);
    void TiltPercentSet(uint8_t percentage);
    uint8_t TiltPercentGet(void);
    void TiltUp();
    void TiltDown();
    void TiltGotoValue(uint16_t tilt);
    void TiltGotoPercent(uint8_t percentage);
    uint8_t TiltToPercent(uint16_t tilt);
    uint16_t PercentToTilt(uint8_t tiltPercent);

    // Commands
    void Open();
    void Close();
    void Stop();
    // Other
    void TiltModeSet(bool mode);
    bool TiltModeGet(void);
    void ToggleTiltMode();
    void StepUp();
    void StepDown();
    bool IsOpen(void);
    bool IsClosed(void);
    bool IsMoving(void);
    // Events
    void PostEvent(AppEvent::EventType event);

private:
    static void LiftTimerCallback(AppTimer & timer, void * context);
    static void TiltTimerCallback(AppTimer & timer, void * context);

    uint8_t mStatus           = 0x03; // bit0: Operational, bit1: Online;
    CoverType mType           = CoverType::Tilt_Lift_blind;
    uint16_t mLiftOpenLimit   = LIFT_OPEN_LIMIT;
    uint16_t mLiftClosedLimit = LIFT_CLOSED_LIMIT;
    uint16_t mLift            = LIFT_CLOSED_LIMIT;
    uint16_t mLiftTarget;
    AppTimer mLiftTimer;
    CoverAction mLiftAction;
    bool mTiltMode;
    uint16_t mTiltOpenLimit   = TILT_OPEN_LIMIT;
    uint16_t mTiltClosedLimit = TILT_CLOSED_LIMIT;
    uint16_t mTilt            = TILT_CLOSED_LIMIT;
    uint16_t mTiltTarget;
    AppTimer mTiltTimer;
    CoverAction mTiltAction;
};
