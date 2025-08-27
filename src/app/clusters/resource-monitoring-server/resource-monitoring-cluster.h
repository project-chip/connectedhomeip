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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/clusters/resource-monitoring-server/replacement-product-list-manager.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {


// forward declarations
class ResourceMonitoringDelegate;

class ResourceMonitoringCluster : public DefaultServerCluster
{

public:

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
    ResourceMonitoringCluster(
        EndpointId aEndpointId,
        ClusterId aClusterId,
        uint32_t aFeatureMap,
        ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
        bool aResetConditionCommandSupported
    );

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

    Protocols::InteractionModel::Status UpdateCondition(uint8_t newCondition);
    Protocols::InteractionModel::Status UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum newChangeIndication);
    Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool newInPlaceIndicator);
    Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> newLastChangedTime);

    /**
     * Sets delegate to passed ResourceMonitoringDelegate pointer. If this is not nullptr, subsequent action is to set instance
     * pointer in the delegate to this (ResourceMonitoringCluster) instance.
     *
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CHIP_ERROR SetDelegate(ResourceMonitoringDelegate* aDelegate);

    // Attribute getters
    uint8_t GetCondition() const;
    ChangeIndicationEnum GetChangeIndication() const;
    DegradationDirectionEnum GetDegradationDirection() const;
    bool GetInPlaceIndicator() const;
    DataModel::Nullable<uint32_t> GetLastChangedTime() const;

    EndpointId GetEndpointId() const { return mEndpointId; }
    ClusterId GetClusterId() const { return mClusterId; }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;    

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & cluster,
                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;       

private:

    ResourceMonitoringDelegate * mDelegate;

    void LoadPersistentAttributes();    

    ResourceMonitoring::ReplacementProductListManager * GetReplacementProductListManagerInstance();
    
    CHIP_ERROR ReadReplaceableProductList(AttributeValueEncoder & encoder);

    DataModel::ActionReturnStatus ReadHepaFilterMonitoringAttribute(
    const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder);

    DataModel::ActionReturnStatus ReadActivatedCarbonFilterMonitoringAttribute(
    const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder);

    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request,
                                           AttributeValueDecoder & decoder);

    std::optional<DataModel::ActionReturnStatus> ResetCondition(const ConcreteCommandPath & commandPath,
                                                                 const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData,
                                                                 CommandHandler * handler);

    chip::Percent mCondition                       = 100;
    DegradationDirectionEnum mDegradationDirection = DegradationDirectionEnum::kDown;
    ChangeIndicationEnum mChangeIndication         = ChangeIndicationEnum::kOk;
    bool mInPlaceIndicator                         = true;
    DataModel::Nullable<uint32_t> mLastChangedTime;
    ReplacementProductListManager * mReplacementProductListManager = nullptr;
    uint32_t mFeatureMap;
    
    bool mResetConditionCommandSupported{false};

    ClusterId mClusterId{};
    EndpointId mEndpointId{};
};

class ResourceMonitoringDelegate
{
    friend class ResourceMonitoringCluster;

private:

    ResourceMonitoringCluster * mInstance = nullptr;
    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of an Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(ResourceMonitoringCluster * aInstance) { mInstance = aInstance; }

    friend class ResourceMonitoringCluster;

protected:

    ResourceMonitoringCluster * GetInstance() { return mInstance; }

public:

    // This constructor deduces the non type template parameter ID to assist in mInstance type definition.
    ResourceMonitoringDelegate(ResourceMonitoringCluster * instance) : mInstance(instance) {}

    ResourceMonitoringDelegate()          = delete; // pointer to instance must be provided at construction
    virtual ~ResourceMonitoringDelegate() = default;

    // The following methods should be overridden by the SDK user to implement the business logic of their application

    /**
     * This init method will be called during Resource Monitoring Server initialization after the instance information has been
     * validated and the instance has been registered. This method should be overridden by the SDK user to initialize the
     * application logic.
     *
     * @return CHIP_NO_ERROR    If the application was initialized successfully. All other values will cause the initialization to
     * fail.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * This method may be overwritten by the SDK User, if the default behaviour is not desired.
     * Preferably, the SDK User should implement the PreResetCondition() and PostResetCondition() methods instead.
     *
     * The cluster implementation will handle all of the resets needed by the spec.
     * - Update the Condition attribute according to the DegradationDirection (if supported)
     * - Update the ChangeIndicator attribute to kOk
     * - Update the LastChangedTime attribute (if supported)
     *
     * The return value will depend on the PreResetCondition() and PostResetCondition() method, if one of them does not return
     * Success, this method will return the failure as well.
     * @return Status::Success      If the command was handled successfully.
     * @return All Other            PreResetCondition() or PostResetCondition() failed, these are application specific.
     */
    virtual Protocols::InteractionModel::Status OnResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something before the reset.
     * If there are some internal states of the devices or some specific methods that must be called, that are needed for the reset
     * and that can fail, they should be done here and not in PostResetCondition().
     *
     * @return Status::Success      All good, the reset may proceed.
     * @return All Other            The reset should not proceed. The reset command will fail.
     */
    virtual Protocols::InteractionModel::Status PreResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something after the reset.
     * If this fails, the attributes will already be updated, so the SDK User should not do something here
     * that can fail and that will affect the state of the device. Do the checks in the PreResetCondition() method instead.
     *
     * @return Status::Success      All good
     * @return All Other            Something went wrong. The attributes will already be updated. But the reset command will report
     *                              the failure.
     */
    virtual Protocols::InteractionModel::Status PostResetCondition();
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip