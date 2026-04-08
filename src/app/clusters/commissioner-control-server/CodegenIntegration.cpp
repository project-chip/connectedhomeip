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

using Status = Protocols::InteractionModel::Status;

namespace chip::app::Clusters::CommissionerControl {

CommissionerControlServer::CommissionerControlServer(Delegate * delegate, EndpointId endpointId) :
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

Status CommissionerControlServer::GetSupportedDeviceCategoriesValue(
    EndpointId endpoint, BitMask<SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const
{
    VerifyOrReturnError(mCluster.IsConstructed() && endpoint == mEndpointId, Status::InvalidInState);
    *supportedDeviceCategories = mCluster.Cluster().GetSupportedDeviceCategories();
    return Status::Success;
}

Status
CommissionerControlServer::SetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                                             const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories)
{
    VerifyOrReturnError(mCluster.IsConstructed() && endpoint == mEndpointId, Status::InvalidInState);
    mCluster.Cluster().SetSupportedDeviceCategories(supportedDeviceCategories);
    return Status::Success;
}

CHIP_ERROR
CommissionerControlServer::GenerateCommissioningRequestResultEvent(EndpointId endpoint,
                                                                   const Events::CommissioningRequestResult::Type & result)
{
    VerifyOrReturnError(mCluster.IsConstructed() && endpoint == mEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mCluster.Cluster().GenerateCommissioningRequestResultEvent(result);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::CommissionerControl

void MatterCommissionerControlClusterInitCallback(EndpointId) {}
void MatterCommissionerControlClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}
void MatterCommissionerControlPluginServerInitCallback() {}
void MatterCommissionerControlPluginServerShutdownCallback() {}
