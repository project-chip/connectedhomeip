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
        ~Timer();

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
        enum ControlAction : uint8_t
        {
            Lift = 0,
            Tilt = 1
        };

        void Init(chip::EndpointId endpoint);

        /**
         * @brief Schedule a lift or a tilt related attribute transition on the ChipEvent queue
         * **This function allocates a CoverWorkData which needs to be freed by the Worker callback**
         *
         * @param action : ControlAction::Lift will ScheduleWork LiftUpdateWorker, while ControlAction::Tilt will ScheduleWork
         * TilitUpdateWorker
         * @param setNewTarget : True will stop any ongoing transition and start a new one. False will continue the active
         * transition updates
         */
        void ScheduleControlAction(ControlAction action, bool setNewTarget);
        // Helper functions that schedule Lift transitions
        inline void LiftGoToTarget() { ScheduleControlAction(ControlAction::Lift, true); }
        inline void LiftContinueToTarget() { ScheduleControlAction(ControlAction::Lift, false); }
        // Helper functions that schedule Tilt transitions
        inline void TiltGoToTarget() { ScheduleControlAction(ControlAction::Tilt, true); }
        inline void TiltContinueToTarget() { ScheduleControlAction(ControlAction::Tilt, false); }

        void PositionSet(chip::EndpointId endpointId, chip::Percent100ths position, ControlAction action);
        void UpdateTargetPosition(OperationalState direction, ControlAction action);

        Type CycleType();

        static void OnLiftTimeout(Timer & timer);
        static void OnTiltTimeout(Timer & timer);

        chip::EndpointId mEndpoint = 0;

        Timer * mLiftTimer = nullptr;
        Timer * mTiltTimer = nullptr;

        OperationalState mLiftOpState = OperationalState::Stall;
        OperationalState mTiltOpState = OperationalState::Stall;

        /**
         * @brief Worker callbacks for the ScheduleControlAction.
         * Those functions compute the operational state, and transititon movement based on the current and target positions
         * for the cover.
         * @param arg Context passed to the schedule worker. In this case, a CoverWorkData pointer
         * The referenced CoverWorkData was allocated by ScheduleControlAction and must be freed by the worker.
         */
        static void LiftUpdateWorker(intptr_t arg);
        static void TiltUpdateWorker(intptr_t arg);
    };

    struct CoverWorkData
    {
        Cover * cover     = nullptr;
        bool setNewTarget = false;
        CoverWorkData(Cover * c, bool t) : cover(c), setNewTarget(t) {}
        ~CoverWorkData() { cover = nullptr; }
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
    Timer * mIconTimer = nullptr;
    LcdIcon mIcon      = LcdIcon::None;
#endif
};
