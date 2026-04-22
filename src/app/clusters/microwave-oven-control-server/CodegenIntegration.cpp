/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
using namespace chip::app::DataModel;

namespace chip::app::Clusters::MicrowaveOvenControl {

namespace {

CHIP_ERROR OptionalAcceptedCommands(const ConcreteClusterPath & path,
                                    std::bitset<MicrowaveOvenControl::Commands::kAcceptedCommandsCount> & optionalAcceptedCommands)
{
    // Make sure that the cluster actually exists on this endpoint.
    const EmberAfCluster * serverCluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnValue(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnError(serverCluster->acceptedCommandList != nullptr, CHIP_NO_ERROR);

    const CommandId * endOfList = serverCluster->acceptedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }

    for (const CommandId * p = serverCluster->acceptedCommandList; p != endOfList; p++)
    {
        if (*p == MicrowaveOvenControl::Commands::AddMoreTime::Id)
        {
            optionalAcceptedCommands.set(MicrowaveOvenControl::Commands::AddMoreTime::Id);
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                   BitMask<MicrowaveOvenControl::Feature> aFeature, Clusters::OperationalState::Instance & aOpStateInstance,
                   Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance) :
    mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(aClusterId), mFeature(aFeature), mOpStateInstance(aOpStateInstance),
    mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
{}

Instance::~Instance()
{
    // Call Deinit() to clean up and unregister the cluster if it is constructed
    // Return value is safely ignored because we are in the destructor
    RETURN_SAFELY_IGNORED Deinit();
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrReturnError(
        emberAfContainsServer(mEndpointId, mClusterId), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.", long(mClusterId)));

    // Exactly one of the PowerAsNumber and PowerInWatts features must be supported, per spec.
    VerifyOrReturnError(
        mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) || mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, feature must support one of PowerInWatts or PowerAsNumber"));

    // Check that the feature bits do not include both PowerAsNumber and PowerInWatts
    VerifyOrReturnError(
        !(mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) &&
          mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts)),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, PowerAsNumber and PowerInWatts are mutually exclusive"));

    // Per spec, the PowerNumberLimits feature is only allowed if the PowerAsNumber feature is supported.
    VerifyOrReturnError(
        !mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits) ||
            mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, PowerNumberLimits feature requires PowerAsNumber"));

    if (emberAfContainsAttribute(mEndpointId, mClusterId, MicrowaveOvenControl::Attributes::WattRating::Id))
    {
        mOptionalAttributeSet.Set<MicrowaveOvenControl::Attributes::WattRating::Id>();
    }

    std::bitset<MicrowaveOvenControl::Commands::kAcceptedCommandsCount> optionalAcceptedCommands;
    VerifyOrReturnError(OptionalAcceptedCommands(ConcreteClusterPath(mEndpointId, mClusterId), optionalAcceptedCommands) ==
                            CHIP_NO_ERROR,
                        CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->SetInstance(this);

    MicrowaveOvenControlCluster::Config config{ mFeature,         mOptionalAttributeSet,      optionalAcceptedCommands,
                                                mOpStateInstance, mMicrowaveOvenModeInstance, *mDelegate };
    mCluster.Create(mEndpointId, config);
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

CHIP_ERROR Instance::Deinit()
{
    if (mDelegate)
    {
        mDelegate->SetInstance(nullptr);
        mDelegate = nullptr;
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
    RETURN_SAFELY_IGNORED mCluster.Cluster().SetCookTimeSec(cookTimeSec);
}

} // namespace chip::app::Clusters::MicrowaveOvenControl

void MatterMicrowaveOvenControlClusterInitCallback(EndpointId) {}
void MatterMicrowaveOvenControlClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}
void MatterMicrowaveOvenControlPluginServerInitCallback() {}
void MatterMicrowaveOvenControlPluginServerShutdownCallback() {}
