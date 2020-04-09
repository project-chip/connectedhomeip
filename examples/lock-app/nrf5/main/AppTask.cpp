/*
 *
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
#include "AppEvent.h"
#include "WDMFeature.h"
#include "LEDWidget.h"

#include <schema/include/BoltLockTrait.h>

#include "app_config.h"
#include "app_timer.h"
#include "app_button.h"
#include "boards.h"

#include "nrf_log.h"

#include "FreeRTOS.h"

#include <Weave/Profiles/WeaveProfiles.h>
#include <Weave/Support/crypto/HashAlgos.h>
#include <Weave/DeviceLayer/SoftwareUpdateManager.h>

using namespace ::nl::Weave::TLV;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Profiles::SoftwareUpdate;

#define FACTORY_RESET_TRIGGER_TIMEOUT       3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE                 (4096)
#define APP_TASK_PRIORITY                   2
#define APP_EVENT_QUEUE_SIZE                10

APP_TIMER_DEF(sFunctionTimer);

static SemaphoreHandle_t sWeaveEventLock;

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

static LEDWidget sStatusLED;
static LEDWidget sLockLED;
static LEDWidget sUnusedLED;
static LEDWidget sUnusedLED_1;

static bool sIsThreadProvisioned              = false;
static bool sIsThreadEnabled                  = false;
static bool sIsThreadAttached                 = false;
static bool sIsPairedToAccount                = false;
static bool sIsServiceSubscriptionEstablished = false;
static bool sHaveBLEConnections               = false;
static bool sHaveServiceConnectivity          = false;

static nl::Weave::Platform::Security::SHA256 sSHA256;

AppTask AppTask::sAppTask;

namespace nl {
namespace Weave {
namespace Profiles {
namespace DataManagement_Current {
namespace Platform {

void CriticalSectionEnter(void)
{
    xSemaphoreTake(sWeaveEventLock, 0);
}

void CriticalSectionExit(void)
{
    xSemaphoreGive(sWeaveEventLock);
}

} // namespace Platform
} // namespace DataManagement_Current
} // namespace Profiles
} // namespace Weave
} // namespace nl

int AppTask::StartAppTask()
{
    ret_code_t ret = NRF_SUCCESS;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        NRF_LOG_INFO("Failed to allocate app event queue");
        ret = NRF_ERROR_NULL;
        APP_ERROR_HANDLER(ret);
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, APP_TASK_PRIORITY, &sAppTaskHandle) !=
        pdPASS)
    {
        ret = NRF_ERROR_NULL;
    }

    return ret;
}

int AppTask::Init()
{
    ret_code_t ret;
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);

    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    sUnusedLED.Init(BSP_LED_2);
    sUnusedLED_1.Init(BSP_LED_3);

    // Initialize buttons
    static app_button_cfg_t sButtons[] = {
        { LOCK_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, ButtonEventHandler },
        { FUNCTION_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, ButtonEventHandler },
    };

    ret = app_button_init(sButtons, ARRAY_SIZE(sButtons), pdMS_TO_TICKS(FUNCTION_BUTTON_DEBOUNCE_PERIOD_MS));
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_button_enable();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_enable() failed");
        APP_ERROR_HANDLER(ret);
    }

    // Initialize Timer for Function Selection
    ret = app_timer_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_timer_create(&sFunctionTimer, APP_TIMER_MODE_SINGLE_SHOT, TimerEventHandler);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_create() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = BoltLockMgr().Init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("BoltLockMgr().Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sWeaveEventLock = xSemaphoreCreateMutex();
    if (sWeaveEventLock == NULL)
    {
        NRF_LOG_INFO("xSemaphoreCreateMutex() failed");
        APP_ERROR_HANDLER(NRF_ERROR_NULL);
    }

    // Initialize WDM Feature
    ret = WdmFeature().Init();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("WdmFeature().Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    SoftwareUpdateMgr().SetEventCallback(this, HandleSoftwareUpdateEvent);

    // Enable timer based Software Update Checks
    SoftwareUpdateMgr().SetQueryIntervalWindow(SWU_INTERVAl_WINDOW_MIN_MS, SWU_INTERVAl_WINDOW_MAX_MS);

    // Print the current software version
    char currentFirmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength+1] = {0};
    size_t currentFirmwareRevLen;
    err = ConfigurationMgr().GetFirmwareRevision(currentFirmwareRev, sizeof(currentFirmwareRev), currentFirmwareRevLen);
    APP_ERROR_CHECK(err);

    NRF_LOG_INFO("Current Firmware Version: %s", currentFirmwareRev);

    return ret;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    ret_code_t ret;
    AppEvent event;

    ret = sAppTask.Init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("AppTask.Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the Weave stack.  Because the
        // Weave event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockWeaveStack()) to avoid blocking other UI activities when the Weave
        // task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockWeaveStack())
        {
            sIsThreadProvisioned              = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled                  = ConnectivityMgr().IsThreadEnabled();
            sIsThreadAttached                 = ConnectivityMgr().IsThreadAttached();
            sHaveBLEConnections               = (ConnectivityMgr().NumBLEConnections() != 0);
            sIsPairedToAccount                = ConfigurationMgr().IsPairedToAccount();
            sHaveServiceConnectivity          = ConnectivityMgr().HaveServiceConnectivity();
            sIsServiceSubscriptionEstablished = WdmFeature().AreServiceSubscriptionsEstablished();
            PlatformMgr().UnlockWeaveStack();
        }

        // Consider the system to be "fully connected" if it has service
        // connectivity and it is able to interact with the service on a regular basis.
        bool isFullyConnected = (sHaveServiceConnectivity && sIsServiceSubscriptionEstablished);

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network yet OR no
        // connectivity to the service OR subscriptions are not fully established
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (isFullyConnected)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled && sIsPairedToAccount && (!sIsThreadAttached || !isFullyConnected))
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }

        sStatusLED.Animate();
        sLockLED.Animate();
        sUnusedLED.Animate();
        sUnusedLED_1.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor;
    ret_code_t ret = NRF_SUCCESS;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (BoltLockMgr().IsUnlocked())
        {
            action = BoltLockManager::LOCK_ACTION;
        }
        else
        {
            action = BoltLockManager::UNLOCK_ACTION;
        }

        actor = Schema::Weave::Trait::Security::BoltLockTrait::BOLT_LOCK_ACTOR_METHOD_PHYSICAL;
    }
    else
    {
        ret = NRF_ERROR_NULL;
    }

    if (ret == NRF_SUCCESS)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            NRF_LOG_INFO("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if (pin_no != LOCK_BUTTON && pin_no != FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == LOCK_BUTTON && button_action == APP_BUTTON_PUSH)
    {
        button_event.Handler = LockActionEventHandler;
    }
    else if (pin_no == FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(void * p_context)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = p_context;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        NRF_LOG_INFO("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);
        sUnusedLED_1.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
        sUnusedLED.Blink(500);
        sUnusedLED_1.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        nl::Weave::DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == APP_BUTTON_PUSH)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();

            if (SoftwareUpdateMgr().IsInProgress())
            {
                NRF_LOG_INFO("Canceling In Progress Software Update");
                SoftwareUpdateMgr().Abort();
            }
            else
            {
                NRF_LOG_INFO("Manual Software Update Triggered");
                SoftwareUpdateMgr().CheckNow();
            }
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sUnusedLED.Set(false);
            sUnusedLED_1.Set(false);

            // Set lock status LED back to show state of lock.
            sLockLED.Set(!BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            NRF_LOG_INFO("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    ret_code_t ret;

    ret = app_timer_stop(sFunctionTimer);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_stop() failed");
        APP_ERROR_HANDLER(ret);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    ret_code_t ret;

    ret = app_timer_start(sFunctionTimer, pdMS_TO_TICKS(aTimeoutInMs), this);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_start() failed");
        APP_ERROR_HANDLER(ret);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        WdmFeature().GetBoltLockTraitDataSource().InitiateLock(aActor);
        NRF_LOG_INFO("Lock Action has been initiated")
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        WdmFeature().GetBoltLockTraitDataSource().InitiateUnlock(aActor);
        NRF_LOG_INFO("Unlock Action has been initiated")
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        NRF_LOG_INFO("Lock Action has been completed")

        WdmFeature().GetBoltLockTraitDataSource().LockingSuccessful();

        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        NRF_LOG_INFO("Unlock Action has been completed")

        WdmFeature().GetBoltLockTraitDataSource().UnlockingSuccessful();

        sLockLED.Set(false);
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            NRF_LOG_INFO("Failed to post event to app task event queue");
        }
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
        NRF_LOG_INFO("Event received with no handler. Dropping event.");
    }
}

void AppTask::InstallEventHandler(AppEvent * aEvent)
{
    SoftwareUpdateMgr().ImageInstallComplete(WEAVE_NO_ERROR);
}

void AppTask::HandleSoftwareUpdateEvent(void *apAppState,
                                        SoftwareUpdateManager::EventType aEvent,
                                        const SoftwareUpdateManager::InEventParam& aInParam,
                                        SoftwareUpdateManager::OutEventParam& aOutParam)
{
    static uint32_t persistedImageLen = 0;
    static char persistedImageURI[WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN+1] = "";

    switch(aEvent)
    {
        case SoftwareUpdateManager::kEvent_PrepareQuery:
        {
            aOutParam.PrepareQuery.PackageSpecification = NULL;
            aOutParam.PrepareQuery.DesiredLocale = NULL;
            break;
        }

        case SoftwareUpdateManager::kEvent_PrepareQuery_Metadata:
        {
            WEAVE_ERROR err;
            bool haveSufficientBattery = true;
            uint32_t certBodyId = 0;

            TLVWriter *writer = aInParam.PrepareQuery_Metadata.MetaDataWriter;

            if (writer)
            {
                // Providing an installed Locale as MetaData is optional. The commented section below provides an example
                // of how one can be added to metadata.

                // TLVType arrayContainerType;
                // err = writer->StartContainer(ProfileTag(::nl::Weave::Profiles::kWeaveProfile_SWU, kTag_InstalledLocales), kTLVType_Array, arrayContainerType);
                // SuccessOrExit(err);
                // err = writer->PutString(AnonymousTag, installedLocale);
                // SuccessOrExit(err);
                // err = writer->EndContainer(arrayContainerType);

                err = writer->Put(ProfileTag(::nl::Weave::Profiles::kWeaveProfile_SWU, kTag_CertBodyId), certBodyId);
                APP_ERROR_CHECK(err);

                err = writer->PutBoolean(ProfileTag(::nl::Weave::Profiles::kWeaveProfile_SWU, kTag_SufficientBatterySWU), haveSufficientBattery);
                APP_ERROR_CHECK(err);
            }
            else
            {
                aOutParam.PrepareQuery_Metadata.Error = WEAVE_ERROR_INVALID_ARGUMENT;
                NRF_LOG_INFO("ERROR ! aOutParam.PrepareQuery_Metadata.MetaDataWriter is NULL");
            }
            break;
        }

        case SoftwareUpdateManager::kEvent_QueryPrepareFailed:
        {
            if (aInParam.QueryPrepareFailed.Error == WEAVE_ERROR_STATUS_REPORT_RECEIVED)
            {
                NRF_LOG_INFO("Software Update failed during prepare: Received StatusReport %s",
                             nl::StatusReportStr(aInParam.QueryPrepareFailed.StatusReport->mProfileId,
                                     aInParam.QueryPrepareFailed.StatusReport->mStatusCode));
            }
            else
            {
                NRF_LOG_INFO("Software Update failed during prepare: %s", nl::ErrorStr(aInParam.QueryPrepareFailed.Error));
            }
            break;
        }

        case SoftwareUpdateManager::kEvent_SoftwareUpdateAvailable:
        {
            WEAVE_ERROR err;

            char currentFirmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength+1] = {0};
            size_t currentFirmwareRevLen;
            err = ConfigurationMgr().GetFirmwareRevision(currentFirmwareRev, sizeof(currentFirmwareRev), currentFirmwareRevLen);
            APP_ERROR_CHECK(err);

            NRF_LOG_INFO("Current Firmware Version: %s", currentFirmwareRev);

            NRF_LOG_INFO("Software Update Available - Priority: %d Condition: %d Version: %s IntegrityType: %d URI: %s",
                                                                                aInParam.SoftwareUpdateAvailable.Priority,
                                                                                aInParam.SoftwareUpdateAvailable.Condition,
                                                                                aInParam.SoftwareUpdateAvailable.Version,
                                                                                aInParam.SoftwareUpdateAvailable.IntegrityType,
                                                                                aInParam.SoftwareUpdateAvailable.URI);

            break;
        }

        case SoftwareUpdateManager::kEvent_FetchPartialImageInfo:
        {
            NRF_LOG_INFO("Fetching Partial Image Information");
            if (strcmp(aInParam.FetchPartialImageInfo.URI, persistedImageURI) == 0)
            {
                NRF_LOG_INFO("Partial image detected in local storage; resuming download at offset %" PRId32, persistedImageLen);
                aOutParam.FetchPartialImageInfo.PartialImageLen = persistedImageLen;
            }
            else
            {
                NRF_LOG_INFO("No partial image detected in local storage");
                aOutParam.FetchPartialImageInfo.PartialImageLen = 0;
            }
            break;
        }

        case SoftwareUpdateManager::kEvent_PrepareImageStorage:
        {
            NRF_LOG_INFO("Preparing Image Storage");

            // Capture state information about the image being downloaded.
            persistedImageLen = 0;
            strncpy(persistedImageURI, aInParam.PrepareImageStorage.URI, sizeof(persistedImageURI));
            persistedImageURI[sizeof(persistedImageURI) - 1] = 0;

            // Prepare to compute the integrity of the image as it is received.
            //
            // This example does not actually store image blocks in persistent storage and merely discards
            // them after computing the SHA over it. As a result, integrity has to be computed as the image
            // blocks are received, rather than over the entire image at the end. This pattern is NOT
            // recommended since the computed integrity will be lost if the device rebooted during download.
            //
            sSHA256.Begin();

            // Tell the SoftwareUpdateManager that storage preparation has completed.
            SoftwareUpdateMgr().PrepareImageStorageComplete(WEAVE_NO_ERROR);
            break;
        }

        case SoftwareUpdateManager::kEvent_StartImageDownload:
        {
            NRF_LOG_INFO("Starting Image Download");
            break;
        }
        case SoftwareUpdateManager::kEvent_StoreImageBlock:
        {
            sSHA256.AddData(aInParam.StoreImageBlock.DataBlock, aInParam.StoreImageBlock.DataBlockLen);
            persistedImageLen += aInParam.StoreImageBlock.DataBlockLen;
            NRF_LOG_INFO("Image Download: %" PRId32 " bytes received", persistedImageLen);
            break;
        }

        case SoftwareUpdateManager::kEvent_ComputeImageIntegrity:
        {
            NRF_LOG_INFO("Computing image integrity");
            NRF_LOG_INFO("Total image length: %" PRId32, persistedImageLen);

            // Make sure that the buffer provided in the parameter is large enough.
            if (aInParam.ComputeImageIntegrity.IntegrityValueBufLen < sSHA256.kHashLength)
            {
                aOutParam.ComputeImageIntegrity.Error = WEAVE_ERROR_BUFFER_TOO_SMALL;
            }
            else
            {
                sSHA256.Finish(aInParam.ComputeImageIntegrity.IntegrityValueBuf);
            }
            break;
        }

        case SoftwareUpdateManager::kEvent_ResetPartialImageInfo:
        {
            // Reset the "persistent" state information related to the image being downloaded,
            // This ensures that the image will be re-downloaded in its entirety during the next
            // software update attempt.
            persistedImageLen = 0;
            persistedImageURI[0] = '\0';
            break;
        }

        case SoftwareUpdateManager::kEvent_ReadyToInstall:
        {
            NRF_LOG_INFO("Image is ready to be installed");
            break;
        }

        case SoftwareUpdateManager::kEvent_StartInstallImage:
        {
            AppTask *_this = static_cast<AppTask*>(apAppState);

            NRF_LOG_INFO("Image Install is not supported in this example application");

            AppEvent event;
            event.Type             = AppEvent::kEventType_Install;
            event.Handler          = InstallEventHandler;
            _this->PostEvent(&event);

            break;
        }

        case SoftwareUpdateManager::kEvent_Finished:
        {
            if (aInParam.Finished.Error == WEAVE_ERROR_NO_SW_UPDATE_AVAILABLE)
            {
                NRF_LOG_INFO("No Software Update Available");
            }
            else if (aInParam.Finished.Error == WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED)
            {
                NRF_LOG_INFO("Software Update Ignored by Application");
            }
            else if (aInParam.Finished.Error == WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED)
            {
                NRF_LOG_INFO("Software Update Aborted by Application");
            }
            else if (aInParam.Finished.Error != WEAVE_NO_ERROR || aInParam.Finished.StatusReport != NULL)
            {
                if (aInParam.Finished.Error == WEAVE_ERROR_STATUS_REPORT_RECEIVED)
                {
                    NRF_LOG_INFO("Software Update failed: Received StatusReport %s",
                                 nl::StatusReportStr(aInParam.Finished.StatusReport->mProfileId,
                                                     aInParam.Finished.StatusReport->mStatusCode));
                }
                else
                {
                    NRF_LOG_INFO("Software Update failed: %s", nl::ErrorStr(aInParam.Finished.Error));
                }
            }
            else
            {
                NRF_LOG_INFO("Software Update Completed");

                // Reset the "persistent" image state information.  Since we don't actually apply the
                // downloaded image, this ensures that the next software update attempt will re-download
                // the image.
                persistedImageLen = 0;
                persistedImageURI[0] = '\0';
            }
            break;
        }

        default:
            nl::Weave::DeviceLayer::SoftwareUpdateManager::DefaultEventHandler(apAppState, aEvent, aInParam, aOutParam);
            break;
    }
}
