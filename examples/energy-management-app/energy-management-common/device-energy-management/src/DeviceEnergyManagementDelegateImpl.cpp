/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include "DeviceEnergyManagementDelegateImpl.h"
#include "DEMManufacturerDelegate.h"
#include "EnergyTimeUtils.h"
#include <app/EventLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

using chip::Protocols::InteractionModel::Status;

using chip::Optional;
using CostsList = DataModel::List<const Structs::CostStruct::Type>;

DeviceEnergyManagementDelegate::DeviceEnergyManagementDelegate() :
    mpDEMManufacturerDelegate(nullptr), mEsaType(ESATypeEnum::kEvse), mEsaCanGenerate(false), mEsaState(ESAStateEnum::kOffline),
    mAbsMinPowerMw(0), mAbsMaxPowerMw(0), mOptOutState(OptOutStateEnum::kNoOptOut), mPowerAdjustmentInProgress(false),
    mPowerAdjustmentStartTimeUtc(0), mPauseRequestInProgress(false)
{}

void DeviceEnergyManagementDelegate::SetDeviceEnergyManagementInstance(DeviceEnergyManagement::Instance & instance)
{
    mpDEMInstance = &instance;
}

uint32_t DeviceEnergyManagementDelegate::HasFeature(Feature feature) const
{
    bool hasFeature = false;

    if (mpDEMInstance != nullptr)
    {
        hasFeature = mpDEMInstance->HasFeature(feature);
    }

    return hasFeature;
}

void DeviceEnergyManagementDelegate::SetDEMManufacturerDelegate(
    DEMManufacturerDelegate & deviceEnergyManagementManufacturerDelegate)
{
    mpDEMManufacturerDelegate = &deviceEnergyManagementManufacturerDelegate;
}

/**
 * @brief Delegate handler for PowerAdjustRequest
 *
 * Note: checking of the validity of the PowerAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should apply a new power setting.
 * It should:
 *   1) notify the appliance - if the appliance hardware cannot be adjusted, then return Failure
 *   2) start a timer (or restart the existing PowerAdjust timer) for duration seconds
 *   3) generate a PowerAdjustStart event (if there is not an existing PowerAdjustRequest running)
 *   4) if appropriate, update the forecast with the new expected end time
 *
 *  and when the timer expires:
 *   5) notify the appliance's that it can resume its intended power setting (or go idle)
 *   6) generate a PowerAdjustEnd event with cause NormalCompletion
 *   7) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::PowerAdjustRequest(const int64_t powerMw, const uint32_t durationS,
                                                          AdjustmentCauseEnum cause)
{
    bool generateEvent = false;

    // If a timer is running, cancel it so we can start it with the new duration
    if (mPowerAdjustmentInProgress)
    {
        DeviceLayer::SystemLayer().CancelTimer(PowerAdjustTimerExpiry, this);
    }
    else
    {
        // Going to start a new power adjustment so will need to generate an event
        generateEvent = true;

        // Record when this PowerAdjustment starts. Note if we do not set this value if a PowerAdjustment is in progress
        CHIP_ERROR err = GetEpochTS(mPowerAdjustmentStartTimeUtc);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Unable to get time: %" CHIP_ERROR_FORMAT, err.Format());
            return Status::Failure;
        }
    }

    //  Update the forecast with the new expected end time
    if (mpDEMManufacturerDelegate != nullptr)
    {
        CHIP_ERROR err = mpDEMManufacturerDelegate->HandleDeviceEnergyManagementPowerAdjustRequest(powerMw, durationS, cause);
        if (err != CHIP_NO_ERROR)
        {
            return Status::Failure;
        }
    }

    SetESAState(ESAStateEnum::kPowerAdjustActive);

    // mPowerAdjustCapabilityStruct is guaranteed to have a value as validated in Instance::HandlePowerAdjustRequest.
    // If it did not have a value, this method would not have been called.
    switch (cause)
    {
    case AdjustmentCauseEnum::kLocalOptimization:
        SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum::kLocalOptimizationAdjustment);
        break;

    case AdjustmentCauseEnum::kGridOptimization:
        SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum::kGridOptimizationAdjustment);
        break;

    default:
        HandlePowerAdjustRequestFailure();
        return Status::Failure;
    }

    // Remember we have a timer running so we don't generate a PowerAdjustStart event should another request come
    // in before this timer expires
    mPowerAdjustmentInProgress = true;

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(durationS), PowerAdjustTimerExpiry, this);
    if (err != CHIP_NO_ERROR)
    {
        // TODO: Note: should the PowerAdjust just initiated be cancelled because an Event could not be logged?
        ChipLogError(AppServer, "Unable to start a PowerAdjustStart timer: %" CHIP_ERROR_FORMAT, err.Format());
        HandlePowerAdjustRequestFailure();
        return Status::Failure;
    }

    if (generateEvent)
    {
        Events::PowerAdjustStart::Type event;
        EventNumber eventNumber;
        err = LogEvent(event, mEndpointId, eventNumber);
        if (CHIP_NO_ERROR != err)
        {
            // TODO: Note: should the PowerAdjust just initiated be cancelled because an Event could not be logged?
            ChipLogError(AppServer, "Unable to generate PowerAdjustStart event: %" CHIP_ERROR_FORMAT, err.Format());
            HandlePowerAdjustRequestFailure();
            return Status::Failure;
        }
    }

    return Status::Success;
}

/**
 * @brief Handle a PowerAdjustRequest failing
 *
 *  Cleans up the PowerAdjust state should the request fail
 */
