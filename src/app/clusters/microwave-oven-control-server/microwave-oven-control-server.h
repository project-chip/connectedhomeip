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
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

constexpr uint32_t kDefaultCookTime    = 30u;
constexpr uint8_t kDefaultPowerSetting = 100u;
constexpr uint32_t kMaxCookTime        = 65535u;
constexpr uint32_t kMinCookTime        = 1u;
constexpr uint16_t kDerivedModeTag     = 16384u;

class Delegate;

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * @brief Creates a Microwave Oven Control cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the Microwave Oven Control cluster to be instantiated.
     * @param aOpStateInstance The reference of Operational State Instance.
     * @param aMicrowaveOvenModeInstance The reference of Microwave Oven Mode Instance.
     * Note: a MicrowaveOvenControl instance must relies on an Operational State instance and a Microwave Oven Mode instance.
     * Caller must ensure those 2 instances are live and initialized before initializing MicorwaveOvenControl instance.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
             Clusters::OperationalState::Instance & aOpStateInstance, Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance);

    ~Instance() override;

    /**
     * @brief Initialise the Microwave Oven Control server instance.
     * This function must be called after defining an Instance class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    /**
     * @brief define the get/set api for the mandatory attributes
     */
    uint32_t GetCookTime() const;
    void SetCookTime(uint32_t cookTime);

    uint8_t GetPowerSetting() const;
    void SetPowerSetting(uint8_t powerSetting);

private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;
    Clusters::OperationalState::Instance & mOpStateInstance;
    Clusters::ModeBase::Instance & mMicrowaveOvenModeInstance;

    uint32_t mCookTime    = kDefaultCookTime;
    uint8_t mPowerSetting = kDefaultPowerSetting;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    /**
     * @brief Handle Command: SetCookingParameters.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * If the operational state is not in 'Stopped', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    void HandleSetCookingParameters(HandlerContext & ctx, const Commands::SetCookingParameters::DecodableType & req);

    /**
     * @brief Handle Command: AddMoreTime.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the cook time value is out of range, returns the Interaction Model status code of CONSTRAINT_ERROR.
     * If the operational state is in 'Error', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    void HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req);
};

/** @brief
 *  Defines methods for implementing application-specific logic for the MicrowaveOvenControl Cluster.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     *   @brief Handle Command Callback in application: SetCookingParameters
     *   @return Returns the Interaction Model status code which was user determined in the business logic
     *   @param  cookMode: the user defined modes which from the Microwave Oven Mode application level
     *   @param  cookTime: the input cook time value
     *   @param  powerSetting: the input power setting value
     */
    virtual Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTime,
                                                                                   uint8_t powerSetting) = 0;

    /**
     *   @brief Handle Command Callback in application: AddMoreTime
     *   @return Returns the Interaction Model status code which was user determined in the business logic
     *   @param  finalCookTime: the cook time value after adding input time
     */
    virtual Protocols::InteractionModel::Status HandleModifyCookTimeCallback(uint32_t finalCookTime) = 0;

    virtual uint8_t GetMinPower() const = 0;

    virtual uint8_t GetMaxPower() const = 0;

    virtual uint8_t GetPowerStep() const = 0;

private:
    friend class Instance;

    Instance * mInstance = nullptr;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

protected:
    Instance * GetInstance() const { return mInstance; }
};

/**
 *  @brief Check if the given cook time is in range
 *  @param cookTime    cookTime that given by user
 */
bool IsCookTimeInRange(uint32_t cookTime);

/**
 *  @brief Check if the given cooking power is in range
 *  @param powerSetting    power setting that given by user
 *  @param minCookPower    the min power setting that defined in application level
 *  @param maxCookPower    the max power setting that defined in application level
 */
bool IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower);

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
