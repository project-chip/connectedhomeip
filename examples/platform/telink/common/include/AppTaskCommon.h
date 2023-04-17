/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AppConfig.h"
#include "AppEventCommon.h"

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
#include "LEDWidget.h"
#endif

#if APP_USE_IDENTIFY_PWM
#include "PWMDevice.h"
#endif

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/telink/FactoryDataProvider.h>
#endif

#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <cstdint>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
constexpr EndpointId kExampleEndpointId = 1;
constexpr uint8_t kDefaultMinLevel      = 0;
constexpr uint8_t kDefaultMaxLevel      = 254;
constexpr uint8_t kButtonPushEvent      = 1;
constexpr uint8_t kButtonReleaseEvent   = 0;
} // namespace

class AppTaskCommon
{
public:
    CHIP_ERROR StartApp();
    void PostEvent(AppEvent * event);

    static void IdentifyEffectHandler(EmberAfIdentifyEffectIdentifier aEffect);

protected:
    CHIP_ERROR InitCommonParts(void);

    void DispatchEvent(AppEvent * event);
    void GetEvent(AppEvent * aEvent);

    void InitButtons(void);

    static void FactoryResetTimerTimeoutCallback(k_timer * timer);
    static void FactoryResetTimerEventHandler(AppEvent * aEvent);
    static void FactoryResetButtonEventHandler(void);
    static void FactoryResetHandler(AppEvent * aEvent);

    static void StartBleAdvButtonEventHandler(void);
    static void StartBleAdvHandler(AppEvent * aEvent);

#if APP_USE_THREAD_START_BUTTON
    static void StartThreadButtonEventHandler(void);
    static void StartThreadHandler(AppEvent * aEvent);
#endif

#if APP_USE_EXAMPLE_START_BUTTON
    static void ExampleActionButtonEventHandler(void);

    void SetExampleButtonCallbacks(EventHandler aAction_CB);
    EventHandler ExampleActionEventHandler;
#endif

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

#if APP_USE_IDENTIFY_PWM
    PWMDevice mPwmIdentifyLed;

    static void ActionIdentifyStateUpdateHandler(k_timer * timer);
    static void UpdateIdentifyStateEventHandler(AppEvent * aEvent);
#endif

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    static void UpdateLedStateEventHandler(AppEvent * aEvent);
    static void LEDStateUpdateHandler(LEDWidget * ledWidget);
    static void UpdateStatusLED(void);
#endif

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::ExternalFlashFactoryData> mFactoryDataProvider;
#endif
};
