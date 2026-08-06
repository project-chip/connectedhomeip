/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstring>
#include <stdbool.h>
#include <stdint.h>
#include <utility>

#if SL_MATTER_DISPLAY_ENABLED
#include "RefrigeratorUI.h"
#endif

#include "AppEvent.h"
#include "BaseApplication.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>
#include <app/data-model/List.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <ble/BLEEndPoint.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {
namespace Clusters {

namespace RefrigeratorAndTemperatureControlledCabinetMode {

const uint8_t ModeNormal      = 0;
const uint8_t ModeRapidCool   = 1;
const uint8_t ModeRapidFreeze = 2;

} // namespace RefrigeratorAndTemperatureControlledCabinetMode

} // namespace Clusters
} // namespace app
} // namespace chip

/**********************************************************
 * AppTask Declaration
 *********************************************************/

class AppTask : public BaseApplication, public chip::app::Clusters::ModeBase::Delegate
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
     * Function posts an event for button processing
     *
     * @param button    APP_FUNCTION_BUTTON
     * @param btnAction button action - SL_SIMPLE_BUTTON_PRESSED,
     *                  SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /** @brief Data model hook invoked when a cluster attribute changes */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /**
     * @brief Constructs the ModeBase::Instance for the cabinet mode cluster on @p endpointId.
     *        Invoked from the emberAf...CabinetModeClusterInitCallback CRTP forwarder.
     */
    void CabinetModeClusterInit(chip::EndpointId endpointId);

    // ModeBase::Delegate interface. Override matching *Impl() hooks in CustomerAppTask to customize.

    /** @brief Cabinet mode cluster delegate initialization hook. */
    CHIP_ERROR Init() override;

    /** @brief Validates the requested mode transition and sets the ChangeToModeResponse status. */
    void HandleChangeToMode(uint8_t newMode,
                            chip::app::Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    /** @brief Supplies the SupportedModes label for @p modeIndex. */
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override;

    /** @brief Supplies the SupportedModes value for @p modeIndex. */
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;

    /** @brief Supplies the SupportedModes tags for @p modeIndex. */
    CHIP_ERROR
    GetModeTagsByIndex(uint8_t modeIndex,
                       chip::app::DataModel::List<chip::app::Clusters::detail::Structs::ModeTagStruct::Type> & tags) override;

protected:
    /**
     * @brief Override of BaseApplication::AppInit() virtual method, called by BaseApplication::Init()
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR AppInit() override;

    /**
     * @brief Refrigerator specific initialization. Sets endpoint composition, tag lists,
     *        and the supported temperature levels delegate.
     */
    CHIP_ERROR InitRefrigerator();

private:
    // Refrigerator state, updated from the datamodel attribute-change hook.
    int16_t mTemperatureSetpoint;

    chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap mMask;
    chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap mState;
};
