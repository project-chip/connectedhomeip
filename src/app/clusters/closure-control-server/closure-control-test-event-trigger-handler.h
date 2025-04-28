/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

 #pragma once

 #include <app-common/zap-generated/cluster-objects.h>
 #include <app/TestEventTriggerDelegate.h>
 
 /**
  * @brief User handler for handling the test event trigger
  *
  * @note If TestEventTrigger is enabled, it needs to be implemented in the app
  *
  * @param eventTrigger Event trigger to handle
  *
  * @retval true on success
  * @retval false if error happened
  */
 bool HandleClosureControlTestEventTrigger(uint64_t eventTrigger);
 
namespace chip {
 
 /*
  * These Test EventTrigger values are specified in the TC_EEVSE test plan
  * and are defined conditions used in test events.
  *
  * They are sent along with the enableKey (manufacturer defined secret)
  * in the General Diagnostic cluster TestEventTrigger command
  */
 enum class ClosureControlTrigger : uint64_t
 {
    // MainState is SetupRequired(7) Test Event | Simulate that the device is in SetupRequired state
    kMainStateIsSetupReuired = 0x0104000000000000,
    
    // MainState is Protected(5) Test Event | Simulate that the device is in protected state
    kMainStateIsProtected = 0x0104000000000001,

    // MainState is Disengaged(6) Test Event | Simulate that the device is in disengaged state
    kMainStateIsDisengaged = 0x0104000000000002,
    
    // MainState Test Event Clear 
    kEventClear = 0x0104000000000003,
    
    //MainState is Error(3) Test Event | Simulate that the device is in error state, add at least one element to the CurrentErrorList attribute
    kMainStateIsError = 0x0104000000000004,
 };
 
 class ClosureControlTestEventTriggerHandler : public TestEventTriggerHandler
 {
 public:
     explicit ClosureControlTestEventTriggerHandler() {}
 
     /** This function must return True if the eventTrigger is recognised and handled
      *  It must return False to allow a higher level TestEvent handler to check other
      *  clusters that may handle it.
      */
     CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
     {
         if (HandleClosureControlTestEventTrigger(eventTrigger))
         {
             return CHIP_NO_ERROR;
         }
         return CHIP_ERROR_INVALID_ARGUMENT;
     }
 };

} // namespace chip