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

// Threshold where it becomes valuable to emit a report before entering idle mode
#ifndef ICD_REPORT_BEFORE_IDLE_THRESHOLD_MS
#define ICD_REPORT_BEFORE_IDLE_THRESHOLD_MS (CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL * 0.75)
#endif

// Time before the end of active mode when to check if a report should be emitted, this aims to provide enough time to emit the
// report befor ethe active period is over. If it is physically impossible to check and emit the report before the end of the active
// mode, this can be left at 0.
#ifndef ICD_ACTIVE_TIME_JITTER_MS
#define ICD_ACTIVE_TIME_JITTER_MS 100
#endif

namespace chip {
namespace app {

class ICDStateObserver
{
public:
    virtual ~ICDStateObserver() {}
    virtual void OnEnterActiveMode()  = 0;
    virtual void OnTransitionToIdle() = 0;
};

} // namespace app
} // namespace chip
