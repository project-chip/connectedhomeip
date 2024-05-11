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

static RvcOperationalState::Instance * gRvcOperationalStateInstance = nullptr;
static RvcOperationalStateDelegate * gRvcOperationalStateDelegate   = nullptr;

static void onOperationalStateTimerTick(System::Layer * systemLayer, void * data);

DataModel::Nullable<uint32_t> RvcOperationalStateDelegate::GetCountdownTime()
{
    if (mCountDownTime.IsNull())
        return DataModel::NullNullable;

    return DataModel::MakeNullable((uint32_t) (mCountDownTime.Value() - mRunningTime));
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
    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void RvcOperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
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

    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, this);
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void RvcOperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
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

    OperationalState::Instance * instance = gRvcOperationalStateInstance;
    OperationalState::OperationalStateEnum state =
        static_cast<OperationalState::OperationalStateEnum>(instance->GetCurrentOperationalState());

    auto countdown_time = delegate->GetCountdownTime();

    if (countdown_time.ValueOr(1) > 0)
    {
        if (state == OperationalState::OperationalStateEnum::kRunning)
        {
            delegate->mRunningTime++;
        }
        else if (state == OperationalState::OperationalStateEnum::kPaused)
        {
            delegate->mPausedTime++;
        }
    }

    if (state == OperationalState::OperationalStateEnum::kRunning || state == OperationalState::OperationalStateEnum::kPaused)
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onOperationalStateTimerTick, delegate);
    }
    else
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onOperationalStateTimerTick, delegate);
    }
}

void RvcOperationalState::Shutdown()
{
    if (gRvcOperationalStateInstance != nullptr)
    {
        delete gRvcOperationalStateInstance;
        gRvcOperationalStateInstance = nullptr;
    }
    if (gRvcOperationalStateDelegate != nullptr)
    {
        delete gRvcOperationalStateDelegate;
        gRvcOperationalStateDelegate = nullptr;
    }
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
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Invalid Attribute Update status: %" CHIP_ERROR_FORMAT, err.Format());
    }
    break;
    case chip::app::Clusters::RvcOperationalState::Attributes::OperationalState::Id: {
        uint8_t m      = static_cast<uint8_t>(buffer[0]);
        CHIP_ERROR err = gRvcOperationalStateInstance->SetOperationalState(m);
        if (CHIP_NO_ERROR == err)
        {
            break;
        }
        ret = chip::Protocols::InteractionModel::Status::UnsupportedWrite;
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
    if(sizeof(*attributeMetadata) <= sizeof(buffer)) {
        memcpy(buffer, &attributeMetadata, sizeof(attributeMetadata));
        return chip::Protocols::InteractionModel::Status::Success;
    }
    return chip::Protocols::InteractionModel::Status::ResourceExhausted;
}

void emberAfRvcOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gRvcOperationalStateInstance == nullptr && gRvcOperationalStateDelegate == nullptr);

    gRvcOperationalStateDelegate        = new chip::app::Clusters::RvcOperationalState::RvcOperationalStateDelegate();
    EndpointId operationalStateEndpoint = 0x01;
    gRvcOperationalStateInstance        = new RvcOperationalState::Instance(gRvcOperationalStateDelegate, operationalStateEndpoint);

    gRvcOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

    gRvcOperationalStateInstance->Init();
}
#endif // MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
