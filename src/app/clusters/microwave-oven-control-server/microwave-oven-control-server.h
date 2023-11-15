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
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

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
     * @param aClusterId The ID of the operational state derived cluster to be instantiated.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId);

    ~Instance() override;

    /**
     * @brief Initialise the Microwave Oven Control server instance.
     * This function must be called after defining an Instance class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    //defined the get/set api for the mandatory attributes
    uint32_t GetCookTime();
    void SetCookTime(uint32_t cookTime);

    uint8_t GetPowerSetting();
    void SetPowerSetting(uint8_t powerSetting);


private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;

    //set default values
    uint32_t mCookTime               = 30;
    uint8_t mPowerSettng             = 100;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable (may return CHIP_IM_GLOBAL_STATUS errors)
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    /**
     * Handle Command: SetCookingParameters.
     */
    void HandleSetCookingParameters(HandlerContext & ctx, const Commands::SetCookingParameters::DecodableType & req);

    /**
     * Handle Command: AddMoreTime.
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
     */
    virtual Protocols::InteractionModel::Status HandleSetCookingParametersCallback(Optional<uint8_t> cookMode,Optional<uint32_t> cookTime,Optional<uint8_t> powerSetting)  = 0;

    /**
     *   @brief Handle Command Callback in application: AddMoreTime
     *   @return Returns the Interaction Model status code which was user determined in the business logic
     */
    virtual Protocols::InteractionModel::Status HandleAddMoreTimeCallback(uint32_t timeToAdd) = 0;

    /**
     *   @brief defined the get/set interface for MinPower
     */
    virtual uint8_t GetMinPower() = 0;
    virtual void SetMinPower() = 0;

    /**
     *   @brief defined the get/set interface for MaxPower
     */
    virtual uint8_t GetMaxPower() = 0;
    virtual void SetMaxPower() = 0;

    /**
     *   @brief defined the get/set interface for PowerStep
     */
    virtual uint8_t GetPowerStep() = 0;
    virtual void SetPowerStep() = 0;

    

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

    //set default values
    uint8_t mMinPower     = static_cast<uint8_t>(10);
    uint8_t mMaxPower     = static_cast<uint8_t>(100);
    uint8_t mPowerStep    = static_cast<uint8_t>(10);
};


} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
