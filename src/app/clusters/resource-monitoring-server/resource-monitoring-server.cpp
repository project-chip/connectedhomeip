/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <platform/DiagnosticDataProvider.h>

// using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

using BootReasonType = GeneralDiagnostics::BootReasonEnum;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

// todo find a cleaner solution by modifying the zap generated code.
EmberAfStatus Instance::GetFeature(uint32_t * value) const
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    EmberAfStatus status =
        emberAfReadAttribute(endpointId, clusterId, ResourceMonitoring::Attributes::FeatureMap::Id, readable, sizeof(temp));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

EmberAfStatus Instance::SetFeatureMap(uint32_t value) const
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpointId, clusterId, ResourceMonitoring::Attributes::FeatureMap::Id, writable,
                                 ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

std::map<uint32_t, Instance *> Instance::ResourceMonitoringAliasesInstanceMap;

CHIP_ERROR Instance::Init()
{
    ChipLogError(Zcl, "ResourceMonitoring: Init");
    // Check that the cluster ID given is a valid mode select alias cluster ID.
    if (!std::any_of(AliasedClusters.begin(), AliasedClusters.end(), [this](ClusterId i) { return i == clusterId; }))
    {
        ChipLogError(Zcl, "ResourceMonitoring: The cluster with ID %lu is not a mode select alias.", long(clusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(endpointId, clusterId))
    {
        ChipLogError(Zcl, "ResourceMonitoring: The cluster with ID %lu was not enabled in zap.", long(clusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ChipLogError(Zcl, "ResourceMonitoring: calling delegate->init()");
    ReturnErrorOnFailure(delegate->Init());

    ResourceMonitoringAliasesInstanceMap[clusterId] = this;

    return CHIP_NO_ERROR;
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    ChipLogError(Zcl, "resourcemonitoring: HandleCommand");
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        //
        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

void Instance::HandleResetCondition(HandlerContext & ctx,
                                    const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData)
{
    // uint8_t newMode = commandData.newMode;

    Status checkIsChangeToThisModeAllowed = delegate->HandleResetCondition();
    if (Status::Success != checkIsChangeToThisModeAllowed)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, checkIsChangeToThisModeAllowed);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
    ChipLogProgress(Zcl, "ResourceMonitor: HandleResetCondition reset done");
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "ResourceMonitoring Instance::InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ResourceMonitoring::Commands::ResetCondition::Id:
        ChipLogDetail(Zcl, "ResourceMonitoring::Commands::ResetCondition");

        HandleCommand<ResourceMonitoring::Commands::ResetCondition::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResetCondition(ctx, commandData); });
        break;
    }
}

bool Instance::HasFeature(ResourceMonitoring::Feature feature) const
{
    bool success;
    uint32_t featureMap;
    success = (GetFeature(&featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success && ((featureMap & to_underlying(feature)) != 0);
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    ChipLogDetail(Zcl, "resourcemonitoring: EnumerateAcceptedCommands");
    callback(HepaFilterMonitoring::Commands::ResetCondition::Id, context);

    return CHIP_NO_ERROR;
}

// Implements the read functionality for non-standard attributes.
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    return CHIP_NO_ERROR;
}

// Implements checking before attribute writes.
CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & attributePath, AttributeValueDecoder & aDecoder)
{

    return CHIP_NO_ERROR;
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip