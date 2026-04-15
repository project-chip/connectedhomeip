/*
 *
 *    Copyright (c) 20232026 Project CHIP Authors
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

#include <app/clusters/microwave-oven-control-server/CodegenIntegration.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip::app::Clusters::MicrowaveOvenControl {

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                   BitMask<MicrowaveOvenControl::Feature> aFeature, Clusters::OperationalState::Instance & aOpStateInstance,
                   Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance) :
    mDelegate(aDelegate),
    mEndpointId(aEndpointId), mClusterId(aClusterId), mFeature(aFeature), mOpStateInstance(aOpStateInstance),
    mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
{}

Instance::~Instance()
{
    RETURN_SAFELY_IGNORED Deinit();
}

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Check if the cluster has been selected in zap
    VerifyOrReturnError(
        emberAfContainsServer(mEndpointId, mClusterId), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.", long(mClusterId)));

    // Exactly one of the PowerAsNumber and PowerInWatts features must be supported, per spec.
    VerifyOrReturnError(
        mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) || mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl,
                     "Microwave Oven Control: feature bits error, feature must support one of PowerInWatts and PowerAsNumber"));

    // Check that the feature bits do not include both PowerAsNumber and PowerInWatts
    VerifyOrReturnError(
        !(mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) &&
          mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts)),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl,
                     "Microwave Oven Control: feature bits error, feature could not support both PowerAsNumber and PowerInWatts"));

    // Per spec, the PowerNumberLimits feature is only allowed if the PowerAsNumber feature is supported.
    VerifyOrReturnError(
        !mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits) ||
            mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(
            Zcl,
            "Microwave Oven Control: feature bits error, if feature supports PowerNumberLimits it must support PowerAsNumber"));

    mDelegate->SetInstance(this);

    MicrowaveOvenControlCluster::OptionalAttributeSet optionalAttributeSet;
    if (emberAfContainsAttribute(mEndpointId, mClusterId, MicrowaveOvenControl::Attributes::WattRating::Id))
    {
        optionalAttributeSet.Set<MicrowaveOvenControl::Attributes::WattRating::Id>();
    }

    mCluster.Create(mEndpointId, mFeature, optionalAttributeSet,
                    MicrowaveOvenControlCluster::Context{ mOpStateInstance, mMicrowaveOvenModeInstance, *mDelegate });
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

CHIP_ERROR Instance::Deinit()
{
    if (mDelegate)
    {
        mDelegate->SetInstance(nullptr);
    }
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
}

bool Instance::HasFeature(MicrowaveOvenControl::Feature feature) const
{
    return mFeature.Has(feature);
}

uint8_t Instance::GetCountOfSupportedWattLevels() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetCountOfSupportedWattLevels();
}

uint32_t Instance::GetCookTimeSec() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetCookTimeSec();
}

void Instance::SetCookTimeSec(uint32_t cookTimeSec)
{
    VerifyOrDie(mCluster.IsConstructed());
    mCluster.Cluster().SetCookTimeSec(cookTimeSec);
}

} // namespace chip::app::Clusters::MicrowaveOvenControl

void MatterMicrowaveOvenControlPluginServerInitCallback() {}
void MatterMicrowaveOvenControlPluginServerShutdownCallback() {}
