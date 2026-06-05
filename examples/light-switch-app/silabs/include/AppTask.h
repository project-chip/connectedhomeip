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
#include <app/ConcreteAttributePath.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/data-model/Nullable.h>
#include <ble/Ble.h>
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

    /** @brief Returns the active app instance */
    static AppTask & GetAppTask();

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    /** @brief Creates and starts the AppTask thread */
    CHIP_ERROR StartAppTask();

    /**
     * @brief Event handler when a button is pressed
     *
     * @param button    APP_FUNCTION_BUTTON or the action button
     * @param btnAction SL_SIMPLE_BUTTON_PRESSED, SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /** @brief AppTask thread event handler for queued button events */
    static void AppEventHandler(AppEvent * aEvent);

    /** @brief Matter-thread work item: notifies the binding manager that a bound cluster changed to drive the outgoing switch command */
    static void SwitchWorkerFunction(intptr_t context);

    /** @brief Matter-thread work item: emits a Generic Switch cluster event for the queued switch action */
    static void GenericSwitchWorkerFunction(intptr_t context);

    /** @brief Sends an OnOff cluster command to the bound peer device for the given binding entry */
    static void ProcessOnOffBindingCommand(chip::CommandId commandId, const chip::app::Clusters::Binding::TableEntry & binding,
                                           chip::OperationalDeviceProxy * peer_device);

    /** @brief Sends a LevelControl cluster command to the bound peer device for the given binding entry */
    static void ProcessLevelControlBindingCommand(BindingCommandData * data,
                                                  const chip::app::Clusters::Binding::TableEntry & binding,
                                                  chip::OperationalDeviceProxy * peer_device);

    /** @brief Data model hook invoked when a cluster attribute changes */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /** @brief Binding manager callback invoked per bound device to send a switch command to the target light */
    static void LightSwitchChangedHandler(const chip::app::Clusters::Binding::TableEntry & binding,
                                          chip::OperationalDeviceProxy * peer_device, void * context);

protected:
    /** @brief Override of `BaseApplication::AppInit()` */
    CHIP_ERROR AppInit() override;

    /** @brief Light switch specific initialization */
    CHIP_ERROR InitLightSwitch(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint);

    /** @brief Handler scheduled on the Matter thread to set up the binding table */
    static void InitBindingHandler(intptr_t arg);
};
