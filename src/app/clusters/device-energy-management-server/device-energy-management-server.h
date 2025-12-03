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

#pragma once

#include "DeviceEnergyManagementDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    Protocols::InteractionModel::Status GetMatterEpochTimeFromUnixTime(uint32_t & currentUtcTime) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR RetrieveAcceptedCommands(const ConcreteClusterPath & cluster,
                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    Protocols::InteractionModel::Status CheckOptOutAllowsRequest(AdjustmentCauseEnum adjustmentCause);
    void HandlePowerAdjustRequest(HandlerContext & ctx, const Commands::PowerAdjustRequest::DecodableType & commandData);
    void HandleCancelPowerAdjustRequest(HandlerContext & ctx,
                                        const Commands::CancelPowerAdjustRequest::DecodableType & commandData);
    void HandleStartTimeAdjustRequest(HandlerContext & ctx, const Commands::StartTimeAdjustRequest::DecodableType & commandData);
    void HandlePauseRequest(HandlerContext & ctx, const Commands::PauseRequest::DecodableType & commandData);
    void HandleResumeRequest(HandlerContext & ctx, const Commands::ResumeRequest::DecodableType & commandData);
    void HandleModifyForecastRequest(HandlerContext & ctx, const Commands::ModifyForecastRequest::DecodableType & commandData);
    void HandleRequestConstraintBasedForecast(HandlerContext & ctx,
                                              const Commands::RequestConstraintBasedForecast::DecodableType & commandData);
    void HandleCancelRequest(HandlerContext & ctx, const Commands::CancelRequest::DecodableType & commandData);
};

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
