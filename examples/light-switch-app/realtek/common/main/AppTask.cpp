/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdlib.h>

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "BindingHandler.h"
#include "Globals.h"
#include "util/RealtekObserver.h"

#include <DeviceInfoProviderImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <inet/EndPointStateOpenThread.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <CHIPDeviceManager.h>
#include <DeviceCallbacks.h>

#include <os_mem.h>
#include <os_task.h>

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#endif

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace app::Clusters::Descriptor::Structs;

#include <platform/CHIPDeviceLayer.h>

#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 5000
#define RESET_TRIGGER_TIMEOUT 1500

#define APP_TASK_STACK_SIZE (4 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

namespace {

#if (CONFIG_1_TO_2_ZAP || CONFIG_1_TO_8_ZAP || CONFIG_1_TO_11_ZAP)

// Switches Namespace: 0x43, tag 0x08 (Custom)
constexpr const uint8_t kNamespaceSwitches = 0x43;
constexpr const uint8_t kTagCustom         = 0x08;

const SemanticTagStruct::Type switch1TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                                                         { chip::app::DataModel::MakeNullable(chip::CharSpan("Switch1", 7)) }) } };
const SemanticTagStruct::Type switch2TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch2", 7))) } };
const SemanticTagStruct::Type switch3TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch3", 7))) } };
const SemanticTagStruct::Type switch4TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch4", 7))) } };
const SemanticTagStruct::Type switch5TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch5", 7))) } };
const SemanticTagStruct::Type switch6TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch6", 7))) } };
const SemanticTagStruct::Type switch7TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch7", 7))) } };
const SemanticTagStruct::Type switch8TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch8", 7))) } };
const SemanticTagStruct::Type switch9TagList[]  = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                         DataModel::MakeNullable(CharSpan("Switch9", 7))) } };
const SemanticTagStruct::Type switch11TagList[] = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                          DataModel::MakeNullable(CharSpan("Switch11", 8))) } };
const SemanticTagStruct::Type switch12TagList[] = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                          DataModel::MakeNullable(CharSpan("Switch12", 8))) } };
const SemanticTagStruct::Type switch13TagList[] = { { .namespaceID = kNamespaceSwitches,
                                                      .tag         = kTagCustom,
                                                      .label       = Optional<DataModel::Nullable<CharSpan>>(
                                                          DataModel::MakeNullable(CharSpan("Switch13", 8))) } };
#endif

static DeviceCallbacks EchoCallbacks;

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

// NOTE! This key is for test/certification only and should not be available in production devices!
static const uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                                0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                                0xcc, 0xdd, 0xee, 0xff };

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
} // namespace

AppTask AppTask::sAppTask;

Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

/**********************************************************
 * Identify Callbacks
 *********************************************************/
namespace {
void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
}
} // namespace

void OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mEffectVariant != Clusters::Identify::EffectVariantEnum::kDefault)
    {
        ChipLogDetail(AppServer, "Identify Effect Variant unsupported. Using default");
    }

    switch (sIdentifyEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        SystemLayer().ScheduleLambda([identify] {
            (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                               identify);
        });
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        SystemLayer().ScheduleLambda([identify] {
            (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
            (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                               identify);
        });
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        SystemLayer().ScheduleLambda(
            [identify] { (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify); });
        sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
    }
}

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

Identify gIdentify = {
    chip::EndpointId{ 1 },   OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerIdentifyEffect,
};

void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    xTaskCreate(AppTaskMain, APP_TASK_NAME, APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void AppTask::AppTaskMain(void * pvParameter)
{
#if defined(FEATURE_TRUSTZONE_ENABLE) && (FEATURE_TRUSTZONE_ENABLE == 1)
    os_alloc_secure_ctx(1024);
#endif

    AppEvent event;

    sAppTask.Init();

    while (true)
    {
        /* Task pend until we have stuff to do */
        if (xQueueReceive(sAppEventQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
    }
}

void AppTask::InitServer(intptr_t arg)
{
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

    chip::Server::GetInstance().Init(initParams);

    InitTag();

    static RealtekObserver sRealtekObserver;
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sRealtekObserver);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
}

void AppTask::InitGpio()
{
    LEDWidget::InitGpio();

    appStatusLED.Init(APP_LED);
    appStatusLED.Set(true);

    identifyLED.Init(IDENTIFY_STATE_LED);
    systemStatusLED.Init(SYSTEM_STATE_LED);

    matter_gpio_init(ButtonEventHandler);
}

CHIP_ERROR AppTask::Init()
{
    size_t check_mem_peak;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "Light switch App Demo!");

    chip::DeviceManager::CHIPDeviceManager & deviceMgr = chip::DeviceManager::CHIPDeviceManager::GetInstance();
    err                                                = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK");
    }

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

#if CONFIG_ENABLE_CHIP_SHELL
    chip::Shell::Engine::Root().Init();
    chip::Shell::Engine::Root().RunMainLoop();
#endif

    check_mem_peak = os_mem_peek(RAM_TYPE_DATA_ON);
    ChipLogProgress(DeviceLayer, "os_mem_peek(RAM_TYPE_DATA_ON) : (%u)", check_mem_peak);

    // Setup switch
    LightSwitch::GetInstance().Init();

    return err;
}

