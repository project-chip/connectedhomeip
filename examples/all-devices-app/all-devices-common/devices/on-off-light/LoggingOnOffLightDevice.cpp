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

#include "LoggingOnOffLightDevice.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip::app {
namespace {

class LoggingOnOffEffectDelegate : public OnOffEffectDelegate
{
public:
    DataModel::ActionReturnStatus TriggerDelayedAllOff(OnOff::DelayedAllOffEffectVariantEnum e) override
    {
        switch (e)
        {
        case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffFastFade:
            ChipLogProgress(DeviceLayer, "DelayedAllOff: FastFade");
            break;
        case OnOff::DelayedAllOffEffectVariantEnum::kNoFade:
            ChipLogProgress(DeviceLayer, "DelayedAllOff: NoFade");
            break;
        case OnOff::DelayedAllOffEffectVariantEnum::kDelayedOffSlowFade:
            ChipLogProgress(DeviceLayer, "DelayedAllOff: SlowFade");
            break;
        default:
            ChipLogProgress(DeviceLayer, "DelayedAllOff: UNKNOWN/INVALID");
            break;
        }
        return Status::Success;
    }

    DataModel::ActionReturnStatus TriggerDyingLight(OnOff::DyingLightEffectVariantEnum e) override
    {

        switch (e)
        {
        case OnOff::DyingLightEffectVariantEnum::kDyingLightFadeOff:
            ChipLogProgress(DeviceLayer, "DyingLight: FadeOff");
            break;
        default:
            ChipLogProgress(DeviceLayer, "DyingLight: UNKNOWN/INVALID");
            break;
        }
        return Status::Success;
    }
};

LoggingOnOffEffectDelegate & LoggingDelegate()
{
    static LoggingOnOffEffectDelegate gOnOffEffectDelegate;
    return gOnOffEffectDelegate;
}

} // namespace

CHIP_ERROR LoggingOnOffLightDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mContext.timerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mScenesTableProvider.SetEndpoint(endpoint);
    mScenesManagementCluster.Create(endpoint,
                                    ScenesManagementCluster::Context{
                                        .groupDataProvider  = &mContext.groupDataProvider,
                                        .fabricTable        = &mContext.fabricTable,
                                        .features           = {},
                                        .sceneTableProvider = mScenesTableProvider,
                                        .supportsCopyScene  = true,
                                    });
    ReturnErrorOnFailure(provider.AddCluster(mScenesManagementCluster.Registration()));

    mOnOffCluster.Create(endpoint,
                         OnOffLightingCluster::Context{
                             .timerDelegate             = mContext.timerDelegate,
                             .effectDelegate            = LoggingDelegate(),
                             .scenesIntegrationDelegate = &mScenesManagementCluster.Cluster(),
                         });
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void LoggingOnOffLightDevice::UnRegister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        mOnOffCluster.Destroy();
    }

    if (mScenesManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mScenesManagementCluster.Cluster()));
        mScenesManagementCluster.Destroy();
    }
}

void LoggingOnOffLightDevice::OnOffDelegate::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffLightDevice::OnOffDelegate::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingOnOffLightDevice::OnOffDelegate::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingOnOffLightDevice::OnOffDelegate::OnOffChanged() -> %s", on ? "ON" : "OFF");
}

} // namespace chip::app
