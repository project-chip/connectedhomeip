/*
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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/clusters/resource-monitoring-server/ResourceMonitoringDelegate.h>
#include <app/clusters/resource-monitoring-server/replacement-product-list-manager.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/HepaFilterMonitoring/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class ResourceMonitoringCluster : public DefaultServerCluster
{

public:
    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
        ResourceMonitoring::Attributes::InPlaceIndicator::Id, ResourceMonitoring::Attributes::LastChangedTime::Id>;

    /**
     * Creates a resource monitoring cluster object. The Init() method needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     *
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId                        The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aFeatureMap                       The feature map of the cluster.
     * @param aDegradationDirection             The degradation direction of the cluster.
     * @param aResetConditionCommandSupported   Whether the ResetCondition command is supported by the cluster.
     */
    ResourceMonitoringCluster(EndpointId aEndpointId, ClusterId aClusterId,
                              const BitFlags<ResourceMonitoring::Feature> enabledFeatures,
                              OptionalAttributeSet optionalAttributeSet,
                              ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
                              bool aResetConditionCommandSupported);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    void SetReplacementProductListManagerInstance(ResourceMonitoring::ReplacementProductListManager * instance);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /**
     * Checks if the given feature is supported by the cluster.
     * @param feature   The aFeature to check.
     *
     * @return true     If the feature is supported.
     * @return false    If the feature is not supported.
     */
    bool HasFeature(ResourceMonitoring::Feature aFeature) const;

    /**
     * Checks if the given attribute is supported by the cluster.
     * @param attributeId  The aAttributeId to check.
     *
     * @return true     If the attribute is supported.
     * @return false    If the attribute is not supported.
     */
    bool HasOptionalAttribute(AttributeId aAttributeId) const;

    CHIP_ERROR Init() { return CHIP_NO_ERROR; };

    Protocols::InteractionModel::Status UpdateCondition(uint8_t newCondition);
    Protocols::InteractionModel::Status
    UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum newChangeIndication);
    Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool newInPlaceIndicator);
    Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> newLastChangedTime);

    /**
     * Sets delegate to passed Delegate pointer. If this is not nullptr, subsequent action is to set instance
     * pointer in the delegate to this (ResourceMonitoringCluster) instance.
     *
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CHIP_ERROR SetDelegate(Delegate * aDelegate);

    // Attribute getters
    uint8_t GetCondition() const;
    ChangeIndicationEnum GetChangeIndication() const;
    DegradationDirectionEnum GetDegradationDirection() const;
    bool GetInPlaceIndicator() const;
    DataModel::Nullable<uint32_t> GetLastChangedTime() const;

    EndpointId GetEndpointId() const { return mPath.mEndpointId; }
    ClusterId GetClusterId() const { return mPath.mClusterId; }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & cluster,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    Delegate * mDelegate;

    void LoadPersistentAttributes();

    ResourceMonitoring::ReplacementProductListManager * GetReplacementProductListManagerInstance();

    CHIP_ERROR ReadReplaceableProductList(AttributeValueEncoder & encoder);

    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);

    std::optional<DataModel::ActionReturnStatus>
    ResetCondition(const ConcreteCommandPath & commandPath,
                   const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData, CommandHandler * handler);

    chip::Percent mCondition                       = 100;
    DegradationDirectionEnum mDegradationDirection = DegradationDirectionEnum::kDown;
    ChangeIndicationEnum mChangeIndication         = ChangeIndicationEnum::kOk;
    bool mInPlaceIndicator                         = true;
    DataModel::Nullable<uint32_t> mLastChangedTime;
    ReplacementProductListManager * mReplacementProductListManager = nullptr;

    bool mResetConditionCommandSupported{ false };

    const BitFlags<ResourceMonitoring::Feature> mEnabledFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
