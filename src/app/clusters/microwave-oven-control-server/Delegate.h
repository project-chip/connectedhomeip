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
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

class Instance;

/** @brief
 *  Defines methods for implementing application-specific logic for the MicrowaveOvenControl Cluster.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     *   @brief Handle Command Callback in application: SetCookingParameters.
     *   @return Returns the Interaction Model status code which was user determined in the business logic.
     *   @param  cookMode: the input cook mode value. Callee needs to define the cooking mode value in the Microwave Oven Mode
     * instance.
     *   @param  cookTimeSec: the input cook time value.
     *   @param  startAfterSetting: if true, the cooking operation will start after handling the command.
     *   @param  powerSettingNum: the input power setting value.
     *   @param  wattSettingIndex: the input watts setting index.
     *   Note: powerSettingNum and wattSettingIndex must be mutually exclusive.
     *   If using power as number, wattSettingIndex will be set to NullOptional.
     *   If using power in watts, powerSettingNum will be set to NullOptional.
     */
    virtual Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                                   bool startAfterSetting,
                                                                                   Optional<uint8_t> powerSettingNum,
                                                                                   Optional<uint8_t> wattSettingIndex) = 0;

    /**
     *   @brief Handle Command Callback in application: AddMoreTime.
     *   @return Returns the Interaction Model status code which was user determined in the business logic.
     *   @param  finalCookTimeSec: the cook time value after adding input time.
     */
    virtual Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec) = 0;

    /**
     *   Get the watt setting from the supported watts list.
     *   @param index The index of the watt setting to be returned. It is assumed that watt setting are indexable from 0 and with no
     * gaps.
     *   @param wattSetting A reference to receive the watt setting on success.
     *   @return Returns a CHIP_NO_ERROR if there was no error and the label was returned successfully,
     *   CHIP_ERROR_NOT_FOUND if the index in beyond the list of available labels.
     */
    virtual CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) = 0;

    virtual uint32_t GetMaxCookTimeSec() const = 0;

    virtual uint8_t GetPowerSettingNum() const = 0;

    virtual uint8_t GetMinPowerNum() const = 0;

    virtual uint8_t GetMaxPowerNum() const = 0;

    virtual uint8_t GetPowerStepNum() const = 0;

    virtual uint8_t GetCurrentWattIndex() const = 0;

    virtual uint16_t GetWattRating() const = 0;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     * @note This method is for internal SDK use and should only be called by the `Instance` constructor and destructor.
     */
    void SetInstance(Instance * aInstance)
    {
        VerifyOrDie(mInstance == nullptr || aInstance == nullptr || mInstance == aInstance);
        mInstance = aInstance;
    }

private:
    Instance * mInstance = nullptr;

protected:
    /**
     * @brief Provides access to the const Instance pointer.
     * This method is placed in the protected section because it must be called by classes derived from the delegate.
     *
     * @return A const pointer to the Instance object associated with this delegate.
     */
    const Instance * GetInstance() const { return mInstance; }

    /**
     * @brief Provides access to the Instance pointer.
     * This method is placed in the protected section because it must be called by classes derived from the delegate.
     *
     * @return A pointer to the Instance object associated with this delegate.
     */
    Instance * GetInstance() { return mInstance; }
};

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
