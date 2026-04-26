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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/microwave-oven-control-server/Delegate.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

constexpr uint32_t kDefaultCookTimeSec = 30u;
constexpr uint32_t kMinCookTimeSec     = 1u;
constexpr uint8_t kDefaultMinPowerNum  = 10u;
constexpr uint8_t kDefaultMaxPowerNum  = 100u;
constexpr uint8_t kDefaultPowerStepNum = 10u;

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
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     * @param aOpStateInstance The reference of Operational State Instance.
     * @param aMicrowaveOvenModeInstance The reference of Microwave Oven Mode Instance.
     * Note: a MicrowaveOvenControl instance must relies on an Operational State instance and a Microwave Oven Mode instance.
     * Caller must ensure those 2 instances are live and initialized before initializing MicorwaveOvenControl instance.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, BitMask<MicrowaveOvenControl::Feature> aFeature,
             Clusters::OperationalState::Instance & aOpStateInstance, Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance);

    ~Instance() override;

    /**
     * @brief Initialise the Microwave Oven Control server instance.
     * This function must be called after defining an Instance class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will returns CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    bool HasFeature(MicrowaveOvenControl::Feature feature) const;

    uint8_t GetCountOfSupportedWattLevels() const;

    uint32_t GetCookTimeSec() const;

    void SetCookTimeSec(uint32_t cookTimeSec);

private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    ClusterId mClusterId;
    BitMask<MicrowaveOvenControl::Feature> mFeature;
    Clusters::OperationalState::Instance & mOpStateInstance;
    Clusters::ModeBase::Instance & mMicrowaveOvenModeInstance;

    uint32_t mCookTimeSec        = kDefaultCookTimeSec;
    uint8_t mSupportedWattLevels = 0;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
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

/**
 *  @brief Check if the given cook time is in range.
 */
bool IsCookTimeSecondsInRange(uint32_t cookTimeSec, uint32_t maxCookTimeSec);

/**
 *  @brief Check if the given cooking power is in range.
 */
bool IsPowerSettingNumberInRange(uint8_t powerSettingNum, uint8_t minCookPowerNum, uint8_t maxCookPowerNum);

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
