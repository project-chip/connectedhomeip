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

#include "ClosureManager.h"
#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
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
    
    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate != nullptr)
    {
        VerifyOrDie(pTestEventDelegate->AddHandler(&ep1.GetDelegate()) == CHIP_NO_ERROR);
    }

    DeviceLayer::PlatformMgr().UnlockChipStack();
}

void ClosureManager::HandleCalibrateActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    ChipLogError(AppServer, "############HandleCalibrateActionTimer 1###############");
    if (manager != nullptr)
    {
        ChipLogError(AppServer, "############HandleCalibrateActionTimer 2###############");
        manager->HandleClosureAction(ClosureManager::Action_t::CALIBRATE_ACTION);
        ChipLogError(AppServer, "############HandleCalibrateActionTimer 3###############");
    }
    else
    {
        ChipLogError(AppServer, "HandleCalibrateActionTimer called with null manager");
    }
}

void ClosureManager::HandleStopActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    ChipLogError(AppServer, "############HandleStopActionTimer 1###############");
    if (manager != nullptr)
    {
        ChipLogError(AppServer, "############HandleStopActionTimer 2###############");
        manager->HandleClosureAction(ClosureManager::Action_t::STOP_ACTION);
        ChipLogError(AppServer, "############HandleStopActionTimer 3###############");
    }
    else
    {
        ChipLogError(AppServer, "HandleStopActionTimer called with null manager");
    }
}

void ClosureManager::HandleMoveToActionTimer(System::Layer * layer, void * aAppState)
{
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);
    ChipLogError(AppServer, "############HandleMoveToActionTimer 1###############");
    if (manager != nullptr)
    {
        ChipLogError(AppServer, "############HandleMoveToActionTimer 2###############");
        manager->HandleClosureAction(ClosureManager::Action_t::MOVE_TO_ACTION);
        ChipLogError(AppServer, "############HandleMoveToActionTimer 3###############");
    }
    else
    {
        ChipLogError(AppServer, "HandleMoveToActionTimer called with null manager");
    }
}

void ClosureManager::HandleClosureAction(ClosureManager::Action_t action)
{
  
  ChipLogError(AppServer, "############HandleClosureAction 1###############");
  ClosureManager & instance = ClosureManager::GetInstance();
  ChipLogError(AppServer, "############HandleClosureAction 2###############");
  switch (action)
  {
    case ClosureManager::Action_t::CALIBRATE_ACTION:
    {
      // Perform hardware calibration
      ChipLogError(AppServer, "############HandleClosureAction 3###############");
      instance.ep1.OnActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
      ChipLogError(AppServer, "############HandleClosureAction 4###############");
            break;
    }
    case ClosureManager::Action_t::STOP_ACTION:
    {
      // Perform hardware stop action
      ChipLogError(AppServer, "############HandleClosureAction 5###############");
      instance.ep1.OnActionComplete(ClosureManager::Action_t::STOP_ACTION);
      ChipLogError(AppServer, "############HandleClosureAction 6###############");
      break;
    }
    case ClosureManager::Action_t::MOVE_TO_ACTION:
    {
            // Perform hardware move to action
            ChipLogError(AppServer, "############HandleClosureAction 7###############");
            instance.ep1.OnActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
            ChipLogError(AppServer, "############HandleClosureAction 8###############");
            break;
    }
    case ClosureManager::Action_t::INVALID_ACTION:
            ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
            break;
        default:
            break;
  }
}

void ClosureManager::OnCalibrateCommand(DataModel::Nullable<ElapsedS> & countdownTime)
{
  ChipLogError(AppServer, "###########OnCalibrateCommand###################");
  DeviceLayer::SystemLayer().CancelTimer(HandleCalibrateActionTimer, this);
  DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(30), HandleCalibrateActionTimer, this);
}

void ClosureManager::OnStopCommand()
{
  ChipLogError(AppServer, "###########OnStopCommand###################");
  DeviceLayer::SystemLayer().CancelTimer(HandleStopActionTimer, this);
  DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(30), HandleStopActionTimer, this);
}

void ClosureManager::OnMoveToCommand(chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime)
{
    ChipLogError(AppServer, "###########OnMoveToCommand###################");
    DeviceLayer::SystemLayer().CancelTimer(HandleMoveToActionTimer, this);
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(30), HandleMoveToActionTimer, this);
}
