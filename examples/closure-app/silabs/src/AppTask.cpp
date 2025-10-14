/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    Copyright (c) 2025 Google LLC.
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
#include "LEDWidget.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <ClosureManager.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#define APP_FUNCTION_BUTTON 0
#define APP_CLOSURE_BUTTON 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
using namespace ::chip::DeviceLayer::Internal;
using namespace chip::TLV;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

static chip::BitMask<Feature> sFeatureMap(Feature::kCalibration);

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Closure-App");
#endif

    // Initialization of Closure Manager and endpoints of closure and closurepanel.
    ClosureManager::GetInstance().Init();

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(false);
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

    SILABS_LOG("App Task started");

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;
    button_event.Handler            = BaseApplication::ButtonHandler;
    AppTask::GetAppTask().PostEvent(&button_event);

    if (button == APP_CLOSURE_BUTTON && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = ClosureButtonActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::ClosureButtonActionEventHandler(AppEvent * aEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        // Check if an action is already in progress
        if (ClosureManager::GetInstance().IsClosureControlMotionInProgress())
        {
            SILABS_LOG("Action is already in progress or active.");
            // Stop the current action
            auto status = ClosureManager::GetInstance().OnStopCommand();
            if (status != chip::Protocols::InteractionModel::Status::Success)
            {
                SILABS_LOG("Failed to stop closure action");
            }
        }
        else
        {
            // Fetch the complete current state of closure with proper locking
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            
            chip::app::DataModel::Nullable<chip::app::Clusters::ClosureControl::GenericOverallCurrentState> currentState;
            CHIP_ERROR err = ClosureManager::GetInstance().GetClosureControlCurrentState(currentState);
            
            if (err != CHIP_NO_ERROR || currentState.IsNull() || 
                !currentState.Value().position.HasValue() || currentState.Value().position.Value().IsNull())
            {
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                SILABS_LOG("Failed to get current closure state");
                return;
            }

            // Get current position and determine target position (toggle)
            auto currentPosition = currentState.Value().position.Value().Value();
            chip::app::Clusters::ClosureControl::TargetPositionEnum targetPosition = 
                (currentPosition == chip::app::Clusters::ClosureControl::CurrentPositionEnum::kFullyOpened)
                    ? chip::app::Clusters::ClosureControl::TargetPositionEnum::kMoveToFullyClosed
                    : chip::app::Clusters::ClosureControl::TargetPositionEnum::kMoveToFullyOpen;

            // Get latch and speed from current state to preserve them in target state
            chip::Optional<bool> latch = chip::NullOptional;
            if (currentState.Value().latch.HasValue() && !currentState.Value().latch.Value().IsNull())
            {
                latch = chip::MakeOptional(currentState.Value().latch.Value().Value());
            }

            chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed = chip::NullOptional;
            if (currentState.Value().speed.HasValue())
            {
                speed = chip::MakeOptional(currentState.Value().speed.Value());
            }

            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            
            // Move to the target position with preserved latch and speed values
            auto status = ClosureManager::GetInstance().OnMoveToCommand(chip::MakeOptional(targetPosition), latch, speed);
            if (status != chip::Protocols::InteractionModel::Status::Success)
            {
                SILABS_LOG("Failed to move closure to target position");
            }
        }
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
        SILABS_LOG("Unhandled event type in ClosureButtonActionEventHandler");
    }
}
