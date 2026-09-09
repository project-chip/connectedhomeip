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
#include "camera-device-interface.h"
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
void AvAnalysisManager::ShutdownApp() {}

/**
 * Delegate command assists
 */
CHIP_ERROR AvAnalysisManager::VerifyZoneIDsAreValid(const std::vector<uint16_t> & aZoneIDs)
{
    if (aZoneIDs.empty())
    {
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(mCameraDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    for (uint16_t zoneId : aZoneIDs)
    {
        if (!mCameraDevice->GetCameraHALInterface().IsValidAnalysisZone(zoneId))
        {
            return CHIP_ERROR_NOT_FOUND;
        }
    }
    return CHIP_NO_ERROR;
}

bool AvAnalysisManager::CanAddContextTriggers()
{
    return true;
}

void AvAnalysisManager::ActiveAmbientContextTriggersUpdated() {}

CHIP_ERROR AvAnalysisManager::PersistentAttributesLoadedCallback()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisManager::TriggerSessionStart(const std::vector<uint16_t> & aZoneIds, bool aZoneIdsNull,
                                                  chip::Optional<chip::NodeId> aSourceNodeId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    DataModel::Nullable<std::vector<uint16_t>> zoneList;
    if (!aZoneIdsNull)
    {
        zoneList.SetNonNull(aZoneIds);
    }

    uint16_t sessionId = 0;
    CHIP_ERROR err     = server->AnalysisSessionStart(sessionId, zoneList, aSourceNodeId.ValueOr(chip::kUndefinedNodeId));
    if (err == CHIP_NO_ERROR)
    {
        mLatestSessionId           = sessionId;
        mHasActiveSession          = true;
        mSessionHasTrackedContexts = false;
        ChipLogProgress(Zcl, "AvAnalysisManager: Started analysis session %u", sessionId);
    }
    else
    {
        ChipLogError(Zcl, "AvAnalysisManager: Failed to start session: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR
AvAnalysisManager::TriggerPerceivedContext(const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aNewContexts,
                                           const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aExpiredContexts,
                                           chip::Optional<uint16_t> aSessionId, chip::Optional<chip::NodeId> aSourceNodeId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // If no active session exists, auto-create a session first without emitting AnalysisSessionStart event
    if (!mHasActiveSession)
    {
        uint16_t sid = aSessionId.ValueOr(0);
        ReturnErrorOnFailure(
            server->CreateActiveSession(sid, aSourceNodeId.ValueOr(chip::kUndefinedNodeId), aSessionId.HasValue()));
        mLatestSessionId           = sid;
        mHasActiveSession          = true;
        mSessionHasTrackedContexts = false;
    }

    uint16_t sessionId = aSessionId.ValueOr(mLatestSessionId);

    if (!aNewContexts.empty())
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        if (!mSessionHasTrackedContexts)
        {
            err = server->InitialTriggeringContextDetected(sessionId, aNewContexts);
            if (err == CHIP_NO_ERROR)
            {
                mSessionHasTrackedContexts = true;
            }
        }
        else
        {
            err = server->NewContextDetected(sessionId, aNewContexts);
        }
        ReturnErrorOnFailure(err);
    }

    if (!aExpiredContexts.empty())
    {
        ReturnErrorOnFailure(server->ContextNoLongerDetected(sessionId, aExpiredContexts));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisManager::TriggerSessionEnd(chip::Optional<uint16_t> aSessionId)
{
    AvAnalysisCluster * server = GetServer();
    VerifyOrReturnError(server != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t sessionId = aSessionId.ValueOr(mLatestSessionId);
    CHIP_ERROR err     = server->AnalysisSessionEnd(sessionId);
    if (err == CHIP_NO_ERROR)
    {
        if (sessionId == mLatestSessionId)
        {
            mHasActiveSession          = false;
            mSessionHasTrackedContexts = false;
        }
        ChipLogProgress(Zcl, "AvAnalysisManager: Ended analysis session %u", sessionId);
    }
    return err;
}
