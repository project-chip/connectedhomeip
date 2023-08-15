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
    UNINITIALIZED, // Before Initialize() success
    NOT_RUNNING,   // After Initialize() success before Start()ing, OR After stop() success
    RUNNING,       // After Start() success
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
     * @param appParameters
     * @return CHIP_ERROR
     */
    CHIP_ERROR Initialize(const matter::casting::support::AppParameters & appParameters);

    /**
     * @brief Starts the Matter server that the CastingApp runs on and calls PostStartRegistrations() to finish starting up the
     * CastingApp.
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if Matter server started successfully, specific error code otherwise.
     */
    CHIP_ERROR Start();

    /**
     * @brief Perform post Matter server startup registrations
     *
     * @return CHIP_ERROR  - CHIP_NO_ERROR if all registrations succeeded, specific error code otherwise
     */
    CHIP_ERROR PostStartRegistrations();

    /**
     * @brief Stops the Matter server that the CastingApp runs on
     *
     * @return CHIP_ERROR - CHIP_NO_ERROR if Matter server stopped successfully, specific error code otherwise.
     */
    CHIP_ERROR Stop();

private:
    CastingApp();
    static CastingApp * _castingApp;

    CastingApp(CastingApp & other) = delete;
    void operator=(const CastingApp &) = delete;

    const matter::casting::support::AppParameters * mAppParameters;

    CastingAppState mState = UNINITIALIZED;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
