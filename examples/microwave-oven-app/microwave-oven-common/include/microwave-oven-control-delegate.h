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
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>
#include <functional>


namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

// This is an application level delegate to handle microwave oven control commands according to the specific
// business logic.
class ExampleMicrowaveOvenControlDelegate : public MicrowaveOvenControl::Delegate
{

private:
    std::function<Protocols::InteractionModel::Status(uint8_t,uint32_t,uint8_t)> mHandleSetCookingParametersCallback;
    std::function<Protocols::InteractionModel::Status(uint32_t)> mHandleAddMoreTimeCallback;

public:
    /**
     * Handle Command Callback in application: set-cooking-parameters
     */
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTime,
                                                                           uint8_t powerSetting) override;

    /**
     * Handle Command Callback in application: add-more-time
     */
    Protocols::InteractionModel::Status HandleAddMoreTimeCallback(uint32_t finalCookTime) override;

    /**
     * Get the value of MinPower.
     */
    uint8_t GetMinPower() const override { return mMinPower; }

    /**
     * Get the value of MaxPower.
     */
    uint8_t GetMaxPower() const override { return mMaxPower; }

    /**
     * Get the value of PowerStep.
     */
    uint8_t GetPowerStep() const override { return mPowerStep; }

    /**
     * Set callback function for set cooking parameters
     */
    void SetMicrowaveOvenControlSetCookingParametersCallback(std::function<Protocols::InteractionModel::Status(uint8_t,uint32_t,uint8_t)> aCallback);

    /**
     * Set callback function for add more time
     */
    void SetMicrowaveOvenControlAddMoreTimeCallback(std::function<Protocols::InteractionModel::Status(uint32_t)> aCallback);
};

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
