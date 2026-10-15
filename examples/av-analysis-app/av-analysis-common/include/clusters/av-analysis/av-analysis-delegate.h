/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdint>
#include <map>
#include <vector>

#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <lib/core/Optional.h>

#include "webrtc-peer-controller/webrtc-peer-controller.h"

namespace chip {
namespace app {

/**
 * Application delegate of the AV Analysis cluster
 */
class AvAnalysisNodeDelegate : public Clusters::AvAnalysisDelegate
{
public:
    void Init(WebRTCPeerController * aPeerController) { mPeerController = aPeerController; }

    // Clusters::AvAnalysisDelegate
    void ShutdownApp() override {}
    CHIP_ERROR VerifyZoneIDsAreValid(const std::vector<uint16_t> & aZoneIDs) override { return CHIP_NO_ERROR; }
    bool CanAddContextTriggers() override { return true; }
    void ActiveAmbientContextTriggersUpdated() override
    {
        ChipLogProgress(AppServer, "AvAnalysisNode: active context triggers updated");
    }
    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }

    /**
     * Starts an analysis session sourced from the given camera; the cluster generates the
     * AnalysisSessionStart event.
     *
     * @param aZoneIds      the zones the session is about; ignored when aZoneIdsNull
     * @param aZoneIdsNull  the session is not about particular zones
     * @param aSourceNodeId the camera the analyzed stream comes from; it must have a connected stream
     *
     * @return CHIP_ERROR_INVALID_ARGUMENT when no connected stream comes from that camera
     */
    CHIP_ERROR TriggerSessionStart(const std::vector<uint16_t> & aZoneIds, bool aZoneIdsNull, Optional<NodeId> aSourceNodeId);

    /**
     * Reports contexts newly perceived and no longer perceived in a session
     */
    CHIP_ERROR TriggerPerceivedContext(const std::vector<Clusters::AvAnalysis::Structs::TrackedContext::Type> & aNewContexts,
                                       const std::vector<Clusters::AvAnalysis::Structs::TrackedContext::Type> & aExpiredContexts,
                                       Optional<uint16_t> aSessionId, Optional<NodeId> aSourceNodeId);

    /**
     * Ends a session
     */
    CHIP_ERROR TriggerSessionEnd(Optional<uint16_t> aSessionId);

    /**
     * A stream from the camera was released: once none of its streams is connected, the sessions
     * sourced from it are ended, each with its AnalysisSessionEnd event.
     */
    void OnStreamReleased(const ScopedNodeId & aCameraNode);

    bool HasActiveSession() const { return !mSessions.empty(); }
    uint16_t GetLatestSessionId() const { return mLatestSessionId; }

private:
    struct Session
    {
        NodeId sourceNodeId     = kUndefinedNodeId;
        bool hasTrackedContexts = false;
    };

    // The camera's connected stream the session is sourced from
    CHIP_ERROR FindSourceStream(Optional<NodeId> aSourceNodeId, WebRTCPeerController::ConnectedStream & aStream) const;

    WebRTCPeerController * mPeerController = nullptr;
    std::map<uint16_t, Session> mSessions;
    uint16_t mLatestSessionId = 0;
};

} // namespace app
} // namespace chip