void DeviceEnergyManagementDelegate::HandlePowerAdjustRequestFailure()
{
    DeviceLayer::SystemLayer().CancelTimer(PowerAdjustTimerExpiry, this);

    SetESAState(ESAStateEnum::kOnline);

    mPowerAdjustmentInProgress = false;

    SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum::kNoAdjustment);

    // TODO
    // Should we inform the mpDEMManufacturerDelegate that PowerAdjustRequest has failed?
}

/**
 * @brief Timer for handling the PowerAdjustRequest
 *
 * This static function calls the non-static HandlePowerAdjustTimerExpiry method.
 */
void DeviceEnergyManagementDelegate::PowerAdjustTimerExpiry(System::Layer * systemLayer, void * delegate)
{
    DeviceEnergyManagementDelegate * dg = reinterpret_cast<DeviceEnergyManagementDelegate *>(delegate);

    dg->HandlePowerAdjustTimerExpiry();
}

/**
 * @brief Timer for handling the completion of a PowerAdjustRequest
 *
 *  When the timer expires:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause NormalCompletion
 *   3) if necessary, update the forecast with new expected end time
 */
void DeviceEnergyManagementDelegate::HandlePowerAdjustTimerExpiry()
{
    ChipLogError(AppServer, "DeviceEnergyManagementDelegate::HandlePowerAdjustTimerExpiry");

    // The PowerAdjustment is no longer in progress
    mPowerAdjustmentInProgress = false;

    SetESAState(ESAStateEnum::kOnline);

    SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum::kNoAdjustment);

    // Generate a PowerAdjustEnd event
    GeneratePowerAdjustEndEvent(CauseEnum::kNormalCompletion);

    // Update the forecast with new expected end time
    if (mpDEMManufacturerDelegate != nullptr)
    {
        mpDEMManufacturerDelegate->HandleDeviceEnergyManagementPowerAdjustCompletion();
    }
}

/**
 * @brief Delegate handler for CancelPowerAdjustRequest
 *
 * Note: checking of the validity of the CancelPowerAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should resume its intended power setting (or go idle).
 *
 * It should:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause code Cancelled
 *   3) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::CancelPowerAdjustRequest()
{
    Status status = Status::Success;

    CHIP_ERROR err = CancelPowerAdjustRequestAndGenerateEvent(DeviceEnergyManagement::CauseEnum::kCancelled);
    if (CHIP_NO_ERROR != err)
    {
        status = Status::Failure;
    }

    return status;
}

/**
 * @brief Handles the cancelation of a PowerAdjust operation
 *
 * This function needs to notify the appliance that it should resume its intended power setting (or go idle).
 *
 * It should:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause code Cancelled
 *   3) if necessary, update the forecast with new expected end time
 */
CHIP_ERROR DeviceEnergyManagementDelegate::CancelPowerAdjustRequestAndGenerateEvent(CauseEnum cause)
{
    DeviceLayer::SystemLayer().CancelTimer(PowerAdjustTimerExpiry, this);

    SetESAState(ESAStateEnum::kOnline);

    mPowerAdjustmentInProgress = false;
    SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum::kNoAdjustment);

    CHIP_ERROR err = GeneratePowerAdjustEndEvent(cause);

    // Notify the appliance's that it can resume its intended power setting (or go idle)
    if (mpDEMManufacturerDelegate != nullptr)
    {
        // It is expected the mpDEMManufacturerDelegate will update the forecast with new expected end time
        // as a consequence of the cancel request.
        err = mpDEMManufacturerDelegate->HandleDeviceEnergyManagementCancelPowerAdjustRequest(cause);
    }

    return err;
}

