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

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    void LoadPersistentAttributes();

    /**
     * Initialise the Resource Monitoring cluster.
     *
     * @return CHIP_ERROR       If the cluster ID given is not a valid Resource Monitoring cluster ID.
     * @return CHIP_ERROR       If the endpoint and cluster ID have not been enabled in zap.
     * @return CHIP_ERROR       If the CommandHandler or Attribute Handler could not be registered.
     * @return CHIP_ERROR       If the AppInit() function returned an error.
     *
     * @return CHIP_NO_ERROR    If the cluster was initialised successfully.
     */
    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    /**
     * returns true if the given feature is supported by the cluster.
     * @param feature   The aFeature to check.
     */
    bool HasFeature(ResourceMonitoring::Feature aFeature) const;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Attribute setters
    chip::Protocols::InteractionModel::Status UpdateCondition(uint8_t aNewCondition);
    chip::Protocols::InteractionModel::Status UpdateChangeIndication(ChangeIndicationEnum aNewChangeIndication);
    chip::Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool aNewInPlaceIndicator);
    chip::Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime);

    // Attribute getters
    uint8_t GetCondition() const;
    ChangeIndicationEnum GetChangeIndication() const;
    DegradationDirectionEnum GetDegradationDirection() const;
    bool GetInPlaceIndicator() const;
    DataModel::Nullable<uint32_t> GetLastChangedTime() const;

private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // attribute Data Store
    chip::Percent mCondition                             = 100;
    const DegradationDirectionEnum mDegradationDirection = DegradationDirectionEnum::kDown;
    ChangeIndicationEnum mChangeIndication               = ChangeIndicationEnum::kOk;
    bool mInPlaceIndicator                               = true;
    DataModel::Nullable<uint32_t> mLastChangedTime;

    uint32_t mFeature;

    const bool mResetConditionCommandSupported = false;

    /**
     * This checks if the clusters instance is a valid ResourceMonitoring cluster based on the AliasedClusters list.
     * @return true     if the cluster is a valid ResourceMonitoring cluster.
     */
    bool IsValidAliasCluster() const;

    /**
     * Internal method to handle the ResetCondition command.
     */
    void HandleResetCondition(HandlerContext & ctx,
                              const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData);

public:
    /**
     * Creates a resource monitoring cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId   The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId    The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aDelegate     A pointer to a delegate that will handle application layer logic.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature,
             ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
             bool aResetConditionCommandSupported) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mDegradationDirection(aDegradationDirection),
        mResetConditionCommandSupported(aResetConditionCommandSupported)
    {
        mEndpointId = aEndpointId;
        mClusterId  = aClusterId;
        mFeature    = aFeature;
    }

    ~Instance() = default;
    // Not copyable or movable
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // the following methods should be overridden by the SDK user to implement the business logic of their application

    /**
     * This init function will be called during Resource Monitoring Server initialization after the instance information has been
     * validated and the instance has been registered. This function should be overridden by the SDK user to initialize the
     * application logic.
     *
     * @return CHIP_NO_ERROR    If the application was initialized successfully. All other values will cause the initialization to
     * fail.
     */
    virtual CHIP_ERROR AppInit() = 0;

    /**
     * This function is to be overridden by a user implemented method to handle the application specifics of the ResetCondition
     * command.
     *
     * @return Status::Success      If the command was handled successfully, all other will cause the command to fail.
     */
    virtual chip::Protocols::InteractionModel::Status OnResetCondition() = 0;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
