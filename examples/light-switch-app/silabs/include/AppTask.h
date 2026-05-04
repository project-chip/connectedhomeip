/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <variant>

#include "AppEvent.h"
#include "BaseApplication.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/data-model/Nullable.h>
#include <ble/Ble.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceLayer.h>

/**********************************************************
 * Defines
 *********************************************************/

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

struct GenericSwitchEventData
{
    chip::EndpointId endpoint;
    chip::EventId event;
};

struct CommandBase
{
    chip::BitMask<chip::app::Clusters::LevelControl::OptionsBitmap> optionsMask;
    chip::BitMask<chip::app::Clusters::LevelControl::OptionsBitmap> optionsOverride;

    CommandBase() : optionsMask(0), optionsOverride(0) {}
};

struct BindingCommandData
{
    chip::EndpointId localEndpointId = 1;
    chip::CommandId commandId;
    chip::ClusterId clusterId;
    bool isGroup = false;

    struct MoveToLevel : public CommandBase
    {
        uint8_t level;
        chip::app::DataModel::Nullable<uint16_t> transitionTime;
    };
    struct Move : public CommandBase
    {
        chip::app::Clusters::LevelControl::MoveModeEnum moveMode;
        chip::app::DataModel::Nullable<uint8_t> rate;
    };
    struct Step : public CommandBase
    {
        chip::app::Clusters::LevelControl::StepModeEnum stepMode;
        uint8_t stepSize;
        chip::app::DataModel::Nullable<uint16_t> transitionTime;
    };
    struct Stop : public CommandBase
    {
    };
    std::variant<MoveToLevel, Move, Step, Stop> commandData;
};

/**********************************************************
 * AppTask Declaration
 *********************************************************/

class AppTask : public BaseApplication
{

public:
    AppTask() = default;

    static AppTask & GetAppTask() { return sAppTask; }

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

    enum class LightSwitchAction : uint8_t
    {
        Toggle,
        On,
        Off
    };

    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    static void AppEventHandler(AppEvent * aEvent);

    CHIP_ERROR InitLightSwitch(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint);

    void GenericSwitchOnInitialPress();

    void GenericSwitchOnShortRelease();

    void TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand = false);

    void TriggerLevelControlAction(chip::app::Clusters::LevelControl::StepModeEnum stepMode, bool isGroupCommand = false);

    chip::app::Clusters::LevelControl::StepModeEnum getStepMode();

    void changeStepMode();

    void SwitchActionEventHandler(uint16_t eventType);

    CHIP_ERROR InitBindingHandler();

    static void SwitchWorkerFunction(intptr_t context);

    static void BindingWorkerFunction(intptr_t context);

    static void PostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value);

    static void OnOffClusterInitCallback(chip::EndpointId endpoint);

private:
    static AppTask sAppTask;
    Timer * longPressTimer = nullptr;
    static bool functionButtonPressed;  // True when button0 is pressed, used to trigger factory reset
    static bool actionButtonPressed;    // True when button1 is pressed, used to initiate toggle or level-up/down
    static bool actionButtonSuppressed; // True when both button0 and button1 are pressed, used to switch step direction
    static bool isButtonEventTriggered; // True when button0 press event is posted to BaseApplication

    /**
     * @brief Override of BaseApplication::AppInit() virtual method, called by BaseApplication::Init()
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR AppInit() override;

    static void OnLongPressTimeout(Timer & timer);

    /**
     * @brief This function will be called when PB1 is
     *        long-pressed to trigger the level-control action
     */
    void HandleLongPress();

    chip::EndpointId mLightSwitchEndpoint                          = chip::kInvalidEndpointId;
    chip::EndpointId mGenericSwitchEndpoint                        = chip::kInvalidEndpointId;
    chip::app::Clusters::LevelControl::StepModeEnum mStepDirection = chip::app::Clusters::LevelControl::StepModeEnum::kUp;

    static void GenericSwitchWorkerFunction(intptr_t context);

    static void InitBindingHandlerInternal(intptr_t arg);

    static void LightSwitchChangedHandler(const chip::app::Clusters::Binding::TableEntry & binding,
                                          chip::OperationalDeviceProxy * peer_device, void * context);

    static void LightSwitchContextReleaseHandler(void * context);

    static void ProcessOnOffUnicastBindingCommand(chip::CommandId commandId,
                                                  const chip::app::Clusters::Binding::TableEntry & binding,
                                                  chip::Messaging::ExchangeManager * exchangeMgr,
                                                  const chip::SessionHandle & sessionHandle);

    static void ProcessOnOffGroupBindingCommand(chip::CommandId commandId,
                                                const chip::app::Clusters::Binding::TableEntry & binding);

    static void ProcessLevelControlUnicastBindingCommand(BindingCommandData * data,
                                                         const chip::app::Clusters::Binding::TableEntry & binding,
                                                         chip::OperationalDeviceProxy * peer_device);

    static void ProcessLevelControlGroupBindingCommand(BindingCommandData * data,
                                                       const chip::app::Clusters::Binding::TableEntry & binding);
};
