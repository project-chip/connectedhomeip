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
#include <operational-state-delegate-impl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/generic-callbacks.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;

CHIP_ERROR OperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    if (index > mOperationalStateList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase)
{
    if (index > mOperationalPhaseList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalPhase = mOperationalPhaseList[index];
    return CHIP_NO_ERROR;
}

void OperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
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

void OperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
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

void OperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
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

void OperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    // placeholder implementation
    auto error = GetInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
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

void OperationalStateDelegate::PostAttributeChangeCallback(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    chip::app::ConcreteAttributePath  info;
    info.mClusterId   = Clusters::OperationalState::Id;
    info.mAttributeId = attributeId;
    info.mEndpointId  = this->mEndpointId;
    MatterPostAttributeChangeCallback(info,type, size, value);
}

static OperationalState::Instance * gOperationalStateInstance = nullptr;
static OperationalStateDelegate * gOperationalStateDelegate   = nullptr;

void OperationalState::Shutdown()
{
    if (gOperationalStateInstance != nullptr)
    {
        delete gOperationalStateInstance;
        gOperationalStateInstance = nullptr;
    }
    if (gOperationalStateDelegate != nullptr)
    {
        delete gOperationalStateDelegate;
        gOperationalStateDelegate = nullptr;
    }
}

void emberAfOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(gOperationalStateInstance == nullptr && gOperationalStateDelegate == nullptr);

    gOperationalStateDelegate           = new OperationalStateDelegate;
    EndpointId operationalStateEndpoint = 0x01;
    gOperationalStateInstance           = new Instance(gOperationalStateDelegate, operationalStateEndpoint);

    gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    gOperationalStateInstance->Init();
    
    gOperationalStateDelegate->mEndpointId = endpointId;
    uint8_t value = to_underlying(OperationalStateEnum::kStopped);
    gOperationalStateDelegate->PostAttributeChangeCallback(Attributes::OperationalState::Id, ZCL_INT8U_ATTRIBUTE_TYPE, sizeof(uint8_t), &value);
}

OperationalState::Instance* OperationalState::GetInstance()
{
    return gOperationalStateInstance;
}

OperationalState::OperationalStateDelegate* OperationalState::GetDelegate()
{
    return gOperationalStateDelegate;
}
