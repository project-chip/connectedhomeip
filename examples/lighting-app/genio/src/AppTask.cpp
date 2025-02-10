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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "ColorFormat.h"
#include "LEDWidget.h"

#include "qrcodegen.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <assert.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/mt793x/NetworkCommissioningWiFiDriver.h>

#if defined(ENABLE_CHIP_SHELL)
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#ifdef portYIELD_FROM_ISR
#define OS_YIELD_FROM_ISR(yield) portYIELD_FROM_ISR(yield)
#elif portEND_SWITCHING_ISR
#define OS_YIELD_FROM_ISR(yield) portEND_SWITCHING_ISR(yield)
#else
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
using chip::Shell::PrintCommandHelp;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;

using namespace chip::app::Clusters;

Engine sShellLightSubCommands;
Engine sShellLightOnOffSubCommands;

#endif // defined(ENABLE_CHIP_SHELL)

namespace {

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
uint8_t sLightLEDLevelChangedCount = 0;

bool sIsWiFiProvisioned = false;
bool sIsWiFiEnabled     = false;
bool sIsWiFiAttached    = false;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE * 2 / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
using namespace chip::DeviceLayer::NetworkCommissioning;
chip::app::Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */,
                                                                                      &GenioWiFiDriver::GetInstance());
#endif

} // namespace

using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

#ifdef ENABLE_CHIP_SHELL

/********************************************************
 * Light shell functions
 *********************************************************/

CHIP_ERROR LightHelpHandler(int argc, char ** argv)
{
    sShellLightSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LightCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return LightHelpHandler(argc, argv);
    }

    return sShellLightSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * OnOff switch shell functions
 *********************************************************/

CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    sShellLightOnOffSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffLightCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return sShellLightOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnLightCommandHandler(int argc, char ** argv)
{
    GetAppTask().PostLightActionRequest(0, LightingManager::ON_ACTION, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OffLightCommandHandler(int argc, char ** argv)
{
    GetAppTask().PostLightActionRequest(0, LightingManager::OFF_ACTION, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorLightCommandHandler(int argc, char ** argv)
{
    uint32_t rgb = 0;

    if (argc < 3)
    {
        return LightHelpHandler(argc, argv);
    }

    rgb = (atoi(argv[2]) << 16) | (atoi(argv[1]) << 8) | atoi(argv[0]);
    MT793X_LOG("ColorLightCommandHandler: %x", rgb);

    GetAppTask().PostLightActionRequest(0, LightingManager::COLOR_ACTION, rgb);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelLightCommandHandler(int argc, char ** argv)
{
    uint8_t level = 0;
    if (argc < 1)
    {
        return LightHelpHandler(argc, argv);
    }

    level = atoi(argv[0]);
    MT793X_LOG("LevelLightCommandHandler: %d", level);
    GetAppTask().PostLightActionRequest(0, LightingManager::LEVEL_ACTION, level);
    return CHIP_NO_ERROR;
}

/********************************************************
 * Occupancy sensor simulation shell functions
 *********************************************************/

CHIP_ERROR OccuPresentLightCommandHandler(int argc, char ** argv)
{
    GetAppTask().OccupancyHandler(true);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OccuClearLightCommandHandler(int argc, char ** argv)
{
    GetAppTask().OccupancyHandler(false);
    return CHIP_NO_ERROR;
}

/********************************************************
 * Register all light commands
 *********************************************************/

/**
 * @brief configures lighting-app matter shell
 *
 */
static void RegisterLightCommands()
{
    static const shell_command_t sLightSubCommands[] = {
        {
            .cmd_func = LightHelpHandler,
            .cmd_name = "help",
            .cmd_help = "Usage: light <subcommand>",
        },
        {
            .cmd_func = OnOffLightCommandHandler,
            .cmd_name = "onoff",
            .cmd_help = "Usage: light onoff <subcommand>",
        },
        {
            .cmd_func = ColorLightCommandHandler,
            .cmd_name = "color",
            .cmd_help = "Usage: light color <r:0~255> <g:0~255> <b:0~255>",
        },
        {
            .cmd_func = LevelLightCommandHandler,
            .cmd_name = "level",
            .cmd_help = "Usage: light level <level:0~100>",
        },
        {
            .cmd_func = OccuPresentLightCommandHandler,
            .cmd_name = "present",
            .cmd_help = "Sends occupancy present event to lighting app",
        },
        {
            .cmd_func = OccuClearLightCommandHandler,
            .cmd_name = "clear",
            .cmd_help = "Sends occupancy clear to lighting app",
        },
    };

    static const shell_command_t sLightOnOffSubCommands[] = {
        {
            .cmd_func = OnOffHelpHandler,
            .cmd_name = "help",
            .cmd_help = "Usage : light ononff <subcommand>",
        },
        {
            .cmd_func = OnLightCommandHandler,
            .cmd_name = "on",
            .cmd_help = "Sends on command to lighting app",
        },
        {
            .cmd_func = OffLightCommandHandler,
            .cmd_name = "off",
            .cmd_help = "Sends off command to lighting app",
        },
    };

    static const shell_command_t sLightCommand[] = {
        {
            .cmd_func = LightCommandHandler,
            .cmd_name = "light",
            .cmd_help = "Lighting-app commands. Usage: light <subcommand>",
        },
    };

    sShellLightOnOffSubCommands.RegisterCommands(sLightOnOffSubCommands, ArraySize(sLightOnOffSubCommands));
    sShellLightSubCommands.RegisterCommands(sLightSubCommands, ArraySize(sLightSubCommands));

    Engine::Root().RegisterCommands(sLightCommand, ArraySize(sLightCommand));
}
#endif // ENABLE_CHIP_SHELL

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == NULL)
    {
        MT793X_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
        return APP_ERROR_CREATE_TASK_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    sLightLEDLevelChangedCount = 0;

    // Wait for the WiFi to be initialized
    MT793X_LOG("APP: Wait WiFi Init");
    vTaskDelay(1000); // TODO
    MT793X_LOG("APP: Done WiFi Init");

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    sWiFiNetworkCommissioningInstance.Init();
#endif

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        MT793X_LOG("funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    MT793X_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    error = LightMgr().Init();
    if (error != CHIP_NO_ERROR)
    {
        MT793X_LOG("LightMgr Init failed");
        appError(error);
    }

    LightMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sStatusLED.Init(LED_STATUS);

    ConfigurationMgr().LogDeviceConfig();

    // PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kSoftAP));

#if defined(ENABLE_CHIP_SHELL)
    RegisterLightCommands();
#endif

    return error;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    CHIP_ERROR error = sAppTask.Init();
    if (error != CHIP_NO_ERROR)
    {
        MT793X_LOG("AppTask.Init() failed");
        appError(error);
    }

    MT793X_LOG("AppTask started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiAttached    = ConnectivityMgr().IsWiFiStationConnected();

            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network
        // yet OR no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sIsWiFiProvisioned && sIsWiFiEnabled && !sIsWiFiAttached)
            {
                sStatusLED.Blink(950, 50);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }
    }
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LightingManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t level  = 0;

    if (aEvent->Type == AppEvent::kEventType_Light)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
        level  = aEvent->LightEvent.Value;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightMgr().IsLightOn() && sLightLEDLevelChangedCount >= 4)
        {
            sLightLEDLevelChangedCount = 1;
            action                     = LightingManager::OFF_ACTION;
        }
        else if (LightMgr().IsLightOn() && sLightLEDLevelChangedCount < 4)
        {
            sLightLEDLevelChangedCount += 1;
            action = LightingManager::LEVEL_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }
        actor = AppEvent::kEventType_Button;
        level = sLightLEDLevelChangedCount * 25;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        if (action == LightingManager::COLOR_ACTION)
        {
            RgbColor_t rbg;
            uint32_t rbg_raw = aEvent->LightEvent.Value;
            rbg.r            = rbg_raw & 0xFF;
            rbg.g            = (rbg_raw & 0xFF00) >> 8;
            rbg.b            = (rbg_raw & 0xFF0000) >> 16;
            initiated        = LightMgr().InitiateAction(AppEvent::kEventType_Light, action, (uint8_t *) &rbg);
        }
        else
        {
            MT793X_LOG("Lighting level: %d", level);
            initiated = LightMgr().InitiateAction(AppEvent::kEventType_Light, action, &level);
        }

        if (!initiated)
        {
            MT793X_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer || sAppTask.mFunctionTimerActive == false)
    {
        return;
    }

    switch (sAppTask.mFunction)
    {
    case kFunction_NoneSelected:
        break;

    case kFunction_LightSwitch:
        // Start timer for user to cancel the facotry reset, if needed
        MT793X_LOG("Factory Reset Triggered.");
        MT793X_LOG("Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
        break;

    case kFunction_FactoryReset:
        MT793X_LOG("Factory Reset Start.");
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
        sStatusLED.Set(true);
        break;

    default:
        break;
    }
}

void AppTask::OccupancyEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Occupancy)
    {
        MT793X_LOG("A Non Occupancy received %d", aEvent->Type);
        return;
    }

    uint8_t attributeValue = aEvent->OccupancytEvent.Present ? 1 : 0;

    MT793X_LOG("Lighting occupancy: %u", attributeValue);

    OccupancySensing::Attributes::Occupancy::Set(1, attributeValue);
}

void AppTask::SingleButtonEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        MT793X_LOG("A Non ButtonEvent received %d", aEvent->Type);
        return;
    }

    if (aEvent->ButtonEvent.Pressed)
    {
        if (sAppTask.mFunctionTimerActive == false)
        {
            /* Start the timer to detect how long Button has been pressed */
            MT793X_LOG("AppTask status LED on");
            sStatusLED.Set(true);
            sAppTask.mFunction = kFunction_LightSwitch;
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
        }
        else
        {
            MT793X_LOG("AppTask function timer already started");
        }
    }
    else
    {
        /* Cancel the timer to detect how long Button has been pressed */
        sAppTask.CancelTimer();

        switch (sAppTask.mFunction)
        {
        case kFunction_LightSwitch:
            MT793X_LOG("AppTask light switch");
            AppEvent event;
            event.Type = AppEvent::kEventType_Button;
            LightActionEventHandler(&event);
            break;
        case kFunction_FactoryReset:
            // factory reset cancelled, restore LED
            MT793X_LOG("AppTask factory reset cancelled");
            break;
        default:
            MT793X_LOG("not handled key release event, mFunction = %x", sAppTask.mFunction);
            break;
        }

        sStatusLED.Set(false);

        sAppTask.mFunction = kFunction_NoneSelected;
    }
}

void AppTask::OccupancyHandler(bool present)
{
    AppEvent occupancy_event                = {};
    occupancy_event.Type                    = AppEvent::kEventType_Occupancy;
    occupancy_event.OccupancytEvent.Present = present;
    occupancy_event.Handler                 = OccupancyEventHandler;
    sAppTask.PostEvent(&occupancy_event);
}

void AppTask::ButtonHandler(const filogic_button_t & button)
{
    AppEvent button_event            = {};
    button_event.Type                = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pressed = button.press;
    button_event.Handler             = SingleButtonEventHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = ButtonTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        MT793X_LOG("app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        MT793X_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        MT793X_LOG("app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    // Action initiated, update the light led
    if (aAction == LightingManager::ON_ACTION)
    {
        MT793X_LOG("Turning light ON")
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        MT793X_LOG("Turning light OFF")
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        MT793X_LOG("Changing light level")
    }
    else if (aAction == LightingManager::COLOR_ACTION)
    {
        MT793X_LOG("Changing light color")
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // action has been completed bon the light
    if (aAction == LightingManager::ON_ACTION)
    {
        MT793X_LOG("Light ON")
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        MT793X_LOG("Light OFF")
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        MT793X_LOG("Light level changed")
    }
    else if (aAction == LightingManager::COLOR_ACTION)
    {
        MT793X_LOG("Light color changed")
    }
}

void AppTask::PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction, uint32_t uValue)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_Light;
    event.LightEvent.Actor  = aActor;
    event.LightEvent.Action = aAction;
    event.LightEvent.Value  = uValue;
    event.Handler           = LightActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken;

            higherPrioTaskWoken = pdFALSE;
            status              = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);
            OS_YIELD_FROM_ISR(higherPrioTaskWoken);
        }
        else
        {
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }

        if (!status)
            MT793X_LOG("Failed to post event to app task event queue");
    }
    else
    {
        MT793X_LOG("Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        MT793X_LOG("Event received with no handler. Dropping event.");
    }
}
