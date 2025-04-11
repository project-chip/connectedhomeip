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

#include <ClosureAppCommonMain.h>
#include <ClosureDimensionManager.h>
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
#define APP_ClOSURE_BUTTON 1

namespace {

constexpr chip::EndpointId kClosureBaseEndpoint = 1;

} // namespace

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureControl::Attributes;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
using namespace ::chip::DeviceLayer::Internal;
using namespace chip::TLV;
using chip::app::Clusters::ClosureDimension::ClosureDimensionManager;
using chip::app::Clusters::ClosureDimension::ClosureDimensionDelegate;

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
ClosureDimensionManager ep2(2);
// ClosureDimensionManager ep3(3);
constexpr const uint8_t kNamespaceClosurePanel = 0x45;
constexpr const uint8_t kNamespaceLift         = 0x00;
constexpr const uint8_t kNamespaceTilt         = 0x01;

EndpointId GetClosureDeviceEndpointId()
{
    return kClosureBaseEndpoint;
}

void ApplicationInit()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    SILABS_LOG("==================================================");
    SILABS_LOG("Closure-app ClosureControl starting. featureMap 0x%08lx", ClosureControl::sFeatureMap.Raw());
    ClosureApplicationInit();
    SILABS_LOG("==================================================");

    SILABS_LOG("Closure-app Dimesnion Manager updated starting");

    const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp2TagList[] = {
        { .namespaceID = kNamespaceClosurePanel,
          .tag         = kNamespaceLift,
          .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
    };
    // const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp3TagList[] = {
    //     { .namespaceID = kNamespaceClosurePanel,
    //       .tag         = kNamespaceTilt,
    //       .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
    // };

    ep2.Init();
    // ep3.Init();

    ep2.getDelegate().SetCallbacks(AppTask::ActionInitiated, AppTask::ActionCompleted);
    // ep3.getDelegate().SetCallbacks(AppTask::ActionInitiated, AppTask::ActionCompleted);

    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gEp2TagList));
    // SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gEp3TagList));

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void AppTask::ActionInitiated(ClosureDimensionDelegate::Action_t action)
{
    SILABS_LOG("==================================================");
    SILABS_LOG("ActionInitiated");
    SILABS_LOG("==================================================");
    // Action initiated,
    if (action == ClosureDimensionDelegate::MOVE_ACTION)
    {
        SILABS_LOG("Starting Motion");
    }
    else if (action == ClosureDimensionDelegate::STEP_ACTION)
    {
        SILABS_LOG("Starting Step");
    }
    else if (action == ClosureDimensionDelegate::MOVE_AND_LATCH_ACTION)
    {
        SILABS_LOG("Starting Motion and Latch");
    }
    else if (action == ClosureDimensionDelegate::TARGET_CHANGE_ACTION)
    {
        SILABS_LOG("Starting Target Change");
    }
    else
    {
        SILABS_LOG("Invalid Action");
    }

}

void AppTask::ActionCompleted(ClosureDimensionDelegate::Action_t action)
{
    SILABS_LOG("==================================================");
    SILABS_LOG("ActionCompleted");
    SILABS_LOG("==================================================");
    // Action initiated,
    if (action == ClosureDimensionDelegate::MOVE_ACTION)
    {
        SILABS_LOG("Motion completed");
    }
    else if (action == ClosureDimensionDelegate::STEP_ACTION)
    {
        SILABS_LOG("Step completed");
    }
    else if (action == ClosureDimensionDelegate::MOVE_AND_LATCH_ACTION)
    {
        SILABS_LOG("Motion and Latch completed");
    }
    else if (action == ClosureDimensionDelegate::TARGET_CHANGE_ACTION)
    {
        SILABS_LOG("Target Change completed");
    }
    else
    {
        SILABS_LOG("Invalid Action");
    }

}

void ApplicationShutdown()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    ClosureApplicationShutdown();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Closure-App");
#endif

    ApplicationInit();

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
}
