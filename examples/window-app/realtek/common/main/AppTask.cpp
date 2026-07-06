/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdlib.h>

#include "AppConfig.h"
#include "AppTask.h"
#include "util/RealtekObserver.h"

#include "LEDWidget.h"
#include "Server.h"
#include "WindowCovering.h"
#include <DeviceInfoProviderImpl.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/core/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CHIPMem.h>

#include <CHIPDeviceManager.h>
#include <DeviceCallbacks.h>

#include "matter_ble.h"
#include "matter_gpio.h"
#include <os_mem.h>
#include <os_msg.h>
#include <os_task.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#endif

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

#define MAX_NUMBER_OF_GAP_MESSAGE 0x10                                                     //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE 0x10                                                      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE) //!< Event message queue size
#define IO_MSG_WAIT_TIMEOUT 500

#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 5000
#define RESET_TRIGGER_TIMEOUT 1500

#define APP_TASK_STACK_SIZE (4 * 1024)
#define APP_TASK_PRIORITY 2

static void * app_task_handle      = NULL; //!< APP Task handle
static void * app_evt_queue_handle = NULL; //!< Event queue handle
static void * app_io_queue_handle  = NULL; //!< IO queue handle

static DeviceCallbacks EchoCallbacks;
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

AppTask AppTask::sAppTask;

LEDWidget identifyLED;

void OnIdentifyStart(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
    identifyLED.Blink(500, 500);
}

void OnIdentifyStop(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
    identifyLED.BlinkStop();
}

void OnTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

bool AppTask::PostMessage(T_IO_MSG * p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    if (app_evt_queue_handle == NULL || app_io_queue_handle == NULL)
    {
        return false;
    }

    if (os_msg_send(app_evt_queue_handle, &event, 0) == false)
    {
        ChipLogError(DeviceLayer, "send_evt_msg_to_app fail");
        return false;
    }

    if (os_msg_send(app_io_queue_handle, p_msg, 0) == false)
    {
        ChipLogError(DeviceLayer, "send_io_msg_to_app fail");
        return false;
    }

    return true;
}

CHIP_ERROR AppTask::StartAppTask()
{
    if (!os_task_create(&app_task_handle, APP_TASK_NAME, AppTaskMain, 0, APP_TASK_STACK_SIZE, APP_TASK_PRIORITY))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    uint8_t event;
    CHIP_ERROR err = CHIP_NO_ERROR;

#if defined(FEATURE_TRUSTZONE_ENABLE) && (FEATURE_TRUSTZONE_ENABLE == 1)
    os_alloc_secure_ctx(1024);
#endif

    os_msg_queue_create(&app_io_queue_handle, "ioQ", MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&app_evt_queue_handle, "evtQ", MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

    gap_start_bt_stack(app_evt_queue_handle, app_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);
    matter_ble_queue_init(app_evt_queue_handle, app_io_queue_handle);

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "sAppTask.Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    while (true)
    {
        if (os_msg_recv(app_evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(app_io_queue_handle, &io_msg, IO_MSG_WAIT_TIMEOUT) == true)
                {
                    switch (io_msg.type)
                    {
                    case IO_MSG_TYPE_GPIO:
                        ButtonHandler(&io_msg);
                        break;

                    case IO_MSG_TYPE_TIMER:
                        FunctionTimerEventHandler(&io_msg);
                        break;

                    default:
                        matter_ble_handle_io_msg(&io_msg);
                        break;
                    }
                }
                else
                {
                    ChipLogError(DeviceLayer, "CRITICAL: Received EVENT_IO_TO_APP but failed to retrieve IO message.");
                }
            }
            else
            {
                gap_handle_msg(event);
            }
        }
    }
}

