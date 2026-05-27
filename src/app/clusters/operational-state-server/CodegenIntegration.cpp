/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include "CodegenIntegration.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;

// ---------------------------------------------------------------------------
// OperationalState::InstanceBase
// ---------------------------------------------------------------------------

InstanceBase::InstanceBase(OperationalStateCluster & cluster, ServerClusterRegistration & registration, Delegate * aDelegate) :
    mDelegate(aDelegate), mCluster(cluster), mRegPtr(&registration)
{
    aDelegate->SetInstance(this);
}

InstanceBase::~InstanceBase()
{
    if (mRegistered)
    {
        ChipLogError(AppServer, "OperationalState::InstanceBase destroyed without Shutdown(); shutting down now.");
        Shutdown();
    }
    if (mDelegate)
    {
        mDelegate->SetInstance(nullptr);
    }
}

CHIP_ERROR InstanceBase::Init()
{
    VerifyOrReturnError(!mRegistered, CHIP_NO_ERROR);
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(*mRegPtr));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void InstanceBase::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered = false;
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster));
}

// ---------------------------------------------------------------------------
// ZAP-generated plugin callbacks — stubs only.
//
// Applications instantiate Instance (or OperationalStateCluster) directly and
// register with the codegen data model provider via Init(). These callbacks
// are intentionally left empty; the lifecycle is application-controlled.
// ---------------------------------------------------------------------------

void emberAfOperationalStateClusterServerInitCallback(EndpointId) {}
void MatterOperationalStateClusterServerShutdownCallback(EndpointId) {}
void MatterOperationalStateClusterServerAttributeChangedCallback(const ConcreteAttributePath &) {}
Protocols::InteractionModel::Status MatterOperationalStateClusterServerPreAttributeChangedCallback(const ConcreteAttributePath &,
                                                                                                   EmberAfAttributeType, uint16_t,
                                                                                                   uint8_t *)
{
    return Protocols::InteractionModel::Status::Success;
}

void emberAfRvcOperationalStateClusterServerInitCallback(EndpointId) {}
void MatterRvcOperationalStateClusterServerShutdownCallback(EndpointId) {}
void MatterRvcOperationalStateClusterServerAttributeChangedCallback(const ConcreteAttributePath &) {}
Protocols::InteractionModel::Status MatterRvcOperationalStateClusterServerPreAttributeChangedCallback(const ConcreteAttributePath &,
                                                                                                      EmberAfAttributeType,
                                                                                                      uint16_t, uint8_t *)
{
    return Protocols::InteractionModel::Status::Success;
}

void emberAfOvenCavityOperationalStateClusterServerInitCallback(EndpointId) {}
void MatterOvenCavityOperationalStateClusterServerShutdownCallback(EndpointId) {}
void MatterOvenCavityOperationalStateClusterServerAttributeChangedCallback(const ConcreteAttributePath &) {}
Protocols::InteractionModel::Status
MatterOvenCavityOperationalStateClusterServerPreAttributeChangedCallback(const ConcreteAttributePath &, EmberAfAttributeType,
                                                                         uint16_t, uint8_t *)
{
    return Protocols::InteractionModel::Status::Success;
}
