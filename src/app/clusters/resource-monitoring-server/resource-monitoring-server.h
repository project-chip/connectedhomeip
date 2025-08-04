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
class Delegate;

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    /**
     * Creates a resource monitoring cluster instance. The Init() method needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     *
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId                        The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aFeatureMap                       The feature map of the cluster.
     * @param aDegradationDirection             The degradation direction of the cluster.
     * @param aResetConditionCommandSupported   Whether the ResetCondition command is supported by the cluster.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeatureMap,
             ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
             bool aResetConditionCommandSupported);

    ~Instance() override;

    // Not copyable or movable
    Instance(const Instance &)             = delete;
    Instance & operator=(const Instance &) = delete;
    Instance(Instance &&)                  = delete;
    Instance & operator=(Instance &&)      = delete;

    /**
     * Initialise the Resource Monitoring cluster.
     *
     * @die                                     If the cluster ID given is not a valid Resource Monitoring cluster ID.
     * @die                                     If the endpoint and cluster ID have not been enabled in zap.
     * @return CHIP_ERROR_INVALID_ARGUMENT      If the CommandHandler or Attribute Handler could not be registered.
     * @return CHIP_ERROR_INCORRECT_STATE       If the CommandHandler was already registered
     * @return CHIP_ERROR_INCORRECT_STATE       If the AttributeAccessInterfaceRegistry::Register fails.
     * @return CHIP_ERROR                       If the AppInit() method returned an error. This is application specific.
     *
     * @return CHIP_NO_ERROR                    If the cluster was initialised successfully.
     */
    CHIP_ERROR Init();

    /**
     * Checks if the given feature is supported by the cluster.
     * @param feature   The aFeature to check.
     *
     * @return true     If the feature is supported.
     * @return false    If the feature is not supported.
     */
    bool HasFeature(ResourceMonitoring::Feature aFeature) const;

    // Attribute setters
    Protocols::InteractionModel::Status UpdateCondition(uint8_t aNewCondition);
    Protocols::InteractionModel::Status UpdateChangeIndication(ChangeIndicationEnum aNewChangeIndication);
    Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool aNewInPlaceIndicator);
    Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime);

    void SetReplacementProductListManagerInstance(ReplacementProductListManager * instance);

    // Attribute getters
    uint8_t GetCondition() const;
    ChangeIndicationEnum GetChangeIndication() const;
    DegradationDirectionEnum GetDegradationDirection() const;
    bool GetInPlaceIndicator() const;
    DataModel::Nullable<uint32_t> GetLastChangedTime() const;

    EndpointId GetEndpointId() const { return mEndpointId; }
    ClusterId GetClusterId() const { return mClusterId; }

private:
    Delegate * mDelegate;

    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // attribute Data Store
    chip::Percent mCondition                       = 100;
    DegradationDirectionEnum mDegradationDirection = DegradationDirectionEnum::kDown;
    ChangeIndicationEnum mChangeIndication         = ChangeIndicationEnum::kOk;
    bool mInPlaceIndicator                         = true;
    DataModel::Nullable<uint32_t> mLastChangedTime;
    ReplacementProductListManager * mReplacementProductListManager = nullptr;

    uint32_t mFeatureMap;

    bool mResetConditionCommandSupported = false;

    ReplacementProductListManager * GetReplacementProductListManagerInstance();

    CHIP_ERROR ReadReplaceableProductList(AttributeValueEncoder & aEncoder);

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    void LoadPersistentAttributes();

    /**
     * Internal method to handle the ResetCondition command.
     */
    void HandleResetCondition(HandlerContext & ctx,
                              const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData);
}; // class Instance

class Delegate
{
    friend class Instance;

private:
    Instance * mInstance = nullptr;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of an Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

protected:
    Instance * GetInstance() { return mInstance; }

public:
    Delegate()          = default;
    virtual ~Delegate() = default;

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
