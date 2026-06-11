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

#include "av-analysis-manager.h"
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AvAnalysis;
using namespace chip::app::Clusters::AvAnalysis::Structs;

    /**
     * Allows the delegate to perform any specific functions such as timer cancellation on a shutdown, this is invoked prior to
     * the destructor, it shall not be invoked as part of the destructor.
     */
    void AvAnalysisManager::ShutdownApp() 
    {}

    /**
     * Delegate command handlers
     */

    /**
     */
    Protocols::InteractionModel::Status
    AvAnalysisManager::EnableContextTriggers() 
    {
        return Protocols::InteractionModel::Status::Success;
    }

    /**
     */
    Protocols::InteractionModel::Status
    AvAnalysisManager::DisableContextTriggers() 
    {
        return Protocols::InteractionModel::Status::Success;
    }

    /**
     */
    Protocols::InteractionModel::Status
    AvAnalysisManager::EstablishAnalysisStream() 
    {
        return Protocols::InteractionModel::Status::Success;
    }

    /**
     */
    Protocols::InteractionModel::Status AvAnalysisManager::ActivateAnalysisStream() 
    {
        return Protocols::InteractionModel::Status::Success;
    }

    /**
     */
    Protocols::InteractionModel::Status AvAnalysisManager::DeactivateAnalysisStream() 
    {
        return Protocols::InteractionModel::Status::Success;        
    }

    /**
     */
    Protocols::InteractionModel::Status AvAnalysisManager::RemoveAnalysisStream() 
    {
        return Protocols::InteractionModel::Status::Success;        
    }
    
    /**
     * Delegate command assists
     */
    CHIP_ERROR AvAnalysisManager::VerifyZoneIDsAreValid(DataModel::DecodableList<uint16_t>)
    {
        return CHIP_NO_ERROR;
    }
    
    bool AvAnalysisManager::CanAddContextTriggers() {return true; }
    
    void AvAnalysisManager::ActiveAmbientContextTriggersUpdated() {}
