/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include "support/AppParameters.h"

namespace matter {
namespace casting {
namespace core {

/**
 * @brief Represents CastingApp state.
 *
 */
enum CastingAppState
{
    CASTING_APP_UNINITIALIZED, // Before Initialize() success
    CASTING_APP_NOT_RUNNING,   // After Initialize() success before Start()ing, OR After stop() success
    CASTING_APP_RUNNING,       // After Start() success
};

/**
 * @brief CastingApp represents an app that can cast content to a Casting Player.
 * This class is a singleton.
 */
class CastingApp
{
public:
    static CastingApp * GetInstance();

    /**
     * @brief Initializes the CastingApp with appParameters
     *
     * @param appParameters AppParameters required to Start up the CastingApp
     * @return CHIP_ERROR
     */
    CHIP_ERROR Initialize(const matter::casting::support::AppParameters & appParameters);

    /**
     * @brief Starts the Matter server that the CastingApp runs on and registers all the necessary delegates
     * CastingApp.
     * If the CastingApp was previously connected to a CastingPlayer and then Stopped by calling the Stop()
     * API, it will re-connect to the CastingPlayer.
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if Matter server started successfully, specific error code otherwise.
     */
    CHIP_ERROR Start();

    /**
     * @brief Stops the Matter server that the CastingApp runs on.
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if Matter server stopped successfully, specific error code otherwise.
     */
    CHIP_ERROR Stop();

    /**
     * @return true, if CastingApp is in CASTING_APP_RUNNING state. false otherwise
     */
    bool isRunning() { return mState == CASTING_APP_RUNNING; }

    /**
     * @brief Tears down all active subscriptions.
     */
    CHIP_ERROR ShutdownAllSubscriptions();

    /**
     * @brief Clears app cache that contains the information about CastingPlayers previously connected to
     */
    CHIP_ERROR ClearCache();

private:
    CastingApp();
    static CastingApp * _castingApp;

    CastingApp(CastingApp & other)     = delete;
    void operator=(const CastingApp &) = delete;

    /**
     * @brief Perform post Matter server startup registrations
     *
     * @return CHIP_ERROR  - CHIP_NO_ERROR if all registrations succeeded, specific error code otherwise
     */
    CHIP_ERROR PostStartRegistrations();

    const matter::casting::support::AppParameters * mAppParameters;

    CastingAppState mState = CASTING_APP_UNINITIALIZED;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
