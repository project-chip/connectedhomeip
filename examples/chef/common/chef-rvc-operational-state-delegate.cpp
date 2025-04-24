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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <chef-rvc-operational-state-delegate.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::RvcOperationalState;
using chip::Protocols::InteractionModel::Status;

#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
#include <chef-rvc-mode-delegate.h>
#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER

static std::unique_ptr<RvcOperationalStateDelegate> gRvcOperationalStateDelegate;
static std::unique_ptr<RvcOperationalState::Instance> gRvcOperationalStateInstance;

RvcOperationalStateDelegate * getRvcOperationalStateDelegate()
{
    return gRvcOperationalStateDelegate.get();
}

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data);

DataModel::Nullable<uint32_t> RvcOperationalStateDelegate::GetCountdownTime()
{
    if (mCountdownTime.IsNull() || mRunningTime > mCountdownTime.Value())
        return DataModel::NullNullable;

    return DataModel::MakeNullable((uint32_t) (mCountdownTime.Value() - mRunningTime));
}

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    if (index >= mOperationalStateList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

void RvcOperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    RvcOperationalState::OperationalStateEnum current_state =
        static_cast<RvcOperationalState::OperationalStateEnum>(gRvcOperationalStateInstance->GetCurrentOperationalState());

    if (current_state == RvcOperationalState::OperationalStateEnum::kPaused)
    {
        err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kNoError));
        return;
    }

    if (current_state == RvcOperationalState::OperationalStateEnum::kRunning)
    {
        auto error =
            gRvcOperationalStateInstance->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kPaused));
        if (error == CHIP_NO_ERROR)
        {
            err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kNoError));
        }
        else
        {
            err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        }
        return;
    }

    err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kCommandInvalidInState));
}

void RvcOperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    RvcOperationalState::OperationalStateEnum current_state =
        static_cast<RvcOperationalState::OperationalStateEnum>(gRvcOperationalStateInstance->GetCurrentOperationalState());

    if (current_state == RvcOperationalState::OperationalStateEnum::kStopped ||
        current_state == RvcOperationalState::OperationalStateEnum::kError)
    {
        err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kUnableToStartOrResume));
        return;
    }

    if (current_state == RvcOperationalState::OperationalStateEnum::kRunning)
    {
        err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kNoError));
        return;
    }

    if (current_state == RvcOperationalState::OperationalStateEnum::kPaused)
    {
        auto error =
            gRvcOperationalStateInstance->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kRunning));
        if (error == CHIP_NO_ERROR)
        {
            err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kNoError));
        }
        else
        {
            err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        }
        return;
    }

    err.Set(to_underlying(RvcOperationalState::ErrorStateEnum::kCommandInvalidInState));
}

void RvcOperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    GetInstance()->GetCurrentOperationalError(current_err);

    if (current_err.errorStateID != to_underlying(OperationalState::ErrorStateEnum::kNoError))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToStartOrResume));
        return;
    }

    RvcOperationalState::OperationalStateEnum current_state =
        static_cast<RvcOperationalState::OperationalStateEnum>(gRvcOperationalStateInstance->GetCurrentOperationalState());

    if (current_state == RvcOperationalState::OperationalStateEnum::kRunning ||
        current_state == RvcOperationalState::OperationalStateEnum::kPaused)
    {
        ChipLogDetail(DeviceLayer, "HandleStartStateCallback: RVC is already started. Current state = %d. Returning.",
                      to_underlying(current_state));
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
        return;
    }

    if (to_underlying(current_state) != to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) &&
        to_underlying(current_state) != to_underlying(RvcOperationalState::OperationalStateEnum::kStopped))
    {
        ChipLogError(
            DeviceLayer,
            "HandleStartStateCallback: RVC must be in either charging or stopped state before starting. current state = %d",
            to_underlying(current_state));
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        // Start RVC run cycle.
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, GetInstance());
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void RvcOperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{

    RvcOperationalState::OperationalStateEnum current_state =
        static_cast<RvcOperationalState::OperationalStateEnum>(gRvcOperationalStateInstance->GetCurrentOperationalState());

    if (current_state != RvcOperationalState::OperationalStateEnum::kRunning &&
        current_state != RvcOperationalState::OperationalStateEnum::kPaused)
    {
        ChipLogDetail(DeviceLayer, "HandleStopStateCallback: RVC not started. Current state = %d. Returning.",
                      to_underlying(current_state));
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
        return;
    }
    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    if (error == CHIP_NO_ERROR)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, this);

        OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
        GetInstance()->GetCurrentOperationalError(current_err);

        Optional<DataModel::Nullable<uint32_t>> totalTime((DataModel::Nullable<uint32_t>(mRunningTime + mPausedTime)));
        Optional<DataModel::Nullable<uint32_t>> pausedTime((DataModel::Nullable<uint32_t>(mPausedTime)));

        GetInstance()->OnOperationCompletionDetected(static_cast<uint8_t>(current_err.errorStateID), totalTime, pausedTime);

        mRunningTime = 0;
        mPausedTime  = 0;
        mCountdownTime.SetNull();
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
        GetInstance()->UpdateCountdownTimeFromDelegate();
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void RvcOperationalStateDelegate::HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err)
{

    RvcOperationalState::OperationalStateEnum current_state =
        static_cast<RvcOperationalState::OperationalStateEnum>(gRvcOperationalStateInstance->GetCurrentOperationalState());

    if (current_state == RvcOperationalState::OperationalStateEnum::kRunning ||
        current_state == RvcOperationalState::OperationalStateEnum::kPaused)
    {
        ChipLogDetail(DeviceLayer, "HandleGoHomeCommandCallback: RVC was started, current state = %d. Stopping RVC.",
                      to_underlying(current_state));
        gRvcOperationalStateDelegate->HandleStopStateCallback(err);
    }

    // Skip SeekingCharger and Docking states and directly go into charging.
    auto error =
        gRvcOperationalStateInstance->SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kCharging));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data)
{
    RvcOperationalStateDelegate * delegate = reinterpret_cast<RvcOperationalStateDelegate *>(data);

    OperationalState::Instance * instance = gRvcOperationalStateInstance.get();
    OperationalState::OperationalStateEnum state =
        static_cast<OperationalState::OperationalStateEnum>(instance->GetCurrentOperationalState());

    if (state != OperationalState::OperationalStateEnum::kRunning &&
        state != OperationalState::OperationalStateEnum::kPaused) // Timer shouldn't run when RVC is not in Running or Paused.
    {
        return;
    }

    if (gRvcOperationalStateDelegate->mCountdownTime.IsNull())
    {
        if (state == OperationalState::OperationalStateEnum::kRunning)
        {
            gRvcOperationalStateDelegate->mCountdownTime.SetNonNull(
                static_cast<uint32_t>(gRvcOperationalStateDelegate->kExampleCountDown));
            gRvcOperationalStateDelegate->mRunningTime = 0;
            gRvcOperationalStateDelegate->mPausedTime  = 0;
            instance->UpdateCountdownTimeFromDelegate();
        }
        else
        { // kPaused
            ChipLogError(DeviceLayer, "RVC timer tick: Invalid state. Device is in kPaused but mCountdownTime is NULL.");
            return;
        }
    }

    if (state == OperationalState::OperationalStateEnum::kRunning)
    {
        gRvcOperationalStateDelegate->mRunningTime++;
    }
    else if (state == OperationalState::OperationalStateEnum::kPaused)
    {
        gRvcOperationalStateDelegate->mPausedTime++;
    }

    uint32_t mPausedTime  = gRvcOperationalStateDelegate->mPausedTime;
    uint32_t mRunningTime = gRvcOperationalStateDelegate->mRunningTime;

    ChipLogDetail(DeviceLayer, "RVC timer tick: Current state = %d. CountdownTime = %d. PauseTime = %d. RunningTime = %d.",
                  to_underlying(state), gRvcOperationalStateDelegate->mCountdownTime.Value(), mPausedTime, mRunningTime);
    if (state == OperationalState::OperationalStateEnum::kRunning)
    {
        // Reported CountDownTime is the remaining time to run = mCountdownTime.Value() - mRunningTime.
        instance->UpdateCountdownTimeFromDelegate();
    }

    if (gRvcOperationalStateDelegate->mCountdownTime.Value() > mRunningTime)
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, delegate);
    }
    else
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);

        CHIP_ERROR err =
            gRvcOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
        if (err == CHIP_NO_ERROR)
        {
            OperationalState::GenericOperationalError current_err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
            gRvcOperationalStateInstance->GetCurrentOperationalError(current_err);

            Optional<DataModel::Nullable<uint32_t>> totalTime((DataModel::Nullable<uint32_t>(mPausedTime + mRunningTime)));
            Optional<DataModel::Nullable<uint32_t>> pausedTime((DataModel::Nullable<uint32_t>(mPausedTime)));

            gRvcOperationalStateInstance->OnOperationCompletionDetected(static_cast<uint8_t>(current_err.errorStateID), totalTime,
                                                                        pausedTime);

            gRvcOperationalStateDelegate->mRunningTime = 0;
            gRvcOperationalStateDelegate->mPausedTime  = 0;
            gRvcOperationalStateDelegate->mCountdownTime.SetNull();
            instance->UpdateCountdownTimeFromDelegate();

#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
            getRvcRunModeInstance()->UpdateCurrentMode(RvcRunMode::ModeIdle);
#endif // MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
        }
    }
}

