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

#include <app/clusters/zone-management-server/CodegenIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

ZoneMgmtServer::ZoneMgmtServer(Delegate & delegate, EndpointId endpointId, BitFlags<Feature> features, uint8_t maxUserDefinedZones,
                               uint8_t maxZones, uint8_t sensitivityMax, const TwoDCartesianVertexStruct & twoDCartesianMax) :
    mEndpointId(endpointId),
    mDelegate(delegate), mFeatures(features), mConfig({ maxUserDefinedZones, maxZones, sensitivityMax, twoDCartesianMax })
{}

ZoneMgmtServer::~ZoneMgmtServer()
{
    Deinit();
}

CHIP_ERROR ZoneMgmtServer::Init()
{
    VerifyOrReturnError(!mCluster.IsConstructed(), CHIP_ERROR_ALREADY_INITIALIZED);

    mCluster.Create(ZoneManagementCluster::Context{
        .delegate   = mDelegate,
        .endpointId = mEndpointId,
        .features   = mFeatures,
        .config     = mConfig,
    });

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        mCluster.Destroy();
        return err;
    }

    if (mPendingAppSensitivity.has_value())
    {
        err = mCluster.Cluster().SetSensitivity(*mPendingAppSensitivity);
        if (err != CHIP_NO_ERROR)
        {
            LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster())));
            mCluster.Destroy();
            return err;
        }

        mPendingAppSensitivity.reset();
    }

    return CHIP_NO_ERROR;
}

void ZoneMgmtServer::Deinit()
{
    if (!mCluster.IsConstructed())
    {
        return;
    }

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster())));
    mCluster.Destroy();
}

Optional<ZoneTriggerControlStruct> ZoneMgmtServer::GetTriggerForZone(uint16_t zoneId) const
{
    VerifyOrReturnValue(mCluster.IsConstructed(), NullOptional,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().GetTriggerForZone(zoneId);
}

Protocols::InteractionModel::Status ZoneMgmtServer::GenerateZoneTriggeredEvent(uint16_t zoneID,
                                                                               ZoneEventTriggeredReasonEnum triggerReason)
{
    VerifyOrReturnValue(mCluster.IsConstructed(), Protocols::InteractionModel::Status::Failure,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().GenerateZoneTriggeredEvent(zoneID, triggerReason);
}

Protocols::InteractionModel::Status ZoneMgmtServer::GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason)
{
    VerifyOrReturnValue(mCluster.IsConstructed(), Protocols::InteractionModel::Status::Failure,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().GenerateZoneStoppedEvent(zoneID, stopReason);
}

CHIP_ERROR ZoneMgmtServer::SetSensitivity(uint8_t sensitivity)
{
    VerifyOrReturnError(sensitivity >= 1 && sensitivity <= mConfig.sensitivityMax, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (!mCluster.IsConstructed())
    {
        mPendingAppSensitivity = sensitivity;
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(mCluster.Cluster().SetSensitivity(sensitivity));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneMgmtServer::AddZone(const ZoneInformationStorage & zone)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().AddZone(zone);
}

CHIP_ERROR ZoneMgmtServer::UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zone)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().UpdateZone(zoneId, zone);
}

CHIP_ERROR ZoneMgmtServer::RemoveZone(uint16_t zoneId)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().RemoveZone(zoneId);
}

Protocols::InteractionModel::Status ZoneMgmtServer::AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger)
{
    VerifyOrReturnValue(mCluster.IsConstructed(), Protocols::InteractionModel::Status::Failure,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().AddOrUpdateTrigger(trigger);
}

Protocols::InteractionModel::Status ZoneMgmtServer::RemoveTrigger(uint16_t zoneId)
{
    VerifyOrReturnValue(mCluster.IsConstructed(), Protocols::InteractionModel::Status::Failure,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: %s called before cluster init", mEndpointId, __func__));
    return mCluster.Cluster().RemoveTrigger(zoneId);
}

uint8_t ZoneMgmtServer::GetSensitivity() const
{
    if (mCluster.IsConstructed())
    {
        return mCluster.Cluster().GetSensitivity();
    }
    return mPendingAppSensitivity.value_or(1);
}

const std::vector<ZoneInformationStorage> & ZoneMgmtServer::GetZones() const
{
    if (mCluster.IsConstructed())
    {
        return mCluster.Cluster().GetZones();
    }
    static const std::vector<ZoneInformationStorage> empty;
    return empty;
}

const std::vector<ZoneTriggerControlStruct> & ZoneMgmtServer::GetTriggers() const
{
    if (mCluster.IsConstructed())
    {
        return mCluster.Cluster().GetTriggers();
    }
    static const std::vector<ZoneTriggerControlStruct> empty;
    return empty;
}

uint8_t ZoneMgmtServer::GetMaxUserDefinedZones() const
{
    return mConfig.maxUserDefinedZones;
}

uint8_t ZoneMgmtServer::GetMaxZones() const
{
    return mConfig.maxZones;
}

uint8_t ZoneMgmtServer::GetSensitivityMax() const
{
    return mConfig.sensitivityMax;
}

const TwoDCartesianVertexStruct & ZoneMgmtServer::GetTwoDCartesianMax() const
{
    return mConfig.twoDCartesianMax;
}

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterZoneManagementClusterInitCallback(chip::EndpointId) {}
void MatterZoneManagementClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}

void MatterZoneManagementPluginServerInitCallback() {}
void MatterZoneManagementPluginServerShutdownCallback() {}
