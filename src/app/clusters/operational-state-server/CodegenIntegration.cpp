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
// OperationalState::Instance
// ---------------------------------------------------------------------------

// Standalone constructor: creates and owns an OperationalStateCluster.
Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config) :
    mDelegate(aDelegate), mOwnedStorage(Platform::MakeUnique<detail::OperationalInstanceBase>(aDelegate, aEndpointId, config)),
    mCluster(mOwnedStorage->mCluster.Cluster()), mRegPtr(&mOwnedStorage->mCluster.Registration())
{
    aDelegate->SetInstance(this);
}

// Protected constructor: cluster storage is owned by the derived class (Rvc, OvenCavity).
Instance::Instance(OperationalStateCluster & cluster, ServerClusterRegistration & registration, Delegate * aDelegate) :
    mDelegate(aDelegate), mCluster(cluster), mRegPtr(&registration)
{
    aDelegate->SetInstance(this);
}

Instance::~Instance()
{
    if (mRegistered)
    {
        ChipLogError(AppServer, "OperationalState::Instance destroyed without Shutdown(); shutting down now.");
        Shutdown();
    }
    if (mDelegate)
    {
        mDelegate->SetInstance(nullptr);
    }
    // mOwnedStorage (Platform::UniquePtr) frees the owned cluster automatically.
}

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(!mRegistered, CHIP_NO_ERROR);
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(*mRegPtr));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered = false;
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster));
}

// ---------------------------------------------------------------------------
// Application-facing API — out-of-line forwarders to the underlying cluster.
//
// Kept out-of-line (rather than inline in the header) so the cluster methods are
// emitted once and not inlined/duplicated into every shim call site.
// ---------------------------------------------------------------------------

CHIP_ERROR Instance::SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase)
{
    return Cluster().SetCurrentPhase(aPhase);
}

CHIP_ERROR Instance::SetOperationalState(uint8_t aOpState)
{
    return Cluster().SetOperationalState(aOpState);
}

DataModel::Nullable<uint8_t> Instance::GetCurrentPhase() const
{
    return Cluster().GetCurrentPhase();
}

uint8_t Instance::GetCurrentOperationalState() const
{
    return Cluster().GetCurrentOperationalState();
}

void Instance::GetCurrentOperationalError(GenericOperationalError & error) const
{
    Cluster().GetCurrentOperationalError(error);
}

void Instance::UpdateCountdownTimeFromDelegate()
{
    Cluster().UpdateCountdownTimeFromDelegate();
}

void Instance::OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError)
{
    Cluster().OnOperationalErrorDetected(aError);
}

void Instance::OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                             const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime,
                                             const Optional<DataModel::Nullable<uint32_t>> & aPausedTime)
{
    Cluster().OnOperationCompletionDetected(aCompletionErrorCode, aTotalOperationalTime, aPausedTime);
}

void Instance::ReportOperationalStateListChange()
{
    Cluster().ReportOperationalStateListChange();
}

void Instance::ReportPhaseListChange()
{
    Cluster().ReportPhaseListChange();
}

bool Instance::IsSupportedPhase(uint8_t aPhase)
{
    return Cluster().IsSupportedPhase(aPhase);
}

bool Instance::IsSupportedOperationalState(uint8_t aState)
{
    return Cluster().IsSupportedOperationalState(aState);
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