void AppTask::SwitchActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (aEvent->ButtonEvent.ButtonIdx == APP_TOGGLE_BUTTON)
        {
            LightSwitch::GetInstance().InitiateActionSwitch(1, Action::Toggle);
        }
        else if (aEvent->ButtonEvent.ButtonIdx == APP_GENERIC_SWITCH_BUTTON)
        {
            if (aEvent->ButtonEvent.Action == true)
            {
                ChipLogProgress(NotSpecified, "Switch release press");
                LightSwitch::GetInstance().GenericSwitchReleasePress();
            }
            else
            {
                ChipLogProgress(NotSpecified, "Switch initial press");
                LightSwitch::GetInstance().GenericSwitchInitialPress();
            }
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnPressed)
{
    if (btnIdx != APP_FUNCTION_BUTTON && btnIdx != APP_TOGGLE_BUTTON && btnIdx != APP_GENERIC_SWITCH_BUTTON)
    {
        return;
    }
    if (!chip::DeviceManager::CHIPDeviceManager::GetInstance().IsInitDone())
    {
        return;
    }
    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);
    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed ? true : false;

    switch (btnIdx)
    {
    case APP_TOGGLE_BUTTON: {
        if (!btnPressed)
        {
            return;
        }

        ChipLogProgress(NotSpecified, "Toggle Button pressed");
        button_event.Handler = SwitchActionEventHandler;
        break;
    }
    case APP_GENERIC_SWITCH_BUTTON: {
        button_event.Handler = SwitchActionEventHandler;
        break;
    }
    case APP_FUNCTION_BUTTON: {
        button_event.Handler = FunctionHandler;
        break;
    }
    default: {
        // invalid button
        return;
    }
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = aAppState;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held for factoryreset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_Reset)
    {
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;
        // Turn off all LEDs before starting blink to make sure blink is coordinated.
        systemStatusLED.Set(false);
        systemStatusLED.Blink(500, 500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    if (aEvent->ButtonEvent.Action == true)
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
            WDT_SystemReset(RESET_ALL, SW_RESET_APP_START);
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

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != nullptr)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(sAppEventQueue, aEvent, &higherPrioTaskWoken);
            portYIELD_FROM_ISR(higherPrioTaskWoken);
        }
        else
        {
            status = xQueueSend(sAppEventQueue, aEvent, 1);
        }

        if (!status)
        {
            ChipLogError(NotSpecified, "Failed to post event to app task event queue");
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Event Queue is nullptr should never happen");
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
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void) {}

void AppTask::InitTag()
{
#if CONFIG_DEFAULT_ZAP
#elif CONFIG_1_TO_2_ZAP
    SetTagList(1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch1TagList));
    SetTagList(2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch2TagList));
    SetTagList(3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch3TagList));
#elif CONFIG_1_TO_8_ZAP
    SetTagList(1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch1TagList));
    SetTagList(2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch2TagList));
    SetTagList(3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch3TagList));
    SetTagList(4, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch4TagList));
    SetTagList(5, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch5TagList));
    SetTagList(6, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch6TagList));
    SetTagList(7, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch7TagList));
    SetTagList(8, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch8TagList));
    SetTagList(9, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch9TagList));
#elif CONFIG_1_TO_11_ZAP
    SetTagList(1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch1TagList));
    SetTagList(2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch2TagList));
    SetTagList(3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch3TagList));
    SetTagList(4, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch4TagList));
    SetTagList(5, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch5TagList));
    SetTagList(6, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch6TagList));
    SetTagList(7, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch7TagList));
    SetTagList(8, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch8TagList));
    SetTagList(9, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch9TagList));
    SetTagList(11, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch11TagList));
    SetTagList(12, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch12TagList));
    SetTagList(13, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(switch13TagList));
#endif
}
