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

#include <app/ServerCluster.h>

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

ServerCluster::ServerCluster(EndpointId aEndpointId, ClusterId aClusterId) :
    AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId), CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId)
{}

CHIP_ERROR ServerCluster::Init()
{
    ChipLogError(NotSpecified, "CLUStER INIT");
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void ServerCluster::Shutdown()
{
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

void EmberClusterAdapterImpl::Init(ServerCluster & cluster)
{
    CHIP_ERROR err = cluster.Init();
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, Zcl, "Cluster initialization failed: %" CHIP_ERROR_FORMAT, err.Format());
}

void EmberClusterAdapterImpl::InitFailureInstancesExceeded()
{
    VerifyOrDieWithMsg(false, Zcl, "Cluster initialization failed: no instance allocated");
}

} // namespace app
} // namespace chip
