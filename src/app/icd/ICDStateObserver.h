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

#ifndef ICD_SLEEP_TIME_JITTER_MS
#define ICD_SLEEP_TIME_JITTER_MS (CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC * 0.75)
#endif

#ifndef ICD_ACTIVE_TIME_JITTER_MS
#define ICD_ACTIVE_TIME_JITTER_MS 300
#endif

namespace chip {
namespace app {

class ICDStateObserver
{
public:
    virtual ~ICDStateObserver() {}
    virtual void OnEnterActiveMode()  = 0;
    virtual void OnTransitionToIdle() = 0;
    virtual void OnICDModeChange()    = 0;
};

} // namespace app
} // namespace chip
