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

#pragma once
#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

class AvAnalysisManager : public AvAnalysisDelegate
{

public:
    AvAnalysisManager() = default;
    ~AvAnalysisManager() = default;

    /**
     * Allows the delegate to perform any specific functions such as timer cancellation on a shutdown, this is invoked prior to
     * the destructor, it shall not be invoked as part of the destructor.
     */
    virtual void ShutdownApp() override;

    /**
     * Delegate command handlers
     */

    /**
     */
    virtual Protocols::InteractionModel::Status
    EnableContextTriggers() override;

    /**
     */
    virtual Protocols::InteractionModel::Status
    DisableContextTriggers() override;

    /**
     */
    virtual Protocols::InteractionModel::Status
    EstablishAnalysisStream() override;

    /**
     */
    virtual Protocols::InteractionModel::Status ActivateAnalysisStream() override;

    /**
     */
    virtual Protocols::InteractionModel::Status DeactivateAnalysisStream() override;

    /**
     */
    virtual Protocols::InteractionModel::Status RemoveAnalysisStream() override;
    
    /**
     * Delegate command assists
     */
    
    virtual CHIP_ERROR VerifyZoneIDsAreValid(DataModel::DecodableList<uint16_t>) override;
    
    virtual bool CanAddContextTriggers() override;
    
    void ActiveAmbientContextTriggersUpdated() override;
        
private:

};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
