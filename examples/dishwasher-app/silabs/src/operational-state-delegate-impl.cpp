/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <app/util/generic-callbacks.h>

#include "DataModelHelper.h"
#include "operational-state-delegate-impl.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;

static std::unique_ptr<OperationalStateDelegate> gOperationalStateDelegate;
static std::unique_ptr<OperationalState::Instance> gOperationalStateInstance;

/**
 * Get the list of supported operational states.
 * Fills in the provided GenericOperationalState with the state at `index` if there is one,
 * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
 * @param index             The index of the state, with 0 representing the first state.
 * @param operationalState  The GenericOperationalState is filled.
 */
CHIP_ERROR OperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    VerifyOrReturnError(index < mOperationalStateList.size(), CHIP_ERROR_NOT_FOUND);

    operationalState = mOperationalStateList[index];

    return CHIP_NO_ERROR;
}

/**
 * Get the list of supported operational phases.
 * Fills in the provided MutableCharSpan with the phase at index `index` if there is one,
 * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
 *
 * If CHIP_ERROR_NOT_FOUND is returned for index 0, that indicates that the PhaseList attribute is null
 * (there are no phases defined at all).
 *
 * @param index             The index of the phase, with 0 representing the first phase.
 * @param operationalPhase  The MutableCharSpan is filled.
 */
CHIP_ERROR OperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    VerifyOrReturnError(index < mOperationalPhaseList.size(), CHIP_ERROR_NOT_FOUND);

    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

/**
 * Handle Command Callback in application: Pause
 * @param[out] get Operational error after callback
 */
void OperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        uint8_t value = to_underlying(OperationalStateEnum::kPaused);
        PostAttributeChangeCallback(Attributes::OperationalState::Id, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(uint8_t), &value);
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

/**
 * Handle Command Callback in application: Resume
 * @param[out] get Operational error after callback
 */
void OperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        uint8_t value = to_underlying(OperationalStateEnum::kRunning);
        PostAttributeChangeCallback(Attributes::OperationalState::Id, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(uint8_t), &value);
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

/**
 * Handle Command Callback in application: Start
 * @param[out] get Operational error after callback
 */
void OperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        uint8_t value = to_underlying(OperationalStateEnum::kRunning);
        PostAttributeChangeCallback(Attributes::OperationalState::Id, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(uint8_t), &value);
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

/**
 * Handle Command Callback in application: Stop
 * @param[out] get Operational error after callback.
 */
void OperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));

    VerifyOrReturn(error == CHIP_NO_ERROR, err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation)));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        uint8_t value = to_underlying(OperationalStateEnum::kStopped);
        PostAttributeChangeCallback(Attributes::OperationalState::Id, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(uint8_t), &value);
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

/**
 * @brief Calls the MatterPostAttributeChangeCallback for the modified attribute on the operationalState instance
 *
 * @param attributeId Id of the attribute that changed
 * @param type        Type of the attribute
 * @param size        Size of the attribute data
 * @param value       Value of the attribute data
 */
void OperationalStateDelegate::PostAttributeChangeCallback(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    ConcreteAttributePath info;
    info.mClusterId   = OperationalState::Id;
    info.mAttributeId = attributeId;
    info.mEndpointId  = mEndpointId;
    MatterPostAttributeChangeCallback(info, type, size, value);
}

void OperationalStateDelegate::SetEndpointId(EndpointId endpointId)
{
    mEndpointId = endpointId;
}

OperationalState::Instance * OperationalState::GetInstance()
{
    return gOperationalStateInstance.get();
}

OperationalState::OperationalStateDelegate * OperationalState::GetDelegate()
{
    return gOperationalStateDelegate.get();
}

void emberAfOperationalStateClusterInitCallback(EndpointId endpointId)
{
    CHIP_ERROR err;

    EndpointId OperationalStateEndpointId = DataModelHelper::GetEndpointIdFromCluster(OperationalState::Id);
    VerifyOrDie((OperationalStateEndpointId != kInvalidEndpointId) && (OperationalStateEndpointId == endpointId));

    VerifyOrDie(!gOperationalStateDelegate && !gOperationalStateInstance);

    gOperationalStateDelegate = std::make_unique<OperationalStateDelegate>();
    VerifyOrDie(gOperationalStateDelegate);

    gOperationalStateInstance =
        std::make_unique<OperationalState::Instance>(gOperationalStateDelegate.get(), OperationalStateEndpointId);
    VerifyOrDie(gOperationalStateInstance);

    err = gOperationalStateInstance->Init();
    VerifyOrDie(CHIP_NO_ERROR == err);

    gOperationalStateInstance->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));

    gOperationalStateDelegate->SetEndpointId(OperationalStateEndpointId);
}

void OperationalState::Shutdown()
{
    if (gOperationalStateInstance)
    {
        gOperationalStateInstance.reset();
    }

    if (gOperationalStateDelegate)
    {
        gOperationalStateDelegate.reset();
    }
}
