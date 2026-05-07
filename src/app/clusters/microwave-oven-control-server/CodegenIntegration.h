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

#pragma once

#include <app/clusters/microwave-oven-control-server/Delegate.h>
#include <app/clusters/microwave-oven-control-server/IntegrationDelegate.h>
#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip::app::Clusters::MicrowaveOvenControl {

class CodegenIntegrationDelegate : public IntegrationDelegate
{
public:
    CodegenIntegrationDelegate(Clusters::OperationalState::Instance & aOpStateInstance,
                               Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance);
    virtual ~CodegenIntegrationDelegate() = default;

    virtual uint8_t GetCurrentOperationalState() const override;
    virtual CHIP_ERROR GetNormalOperatingMode(uint8_t & mode) const override;
    virtual bool IsSupportedMode(uint8_t mode) const override;
    virtual bool IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const override;

private:
    Clusters::OperationalState::Instance & mOpStateInstance;
    Clusters::ModeBase::Instance & mMicrowaveOvenModeInstance;
};

class Instance
{
public:
    /**
     * @brief Creates a Microwave Oven Control cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the Microwave Oven Control cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     * @param aOpStateInstance The reference of Operational State Instance.
     * @param aMicrowaveOvenModeInstance The reference of Microwave Oven Mode Instance.
     * Note: a MicrowaveOvenControl instance must rely on an Operational State instance and a Microwave Oven Mode instance.
     * Caller must ensure those 2 instances are live and initialized before initializing Microwave Oven Control instance.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, BitMask<MicrowaveOvenControl::Feature> aFeature,
             Clusters::OperationalState::Instance & aOpStateInstance, Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance);

    /**
     * @brief Destroys a Microwave Oven Control cluster instance. The Deinit() function needs to be called for this instance
     * to be unregistered and stopped from being called by the interaction model.
     */
    ~Instance();

    /**
     * @brief Initialise the Microwave Oven Control server instance.
     * This function must be called after defining an Instance class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will returns CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    /**
     * @brief Uninitialise the Microwave Oven Control server instance.
     * @return Returns an error if the CommandHandler unregistration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Deinit();

    /**
     * @brief Get the count of supported watt levels.
     * @return The count of supported watt levels.
     */
    uint8_t GetCountOfSupportedWattLevels() const;

    /**
     * @brief Get the cook time in seconds.
     * @return The cook time in seconds.
     */
    uint32_t GetCookTimeSec() const;

    /**
     * @brief Set the cook time in seconds.
     * @param cookTimeSec The cook time in seconds.
     */
    void SetCookTimeSec(uint32_t cookTimeSec);

    /**
     * @brief Check if the feature is supported by this instance.
     * @param feature The feature to check.
     * @return True if the feature is supported, false otherwise.
     */
    bool HasFeature(MicrowaveOvenControl::Feature feature) const { return mFeature.Has(feature); }

private:
    Delegate * mDelegate{};
    ConcreteClusterPath mClusterPath{};
    BitMask<MicrowaveOvenControl::Feature> mFeature{};
    CodegenIntegrationDelegate mIntegrationDelegate;
    MicrowaveOvenControlCluster::OptionalAttributeSet mOptionalAttributeSet{};

    // The Code Driven MicrowaveOvenControl instance (lazy-initialized)
    chip::app::LazyRegisteredServerCluster<MicrowaveOvenControlCluster> mCluster;
};

} // namespace chip::app::Clusters::MicrowaveOvenControl
