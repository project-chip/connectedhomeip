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

#include <app/util/config.h>
#include <app/clusters/device-energy-management-server/device-energy-management-server.h>


using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

class DEMManufacturerDelegate;

/**
 * The application delegate.
 */
class DeviceEnergyManagementDelegate : public DeviceEnergyManagement::Delegate
{
public:
    DeviceEnergyManagementDelegate();

    void SetDeviceEnergyManagementInstance(DeviceEnergyManagement::Instance & instance);

    void SetDEMManufacturerDelegate(DEMManufacturerDelegate & deviceEnergyManagementManufacturerDelegate);

    /**
     *
     * Implement the DeviceEnergyManagement::Delegate interface
     *
     */

    /**
     * @brief Implements a handler to begin to adjust client power
     *        consumption/generation to the level requested.
     *
     * @param power Milli-Watts the ESA SHALL use during the adjustment period.
     * @param duration The duration that the ESA SHALL maintain the requested power for.
     * @return  Success if the adjustment is accepted; otherwise the command SHALL be rejected with appropriate error.
     */
    virtual Status PowerAdjustRequest(const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause) override;

    /**
     * @brief Make the ESA end the active power adjustment session & return to normal (or idle) power levels.
     *        The ESA SHALL also generate an PowerAdjustEnd Event and the ESAState SHALL be restored to Online.
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    virtual Status CancelPowerAdjustRequest() override;

    /**
     * @brief The ESA SHALL update its Forecast attribute with the RequestedStartTime including a new ForecastID.
     *
     *   If the ESA supports ForecastAdjustment, and the ESAState is not UserOptOut and the RequestedStartTime is after
     *   the EarliestStartTime and the resulting EndTime is before the LatestEndTime, then ESA SHALL accept the request
     *   to modify the Start Time.
     *   A client can estimate the entire Forecast sequence duration by computing the EndTime - StartTime fields from the
     *   Forecast attribute, and therefore avoid scheduling the start time too late.
     *
     * @param requestedStartTime The requested start time in UTC that the client would like the appliance to shift its power
     * forecast to.
     * @param cause    Who (Grid/local) is triggering this change.
     *
     * @return Success if the StartTime in the Forecast is updated, otherwise the command SHALL be rejected with appropriate
     * IM_Status.
     */
    virtual Status StartTimeAdjustRequest(const uint32_t requestedStartTime, AdjustmentCauseEnum cause) override;

