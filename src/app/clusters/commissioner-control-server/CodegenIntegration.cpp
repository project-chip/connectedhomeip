/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/commissioner-control-server/CodegenIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

namespace chip::app::Clusters {

CommissionerControlServer::CommissionerControlServer(CommissionerControl::Delegate * delegate, EndpointId endpointId) :
    mDelegate(delegate), mEndpointId(endpointId)
{}

CommissionerControlServer::~CommissionerControlServer()
{
    RETURN_SAFELY_IGNORED Deinit();
}

CHIP_ERROR CommissionerControlServer::Init()
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCluster.Create(mEndpointId, *mDelegate);
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

CHIP_ERROR CommissionerControlServer::Deinit()
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
}

void CommissionerControlServer::SetSupportedDeviceCategories(
    const BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories)
{
    VerifyOrDie(mCluster.IsConstructed());
    mCluster.Cluster().SetSupportedDeviceCategories(supportedDeviceCategories);
}

BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> CommissionerControlServer::GetSupportedDeviceCategories() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetSupportedDeviceCategories();
}

void CommissionerControlServer::GenerateCommissioningRequestResultEvent(
    const CommissionerControl::Events::CommissioningRequestResult::Type & result)
{
    VerifyOrDie(mCluster.IsConstructed());
    mCluster.Cluster().GenerateCommissioningRequestResultEvent(result);
}

} // namespace chip::app::Clusters

void MatterCommissionerControlClusterInitCallback(EndpointId) {}
void MatterCommissionerControlClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}
void MatterCommissionerControlPluginServerInitCallback() {}
void MatterCommissionerControlPluginServerShutdownCallback() {}
