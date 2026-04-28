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

#include <app/clusters/microwave-oven-control-server/CodegenIntegration.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using Status = Protocols::InteractionModel::Status;

namespace chip::app::Clusters::MicrowaveOvenControl {

using namespace Attributes;
using namespace Commands;

CodegenIntegrationDelegate::CodegenIntegrationDelegate(Clusters::OperationalState::Instance & aOpStateInstance,
                                                       Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance) :
    mOpStateInstance(aOpStateInstance),
    mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
{}

uint8_t CodegenIntegrationDelegate::GetCurrentOperationalState() const
{
    return mOpStateInstance.GetCurrentOperationalState();
}

CHIP_ERROR CodegenIntegrationDelegate::GetNormalOperatingMode(uint8_t & mode) const
{
    return mMicrowaveOvenModeInstance.GetModeValueByModeTag(to_underlying(MicrowaveOvenMode::ModeTag::kNormal), mode);
}

bool CodegenIntegrationDelegate::IsSupportedMode(uint8_t mode) const
{
    return mMicrowaveOvenModeInstance.IsSupportedMode(mode);
}

bool CodegenIntegrationDelegate::IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const
{
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsList;

    CodegenDataModelProvider & model = CodegenDataModelProvider::Instance();

    VerifyOrReturnValue(model.AcceptedCommands(ConcreteClusterPath(endpointId, OperationalState::Id), acceptedCommandsList) ==
                            CHIP_NO_ERROR,
                        false);
    auto acceptedCommands = acceptedCommandsList.TakeBuffer();

    return std::find_if(acceptedCommands.begin(), acceptedCommands.end(),
                        [commandId](const DataModel::AcceptedCommandEntry & entry) { return entry.commandId == commandId; }) !=
        acceptedCommands.end();
}

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, BitMask<Feature> aFeature,
                   OperationalState::Instance & aOpStateInstance, ModeBase::Instance & aMicrowaveOvenModeInstance) :
    mDelegate(aDelegate),
    mClusterPath(aEndpointId, aClusterId), mFeature(aFeature), mIntegrationDelegate(aOpStateInstance, aMicrowaveOvenModeInstance)
{
    VerifyOrDie(aClusterId == MicrowaveOvenControl::Id);
}

Instance::~Instance()
{
    // Call Deinit() to clean up and unregister the cluster if it is constructed.
    // Return value is safely ignored because we are in the destructor and this is just
    // a cleanup and we do not want to Log the error because Deinit() is part of the API
    // and could be called directly by the app.
    // If the cluster is not constructed, Deinit() will return CHIP_ERROR_INCORRECT_STATE.
    RETURN_SAFELY_IGNORED Deinit();
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrReturnError(emberAfContainsServer(mClusterPath.mEndpointId, mClusterPath.mClusterId), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.",
                                     long(mClusterPath.mClusterId)));

    // Exactly one of the PowerAsNumber and PowerInWatts features must be supported, per spec.
    VerifyOrReturnError(
        mFeature.Has(Feature::kPowerAsNumber) || mFeature.Has(Feature::kPowerInWatts), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, feature must support one of PowerInWatts or PowerAsNumber"));

    // Check that the feature bits do not include both PowerAsNumber and PowerInWatts
    VerifyOrReturnError(
        !(mFeature.Has(Feature::kPowerAsNumber) && mFeature.Has(Feature::kPowerInWatts)), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, PowerAsNumber and PowerInWatts are mutually exclusive"));

    // Per spec, the PowerNumberLimits feature is only allowed if the PowerAsNumber feature is supported.
    VerifyOrReturnError(
        !mFeature.Has(Feature::kPowerNumberLimits) || mFeature.Has(Feature::kPowerAsNumber), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, PowerNumberLimits feature requires PowerAsNumber"));

    if (emberAfContainsAttribute(mClusterPath.mEndpointId, mClusterPath.mClusterId, WattRating::Id))
    {
        mOptionalAttributeSet.Set<WattRating::Id>();
    }

    const EmberAfCluster * cluster = emberAfFindServerCluster(mClusterPath.mEndpointId, mClusterPath.mClusterId);
    VerifyOrReturnValue(cluster != nullptr, CHIP_ERROR_NOT_FOUND);

    bool supportsAddMoreTime = false;
    // NOTE: acceptedCommandList is KNOWN to not be nullptr because MicrowaveOvenControl should have mandatory commands
    //       as such we iterate here directly without an 'empty list' (i.e. nullptr) check.
    for (const CommandId * cmd = cluster->acceptedCommandList; *cmd != kInvalidCommandId; cmd++)
    {
        if (*cmd == Commands::AddMoreTime::Id)
        {
            supportsAddMoreTime = true;
            break;
        }
    }

    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->SetInstance(this);

    MicrowaveOvenControlCluster::Config config{ .feature              = mFeature,
                                                .optionalAttributeSet = mOptionalAttributeSet,
                                                .supportsAddMoreTime  = supportsAddMoreTime,
                                                .integrationDelegate  = mIntegrationDelegate,
                                                .appDelegate          = *mDelegate };
    mCluster.Create(mClusterPath.mEndpointId, config);
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
