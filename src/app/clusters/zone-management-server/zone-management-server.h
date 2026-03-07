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

#include <app/server-cluster/ServerClusterContext.h>

#include <optional>

// The compatibility wrapper depends on the code-driven cluster, but the shared
// delegate and storage types are split into ZoneManagementCommon.h so the cluster
// header does not need to include this legacy wrapper header.
#include "ZoneManagementCluster.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

class ZoneMgmtServer
{
public:
    ZoneMgmtServer(Delegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures, uint8_t aMaxUserDefinedZones,
                   uint8_t aMaxZones, uint8_t aSensitivityMax, const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
        mDelegate(aDelegate),
        mCluster(ZoneManagementCluster::Context{
            aDelegate, aEndpointId, aFeatures, { aMaxUserDefinedZones, aMaxZones, aSensitivityMax, aTwoDCartesianMax } })
    {
        aDelegate.SetZoneMgmtServer(this);
    }

    ~ZoneMgmtServer() { mDelegate.SetZoneMgmtServer(nullptr); }

    CHIP_ERROR Init()
    {
        VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR err = mCluster.Startup(*mContext, mPendingAppSensitivity);
        if (err == CHIP_NO_ERROR)
        {
            mInitialized = true;
            mSensitivity = mCluster.GetSensitivity();
            mPendingAppSensitivity.reset();
        }
        return err;
    }

    void Deinit()
    {
        if (!mInitialized)
        {
            return;
        }

        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        mInitialized = false;
    }

    const std::vector<ZoneInformationStorage> & GetZones() const { return mCluster.GetZones(); }
    uint8_t GetMaxZones() const { return mCluster.GetMaxZones(); }
    uint8_t GetSensitivity() const { return mSensitivity; }
    Optional<ZoneTriggerControlStruct> GetTriggerForZone(uint16_t zoneId) const { return mCluster.GetTriggerForZone(zoneId); }

    Protocols::InteractionModel::Status GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason);
    Protocols::InteractionModel::Status GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason);

    bool HasFeature(Feature feature) const;

    CHIP_ERROR SetSensitivity(uint8_t aSensitivity)
    {
        VerifyOrReturnError(aSensitivity >= 1 && aSensitivity <= mCluster.GetSensitivityMax(),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
        VerifyOrReturnValue(aSensitivity != mSensitivity, CHIP_NO_ERROR);

        if (!mInitialized)
        {
            mSensitivity           = aSensitivity;
            mPendingAppSensitivity = aSensitivity;
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(mCluster.SetSensitivity(aSensitivity));
        mSensitivity = mCluster.GetSensitivity();
        return CHIP_NO_ERROR;
    }

    const std::vector<ZoneTriggerControlStruct> & GetTriggers() const { return mCluster.GetTriggers(); }
    uint8_t GetMaxUserDefinedZones() const { return mCluster.GetMaxUserDefinedZones(); }
    uint8_t GetSensitivityMax() const { return mCluster.GetSensitivityMax(); }
    const TwoDCartesianVertexStruct & GetTwoDCartesianMax() const { return mCluster.GetTwoDCartesianMax(); }
    CHIP_ERROR RemoveZone(uint16_t zoneId) { return mCluster.RemoveZone(zoneId); }
    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneId) { return mCluster.RemoveTrigger(zoneId); }

private:
    Delegate & mDelegate;
    ZoneManagementCluster mCluster;
    ServerClusterContext * mContext = nullptr;
    bool mInitialized               = false;
    uint8_t mSensitivity            = 1;
    std::optional<uint8_t> mPendingAppSensitivity;
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