void AppTask::InitServer(intptr_t context)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);

    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    err = chip::Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Server::GetInstance().Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    static RealtekObserver sRealtekObserver;
    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sRealtekObserver);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "AddFabricDelegate failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));

    WindowCovering::Instance().PositionLEDUpdate(WindowCovering::MoveType::LIFT);
    WindowCovering::Instance().PositionLEDUpdate(WindowCovering::MoveType::TILT);
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnPressed)
{
    if (!chip::DeviceManager::CHIPDeviceManager::GetInstance().IsInitDone())
    {
        return;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    T_IO_MSG io_msg;

    io_msg.type    = IO_MSG_TYPE_GPIO;
    io_msg.subtype = btnIdx;
    io_msg.u.param = btnPressed;

    PostMessage(&io_msg);
}

void AppTask::ButtonHandler(T_IO_MSG * p_msg)
{
    uint8_t key         = p_msg->subtype;
    uint32_t btnPressed = p_msg->u.param;

    switch (key)
    {
    case APP_TOGGLE_BUTTON:
        if (btnPressed)
        {
            if (WindowCovering::Instance().GetMoveType() == WindowCovering::MoveType::LIFT)
            {
                WindowCovering::Instance().SetMoveType(WindowCovering::MoveType::TILT);
                ChipLogProgress(DeviceLayer, "Window covering move: tilt");
            }
            else
            {
                WindowCovering::Instance().SetMoveType(WindowCovering::MoveType::LIFT);
                ChipLogProgress(DeviceLayer, "Window covering move: lift");
            }
        }
        break;

    case APP_CLOSE_BUTTON:
        if (btnPressed)
        {
            // ScheduleWork is asynchronous and returns immediately.
            // Failure means the work couldn't be queued, but there's nothing
            // we can do about it here, so the return value can be safely ignored.
            RETURN_SAFELY_IGNORED(PlatformMgr().ScheduleWork(CloseHandler));
        }
        break;

    case APP_OPEN_BUTTON:
        if (btnPressed)
        {
            // ScheduleWork is asynchronous and returns immediately.
            // Failure means the work couldn't be queued, but there's nothing
            // we can do about it here, so the return value can be safely ignored.
            RETURN_SAFELY_IGNORED(PlatformMgr().ScheduleWork(OpenHandler));
        }
        break;

    case APP_FUNCTION_BUTTON: {
        if (btnPressed)
        {
            if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
            {
                ChipLogProgress(NotSpecified, "[BTN] Hold to select function:");
                ChipLogProgress(NotSpecified, "[BTN] - Reset (0-1.5s)");
                ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6.5s)");

                sAppTask.StartTimer(RESET_TRIGGER_TIMEOUT);
                sAppTask.mFunction = kFunction_Reset;
            }
        }
        else
        {
            // If the button was released before 1.5sec, trigger RESET.
            if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_Reset)
            {
                sAppTask.CancelTimer();
                sAppTask.mFunction = kFunction_NoneSelected;

                chip::DeviceManager::CHIPDeviceManager::GetInstance().Shutdown();
                WDG_SystemReset(RESET_ALL, SW_RESET_APP_START);
            }
            else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
            {
                EchoCallbacks.UpdateStatusLED();
                sAppTask.CancelTimer();
                sAppTask.mFunction = kFunction_NoneSelected;
                ChipLogProgress(NotSpecified, "[BTN] Factory Reset has been Canceled");
            }
        }
    }
    break;

    default:
        break;
    }
}

void AppTask::OpenHandler(intptr_t)
{
    WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingUpOrOpen);
}

void AppTask::CloseHandler(intptr_t)
{
    WindowCovering::Instance().SetSingleStepTarget(OperationalState::MovingDownOrClose);
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
{
    T_IO_MSG timer_msg;

    timer_msg.type    = IO_MSG_TYPE_TIMER;
    timer_msg.subtype = 0;
    timer_msg.u.buf   = aAppState;

    PostMessage(&timer_msg);
}

void AppTask::FunctionTimerEventHandler(T_IO_MSG * p_msg)
{
    // If we reached here, the button was held for factoryreset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_Reset)
    {
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;
        // Turn off all LEDs before starting blink to make sure blink is coordinated.
        identifyLED.Set(false);
        identifyLED.Blink(500, 500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::CancelTimer()
{
    SystemLayer().ScheduleLambda([this] {
        chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
        this->mFunctionTimerActive = false;
    });
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    SystemLayer().ScheduleLambda([aTimeoutInMs, this] {
        CHIP_ERROR err;
        chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
        err =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutInMs), TimerEventHandler, this);
        SuccessOrExit(err);

        this->mFunctionTimerActive = true;
    exit:
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
        }
    });
}

void AppTask::InitGpio(void)
{
    LEDWidget::InitGpio();

    identifyLED.Init(1);
    DeviceCallbacks::UpdateStatusLED();

    matter_gpio_init(ButtonEventHandler);
}

CHIP_ERROR AppTask::Init()
{
    size_t check_mem_peak;

    ChipLogProgress(DeviceLayer, "Window App Demo!");
    CHIP_ERROR err = CHIP_NO_ERROR;

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    err                           = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK");
    }

    // ScheduleWork is asynchronous, failure means work couldn't be queued.
    // Since this is init time, if it fails the system won't work anyway,
    // so the return value can be safely ignored.
    RETURN_SAFELY_IGNORED(PlatformMgr().ScheduleWork(InitServer));

#if CONFIG_ENABLE_CHIP_SHELL
    chip::Shell::Engine::Root().Init();
    chip::Shell::Engine::Root().RunMainLoop();
#endif

    check_mem_peak = os_mem_peek(RAM_TYPE_DATA_ON);
    ChipLogProgress(DeviceLayer, "os_mem_peek(RAM_TYPE_DATA_ON) : (%u)", check_mem_peak);

    return err;
}
