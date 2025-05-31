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

/*
 * @class ClosureManager
 * @brief Manages the initialization and operations related to closure and
 *        closure panel endpoints in the application.
 *
 * @note This class is part of the closure application example
 */

#pragma once

#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"
#include <lib/core/DataModelTypes.h>

class ClosureManager
{
public:
    enum Action_t
    {
        CALIBRATE_ACTION = 0,
        STOP_ACTION      = 1,
        MOVE_TO_ACTION   = 2,
        INVALID_ACTION   = 3
    }Action;
    /**
     * @brief Initializes the ClosureManager.
     *
     * This method sets up the necessary resources and configurations required
     * for the Closure Application to function properly.
     */
    void Init();

    static ClosureManager & GetInstance() { return sClosureMgr; }
    void OnCalibrateCommand(chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime);
    void OnMoveToCommand(chip::app::DataModel::Nullable<chip::ElapsedS> & countdownTime);
    void OnStopCommand();
    
    chip::app::Clusters::ClosureControl::ClosureControlEndpoint ep1{ kClosureEndpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep2{ kClosurePanel1Endpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep3{ kClosurePanel2Endpoint };

private:
    static ClosureManager sClosureMgr;
    // Define the endpoint ID for the Closure
    static constexpr chip::EndpointId kClosureEndpoint       = 1;
    static constexpr chip::EndpointId kClosurePanel1Endpoint = 2;
    static constexpr chip::EndpointId kClosurePanel2Endpoint = 3;
    
    /**
     * @brief
     *  The callback function to be called when "movement timer" expires.
     */
    static void HandleCalibrateActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleStopActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleMoveToActionTimer(chip::System::Layer * layer, void * aAppState);
    
    void HandleClosureAction(ClosureManager::Action_t action);

    
};
