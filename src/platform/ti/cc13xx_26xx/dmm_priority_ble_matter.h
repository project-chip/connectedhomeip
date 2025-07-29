/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
/*!****************************************************************************
 *  @file  dmm_priority_ble_matter.h
 *
 *  @brief      Global Priority Table
 *
 *  DMM enables devices to run multiple wireless protocol stacks concurrently.
 *  The DMMSch is to enable concurrent operation of multiple stacks with minimized conflicts
 *  so that it does not cause significant performance degradation.
 *  The DMMSch uses Application Level, Stack Level information, and Global Priority Table (GPT) when scheduling a command.
 *  Stack Level information (provided by stack) is embedded in each RF command and it includes:
 *  - Start Type, Start Time, AllowDelay, Priority, Activity, etc.
 *  Application Level information (provided by User via the Policy Table) includes:
 *  - Application State Name, Weight, AppliedActivity, Pause, etc.
 *  Global Priority Table (GPT)
 *  - GPT defines relative priorities of the two stacks
 *  - GPT consists of three parameters: Stack Activity, Priority of the activity (Normal, High, Urgent), and Global Priority Number
 *  - GPT input: Stack Activity  + Priority of the activity (Normal, High, Urgent)
 *  - GPT output: Global Priority Number
 *  Final Priority = GPT (Stack Activity  + Priority of the activity) +  .weight (in the Policy Table)
 *
 *  # GPT for BLE Stack + Thread Stack #
 *  ___________________________________________________________________________________________________
 *  | BLE Activity + Priority        | 15.4 Activity + Priority         | GTP value
 *  ____________________________________________________________________________________________________
 *  | Connect             - Normal   |                                  | 75
 *  ____________________________________________________________________________________________________
 *
 *  # GPT APIs are defined in the dmm_policy.c#
 *    - DMMPolicy_getGlobalPriority(): Get the global activity based on stack activity.
 *    - DMMPolicy_getDefaultPriority(): Get the default priority of the stack
 *    - DMMPolicy_getGPTStatus():  check if the global priority table is available
 *
 *
 * ********************************************************************************/
#ifndef dmm_priority_ble_thread__H_
#define dmm_priority_ble_thread__H_

#include <dmm_policy.h>

//! \brief The number of activities
#define ACTIVITY_NUM_BLE 4
#define ACTIVITY_NUM_THREAD 5

extern GlobalTable globalPriorityTable_bleLthreadH[DMMPOLICY_NUM_STACKS];

#endif // dmm_priority_ble_thread__H_
