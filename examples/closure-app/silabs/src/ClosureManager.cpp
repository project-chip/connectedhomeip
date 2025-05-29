/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <AppTask.h>
#include "ClosureManager.h"
#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"
#include "cmsis_os2.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {

// Define the Namespace and Tag for the endpoint
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure.adoc
constexpr uint8_t kNamespaceClosure   = 0x44;
constexpr uint8_t kTagClosureCovering = 0x00;
// Derived from
// https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure-Covering.adoc
constexpr uint8_t kNamespaceCovering   = 0x46;
constexpr uint8_t kTagCoveringVenetian = 0x03;
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-ClosurePanel.adoc
constexpr uint8_t kNamespaceClosurePanel = 0x45;
constexpr uint8_t kTagClosurePanelLift   = 0x00;
constexpr uint8_t kTagClosurePanelTilt   = 0x01;

// Define the list of semantic tags for the endpoint
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp3TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelTilt,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
};

} // namespace

ClosureManager ClosureManager::sClosureMgr;

void ClosureManager::Init()
{
    DeviceLayer::PlatformMgr().LockChipStack();

    // Closure endpoints initialization
    ep1.Init();
    ep2.Init();
    ep3.Init();

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp1TagList));
    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp2TagList));
    SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp3TagList));

    DeviceLayer::PlatformMgr().UnlockChipStack();
}

void ClosureManager::OnCalibrateCommand(DataModel::Nullable<ElapsedS> & countdownTime)
{
  ChipLogError(AppServer, "###########OnCalibrateCommand###################");
  osDelay(1000);
    AppEvent event;
    event.Type    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = Action_t::CALIBRATE_ACTION;
    event.Handler = HandleClosureAction;
    AppTask::GetAppTask().PostEvent(&event);
}

void ClosureManager::OnStopCommand()
{
  ChipLogError(AppServer, "###########OnStopCommand###################");
  osDelay(1000);
    AppEvent event;
    event.Type    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = Action_t::STOP_ACTION;
    event.Handler = HandleClosureAction;
    AppTask::GetAppTask().PostEvent(&event);
}

void ClosureManager::OnMoveToCommand(chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime)
{
    ChipLogError(AppServer, "###########OnMoveToCommand###################");
    osDelay(1000);
    AppEvent event;
    event.Type    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = Action_t::MOVE_TO_ACTION;
    event.Handler = HandleClosureAction;
    AppTask::GetAppTask().PostEvent(&event);
}

void ClosureManager::HandleClosureAction(AppEvent * aEvent)
{
  ChipLogError(AppServer, "############HandleClosureAction 1###############"); 
  osDelay(1000);
    if (aEvent->Type == AppEvent::kEventType_Closure)
    {
      ChipLogError(AppServer, "############HandleClosureAction 2###############"); 
      osDelay(1000);
        switch (aEvent->ClosureEvent.Action)
        {
        case Action_t::CALIBRATE_ACTION:
        {
            // Perform hardware calibration
            ChipLogError(AppServer, "############HandleClosureAction 3###############"); 
            osDelay(1000);
            chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { GetInstance().ep1.OnActionComplete(Action_t::CALIBRATE_ACTION); });
            ChipLogError(AppServer, "############HandleClosureAction 4###############"); 
            osDelay(1000);
            break;
        }
        case Action_t::STOP_ACTION:
        {
            // Perform hardware stop action
            ChipLogError(AppServer, "############HandleClosureAction 5###############"); 
            osDelay(1000);
            chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { GetInstance().ep1.OnActionComplete(Action_t::STOP_ACTION); });
            ChipLogError(AppServer, "############HandleClosureAction 6###############"); 
            osDelay(1000);
            break;
        }
        case Action_t::MOVE_TO_ACTION:
        {
            // Perform hardware move to action
            ChipLogError(AppServer, "############HandleClosureAction 7###############"); 
            osDelay(1000);
            chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { GetInstance().ep1.OnActionComplete(Action_t::MOVE_TO_ACTION); });
            ChipLogError(AppServer, "############HandleClosureAction 8###############"); 
            osDelay(1000);
            break;
        }
        case Action_t::INVALID_ACTION:
            ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
            break;
        default:
            break;
        }
    }
}