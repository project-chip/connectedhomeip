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

#include "clusters/av-analysis/av-analysis-delegate.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

using namespace chip::app::Clusters;

CHIP_ERROR AvAnalysisNodeDelegate::FindSourceStream(Optional<NodeId> aSourceNodeId,
                                                    WebRTCPeerController::ConnectedStream & aStream) const
{
    VerifyOrReturnError(mPeerController != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(aSourceNodeId.HasValue(), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(AppServer, "AvAnalysisNode: a session needs the camera it is sourced from"));

    bool found = false;
    for (const auto & stream : mPeerController->ConnectedStreams())
    {
        if (stream.cameraNode.GetNodeId() != aSourceNodeId.Value())
        {
            continue;
        }
        if (!found || stream.connectedAtEpochUs < aStream.connectedAtEpochUs)
        {
            aStream = stream;
            found   = true;
        }
    }
    VerifyOrReturnError(found, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(AppServer, "AvAnalysisNode: no connected stream from node " ChipLogFormatX64,
                                     ChipLogValueX64(aSourceNodeId.Value())));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisNodeDelegate::TriggerSessionStart(const std::vector<uint16_t> & aZoneIds, bool aZoneIdsNull,
                                                       Optional<NodeId> aSourceNodeId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    WebRTCPeerController::ConnectedStream source;
    ReturnErrorOnFailure(FindSourceStream(aSourceNodeId, source));

    DataModel::Nullable<std::vector<uint16_t>> zoneList;
    if (!aZoneIdsNull)
    {
        zoneList.SetNonNull(aZoneIds);
    }

    uint16_t sessionId = 0;
    CHIP_ERROR err = server->AnalysisSessionStart(sessionId, zoneList, source.cameraNode.GetNodeId(), source.connectedAtEpochUs);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(AppServer, "AvAnalysisNode: analysis session not started: %" CHIP_ERROR_FORMAT, err.Format()));

    mSessions[sessionId] = Session{ source.cameraNode.GetNodeId(), false };
    mLatestSessionId     = sessionId;
    ChipLogProgress(AppServer, "AvAnalysisNode: analysis session %u started, sourced from node " ChipLogFormatX64, sessionId,
                    ChipLogValueX64(source.cameraNode.GetNodeId()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR
AvAnalysisNodeDelegate::TriggerPerceivedContext(const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aNewContexts,
                                                const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aExpiredContexts,
                                                Optional<uint16_t> aSessionId, Optional<NodeId> aSourceNodeId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t sessionId = aSessionId.ValueOr(mLatestSessionId);
    auto it            = mSessions.find(sessionId);
    if (it == mSessions.end())
    {
        VerifyOrReturnError(!aSessionId.HasValue() || !HasActiveSession(), CHIP_ERROR_NOT_FOUND,
                            ChipLogError(AppServer, "AvAnalysisNode: analysis session %u is unknown", sessionId));

        WebRTCPeerController::ConnectedStream source;
        ReturnErrorOnFailure(FindSourceStream(aSourceNodeId, source));

        sessionId = aSessionId.ValueOr(0);
        ReturnErrorOnFailure(server->CreateActiveSession(sessionId, source.cameraNode.GetNodeId(), source.connectedAtEpochUs,
                                                         aSessionId.HasValue()));
        it               = mSessions.emplace(sessionId, Session{ source.cameraNode.GetNodeId(), false }).first;
        mLatestSessionId = sessionId;
        ChipLogProgress(AppServer,
                        "AvAnalysisNode: analysis session %u created for perceived context, sourced from node " ChipLogFormatX64,
                        sessionId, ChipLogValueX64(source.cameraNode.GetNodeId()));
    }

    if (!aNewContexts.empty())
    {
        if (!it->second.hasTrackedContexts)
        {
            ReturnErrorOnFailure(server->InitialTriggeringContextDetected(sessionId, aNewContexts));
            it->second.hasTrackedContexts = true;
        }
        else
        {
            ReturnErrorOnFailure(server->NewContextDetected(sessionId, aNewContexts));
        }
    }

    if (!aExpiredContexts.empty())
    {
        ReturnErrorOnFailure(server->ContextNoLongerDetected(sessionId, aExpiredContexts));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisNodeDelegate::TriggerSessionEnd(Optional<uint16_t> aSessionId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const uint16_t sessionId = aSessionId.ValueOr(mLatestSessionId);
    auto it                  = mSessions.find(sessionId);
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND,
                        ChipLogError(AppServer, "AvAnalysisNode: analysis session %u is unknown", sessionId));

    ReturnErrorOnFailure(server->AnalysisSessionEnd(sessionId));
    mSessions.erase(it);
    ChipLogProgress(AppServer, "AvAnalysisNode: analysis session %u ended", sessionId);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
