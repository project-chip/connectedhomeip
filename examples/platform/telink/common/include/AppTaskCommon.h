/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/telink/FactoryDataProvider.h>
#endif

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <cstdint>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
inline constexpr EndpointId kExampleEndpointId = 1;
inline constexpr uint8_t kDefaultMinLevel      = 0;
inline constexpr uint8_t kDefaultMaxLevel      = 254;
inline constexpr uint8_t kButtonPushEvent      = 1;
inline constexpr uint8_t kButtonReleaseEvent   = 0;
} // namespace

class LedManager;
class PwmManager;
class ButtonManager;

struct Identify;

class AppTaskCommon
{
public:
#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
    void PowerOnFactoryReset(void);
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */
    CHIP_ERROR StartApp();
    void PostEvent(AppEvent * event);

    static void IdentifyEffectHandler(Clusters::Identify::EffectIdentifierEnum aEffect);
    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

#ifdef CONFIG_CHIP_PW_RPC
    enum ButtonId_t
    {
        kButtonId_ExampleAction = 1,
        kButtonId_FactoryReset,
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        kButtonId_StartThread,
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
        kButtonId_StartWiFi,
#endif
        kButtonId_StartBleAdv
    } ButtonId;
#endif

protected:
    CHIP_ERROR InitCommonParts(void);
    void PrintFirmwareInfo(void);

    void DispatchEvent(AppEvent * event);
    void GetEvent(AppEvent * aEvent);

    void InitLeds();
    virtual void LinkLeds(LedManager & ledManager);
    void InitPwms();
    virtual void LinkPwms(PwmManager & pwmManager);
    void InitButtons(void);
    virtual void LinkButtons(ButtonManager & buttonManager);

    static void FactoryResetTimerTimeoutCallback(k_timer * timer);
    static void FactoryResetTimerEventHandler(AppEvent * aEvent);
    static void FactoryResetButtonEventHandler(void);
    static void FactoryResetHandler(AppEvent * aEvent);

    static void StartBleAdvButtonEventHandler(void);
    static void StartBleAdvHandler(AppEvent * aEvent);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    static void StartThreadButtonEventHandler(void);
    static void StartThreadHandler(AppEvent * aEvent);
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    static void StartWiFiButtonEventHandler(void);
    static void StartWiFiHandler(AppEvent * aEvent);
#endif

#if CONFIG_TELINK_OTA_BUTTON_TEST
    static void TestOTAButtonEventHandler(void);
    static void TestOTAHandler(AppEvent * aEvent);
#endif

    static void ExampleActionButtonEventHandler(void);

    void SetExampleButtonCallbacks(EventHandler aAction_CB);
    EventHandler ExampleActionEventHandler;

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void UpdateStatusLED(void);

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::ExternalFlashFactoryData> mFactoryDataProvider;
#endif

#ifdef CONFIG_CHIP_PW_RPC
    friend class chip::rpc::TelinkButton;
    static void ButtonEventHandler(ButtonId_t btnId, bool btnPressed);
#endif
};
