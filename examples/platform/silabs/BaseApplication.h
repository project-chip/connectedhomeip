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

#include "AppEvent.h"
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/AppDelegate.h>
#include <app/util/config.h>
#include <ble/Ble.h>
#include <cmsis_os2.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>

#include "LEDWidget.h"

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
#include <app/clusters/identify-server/identify-server.h>
#endif

#ifdef DISPLAY_ENABLED
#include "demo-ui.h"
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

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

class BaseApplicationDelegate : public AppDelegate, public chip::FabricTable::Delegate
{
public:
    bool isCommissioningInProgress() { return isComissioningStarted; }

private:
    // AppDelegate
    bool isComissioningStarted = false;
    void OnCommissioningSessionStarted() override;
    void OnCommissioningSessionStopped() override;
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override;
    void OnCommissioningWindowClosed() override;

    // FabricTable::Delegate
    void OnFabricCommitted(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override;
    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override;
};

/**********************************************************
 * BaseApplication Declaration
 *********************************************************/

class BaseApplication
{

public:
    BaseApplication() = default;
    virtual ~BaseApplication(){};
    static bool sIsProvisioned;
    static bool sIsFactoryResetTriggered;
    static LEDWidget * sAppActionLed;
    static BaseApplicationDelegate sAppDelegate;

    /**
     * @brief Create AppTask task and Event Queue
     * If an error occurs during creation, application will hang after printing out error code
     *
     * @return CHIP_ERROR CHIP_NO_ERROR if no errors
     */
    CHIP_ERROR StartAppTask(osThreadFunc_t taskFunction);

    /**
     * @brief Links the application specific led to the baseApplication context
     * in order to synchronize both LED animations.
     * Some apps may not have an application led or no animation patterns.
     *
     * @param appLed Pointer to the configure LEDWidget for the application defined LED
     */
    void LinkAppLed(LEDWidget * appLed) { sAppActionLed = appLed; }

    /**
     * @brief Remove the app Led linkage form the baseApplication context
     */
    void UnlinkAppLed() { sAppActionLed = nullptr; }

    /**
     * @brief PostEvent function that add event to AppTask queue for processing
     *
     * @param event AppEvent to post
     */
    static void PostEvent(const AppEvent * event);

    /**
     * @brief Overridable function used to update display on button press
     */
    virtual void UpdateDisplay();

#ifdef DISPLAY_ENABLED
    /**
     * @brief Return LCD object
     */
    static SilabsLCD & GetLCD(void);

    static void UpdateLCDStatusScreen(bool withChipStackLock = true);
#endif

    /**
     * @brief Function called to start the LED light timer
     */
    static void StartStatusLEDTimer(void);

    /**
     * @brief Function to stop LED light timer
     *        Turns off Status LED before stopping timer
     */
    static void StopStatusLEDTimer(void);
    static bool GetProvisionStatus(void);

    static void StartFactoryResetSequence(void);
    static void CancelFactoryResetSequence(void);

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
    // Idenfiy server command callbacks.
    static void OnIdentifyStart(Identify * identify);
    static void OnIdentifyStop(Identify * identify);
    static void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState);
    static void OnTriggerIdentifyEffect(Identify * identify);
#endif

    /**
     * @brief Updates the static boolean isCommissioned to the desired state
     *
     */
    static void UpdateCommissioningStatus(bool newState);

    /**
     * @brief Called when the last Fabric is removed, clears all Fabric related data, including Thread and Wifi provision.
     * @note This function preserves some NVM3 data that is not Fabric scoped, like Attribute Value or Boot Count.
     */
    static void DoProvisioningReset();

protected:
    CHIP_ERROR Init();
    CHIP_ERROR BaseInit();
    /** @brief Template for to implement a Application specific init.
     *              Function is called after the BaseApplication::Init function.
     */
    virtual CHIP_ERROR AppInit() = 0;

    /**
     * @brief Function called to start the function timer
     *
     * @param aTimeoutMs timer duration in ms
     */
    static void StartFunctionTimer(uint32_t aTimeoutMs);

    /**
     * @brief Function to stop function timer
     */
    static void CancelFunctionTimer(void);

    /**
     * @brief Function call event callback function for processing
     *
     * @param event triggered event to be processed
     */
    void DispatchEvent(AppEvent * event);

    /**
     * @brief Function Timer finished callback function
     *        Post an FunctionEventHandler event
     *
     * @param timerCbArg argument to the timer callback function assigned at timer creation
     */
    static void FunctionTimerEventHandler(void * timerCbArg);

    /**
     * @brief Timer Event processing function
     *        Trigger factory if Press and Hold duration is respected
     *
     * @param aEvent post event being processed
     */
    static void FunctionEventHandler(AppEvent * aEvent);

    /**
     * @brief PB0 Button event processing function
     *        Press and hold will trigger a factory reset timer start
     *        Press and release will restart BLEAdvertising if not commisionned
     *
     * @param aEvent button event being processed
     */
    static void ButtonHandler(AppEvent * aEvent);

    /**
     * @brief Light Timer finished callback function
     *        Calls LED processing function
     *
     * @param timerCbArg argument to the timer callback function assigned at timer creation
     */
    static void LightTimerEventHandler(void * timerCbArg);

    /**
     * @brief Updates device LEDs
     */
    static void LightEventHandler();

    /**
     * @brief Activate a set of Led patterns of the Status led
     *        Identify patterns and Trigger effects have priority
     *        If no identification patterns are in progress, we provide
     *        commissioning status feedback.
     *
     * @return True if a Led pattern was set, otherwise, returns false.
     */
    static bool ActivateStatusLedPatterns();

    /**
     * @brief Start the factory Reset process
     *  Almost identical to Server::ScheduleFactoryReset()
     *  but doesn't call GetFabricTable().DeleteAllFabrics(); which deletes Key per key.
     *  With our KVS platform implementation this is a lot slower than deleting the whole kvs section
     *  our silabs nvm3 driver which end up being doing in ConfigurationManagerImpl::DoFactoryReset(intptr_t arg).
     */
    static void ScheduleFactoryReset();

    static void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t);

    /**
     * @brief Outputs the QRcode payload and URL to the QR code in the logs
     *        and conditionally on the device LCD.
     *
     * @param refreshLCD When true, The LCD of the device will be refreshed to show the QR code
     */
    static void OutputQrCode(bool refreshLCD);

    /**********************************************************
     * Protected Attributes declaration
     *********************************************************/
    bool mSyncClusterToButtonAction;

private:
    static void InitOTARequestorHandler(chip::System::Layer * systemLayer, void * appState);
};
