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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/util/af.h>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    // This map holds pointers to all initialised Resource Monitoring instances. It provides a way to access all Resource Monitoring
    // clusters.
    static std::map<uint32_t, Instance *> ResourceMonitoringAliasesInstanceMap;

    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // Checks if this instance supports a feature
    bool HasFeature(ResourceMonitoring::Feature feature) const;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Attribute setters
    chip::Protocols::InteractionModel::Status UpdateCondition(uint8_t aNewCondition);
    chip::Protocols::InteractionModel::Status
    UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum aNewChangeIndication);
    chip::Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool aNewInPlaceIndicator);

    // Attribute getters
    uint8_t GetCondition() const;
    chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum GetChangeIndication() const;
    bool GetInPlaceIndicator() const;

private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // attribute Data Store
    chip::Percent mCondition;
    const ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type mDegradationDirection;
    ResourceMonitoring::Attributes::ChangeIndication::TypeInfo::Type mChangeIndication;
    ResourceMonitoring::Attributes::InPlaceIndicator::TypeInfo::Type mInPlaceIndicator;

    uint32_t mFeature;

    // todo description
    bool IsAliascluster() const;

    // todo description
    void HandleResetCondition(HandlerContext & ctx,
                              const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData);

public:
    /**
     * Creates a mode select cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aDelegate A pointer to a delegate that will handle application layer logic.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature,
             ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mDegradationDirection(aDegradationDirection)
    {
        mEndpointId = aEndpointId;
        mClusterId  = aClusterId;
        mFeature    = aFeature;
    }

    ~Instance() override { ResourceMonitoringAliasesInstanceMap.erase(mClusterId); }

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // the following methods will be overridden by the SDK user to implement the business logic of their application

    /**
     * todo description
     */
    virtual CHIP_ERROR AppInit() = 0;

    /**
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param mode
     */
    virtual chip::Protocols::InteractionModel::Status OnResetCondition();
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip