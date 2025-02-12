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

#if !defined(GP_APP_DIVERSITY_POWERCYCLECOUNTING)
#error This application requires powercycle counting.
#endif

#include "gpSched.h"
#include "powercycle_counting.h"
#include "qvIO.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GenericFaultTestEventTriggerHandler.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/TypeTraits.h>

#include <app/util/persistence/DeferredAttributePersistenceProvider.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <inet/EndPointStateOpenThread.h>

#include <DeviceInfoProviderImpl.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define OTA_START_TRIGGER_TIMEOUT 1500

#define APP_TASK_STACK_SIZE (3 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define QPG_LIGHT_ENDPOINT_ID (1)
#define SECONDS_IN_HOUR (3600)                                              // we better keep this 3600
#define TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS (1 * SECONDS_IN_HOUR) // increment every hour

static uint8_t countdown = 0;

namespace {
constexpr EndpointId kLightEndpointId = 1;
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sHaveBLEConnections      = false;
bool sIsBLEAdvertisingEnabled = false;

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];

StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

// Define a custom attribute persister which makes actual write of the ColorX attribute value
// to the non-volatile storage only when it has remained constant for 5 seconds. This is to reduce
// the flash wearout when the attribute changes frequently as a result of MoveToLevel command.
// DeferredAttribute object describes a deferred attribute, but also holds a buffer with a value to
// be written, so it must live so long as the DeferredAttributePersistenceProvider object.
//
DeferredAttribute gPersisters[] = {
    DeferredAttribute(
        ConcreteAttributePath(kLightEndpointId, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentHue::Id)),
    DeferredAttribute(ConcreteAttributePath(kLightEndpointId, Clusters::ColorControl::Id,
                                            Clusters::ColorControl::Attributes::CurrentSaturation::Id)),
    DeferredAttribute(
        ConcreteAttributePath(kLightEndpointId, Clusters::LevelControl::Id, Clusters::LevelControl::Attributes::CurrentLevel::Id))

};

// Deferred persistence will be auto-initialized as soon as the default persistence is initialized
DefaultAttributePersistenceProvider gSimpleAttributePersistence;
DeferredAttributePersistenceProvider gDeferredAttributePersister(gSimpleAttributePersistence,
                                                                 Span<DeferredAttribute>(gPersisters, 3),
                                                                 System::Clock::Milliseconds32(5000));

/**********************************************************
 * Identify Callbacks
 *********************************************************/

void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
}

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

Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerIdentifyEffect,
};

/**********************************************************
 * OffWithEffect Callbacks
 *********************************************************/

void OnTriggerOffWithEffect(OnOffEffect * effect)
{
    auto effectId      = effect->mEffectIdentifier;
    auto effectVariant = effect->mEffectVariant;

    // Uses print outs until we can support the effects
    if (effectId == Clusters::OnOff::EffectIdentifierEnum::kDelayedAllOff)
    {
        auto typedEffectVariant = static_cast<Clusters::OnOff::DelayedAllOffEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffFastFade");
        }
        else if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kNoFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kNoFade");
        }
        else if (typedEffectVariant == Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade)
        {
            ChipLogProgress(Zcl, "DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade");
        }
    }
    else if (effectId == Clusters::OnOff::EffectIdentifierEnum::kDyingLight)
    {
        auto typedEffectVariant = static_cast<Clusters::OnOff::DyingLightEffectVariantEnum>(effectVariant);
        if (typedEffectVariant == Clusters::OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff)
        {
            ChipLogProgress(Zcl, "DyingLightEffectVariantEnum::kDyingLightFadeOff");
        }
    }
}

OnOffEffect gEffect = {
    chip::EndpointId{ 1 },
    OnTriggerOffWithEffect,
    Clusters::OnOff::EffectIdentifierEnum::kDelayedAllOff,
    to_underlying(Clusters::OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade),
};

} // namespace

AppTask AppTask::sAppTask;

