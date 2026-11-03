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

class CameraDeviceInterface;

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

class AvAnalysisManager : public AvAnalysisDelegate
{

public:
    AvAnalysisManager()  = default;
    ~AvAnalysisManager() = default;

    /**
     * Allows the delegate to perform any specific functions such as timer cancellation on a shutdown, this is invoked prior to
     * the destructor, it shall not be invoked as part of the destructor.
     */
    virtual void ShutdownApp() override;

    /**
     * Delegate command assists
     */
    virtual CHIP_ERROR VerifyZoneIDsAreValid(const std::vector<uint16_t> & aZoneIDs) override;

    virtual bool CanAddContextTriggers() override;

    void ActiveAmbientContextTriggersUpdated() override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    /**
     * Camera App interface
     */
    void SetCameraDevice(CameraDeviceInterface * aCameraDevice) { mCameraDevice = aCameraDevice; }

    /**
     * Context event detection handling
     */
    void OnAmbientContextTriggeredEvent(uint8_t namespaceId, uint8_t tagId,
                                        Optional<DataModel::Nullable<std::vector<uint16_t>>> zoneIds, uint16_t identifiedContextId,
                                        bool & triggeredContextEnabled);

    // Simulation triggers for app-pipe commands
    CHIP_ERROR TriggerSessionStart(const std::vector<uint16_t> & aZoneIds, bool aZoneIdsNull = false,
                                   chip::Optional<chip::NodeId> aSourceNodeId = chip::NullOptional);

    CHIP_ERROR TriggerPerceivedContext(const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aNewContexts,
                                       const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aExpiredContexts,
                                       chip::Optional<uint16_t> aSessionId        = chip::NullOptional,
                                       chip::Optional<chip::NodeId> aSourceNodeId = chip::NullOptional);

    CHIP_ERROR TriggerSessionEnd(chip::Optional<uint16_t> aSessionId = chip::NullOptional);

    uint16_t GetLatestSessionId() const { return mLatestSessionId; }
    bool HasActiveSession() const { return mHasActiveSession; }

private:
    CameraDeviceInterface * mCameraDevice = nullptr;
    uint16_t mLatestSessionId             = 0;
    bool mHasActiveSession                = false;
    bool mSessionHasTrackedContexts       = false;
};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
