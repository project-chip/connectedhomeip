/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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


#include <app/clusters/closure-control-server/closure-control-test-event-trigger-handler.h>
#include "ClosureControlEndpoint.h"
#include <lib/support/CodeUtils.h>
 
 using namespace chip;
 using namespace chip::app;
 using namespace chip::app::Clusters;
 using namespace chip::app::Clusters::ClosureControl;
 
 namespace {
 
 void SetTestEventTrigger_MainStateIsSetupReuired()
 {
    ClosureControlDelegate &delegate  = ClosureControlEndpoint::sClosureCntrlEndpoint.GetDelegate();
    delegate.GetLogic()->SetMainState(MainStateEnum::kSetupRequired);
 }
 
 void SetTestEventTrigger_MainStateIsProtected()
 {
    ClosureControlDelegate &delegate  = ClosureControlEndpoint::sClosureCntrlEndpoint.GetDelegate();
    delegate.GetLogic()->SetMainState(MainStateEnum::kProtected);
 }
 
 void SetTestEventTrigger_MainStateIsError()
 {
    ClosureControlDelegate &delegate  = ClosureControlEndpoint::sClosureCntrlEndpoint.GetDelegate();
    delegate.GetLogic()->SetMainState(MainStateEnum::kError);
 }
 
 void SetTestEventTrigger_MainStateIsDisengaged()
 {
    ClosureControlDelegate &delegate  = ClosureControlEndpoint::sClosureCntrlEndpoint.GetDelegate();
    delegate.GetLogic()->SetMainState(MainStateEnum::kDisengaged);
 }
 
 void SetTestEventTrigger_ClearErrors()
 {
    // TODO: Clear Error List
 }
 
} // namespace 
 
 bool HandleClosureControlTestEventTrigger(uint64_t eventTrigger)
 {
     ClosureControlTrigger trigger = static_cast<ClosureControlTrigger>(eventTrigger);
 
     switch (trigger)
     {
     case ClosureControlTrigger::kMainStateIsSetupReuired:
        SetTestEventTrigger_MainStateIsSetupReuired();
        ChipLogProgress(Support, "[MainState-Test-Event] => Simulate that the device is in SetupRequired state");
        break;
    case ClosureControlTrigger::kMainStateIsProtected:
        SetTestEventTrigger_MainStateIsProtected();
        ChipLogProgress(Support, "[MainState-Test-Event] => Simulate that the device is in Protected state");
        break;
        case ClosureControlTrigger::kMainStateIsDisengaged:
        SetTestEventTrigger_MainStateIsDisengaged();
        ChipLogProgress(Support, "[MainState-Test-Event] => Simulate that the device is in Disengaged state");
        break;
        case ClosureControlTrigger::kEventClearErrors:
        SetTestEventTrigger_ClearErrors();
        ChipLogProgress(Support, "[MainState-Test-Event] => Simulate that the device to clear all errors");
        break;
        case ClosureControlTrigger::kMainStateIsError:
        SetTestEventTrigger_MainStateIsError();
        ChipLogProgress(Support, "[MainState-Test-Event] => Simulate that the device is in Error state");
        break;
         
     default:
         return false;
     }
 
     return true;
 }
 