    /**
     * @brief Handler for PauseRequest command
     *
     *   If the ESA supports FA and the SlotIsPauseable field is true in the ActiveSlotNumber
     *   index in the Slots list, and the ESAState is not UserOptOut then the ESA SHALL allow its current
     *   operation to be Paused.
     *
     *   During this state the ESA SHALL not consume or produce significant power (other than required to keep its
     *   basic control system operational).
     *
     * @param duration Duration that the ESA SHALL be paused for.
     * @return  Success if the ESA is paused, otherwise returns other IM_Status.
     */
    virtual Status PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause) override;

    /**
     * @brief Handler for ResumeRequest command
     *
     *   If the ESA supports FA and it is currently Paused then the ESA SHALL resume its operation.
     *   The ESA SHALL also generate a Resumed Event and the ESAState SHALL be updated accordingly to
     *   reflect its current state.
     *
     * @return  Success if the ESA is resumed, otherwise returns other IM_Status.
     */
    virtual Status ResumeRequest() override;

    /**
     * @brief Handler for ModifyForecastRequest
     *
     *   If the ESA supports FA, and the ESAState is not UserOptOut it SHALL attempt to adjust its power forecast.
     *   This allows a one or more modifications in a single command by sending a list of modifications (one for each 'slot').
     *   Attempts to modify slots which have already past, SHALL result in the entire command being rejected.
     *   If the ESA accepts the requested Forecast then it SHALL update its Forecast attribute (incrementing its ForecastID)
     *   and run the revised Forecast as its new intended operation.
     *
     * @param forecastID Indicates the ESA ForecastID that is to be modified.
     * @param slotAdjustments List of adjustments to be applied to the ESA, corresponding to the expected ESA forecastID.
     * @return  Success if the entire list of SlotAdjustmentStruct are accepted, otherwise the command
     *          SHALL be rejected returning other IM_Status.
     */
    virtual Status
    ModifyForecastRequest(const uint32_t forecastID,
                          const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
                          AdjustmentCauseEnum cause) override;

    /**
     * @brief Handler for RequestConstraintBasedForecast
     *
     *   The ESA SHALL inspect the requested power limits to ensure that there are no overlapping elements. The ESA
     *   manufacturer may also reject the request if it could cause the user’s preferences to be breached (e.g. may
     *   cause the home to be too hot or too cold, or a battery to be insufficiently charged).
     *   If the ESA can meet the requested power limits, it SHALL regenerate a new Power Forecast with a new ForecastID.
     *
     * @param constraints  Sequence of turn up/down power requests that the ESA is being asked to constrain its operation within.
     * @return  Success if successful, otherwise the command SHALL be rejected returning other IM_Status.
     */
    virtual Status
    RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> & constraints,
                                   AdjustmentCauseEnum cause) override;

    /**
     * @brief Handler for CancelRequest
     *
     *   The ESA SHALL attempt to cancel the effects of any previous adjustment request commands, and re-evaluate its
     *   forecast for intended operation ignoring those previous requests.
     *
     *   If the ESA ForecastStruct ForecastUpdateReason was already `Internal Optimization`, then the command SHALL
     *   be rejected with FAILURE.
     *
     *   If the command is accepted, the ESA SHALL update its ESAState if required, and the command status returned
     *   SHALL be SUCCESS.
     *
     *   The ESA SHALL update its Forecast attribute to match its new intended operation, and update the
     *   ForecastStruct.ForecastUpdateReason to `Internal Optimization`
     *
     * @return  Success if successful, otherwise the command SHALL be rejected returning other IM_Status.
     */
    virtual Status CancelRequest() override;

    // ------------------------------------------------------------------
    // Overridden DeviceEnergyManagement::Delegate Get attribute methods
    virtual ESATypeEnum GetESAType() override;
    virtual bool GetESACanGenerate() override;
    virtual ESAStateEnum GetESAState() override;
    virtual int64_t GetAbsMinPower() override;
    virtual int64_t GetAbsMaxPower() override;
    virtual DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & GetPowerAdjustmentCapability() override;
    virtual DataModel::Nullable<Structs::ForecastStruct::Type> & GetForecast() override;
    virtual OptOutStateEnum GetOptOutState() override;

    // ------------------------------------------------------------------
    // Overridden DeviceEnergyManagement::Delegate Set attribute methods
    virtual CHIP_ERROR SetESAType(ESATypeEnum) override;
    virtual CHIP_ERROR SetESACanGenerate(bool) override;
    virtual CHIP_ERROR SetESAState(ESAStateEnum) override;
    virtual CHIP_ERROR SetAbsMinPower(int64_t) override;
    virtual CHIP_ERROR SetAbsMaxPower(int64_t) override;
    virtual CHIP_ERROR SetPowerAdjustmentCapability(DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> &) override;
    virtual CHIP_ERROR SetForecast(DataModel::Nullable<Structs::ForecastStruct::Type> &) override;
    virtual CHIP_ERROR SetOptOutState(OptOutStateEnum) override;

    // Returns whether the DeviceEnergyManagement is supported
    uint32_t HasFeature(Feature feature) const;

private:
    /**
     * @brief Handle a PowerAdjustRequest failing
     *
     *  Cleans up the PowerAdjust state should the request fail
     */
    void HandlePowerAdjustRequestFailure();

    // Methods to handle when a PowerAdjustment completes
    static void PowerAdjustTimerExpiry(System::Layer * systemLayer, void * delegate);
    void HandlePowerAdjustTimerExpiry();

    // Method to cancel a PowerAdjustment
    CHIP_ERROR CancelPowerAdjustRequestAndSendEvent(CauseEnum cause);

    // Method to send a PowerAdjustEnd event
    CHIP_ERROR SendPowerAdjustEndEvent(CauseEnum cause);

    /**
     * @brief Handle a PauseRequest failing
     *
     *  Cleans up the state should the PauseRequest fail
     */
    void HandlePauseRequestFailure();

    // Methods to handle when a PauseRequest completes
    static void PauseRequestTimerExpiry(System::Layer * systemLayer, void * delegate);
    void HandlePauseRequestTimerExpiry();

    // Method to cancel a PauseRequest
    CHIP_ERROR CancelPauseRequestAndSendEvent(CauseEnum cause);

    // Method to send a Paused event
    CHIP_ERROR SendResumedEvent(CauseEnum cause);

private:
    // Have a pointer to partner instance object
    DeviceEnergyManagement::Instance *mpDEMInstance;

    // The DEMManufacturerDelegate object knows how to handle
    // manufacturer/product specific operations
    DEMManufacturerDelegate *mpDEMManufacturerDelegate;

    // Various attributes
    ESATypeEnum mEsaType;
    bool mEsaCanGenerate;
    ESAStateEnum mEsaState;
    int64_t mAbsMinPower;
    int64_t mAbsMaxPower;
    OptOutStateEnum mOptOutState;

    DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> mPowerAdjustCapabilityStruct;

    DataModel::Nullable<Structs::ForecastStruct::Type> mForecast;

    // Keep track whether a PowerAdjustment is in progress
    bool mPowerAdjustmentInProgress;

    // Keep track of when that PowerAdjustment started
    uint32_t mPowerAdjustmentStartTime;

    // Keep track whether a PauseRequest is in progress
    bool mPauseRequestInProgress;

    // Keep track of when that PauseRequest started
    uint32_t mPauseRequestStartTime;
};

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
