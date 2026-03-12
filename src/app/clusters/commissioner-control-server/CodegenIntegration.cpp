/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/commissioner-control-server/CommissionerControlCluster.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

namespace chip::app::Clusters {

CommissionerControlServer::CommissionerControlServer(EndpointId endpointId, CommissionerControl::Delegate * delegate) :
    mEndpointId(endpointId), mDelegate(delegate)
{}

CommissionerControlServer::~CommissionerControlServer()
{
    if (mCluster.IsConstructed())
    {
        RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
    }
}

CHIP_ERROR CommissionerControlServer::Init()
{
    mCluster.Create(mEndpointId, mDelegate);
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

} // namespace chip::app::Clusters

void MatterCommissionerControlClusterInitCallback(EndpointId) {}
void MatterCommissionerControlClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}
void MatterCommissionerControlPluginServerInitCallback() {}
void MatterCommissionerControlPluginServerShutdownCallback() {}
