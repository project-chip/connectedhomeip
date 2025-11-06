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
#include "ClosureUI.h"
#include "ClosureUIStrings.h"
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
#include <stdio.h>

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
    GetLCD().SetCustomUI(ClosureUI::DrawUI);
#endif

    // Initialization of Closure Manager and endpoints of closure and closurepanel.
    ClosureManager::GetInstance().Init();

// Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    UpdateClosureUI();
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
#endif // DISPLAY_ENABLED

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
}

#ifdef DISPLAY_ENABLED
void AppTask::UpdateClosureUI()
{
    ClosureManager & closureManager = ClosureManager::GetInstance();

    DeviceLayer::PlatformMgr().LockChipStack();
    auto uiData = closureManager.GetClosureUIData();
    DeviceLayer::PlatformMgr().UnlockChipStack();

    ClosureUI::SetMainState(uiData.mainState);

    const char * positionSuffix = ClosureUIStrings::POSITION_SUFFIX_UNKNOWN;
    if (!uiData.overallCurrentState.IsNull() && uiData.overallCurrentState.Value().position.HasValue() &&
        !uiData.overallCurrentState.Value().position.Value().IsNull())
    {
        switch (uiData.overallCurrentState.Value().position.Value().Value())
        {
        case chip::app::Clusters::ClosureControl::CurrentPositionEnum::kFullyClosed:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_CLOSED;
            break;
        case chip::app::Clusters::ClosureControl::CurrentPositionEnum::kFullyOpened:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_OPEN;
            break;
        case chip::app::Clusters::ClosureControl::CurrentPositionEnum::kPartiallyOpened:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_PARTIAL;
            break;
        case chip::app::Clusters::ClosureControl::CurrentPositionEnum::kOpenedForPedestrian:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_PEDESTRIAN;
            break;
        case chip::app::Clusters::ClosureControl::CurrentPositionEnum::kOpenedForVentilation:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_VENTILATION;
            break;
        default:
            positionSuffix = ClosureUIStrings::POSITION_SUFFIX_UNKNOWN;
            break;
        }
    }
    ClosureUI::FormatAndSetPosition(positionSuffix);

    const char * latchSuffix = ClosureUIStrings::LATCH_SUFFIX_UNKNOWN;
    if (!uiData.overallCurrentState.IsNull() && uiData.overallCurrentState.Value().latch.HasValue() &&
        !uiData.overallCurrentState.Value().latch.Value().IsNull())
    {
        latchSuffix = uiData.overallCurrentState.Value().latch.Value().Value() ? ClosureUIStrings::LATCH_SUFFIX_ENGAGED
                                                                               : ClosureUIStrings::LATCH_SUFFIX_RELEASED;
    }
    ClosureUI::FormatAndSetLatch(latchSuffix);

    const char * secureSuffix = ClosureUIStrings::SECURE_SUFFIX_UNKNOWN;
    if (!uiData.overallCurrentState.IsNull() && !uiData.overallCurrentState.Value().secureState.IsNull())
    {
        secureSuffix = uiData.overallCurrentState.Value().secureState.Value() ? ClosureUIStrings::SECURE_SUFFIX_YES
                                                                              : ClosureUIStrings::SECURE_SUFFIX_NO;
    }
    ClosureUI::FormatAndSetSecure(secureSuffix);

    const char * speedSuffix = ClosureUIStrings::SPEED_SUFFIX_UNKNOWN;
    if (!uiData.overallCurrentState.IsNull() && uiData.overallCurrentState.Value().speed.HasValue())
    {
        switch (uiData.overallCurrentState.Value().speed.Value())
        {
        case chip::app::Clusters::Globals::ThreeLevelAutoEnum::kLow:
            speedSuffix = ClosureUIStrings::SPEED_SUFFIX_LOW;
            break;
        case chip::app::Clusters::Globals::ThreeLevelAutoEnum::kMedium:
            speedSuffix = ClosureUIStrings::SPEED_SUFFIX_MEDIUM;
            break;
        case chip::app::Clusters::Globals::ThreeLevelAutoEnum::kHigh:
            speedSuffix = ClosureUIStrings::SPEED_SUFFIX_HIGH;
            break;
        case chip::app::Clusters::Globals::ThreeLevelAutoEnum::kAuto:
            speedSuffix = ClosureUIStrings::SPEED_SUFFIX_AUTO;
            break;
        default:
            speedSuffix = ClosureUIStrings::SPEED_SUFFIX_UNKNOWN;
            break;
        }
    }
    ClosureUI::FormatAndSetSpeed(speedSuffix);

#ifdef SL_WIFI
    if (ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        AppTask::GetAppTask().GetLCD().WriteDemoUI(false); // State doesn't matter for custom UI
    }
}
#endif // DISPLAY_ENABLED
