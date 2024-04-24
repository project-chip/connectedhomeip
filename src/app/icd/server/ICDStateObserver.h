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

/**
 * @brief Public API used by the ICDManager to expose when different events occur.
 *        ICDManager::RegisterObserver can be used to register as an Observer to be notified when these events occur.
 *        These functions are called synchronously.
 */
class ICDStateObserver
{
public:
    virtual ~ICDStateObserver() {}

    /**
     * @brief API called when the ICD enters ActiveMode. API isn't called if we need to extend the remaining active mode timer
     * duration. API is called after the ICDManager has finished executing its internal actions.
     */
    virtual void OnEnterActiveMode() = 0;

    /**
     * @brief API called when the ICD enters IdleMode.
     *        API is called after the ICDManager has finished executing its internal actions.
     */
    virtual void OnEnterIdleMode() = 0;

    /**
     * @brief API is called when the ICD is about to enter IdleMode. API is called when there is `ICD_ACTIVE_TIME_JITTER_MS` of time
     *        remaining to the active mode timer.
     *        This API is only called once per transition from ActiveMode to IdleMode.
     *        If OnTransitionToIdle triggers the active mode timer to increase, the next time we are about to enter IdleMode,
     *        this API will not be called.
     */
    virtual void OnTransitionToIdle() = 0;

    /**
     * @brief API is called when the ICD changes operating mode. This API is only called if the ICD changes state, not when it
     *        remains in the same state.
     *        API is called after the ICDManager has finished executing its internal actions.
     */
    virtual void OnICDModeChange() = 0;
};

} // namespace app
} // namespace chip