namespace {
constexpr int extDiscTimeoutSecs = 20;
}

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
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), nullptr, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void AppTask::InitServer(intptr_t arg)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    VerifyOrDie(gSimpleAttributePersistence.Init(initParams.persistentStorageDelegate) == CHIP_NO_ERROR);
    gExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);

    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);

    // Use GenericFaultTestEventTriggerHandler to inject faults
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static GenericFaultTestEventTriggerHandler sFaultTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sFaultTestEventTriggerHandler) == CHIP_NO_ERROR);
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider        = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    chip::Server::GetInstance().Init(initParams);

    app::SetAttributePersistenceProvider(&gDeferredAttributePersister);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif

    // Open commissioning after boot if no fabric was available
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    {
        ChipLogProgress(NotSpecified, "No fabrics, starting commissioning.");
        AppTask::OpenCommissioning((intptr_t) 0);
    }
}

void AppTask::OpenCommissioning(intptr_t arg)
{
    // Enable BLE advertisements

    SystemLayer().ScheduleLambda([] {
        CHIP_ERROR err;
        err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");
        }
    });
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);

    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);

    // Setup light
    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "LightingMgr().Init() failed");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Setup button handler
    qvIO_SetBtnCallback(ButtonEventHandler);

    // Log device configuration
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
    sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
    sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
    sIsBLEAdvertisingEnabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    UpdateLEDs();

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS),
                                                      TotalHoursTimerHandler, this);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        // Toggle light
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 0, 0);
    }
    if (aEvent->Type == AppEvent::kEventType_Level && aEvent->ButtonEvent.Action != 0)
    {
        // Toggle Dimming of light between 2 fixed levels
        uint8_t val = 0x0;
        val         = LightingMgr().GetLevel() == 0x40 ? 0xfe : 0x40;
        action      = LightingManager::LEVEL_ACTION;

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 1, &val);
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    if (btnIdx != APP_ON_OFF_BUTTON && btnIdx != APP_FUNCTION_BUTTON && btnIdx != APP_LEVEL_BUTTON)
    {
        return;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_ON_OFF_BUTTON && btnPressed == true)
    {
        // Hand off to Light handler - On/Off light
        button_event.Handler = LightingActionEventHandler;
    }
    else if (btnIdx == APP_LEVEL_BUTTON)
    {
        // Hand off to Light handler - Change level of light
        button_event.Type    = AppEvent::kEventType_Level;
        button_event.Handler = LightingActionEventHandler;
    }
    else if (btnIdx == APP_FUNCTION_BUTTON)
    {
        // Hand off to Functionality handler - depends on duration of press
        button_event.Handler = FunctionHandler;
    }
    else
    {
        return;
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

void AppTask::TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogProgress(NotSpecified, "HourlyTimer");

    CHIP_ERROR err;
    uint32_t totalOperationalHours = 0;

    err = ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);

    if (err == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS / SECONDS_IN_HOUR));
    }
    else if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        totalOperationalHours = 0; // set this explicitly to 0 for safety
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours +
                                                      (TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS / SECONDS_IN_HOUR));
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(TOTAL_OPERATIONAL_HOURS_SAVE_INTERVAL_SECONDS),
                                                      TotalHoursTimerHandler, nullptr);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
        ChipLogProgress(NotSpecified, "[BTN] Release button now to start Software Updater");
        ChipLogProgress(NotSpecified, "[BTN] Hold to trigger Factory Reset");
        sAppTask.mFunction = kFunction_SoftwareUpdate;
        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT - OTA_START_TRIGGER_TIMEOUT);
    }
    // If we reached here, the button was held past OTA_START_TRIGGER_TIMEOUT,
    // initiate OTA update
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        qvIO_LedSet(SYSTEM_STATE_LED, false);

        qvIO_LedBlink(SYSTEM_STATE_LED, 500, 500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        SystemLayer().ScheduleLambda([] { chip::Server::GetInstance().ScheduleFactoryReset(); });
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == true)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            ChipLogProgress(NotSpecified, "[BTN] Hold to select function:");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger BLE adv (0-1.5s)");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger OTA (1.5-3s)");
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6s)");

            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            if (ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                ChipLogProgress(NotSpecified, "BLE advertising already in progress.");
            }
            else
            {
                // Enable BLE advertisements and pairing window
                AppTask::OpenCommissioning((intptr_t) 0);
                ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");
            }
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();

            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "[BTN] Triggering OTA Query");

            TriggerOTAQuery();
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
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

