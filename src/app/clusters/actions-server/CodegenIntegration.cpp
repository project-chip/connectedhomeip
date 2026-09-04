/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;

namespace {

// Maximum SetupURL length per the Matter spec (Actions cluster, SetupURL attribute).
constexpr size_t kMaxSetupURLLength = 512u;

ActionsCluster::OptionalAttributesSet BuildOptionalAttributes(EndpointId endpointId)
{
    ActionsCluster::OptionalAttributesSet optionalAttributes;
    if (emberAfContainsAttribute(endpointId, Actions::Id, Attributes::SetupURL::Id))
    {
        optionalAttributes.template ForceSet<Attributes::SetupURL::Id>();
    }
    return optionalAttributes;
}

// Returns the SetupURL attribute value as a std::string, or an empty string on failure.
// An empty return value means the attribute is absent or unreadable.
std::string ReadSetupURL(EndpointId endpointId)
{
    VerifyOrReturnValue(emberAfContainsAttribute(endpointId, Actions::Id, Attributes::SetupURL::Id), std::string());
    // Use a stack buffer for the Ember read; the result is then copied into a std::string.
    char buf[kMaxSetupURLLength];
    MutableCharSpan urlSpan(buf);
    VerifyOrReturnValue(Attributes::SetupURL::GetDefault(endpointId, urlSpan) == Protocols::InteractionModel::Status::Success,
                        std::string());
    return std::string(urlSpan.data(), urlSpan.size());
}

std::optional<CharSpan> SetupURLSpan(const std::string & url)
{
    VerifyOrReturnValue(!url.empty(), std::nullopt);
    return CharSpan(url.data(), url.size());
}

} // namespace

uint8_t ActionsServer::sInstanceCount = 0;

ActionsServer::ActionsServer(EndpointId endpointId, Delegate & delegate) :
    mSetupURL(ReadSetupURL(endpointId)),
    mCluster(endpointId, delegate, BuildOptionalAttributes(endpointId), SetupURLSpan(mSetupURL))
{
    // The Actions cluster has "Scope: Node" per the Matter spec. However, a device can have
    // multiple aggregator endpoints (e.g. a Zigbee bridge on EP1 and a Z-Wave bridge on EP2),
    // and each aggregator may host its own Actions cluster instance. Multiple instances are
    // therefore valid; this counter is retained for diagnostic purposes only.
    if (++sInstanceCount > 1)
    {
        ChipLogDetail(Zcl, "ActionsServer: %u instances active (multiple aggregator endpoints in use).", sInstanceCount);
    }
}

ActionsServer::~ActionsServer()
{
    if (mRegistered)
    {
        // Shutdown() was not called before destruction. Call it now to avoid
        // leaving a dangling pointer in the data model provider registry.
        ChipLogError(AppServer, "ActionsServer destroyed without Shutdown() being called; shutting down now.");
        Shutdown();
    }
    --sInstanceCount;
}

CHIP_ERROR ActionsServer::Init()
{
    VerifyOrReturnError(!mRegistered, CHIP_NO_ERROR);
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration()));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void ActionsServer::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered    = false;
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        [[maybe_unused]] const ConcreteClusterPath path = mCluster.Cluster().GetPaths()[0];
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT, path.mEndpointId,
                     ChipLogValueMEI(path.mClusterId), err.Format());
    }
}
void ActionsServer::ActionListModified(EndpointId aEndpoint)
{
    VerifyOrReturn(aEndpoint == mCluster.Cluster().GetPaths()[0].mEndpointId);
    mCluster.Cluster().ActionListModified();
}

void ActionsServer::EndpointListModified(EndpointId aEndpoint)
{
    VerifyOrReturn(aEndpoint == mCluster.Cluster().GetPaths()[0].mEndpointId);
    mCluster.Cluster().EndpointListsModified();
}

CHIP_ERROR ActionsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // 1. Construct the request using the path and a default SubjectDescriptor.
    Access::SubjectDescriptor subjectDescriptor;
    DataModel::ReadAttributeRequest request(aPath, subjectDescriptor);

    // 2. Delegate the read operation to the new ActionsCluster implementation
    DataModel::ActionReturnStatus status = mCluster.Cluster().ReadAttribute(request, aEncoder);

    return status.GetUnderlyingError();
}
// ZAP-generated plugin callbacks are left as stubs. Applications instantiate ActionsServer
// directly (not through these callbacks) and register it with the codegen data model
// provider via Init(). This is consistent with the code-driven cluster pattern where the
// application owns the cluster lifecycle rather than the ZAP-generated scaffolding.
void MatterActionsClusterInitCallback(EndpointId endpointId) {}
void MatterActionsClusterShutdownCallback(chip::EndpointId endpointId, MatterClusterShutdownType type) {}
void MatterActionsPluginServerInitCallback() {}
void MatterActionsPluginServerShutdownCallback() {}