void RvcOperationalState::Shutdown()
{
    gRvcOperationalStateInstance.reset();
    gRvcOperationalStateDelegate.reset();
}

chip::Protocols::InteractionModel::Status chefRvcOperationalStateWriteCallback(chip::EndpointId endpointId,
                                                                               chip::ClusterId clusterId,
                                                                               const EmberAfAttributeMetadata * attributeMetadata,
                                                                               uint8_t * buffer)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcOperationalStateInstance != nullptr);
    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::RvcOperationalState::Attributes::CurrentPhase::Id: {
        uint8_t m = static_cast<uint8_t>(buffer[0]);
        DataModel::Nullable<uint8_t> aPhase(m);
        CHIP_ERROR err = gRvcOperationalStateInstance->SetCurrentPhase(aPhase);
        if (CHIP_NO_ERROR == err)
        {
            break;
        }
        ret = chip::Protocols::InteractionModel::Status::ConstraintError;
        ChipLogError(DeviceLayer, "Invalid Attribute Update status: %" CHIP_ERROR_FORMAT, err.Format());
    }
    break;
    case chip::app::Clusters::RvcOperationalState::Attributes::OperationalState::Id: {
        uint8_t currentState = gRvcOperationalStateInstance->GetCurrentOperationalState();
        uint8_t m            = static_cast<uint8_t>(buffer[0]);
        CHIP_ERROR err       = gRvcOperationalStateInstance->SetOperationalState(m);

        if (currentState == to_underlying(OperationalState::OperationalStateEnum::kStopped) &&
            m == to_underlying(OperationalState::OperationalStateEnum::kRunning))
        {
            gRvcOperationalStateDelegate->mCountdownTime.SetNonNull(
                static_cast<uint32_t>(gRvcOperationalStateDelegate->kExampleCountDown));
        }

        if (CHIP_NO_ERROR == err)
        {
            break;
        }
        ret = chip::Protocols::InteractionModel::Status::ConstraintError;
        ChipLogError(DeviceLayer, "Invalid Attribute Update status: %" CHIP_ERROR_FORMAT, err.Format());
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status chefRvcOperationalStateReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                              const EmberAfAttributeMetadata * attributeMetadata,
                                                                              uint8_t * buffer, uint16_t maxReadLength)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;
    chip::AttributeId attributeId                 = attributeMetadata->attributeId;
    switch (attributeId)
    {
    case chip::app::Clusters::RvcOperationalState::Attributes::CurrentPhase::Id: {

        app::DataModel::Nullable<uint8_t> currentPhase = gRvcOperationalStateInstance->GetCurrentPhase();
        if (currentPhase.IsNull())
        {
            ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
            break;
        }
        *buffer = currentPhase.Value();
    }
    break;
    case chip::app::Clusters::RvcOperationalState::Attributes::OperationalState::Id: {
        *buffer = gRvcOperationalStateInstance->GetCurrentOperationalState();
    }
    break;
    default:
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfRvcOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gRvcOperationalStateDelegate && !gRvcOperationalStateInstance);

    gRvcOperationalStateDelegate = std::make_unique<RvcOperationalStateDelegate>();
    gRvcOperationalStateInstance = std::make_unique<RvcOperationalState::Instance>(gRvcOperationalStateDelegate.get(), endpointId);
    gRvcOperationalStateInstance->Init();
}
#endif // MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