void AppTask::ActionInitiated(LightingManager::Action_t aAction)
{
    // Placeholder for light action
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes on");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes off ");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // Placeholder for light action completed
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light Off Action has been completed");
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != nullptr)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
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
void AppTask::UpdateClusterState(void)
{
    SystemLayer().ScheduleLambda([] {
        ChipLogProgress(NotSpecified, "UpdateClusterState");

        // Write the new on/off value
        Protocols::InteractionModel::Status status =
            Clusters::OnOff::Attributes::OnOff::Set(QPG_LIGHT_ENDPOINT_ID, LightingMgr().IsTurnedOn());

        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating on/off %x", to_underlying(status));
        }

        // Write new level value
        status = Clusters::LevelControl::Attributes::CurrentLevel::Set(QPG_LIGHT_ENDPOINT_ID, LightingMgr().GetLevel());
        if (status != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(NotSpecified, "ERR: updating level %x", to_underlying(status));
        }
    });
}

void AppTask::UpdateLEDs(void)
{
    // If system has "full connectivity", keep the LED On constantly.
    //
    // If thread and service provisioned, but not attached to the thread network
    // yet OR no connectivity to the service OR subscriptions are not fully
    // established THEN blink the LED Off for a short period of time.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink
    // the LEDs at an even rate of 100ms.
    //
    // Otherwise, turn the LED OFF.
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        qvIO_LedSet(SYSTEM_STATE_LED, true);
    }
    else if (sIsThreadProvisioned && !sIsThreadEnabled)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 950, 50);
    }
    else if (sHaveBLEConnections)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 100, 100);
    }
    else if (sIsBLEAdvertisingEnabled)
    {
        qvIO_LedBlink(SYSTEM_STATE_LED, 50, 50);
    }
    else
    {
        // not commisioned yet
        qvIO_LedSet(SYSTEM_STATE_LED, false);
    }
}

void AppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kServiceProvisioningChange: {
        sIsThreadProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kThreadConnectivityChange: {
        sIsThreadEnabled = (event->ThreadConnectivityChange.Result == kConnectivity_Established);
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionEstablished: {
        sHaveBLEConnections = true;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        sHaveBLEConnections = false;
        UpdateLEDs();
        break;
    }

    case DeviceEventType::kCHIPoBLEAdvertisingChange: {
        sIsBLEAdvertisingEnabled = (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started);
        UpdateLEDs();
        break;
    }

    default:
        break;
    }
}

static void NextCountdown(void)
{
    if (countdown > 0)
    {
        LightingMgr().InitiateAction((countdown % 2 == 0) ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, 0, 0);
        countdown--;
        gpSched_ScheduleEvent(1000000UL, NextCountdown);
    }
    else
    {
        SystemLayer().ScheduleLambda([] { ConfigurationMgr().InitiateFactoryReset(); });
    }
}

extern "C" {
void gpAppFramework_Reset_cbTriggerResetCountCompleted(void)
{
    uint8_t resetCount = gpAppFramework_Reset_GetResetCount();

    ChipLogProgress(NotSpecified, "%d resets so far", resetCount);
    if (resetCount == 10)
    {
        ChipLogProgress(NotSpecified, "Factory Reset Triggered!");
        countdown = 5;
        NextCountdown();
    }
}
}
