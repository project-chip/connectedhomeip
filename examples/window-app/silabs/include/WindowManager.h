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

#pragma once

#include <app/clusters/window-covering-server/window-covering-server.h>
#include <lib/core/CHIPError.h>

#include "AppEvent.h"

#include "LEDWidget.h"
#include <cmsis_os2.h>
#include <string>
#ifdef DISPLAY_ENABLED
#include <LcdPainter.h>
#endif

using namespace chip::app::Clusters::WindowCovering;

class WindowManager
{
public:
    static WindowManager sWindow;
    struct Timer
    {
        typedef void (*Callback)(Timer & timer);

        Timer(uint32_t timeoutInMs, Callback callback, void * context);

        void Start();
        void Stop();
        void Timeout();

        Callback mCallback = nullptr;
        void * mContext    = nullptr;
        bool mIsActive     = false;

        osTimerId_t mHandler = nullptr;

    private:
        static void TimerCallback(void * timerCbArg);
    };

    struct Cover
    {
        void Init(chip::EndpointId endpoint);

        void LiftUpdate(bool newTarget);
        void LiftGoToTarget() { LiftUpdate(true); }
        void LiftContinueToTarget() { LiftUpdate(false); }
        void LiftStepToward(OperationalState direction);
        void LiftSchedulePositionSet(chip::Percent100ths position) { SchedulePositionSet(position, false); }
        void LiftScheduleOperationalStateSet(OperationalState opState) { ScheduleOperationalStateSet(opState, false); }

        void TiltUpdate(bool newTarget);
        void TiltGoToTarget() { TiltUpdate(true); }
        void TiltContinueToTarget() { TiltUpdate(false); }
        void TiltStepToward(OperationalState direction);
        void TiltSchedulePositionSet(chip::Percent100ths position) { SchedulePositionSet(position, true); }
        void TiltScheduleOperationalStateSet(OperationalState opState) { ScheduleOperationalStateSet(opState, true); }

        void UpdateTargetPosition(OperationalState direction, bool isTilt);

        Type CycleType();

        static void OnLiftTimeout(Timer & timer);
        static void OnTiltTimeout(Timer & timer);

        chip::EndpointId mEndpoint = 0;

        Timer * mLiftTimer = nullptr;
        Timer * mTiltTimer = nullptr;

        OperationalState mLiftOpState = OperationalState::Stall;
        OperationalState mTiltOpState = OperationalState::Stall;

        struct CoverWorkData
        {
            chip::EndpointId mEndpointId;
            bool isTilt;

            union
            {
                chip::Percent100ths percent100ths;
                OperationalState opState;
            };
        };

        void SchedulePositionSet(chip::Percent100ths position, bool isTilt);
        static void CallbackPositionSet(intptr_t arg);
        void ScheduleOperationalStateSet(OperationalState opState, bool isTilt);
        static void CallbackOperationalStateSet(intptr_t arg);
    };

    static WindowManager & Instance();

    WindowManager();

    CHIP_ERROR Init();
    void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId);

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);
    void UpdateLED();
    void UpdateLCD();

    static void GeneralEventHandler(AppEvent * aEvent);

    static void OnIconTimeout(WindowManager::Timer & timer);

protected:
    Cover & GetCover();
    Cover * GetCover(chip::EndpointId endpoint);

    static void OnLongPressTimeout(Timer & timer);

    Timer * mLongPressTimer = nullptr;
    bool mTiltMode          = false;
    bool mUpPressed         = false;
    bool mDownPressed       = false;
    bool mUpSuppressed      = false;
    bool mDownSuppressed    = false;
    bool mResetWarning      = false;

private:
    void HandleLongPress();
    void DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute);

    Cover mCoverList[WINDOW_COVER_COUNT];
    uint8_t mCurrentCover = 0;

    LEDWidget mActionLED;
#ifdef DISPLAY_ENABLED
    Timer mIconTimer;
    LcdIcon mIcon = LcdIcon::None;
#endif
};