/**
 * @brief Generate a PowerAdjustEvent
 *
 */
CHIP_ERROR DeviceEnergyManagementDelegate::GeneratePowerAdjustEndEvent(CauseEnum cause)
{
    Events::PowerAdjustEnd::Type event;
    EventNumber eventNumber;
    event.cause = cause;

    uint32_t timeNowUtc;
    CHIP_ERROR err = GetEpochTS(timeNowUtc);
    if (err == CHIP_NO_ERROR)
    {
        event.duration = timeNowUtc - mPowerAdjustmentStartTimeUtc;
    }
    else
    {
        ChipLogError(AppServer, "Unable to get time: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    if (mpDEMManufacturerDelegate != nullptr)
    {
        event.energyUse = mpDEMManufacturerDelegate->GetApproxEnergyDuringSession();
    }
    else
    {
        event.energyUse = 0;
    }

    err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to generate PowerAdjustEnd event: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return err;
}

/**
 * @brief Delegate handler for StartTimeAdjustRequest
 *
 * Note: checking of the validity of the StartTimeAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that the forecast has been updated by a client.
 *
 * It should:
 *      1) update the forecast attribute with the revised start time
 *      2) send a callback notification to the appliance so it can refresh its internal schedule
 */
Status DeviceEnergyManagementDelegate::StartTimeAdjustRequest(const uint32_t requestedStartTimeUtc, AdjustmentCauseEnum cause)
{
    if (mForecast.IsNull())
    {
        return Status::Failure;
    }

    switch (cause)
    {
    case AdjustmentCauseEnum::kLocalOptimization:
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kLocalOptimization;
        break;
    case AdjustmentCauseEnum::kGridOptimization:
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kGridOptimization;
        break;
    default:
        ChipLogDetail(AppServer, "Bad cause %d", to_underlying(cause));
        return Status::Failure;
        break;
    }

    mForecast.Value().forecastID++;

    uint32_t durationS = mForecast.Value().endTime - mForecast.Value().startTime; // the current entire forecast duration

    // Save the start and end time in case there is an issue with the mpDEMManufacturerDelegate handling this
    // startTimeAdjustment request
    uint32_t savedStartTime = mForecast.Value().startTime;
    uint32_t savedEndTime   = mForecast.Value().endTime;

    /* Modify start time and end time */
    mForecast.Value().startTime = requestedStartTimeUtc;
    mForecast.Value().endTime   = requestedStartTimeUtc + durationS;

    if (mpDEMManufacturerDelegate != nullptr)
    {
        CHIP_ERROR err =
            mpDEMManufacturerDelegate->HandleDeviceEnergyManagementStartTimeAdjustRequest(requestedStartTimeUtc, cause);
        if (err != CHIP_NO_ERROR)
        {
            // Reset state
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;
            mForecast.Value().startTime            = savedStartTime;
            mForecast.Value().endTime              = savedEndTime;

            MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

            return Status::Failure;
        }
    }

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

    return Status::Success;
}

/**
 * @brief Delegate handler for Pause Request
 *
 * Note: checking of the validity of the Pause Request has been done by the lower layer
 *
 * This function needs to notify the appliance that it should now pause.
 * It should:
 *   1) pause the appliance - if the appliance hardware cannot be paused, then return Failure
 *   2) start a timer for duration seconds
 *   3) generate a Paused event
 *   4) update the forecast with the new expected end time
 *
 *  and when the timer expires:
 *   5) restore the appliance's operational state
 *   6) generate a Resumed event
 *   7) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::PauseRequest(const uint32_t durationS, AdjustmentCauseEnum cause)
{
    bool generateEvent = false;

    // If a timer is running, cancel it so we can start it with the new duration
    if (mPauseRequestInProgress)
    {
        DeviceLayer::SystemLayer().CancelTimer(PauseRequestTimerExpiry, this);
    }
    else
    {
        generateEvent = true;

        // Remember we have a timer running so we don't generate a Paused event should another request come
        // in before this timer expires
        mPauseRequestInProgress = true;
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(durationS), PauseRequestTimerExpiry, this);
    if (err != CHIP_NO_ERROR)
    {
        HandlePauseRequestFailure();
        return Status::Failure;
    }

    // Pause the appliance
    if (mpDEMManufacturerDelegate != nullptr)
    {
        // It is expected that the mpDEMManufacturerDelegate will update the forecast with the new expected end time
        err = mpDEMManufacturerDelegate->HandleDeviceEnergyManagementPauseRequest(durationS, cause);
        if (err != CHIP_NO_ERROR)
        {
            HandlePauseRequestFailure();
            return Status::Failure;
        }
    }

    if (generateEvent)
    {
        Events::Paused::Type event;
        EventNumber eventNumber;
        err = LogEvent(event, mEndpointId, eventNumber);
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(AppServer, "Unable to generate Paused event: %" CHIP_ERROR_FORMAT, err.Format());
            HandlePauseRequestFailure();
            return Status::Failure;
        }
    }

    SetESAState(ESAStateEnum::kPaused);

    // Update the forecaseUpdateReason based on the AdjustmentCause
    if (cause == AdjustmentCauseEnum::kLocalOptimization)
    {
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kLocalOptimization;

        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
    }
    else if (cause == AdjustmentCauseEnum::kGridOptimization)
    {
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kGridOptimization;

        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
    }

    return Status::Success;
}

/**
 * @brief Handle a PauseRequest failing
 *
 *  Cleans up the state should the PauseRequest fail
 */
void DeviceEnergyManagementDelegate::HandlePauseRequestFailure()
{
    DeviceLayer::SystemLayer().CancelTimer(PowerAdjustTimerExpiry, this);

    SetESAState(ESAStateEnum::kOnline);

    mPauseRequestInProgress = false;

    // TODO
    // Should we inform the mpDEMManufacturerDelegate that PauseRequest has failed?
}

/**
 * @brief Timer for handling the PauseRequest
 *
 * This static function calls the non-static HandlePauseRequestTimerExpiry method.
 */
void DeviceEnergyManagementDelegate::PauseRequestTimerExpiry(System::Layer * systemLayer, void * delegate)
{
    DeviceEnergyManagementDelegate * dg = reinterpret_cast<DeviceEnergyManagementDelegate *>(delegate);

    dg->HandlePauseRequestTimerExpiry();
}

/**
 * @brief Timer for handling the completion of a PauseRequest
 *
 *  When the timer expires:
 *   1) restore the appliance's operational state
 *   2) generate a Resumed event
 *   3) if necessary, update the forecast with new expected end time
 */
void DeviceEnergyManagementDelegate::HandlePauseRequestTimerExpiry()
{
    // The PauseRequestment is no longer in progress
    mPauseRequestInProgress = false;

    SetESAState(ESAStateEnum::kOnline);

    // Generate a Resumed event
    GenerateResumedEvent(CauseEnum::kNormalCompletion);

    // It is expected the mpDEMManufacturerDelegate will update the forecast with new expected end time
    if (mpDEMManufacturerDelegate != nullptr)
    {
        mpDEMManufacturerDelegate->HandleDeviceEnergyManagementPauseCompletion();
    }
}

/**
 * @brief Handles the cancelation of a pause operation
 *
 * This function needs to notify the appliance that it should resume its intended power setting (or go idle).
 *
 * It should:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause code Cancelled
 *   3) if necessary, update the forecast with new expected end time
 */
CHIP_ERROR DeviceEnergyManagementDelegate::CancelPauseRequestAndGenerateEvent(CauseEnum cause)
{
    mPauseRequestInProgress = false;

    SetESAState(ESAStateEnum::kOnline);

    DeviceLayer::SystemLayer().CancelTimer(PauseRequestTimerExpiry, this);

    CHIP_ERROR err  = GenerateResumedEvent(cause);
    CHIP_ERROR err2 = CHIP_NO_ERROR;

    // Notify the appliance's that it can resume its intended power setting (or go idle)
    if (mpDEMManufacturerDelegate != nullptr)
    {
        // It is expected that the mpDEMManufacturerDelegate will update the forecast with new expected end time
        err2 = mpDEMManufacturerDelegate->HandleDeviceEnergyManagementCancelPauseRequest(cause);
    }

    // Need to pick one of the error codes two return...
    if (err == CHIP_NO_ERROR && err2 == CHIP_NO_ERROR)
    {
        return CHIP_NO_ERROR;
    }

    if (err2 != CHIP_NO_ERROR)
    {
        return err2;
    }

    return err;
}

/**
 * @brief Generate a Resumed event
 *
 */
CHIP_ERROR DeviceEnergyManagementDelegate::GenerateResumedEvent(CauseEnum cause)
{
    Events::Resumed::Type event;
    EventNumber eventNumber;
    event.cause = cause;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to generate Resumed event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

/**
 * @brief Delegate handler for ResumeRequest
 *
 * Note: checking of the validity of the ResumeRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should now resume operation
 *
 * It should:
 *   1) restore the appliance's operational state
 *   2) generate a Resumed event
 *   3) update the forecast with new expected end time (given that the pause duration was shorter than originally requested)
 *
 */
Status DeviceEnergyManagementDelegate::ResumeRequest()
{
    Status status = Status::Failure;

    if (mPauseRequestInProgress)
    {
        // Guard against mForecast being null
        if (!mForecast.IsNull())
        {
            // The PauseRequest has effectively been cancelled so as a result the device should
            // go back to InternalOptimisation
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;

            MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
        }

        CHIP_ERROR err = CancelPauseRequestAndGenerateEvent(CauseEnum::kCancelled);
        if (err == CHIP_NO_ERROR)
        {
            status = Status::Success;
        }
    }

    return status;
}

/**
 * @brief Delegate handler for ModifyForecastRequest
 *
 * Note: Only basic checking of the validity of the ModifyForecastRequest has been
 * done by the lower layer. This is a more complex use-case and requires higher-level
 * work by the delegate.
 *
 * It should:
 *      1) determine if the new forecast adjustments are acceptable to the appliance
 *       - if not return Failure. For example, if it may cause the home to be too hot
 *         or too cold, or a battery to be insufficiently charged
 *      2) if the slot adjustments are acceptable, then update the forecast
 *      3) notify the appliance to follow the revised schedule
 */
Status DeviceEnergyManagementDelegate::ModifyForecastRequest(
    const uint32_t forecastID, const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
    AdjustmentCauseEnum cause)
{
    Status status = Status::Success;

    if (mForecast.IsNull())
    {
        status = Status::Failure;
    }
    else if (mForecast.Value().forecastID != forecastID)
    {
        status = Status::Failure;
    }
    else if (mpDEMManufacturerDelegate != nullptr)
    {
        // Determine if the new forecast adjustments are acceptable to the appliance
        CHIP_ERROR err = mpDEMManufacturerDelegate->HandleModifyForecastRequest(forecastID, slotAdjustments, cause);
        if (err != CHIP_NO_ERROR)
        {
            status = Status::Failure;
        }
    }

    if (status == Status::Success)
    {
        switch (cause)
        {
        case AdjustmentCauseEnum::kLocalOptimization:
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kLocalOptimization;
            break;
        case AdjustmentCauseEnum::kGridOptimization:
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kGridOptimization;
            break;
        default:
            // Already checked in chip::app::Clusters::DeviceEnergyManagement::Instance::HandleModifyForecastRequest
            break;
        }

        mForecast.Value().forecastID++;

        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
    }

    return status;
}

/**
 * @brief Delegate handler for RequestConstraintBasedForecast
 *
 * Note: Only basic checking of the validity of the RequestConstraintBasedForecast has been
 * done by the lower layer. This is a more complex use-case and requires higher-level
 * work by the delegate.
 *
 * It should:
 *      1) perform a higher level optimization (e.g. using tariff information, and user preferences)
 *      2) if a solution can be found, then update the forecast, else return Failure
 *      3) notify the appliance to follow the revised schedule
 */
Status DeviceEnergyManagementDelegate::RequestConstraintBasedForecast(
    const DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> & constraints, AdjustmentCauseEnum cause)
{
    Status status = Status::Success;

    if (mForecast.IsNull())
    {
        status = Status::Failure;
    }
    else if (mpDEMManufacturerDelegate != nullptr)
    {
        // Determine if the new forecast adjustments are acceptable to the appliance
        CHIP_ERROR err = mpDEMManufacturerDelegate->RequestConstraintBasedForecast(constraints, cause);
        if (err != CHIP_NO_ERROR)
        {
            status = Status::Failure;
        }
    }

    if (status == Status::Success)
    {
        switch (cause)
        {
        case AdjustmentCauseEnum::kLocalOptimization:
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kLocalOptimization;
            break;
        case AdjustmentCauseEnum::kGridOptimization:
            mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kGridOptimization;
            break;
        default:
            // Already checked in chip::app::Clusters::DeviceEnergyManagement::Instance::HandleModifyForecastRequest
            break;
        }

        mForecast.Value().forecastID++;

        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

        status = Status::Success;
    }

    return status;
}

/**
 * @brief Delegate handler for CancelRequest
 *
 * Note: This is a more complex use-case and requires higher-level work by the delegate.
 *
 * It SHALL:
 *      1) Check if the forecastUpdateReason was already InternalOptimization (and reject the command)
 *      2) Update its forecast (based on its optimization strategy) ignoring previous requests
 *      3) Update its Forecast attribute to match its new intended operation, and update the
 *         ForecastStruct.ForecastUpdateReason to `Internal Optimization`.
 */
Status DeviceEnergyManagementDelegate::CancelRequest()
{
    Status status = Status::Success;

    mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

    /* It is expected the mpDEMManufacturerDelegate will cancel the effects of any previous adjustment
     * request commands, and re-evaluate its forecast for intended operation ignoring those previous
     * requests.
     */
    if (mpDEMManufacturerDelegate != nullptr)
    {
        CHIP_ERROR error = mpDEMManufacturerDelegate->HandleDeviceEnergyManagementCancelRequest();
        if (error != CHIP_NO_ERROR)
        {
            status = Status::Failure;
        }
    }

    return status;
}

// ------------------------------------------------------------------
// Get attribute methods
ESATypeEnum DeviceEnergyManagementDelegate::GetESAType()
{
    return mEsaType;
}

bool DeviceEnergyManagementDelegate::GetESACanGenerate()
{
    return mEsaCanGenerate;
}

ESAStateEnum DeviceEnergyManagementDelegate::GetESAState()
{
    return mEsaState;
}

int64_t DeviceEnergyManagementDelegate::GetAbsMinPower()
{
    return mAbsMinPowerMw;
}

int64_t DeviceEnergyManagementDelegate::GetAbsMaxPower()
{
    return mAbsMaxPowerMw;
}

const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> &
DeviceEnergyManagementDelegate::GetPowerAdjustmentCapability()
{
    return mPowerAdjustCapabilityStruct;
}

const DataModel::Nullable<Structs::ForecastStruct::Type> & DeviceEnergyManagementDelegate::GetForecast()
{
    ChipLogDetail(Zcl, "DeviceEnergyManagementDelegate::GetForecast");

    return mForecast;
}

OptOutStateEnum DeviceEnergyManagementDelegate::GetOptOutState()
{
    ChipLogDetail(AppServer, "mOptOutState %d", to_underlying(mOptOutState));
    return mOptOutState;
}

// ------------------------------------------------------------------
// Set attribute methods

CHIP_ERROR DeviceEnergyManagementDelegate::SetESAType(ESATypeEnum newValue)
{
    ESATypeEnum oldValue = mEsaType;

    if (newValue >= ESATypeEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mEsaType = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaType updated to %d", static_cast<int>(mEsaType));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAType::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetESACanGenerate(bool newValue)
{
    bool oldValue = mEsaCanGenerate;

    mEsaCanGenerate = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaCanGenerate updated to %d", static_cast<int>(mEsaCanGenerate));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESACanGenerate::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetESAState(ESAStateEnum newValue)
{
    ESAStateEnum oldValue = mEsaState;

    if (newValue >= ESAStateEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mEsaState = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaState updated to %d", static_cast<int>(mEsaState));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetAbsMinPower(int64_t newValueMw)
{
    int64_t oldValueMw = mAbsMinPowerMw;

    mAbsMinPowerMw = newValueMw;
    if (oldValueMw != newValueMw)
    {
        ChipLogDetail(AppServer, "mAbsMinPower updated to " ChipLogFormatX64, ChipLogValueX64(mAbsMinPowerMw));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, AbsMinPower::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetAbsMaxPower(int64_t newValueMw)
{
    int64_t oldValueMw = mAbsMaxPowerMw;

    mAbsMaxPowerMw = newValueMw;
    if (oldValueMw != newValueMw)
    {
        ChipLogDetail(AppServer, "mAbsMaxPower updated to " ChipLogFormatX64, ChipLogValueX64(mAbsMaxPowerMw));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, AbsMaxPower::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DeviceEnergyManagementDelegate::SetPowerAdjustmentCapability(
    const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & powerAdjustCapabilityStruct)
{
    assertChipStackLockedByCurrentThread();

    mPowerAdjustCapabilityStruct = powerAdjustCapabilityStruct;

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, PowerAdjustmentCapability::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DeviceEnergyManagementDelegate::SetPowerAdjustmentCapabilityPowerAdjustReason(PowerAdjustReasonEnum powerAdjustReason)
{
    assertChipStackLockedByCurrentThread();

    mPowerAdjustCapabilityStruct.Value().cause = powerAdjustReason;

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, PowerAdjustmentCapability::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetForecast(const DataModel::Nullable<Structs::ForecastStruct::Type> & forecast)
{
    assertChipStackLockedByCurrentThread();

    // TODO see Issue #31147
    mForecast = forecast;

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetOptOutState(OptOutStateEnum newValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    OptOutStateEnum oldValue = mOptOutState;

    // The OptOutState is cumulative
    if ((oldValue == OptOutStateEnum::kGridOptOut && newValue == OptOutStateEnum::kLocalOptOut) ||
        (oldValue == OptOutStateEnum::kLocalOptOut && newValue == OptOutStateEnum::kGridOptOut))
    {
        mOptOutState = OptOutStateEnum::kOptOut;
    }
    else
    {
        mOptOutState = newValue;
    }

    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mOptOutState updated to %d mPowerAdjustmentInProgress %d", to_underlying(mOptOutState),
                      mPowerAdjustmentInProgress);
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, OptOutState::Id);
    }

    // Cancel any outstanding PowerAdjustment if necessary
    if (mPowerAdjustmentInProgress)
    {
        if ((newValue == OptOutStateEnum::kLocalOptOut &&
             GetPowerAdjustmentCapability().Value().cause == PowerAdjustReasonEnum::kLocalOptimizationAdjustment) ||
            (newValue == OptOutStateEnum::kGridOptOut &&
             GetPowerAdjustmentCapability().Value().cause == PowerAdjustReasonEnum::kGridOptimizationAdjustment) ||
            newValue == OptOutStateEnum::kOptOut)
        {
            err = CancelPowerAdjustRequestAndGenerateEvent(DeviceEnergyManagement::CauseEnum::kUserOptOut);
        }
    }

    // Cancel any outstanding PauseRequest if necessary
    if (mPauseRequestInProgress)
    {
        // Cancel any outstanding PauseRequest
        if ((newValue == OptOutStateEnum::kLocalOptOut &&
             mForecast.Value().forecastUpdateReason == ForecastUpdateReasonEnum::kLocalOptimization) ||
            (newValue == OptOutStateEnum::kGridOptOut &&
             mForecast.Value().forecastUpdateReason == ForecastUpdateReasonEnum::kGridOptimization) ||
            newValue == OptOutStateEnum::kOptOut)
        {
            err = CancelPauseRequestAndGenerateEvent(DeviceEnergyManagement::CauseEnum::kUserOptOut);
        }
    }

    if (!mForecast.IsNull())
    {
        switch (mForecast.Value().forecastUpdateReason)
        {
        case ForecastUpdateReasonEnum::kInternalOptimization:
            // We don't need to redo a forecast since its internal already
            break;
        case ForecastUpdateReasonEnum::kLocalOptimization:
            if ((mOptOutState == OptOutStateEnum::kOptOut) || (mOptOutState == OptOutStateEnum::kLocalOptOut))
            {
                mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;

                MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
                // Generate a new forecast with Internal Optimization
                // TODO
            }
            break;
        case ForecastUpdateReasonEnum::kGridOptimization:
            if ((mOptOutState == OptOutStateEnum::kOptOut) || (mOptOutState == OptOutStateEnum::kGridOptOut))
            {
                mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;

                MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);
                // Generate a new forecast with Internal Optimization
                // TODO
            }
            break;
        default:
            ChipLogDetail(AppServer, "Bad ForecastUpdateReasonEnum value of %d",
                          to_underlying(mForecast.Value().forecastUpdateReason));
            return CHIP_ERROR_BAD_REQUEST;
            break;
        }
    }

    return err;
}
